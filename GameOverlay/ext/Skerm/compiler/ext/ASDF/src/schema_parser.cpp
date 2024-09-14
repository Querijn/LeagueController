#include "asdf/schema_parser.hpp"
#include "asdf/error_handling.hpp"
#include "asdf/serialisation_parser.hpp"
#include "hash_helper.hpp"

#include <pugixml.hpp>
#include <mstch/mstch.hpp>

#include <set>
#include <filesystem>
#include <fstream>

namespace ASDF
{
	namespace fs = std::filesystem;

	SchemaParser::SchemaParser(TypeInfo & a_Info) :
		m_Info(a_Info), m_ErrorHandler(a_Info.GetErrorHandler())
	{
	}

	void SchemaParser::AddFolderFile(std::string_view a_File, std::unordered_map<std::string, std::vector<std::string>>& a_FilesToParseLater)
	{
		auto t_Result = AddFile(a_File);

		switch (t_Result.Type)
		{
		case AddResultType::Added:
			printf("- Added '%s'\n", a_File.data());

			for (auto& t_File : a_FilesToParseLater[t_Result.Info])
				AddFolderFile(t_File, a_FilesToParseLater);
			break;

		case AddResultType::CanNotParse:
			printf("- Error: Unable to parse '%s': %s\n", a_File.data(), t_Result.Info.c_str());
			break;

		case AddResultType::NothingAdded:
			printf("- Error: Nothing was found in '%s'\n", a_File.data());
			break;

			// When doing this, the name of the parent is added in Info, and we parse this file after we find the parent.
		case AddResultType::ExtendUndefined:
			printf("- Notice: '%s' requires '%s', delaying parse\n", a_File.data(), t_Result.Info.c_str());
			a_FilesToParseLater[t_Result.Info].push_back(a_File.data());
			break;
		}
	}

	void SchemaParser::AddFolder(std::string_view a_Folder)
	{
		printf("ASDF was instructed to add files from a folder (Source folder: '%s')\n", a_Folder.data());

		fs::path t_Folder = fs::absolute(a_Folder);
		if (fs::exists(t_Folder) == false)
		{
			printf("- Folder '%s' does not exist!\n", t_Folder.generic_string().c_str());
			return;
		}

		std::unordered_map<std::string, std::vector<std::string>> t_FilesToParseLater;
		for (auto& t_Entry : fs::recursive_directory_iterator(a_Folder))
		{
			if (t_Entry.is_regular_file() == false)
				continue;

			auto t_File = t_Entry.path().generic_string();
			AddFolderFile(t_File, t_FilesToParseLater);
		}
	}

	void ConstructAlternativeArguments(const pugi::xml_node& a_Struct, std::map<std::string, std::string>& a_ArgumentMap, std::initializer_list<std::string> a_ExistingArguments)
	{
		for (auto i = a_Struct.attributes_begin(); i != a_Struct.attributes_end(); ++i)
		{
			// Skip default parameters
			bool t_Found = false;
			for (auto& t_String : a_ExistingArguments)
			{
				if (stricmp(i->name(), t_String.c_str()) != 0)
					continue;

				t_Found = true;
				break;
			}

			// Add unknown one
			if (t_Found == false)
				a_ArgumentMap[i->name()] = i->value();
		}
	}

	SchemaParser::AddResult SchemaParser::AddFile(std::string_view a_File)
	{
		pugi::xml_document t_Document;
		pugi::xml_parse_result t_Result = t_Document.load_file(a_File.data());
		if (!t_Result)
		{
			m_ErrorHandler.AssertError(t_Result, "Could not load XML document '%s'.", a_File.data());
			return { AddResultType::CanNotParse, std::string("XML failed to parse: ") + t_Result.description() };
		}

		if (pugi::xml_node t_Struct = t_Document.child("Struct"))
		{
			std::string t_Name(t_Struct.attribute("Name").as_string());
			std::string t_Extends(t_Struct.attribute("Extends").as_string());
			std::string t_PointerSizeString(t_Struct.attribute("PointerSize").as_string());

			std::map<std::string, std::string> t_ArgumentMap;
			ConstructAlternativeArguments(t_Struct, t_ArgumentMap, { "Name", "Extends", "PointerSize" });

			std::vector<TypeInfo::StructElement::Member> t_Members;
			TypeInfo::StructElement* t_Parent = nullptr;
			if (t_Extends.empty() == false)
			{
				const TypeInfo::BaseElement* t_ParentAsBase = m_Info.GetElement(t_Extends);
				
				// No error on this one because it's potentially fine. In AddFolder it might find the actual parent and parse it again.
				if (!t_ParentAsBase)
					return { AddResultType::ExtendUndefined, t_Extends };

				if (t_ParentAsBase->Type != TypeInfo::ElementType::StructType)
				{
					m_ErrorHandler.AssertError(false, "Parent of %s ('%s') is not a struct!", t_Name.c_str(), t_Extends.c_str());
					return { AddResultType::CanNotParse, "Parent is not a struct" };
				}

				t_Parent = (TypeInfo::StructElement*)t_ParentAsBase;

				t_Members = t_Parent->Members; // Copy
				for (auto& t_Member : t_Members)
					t_Member.CameFromParent = true; // Mark as "from parent"
			}

			TypeInfo::BlobPointerSize t_PointerSize = TypeInfo::BlobPointerSize::Size16;
			if (t_PointerSizeString.empty() == false)
			{
				if (t_PointerSizeString == "16")
					t_PointerSize = TypeInfo::BlobPointerSize::Size16;
				else if (t_PointerSizeString == "32")
					t_PointerSize = TypeInfo::BlobPointerSize::Size32;
				else if (t_PointerSizeString == "64")
					t_PointerSize = TypeInfo::BlobPointerSize::Size64;
			}

			for (pugi::xml_node t_Node : t_Struct.children("Item"))
			{
				TypeInfo::StructElement::Member t_Item;

				t_Item.CameFromParent = false;
				t_Item.VariableName = t_Node.attribute("Name").as_string();
				t_Item.Type = t_Node.attribute("Type").as_string();
				t_Item.DefaultValue = t_Node.attribute("Value").as_string();
				t_Item.IsArray = t_Node.attribute("IsArray").as_bool();
				t_Item.IsPointer = t_Node.attribute("IsPointer").as_bool();
				t_Item.PrefersInline = t_Node.attribute("PreferInline").as_bool();
				t_Item.ArrayCountForMember = t_Node.attribute("IsArrayCountFor").as_string();

				t_Members.push_back(t_Item);
			}

			const TypeInfo::StructElement& t_ResultStruct = m_Info.AddStruct(t_Name, std::move(t_Members), std::move(t_ArgumentMap), t_PointerSize, t_Parent);
			if (t_Parent)
				t_Parent->Extensions.push_back(&t_ResultStruct);

			m_FileToObjectMap[a_File.data()] = t_Name;
			return { AddResultType::Added, t_Name };
		}

		pugi::xml_node t_Enum = t_Document.child("Enum");
		std::vector<std::string> t_MissingValues;
		if (t_Enum)
		{
			std::string t_Name(t_Enum.attribute("Name").as_string());

			std::map<std::string, std::string> t_ArgumentMap;
			ConstructAlternativeArguments(t_Enum, t_ArgumentMap, { "Name" });

			int64_t t_Iterator = 0;

			std::vector<TypeInfo::EnumElement::Value> t_Values;
			for (pugi::xml_node t_Node : t_Enum.children("Item"))
			{
				TypeInfo::EnumElement::Value t_Item;

				// The name of the value (EnumName::ValueName)
				if (pugi::xml_attribute t_NameAttribute = t_Node.attribute("Name"))
					t_Item.ValueName = t_NameAttribute.as_string();

				// The value of that name (EnumName::ValueName = Value)
				if (pugi::xml_attribute t_ValueAttribute = t_Node.attribute("Value"))
				{
					t_Item.RepresentingNumber = t_ValueAttribute.as_int();
					if (t_Item.RepresentingNumber >= t_Iterator)
						t_Iterator = t_Item.RepresentingNumber + 1;
				}
				else
				{
					t_MissingValues.push_back(t_Item.ValueName);
				}

				t_Values.push_back(t_Item);
			}

			for (auto& t_Value : t_Values)
			{
				auto index = std::find(t_MissingValues.begin(), t_MissingValues.end(), t_Value.ValueName);
				if (index != t_MissingValues.end())
					t_Value.RepresentingNumber = t_Iterator++;
			}

			m_Info.AddEnum(t_Name, std::move(t_Values), std::move(t_ArgumentMap));
			m_FileToObjectMap[a_File.data()] = t_Name;
			return { AddResultType::Added, t_Name };
		}

		m_ErrorHandler.AssertError(false, "Could not determine base element of '%s'.", a_File.data());
		return { AddResultType::NothingAdded, a_File.data() };
	}

	void OutputEnumData(const TypeInfo::EnumElement* a_EnumElement, mstch::map& a_ObjectContext)
	{
		mstch::array t_ItemsContext;
		for (size_t i = 0; i < a_EnumElement->Values.size(); i++)
		{
			const TypeInfo::EnumElement::Value& t_Item = a_EnumElement->Values[i];
			mstch::map t_ItemContext;

			t_ItemContext["Name"] = std::string(t_Item.ValueName);
			t_ItemContext["Value"] = std::to_string(t_Item.RepresentingNumber);

			t_ItemContext["Last"] = i == a_EnumElement->Values.size() - 1;
			t_ItemsContext.push_back(t_ItemContext);
		}
		a_ObjectContext["Items"] = t_ItemsContext;
	}

	void MakeContext(mstch::map& a_Context, const std::string& a_ItemType, const TypeInfo::StructElement::Member& a_Member, TypeInfo::BlobPointerSize a_PointerSize, const TypeInfo::BaseElement* a_Element, SerialisationParser& a_SerialisationParser, TypeInfo& a_Info)
	{
		std::string t_Header = a_Info.GetElementFile(*a_Element);

		for (const auto& t_ArgumentPair : a_Element->TypeArguments)
			a_Context["Member" + t_ArgumentPair.first] = t_ArgumentPair.second;

		a_Context["Name"] = std::string(a_Member.VariableName);
		a_Context["Value"] = std::string(a_Member.DefaultValue);
		a_Context["Type"] = a_ItemType;

		a_Context["CameFromParent"] = a_Member.CameFromParent;
		a_Context["IsArray"] = a_Member.IsArray;
		a_Context["IsPointer"] = a_Member.IsPointer;
		a_Context["IsStruct"] = a_Element->Type == TypeInfo::ElementType::StructType;
		a_Context["IsEnum"] = a_Element->Type == TypeInfo::ElementType::EnumType;
		a_Context["IsPlain"] = a_Element->Type == TypeInfo::ElementType::PlainType;
		a_Context["IsBaseASDF"] = a_Element->IsBaseASDF;
		a_Context["ElementSize"] = (unsigned int)(a_Member.IsPointer ? a_Info.GetPointerByteSize(a_PointerSize) : a_SerialisationParser.GetMaxPackedSize(*a_Element));
		a_Context["Header"] = t_Header.empty() == false ? t_Header : "";
	}

	struct DeclarationCompare
	{
		using Element = std::set<mstch::map>::key_type;
		bool operator() (const Element& a_LeftElement, const Element& a_RightElement) const
		{
			auto t_Left = a_LeftElement.find("Name");
			auto t_Right = a_RightElement.find("Name");
			return std::get<std::string>(t_Left->second) < std::get<std::string>(t_Right->second);
		}
	};

	void OutputStructData(const TypeInfo::StructElement* a_Struct, mstch::map& a_ObjectContext, std::set<std::string>& a_Headers, SerialisationParser& a_SerialisationParser, TypeInfo& a_Info)
	{
		std::string t_Header = a_Info.GetElementFile(*a_Struct);
		if (t_Header.empty() == false)
			a_ObjectContext["Header"] = t_Header;

		std::set<mstch::map, DeclarationCompare> t_Declarations;
		mstch::array t_ItemsContext;
		for (size_t i = 0; i < a_Struct->Members.size(); i++)
		{
			const TypeInfo::StructElement::Member& t_Item = a_Struct->Members[i];
			auto t_Element = a_Info.GetElement(t_Item.Type);
			a_Info.GetErrorHandler().AssertError(t_Element, "Could not find element '%s' (member of '%s')", t_Item.Type.c_str(), a_Struct->Name.c_str());
			if (t_Element == nullptr)
				continue;

			mstch::map t_ItemContext;
			std::string t_ItemType(t_Item.Type);
			MakeContext(t_ItemContext, t_ItemType, t_Item, a_Struct->PointerSize, t_Element, a_SerialisationParser, a_Info);
			t_ItemContext["Last"] = i == a_Struct->Members.size() - 1;

			switch (t_Element->Type)
			{
			case TypeInfo::ElementType::StructType:
			case TypeInfo::ElementType::EnumType:
				if (t_Item.IsArray || t_Item.IsPointer)
				{
					t_Declarations.insert(t_ItemContext);
					break;
				}
				// Intentionally fall through

			case TypeInfo::ElementType::PlainType:
			{
				std::string t_Header = a_Info.GetElementFile(*t_Element);
				if (t_Header.size() != 0)
					a_Headers.emplace(t_Header);
			}
			}

			t_ItemsContext.push_back(t_ItemContext);
		}
		a_ObjectContext["Items"] = t_ItemsContext;

		mstch::array t_DeclareContext;
		for (auto& t_Declaration : t_Declarations)
			t_DeclareContext.push_back(t_Declaration);
		a_ObjectContext["Declares"] = t_DeclareContext;
		a_ObjectContext["ElementSize"] = (unsigned int)a_SerialisationParser.GetMaxPackedSize(*a_Struct);
		a_ObjectContext["PointerSize"] = a_Info.GetPointerTypeName(a_Struct->PointerSize);

		if (a_Struct->Parent != nullptr)
			a_ObjectContext["Parent"] = a_Struct->Parent->Name;

		auto& t_Order = a_SerialisationParser.GetSerialisationOrder(*a_Struct);
		mstch::array t_SerialisationOrder;
		int i = 0;
		for (auto& t_Item : t_Order)
		{
			if (t_Item.Member == nullptr)
				continue;

			auto t_Element = t_Item.Element;
			a_Info.GetErrorHandler().AssertError(t_Element, "Could not find serialisation element '%s' (member of '%s')", t_Item.Member->VariableName.c_str(), a_Struct->Name.c_str());
			if (t_Element == nullptr)
				continue;

			mstch::map t_ItemContext;
			std::string t_ItemType(t_Item.Member->Type);

			MakeContext(t_ItemContext, t_ItemType, *t_Item.Member, a_Struct->PointerSize, t_Element, a_SerialisationParser, a_Info);
			t_ItemContext["Last"] = i++ == a_Struct->Members.size() - 1;

			// If has array, add the name of the count item
			if (t_Item.Member->IsArray)
			{
				// Find the array count
				bool t_Found = false;
				for (auto t_Member : t_Item.Member->Parent->Members)
				{
					if (t_Member.ArrayCountForMember != t_Item.Member->VariableName)
						continue;

					t_ItemContext["CountItem"] = t_Member.VariableName;
					t_Found = true;
					break;
				}

				a_Info.GetErrorHandler().AssertError(t_Found, "Found an array (%s), but could not find its array count item!", t_Item.Member->VariableName);
			}

			t_SerialisationOrder.push_back(t_ItemContext);
		}
		a_ObjectContext["SerialisationOrder"] = t_SerialisationOrder;
	}

	void SchemaParser::OutputNative(std::string_view a_OutputFolder, std::string_view a_MoustacheFile, std::string_view a_FileExtension, SerialisationParser& a_SerialisationParser)
	{
		printf("ASDF was instructed to output native files (Output folder: '%s', Moustache file: '%s', extension: '%s', %zu files)\n", a_OutputFolder.data(), a_MoustacheFile.data(), a_FileExtension.data(), m_FileToObjectMap.size());
		for (auto& t_FileObject : m_FileToObjectMap)
			OutputSingle(t_FileObject.second, a_OutputFolder, a_MoustacheFile, a_FileExtension, a_SerialisationParser);
	}

	void WriteMoustacheFile(const fs::path& a_OutputPath, const mstch::map& a_Context, std::string_view a_MoustacheFile, ErrorHandler& a_ErrorHandler)
	{
		fs::path t_MoustachePath(a_MoustacheFile);
		a_ErrorHandler.AssertFatal(fs::exists(t_MoustachePath), "Could not find moustache file '%s'", a_MoustacheFile.data());

		// Load in moustache file
		std::ifstream t_FileStream(t_MoustachePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

		size_t t_FileSize = static_cast<size_t>(t_FileStream.tellg());
		t_FileStream.seekg(0, std::ios::beg);

		std::string t_RenderResult;
		if (t_FileSize != 0)
		{
			std::vector<char> t_Bytes(t_FileSize);
			t_FileStream.read(&t_Bytes[0], t_FileSize);
			t_FileStream.close();

			std::string t_View(&t_Bytes[0], t_FileSize);

			t_RenderResult = mstch::render(t_View, a_Context);
		}

		std::ofstream t_Output(a_OutputPath, std::ios::binary);
		t_Output << t_RenderResult;
		t_Output.close();

		printf("- Wrote file '%s'\n", a_OutputPath.generic_string().c_str());
	}

	void SchemaParser::OutputSingle(std::string_view a_ObjectName, std::string_view a_OutputFolder, std::string_view a_MoustacheFile, std::string_view a_FileExtension, SerialisationParser& a_SerialisationParser)
	{
		mstch::map t_Context;
		mstch::map t_ObjectContext;
		std::string t_FileName;

		std::set<std::string> t_HeadersToInclude;
		auto* t_ObjectDeclaration = m_Info.GetElement(a_ObjectName);
		m_ErrorHandler.AssertFatal(t_ObjectDeclaration, "Could not output native object %s, it was not defined!", a_ObjectName.data());

		const TypeInfo::StructElement* t_Struct = t_ObjectDeclaration->Type == TypeInfo::ElementType::StructType ? static_cast<const TypeInfo::StructElement*>(t_ObjectDeclaration) : nullptr;
		const TypeInfo::EnumElement* t_Enum = t_ObjectDeclaration->Type == TypeInfo::ElementType::EnumType ? static_cast<const TypeInfo::EnumElement*>(t_ObjectDeclaration) : nullptr;

		t_ObjectContext["Name"] = (t_FileName = t_ObjectDeclaration->Name);
		for (const auto& t_ArgumentPair : t_ObjectDeclaration->TypeArguments)
			t_ObjectContext[t_ArgumentPair.first] = t_ArgumentPair.second;

		if (t_Enum)
			OutputEnumData(t_Enum, t_ObjectContext);
		else if (t_Struct)
			OutputStructData(t_Struct, t_ObjectContext, t_HeadersToInclude, a_SerialisationParser, m_Info);
		else
			m_ErrorHandler.AssertFatal(t_Enum || t_Struct, "Native object %s is neither an enum nor a struct.", a_ObjectName.data());

		std::string t_ObjectFile = m_Info.GetElementFile(*t_ObjectDeclaration, a_FileExtension);

		// Copy the headers to an an array
		mstch::array t_HeadersArrayContext;
		for (auto& t_Header : t_HeadersToInclude)
			t_HeadersArrayContext.push_back(t_Header);
		t_ObjectContext["Headers"] = t_HeadersArrayContext;

		t_Context[t_Struct ? "Struct" : "Enum"] = t_ObjectContext;

		fs::path t_OutputPath(a_OutputFolder);
		t_OutputPath.append(t_ObjectFile);
		fs::create_directories(t_OutputPath.parent_path());

		WriteMoustacheFile(t_OutputPath, t_Context, a_MoustacheFile, m_ErrorHandler);
	}
	
	void SchemaParser::OutputIndex(std::string_view a_OutputFile, std::string_view a_MoustacheFile)
	{
		printf("ASDF was instructed to output index file (Output file: '%s', Moustache file: '%s', %zu files)\n", a_OutputFile.data(), a_MoustacheFile.data(), m_FileToObjectMap.size());

		mstch::map t_Context;
		mstch::array t_ArrayContext;

		for (auto& t_FileObject : m_FileToObjectMap)
		{
			auto* t_ObjectDeclaration = m_Info.GetElement(t_FileObject.second);
			m_ErrorHandler.AssertFatal(t_ObjectDeclaration, "Could not output native object %s, it was not defined!", t_FileObject.second.c_str());

			mstch::map t_FileContext;
			for (const auto& t_ArgumentPair : t_ObjectDeclaration->TypeArguments)
				t_FileContext[t_ArgumentPair.first] = t_ArgumentPair.second;

			std::string t_FileName = m_Info.GetElementFile(*t_ObjectDeclaration, "");
			t_FileContext["Header"] = t_FileName + ".hpp";
			t_FileContext["Source"] = t_FileName + ".cpp";

			t_ArrayContext.push_back(t_FileContext);
		}

		t_Context["Files"] = t_ArrayContext;

		fs::path t_OutputPath(a_OutputFile);
		fs::create_directories(t_OutputPath.parent_path());

		WriteMoustacheFile(t_OutputPath, t_Context, a_MoustacheFile, m_ErrorHandler);
	}
}
