#include "asdf/type_info.hpp"
#include "asdf/error_handling.hpp"

#include <cassert>
#include <string>
#include <locale>
#include <filesystem>

namespace ASDF
{
	namespace fs = std::filesystem;
	TypeInfo::TypeInfo(ErrorHandler& a_ErrorHandler) :
		m_ErrorHandler(a_ErrorHandler)
	{
		AddPlain("uint8_t", "0", "cstdint");
		AddPlain("uint16_t", "0", "cstdint");
		AddPlain("uint32_t", "0", "cstdint");
		AddPlain("uint64_t", "0", "cstdint");
		AddPlain("int8_t", "0", "cstdint");
		AddPlain("int16_t", "0", "cstdint");
		AddPlain("int32_t", "0", "cstdint");
		AddPlain("int64_t", "0", "cstdint");
		AddPlain("float", "0.0f");
		AddPlain("double", "0.0");
		AddPlain("char", "''");
		AddPlain("bool", "0");

		// String
		{
			StructElement::Member t_Content;
			t_Content.VariableName = "Content";
			t_Content.Type = "char";
			t_Content.IsArray = true;
			t_Content.PrefersInline = true;

			StructElement::Member t_Size;
			t_Size.VariableName = "Size";
			t_Size.Type = "uint32_t";
			t_Size.IsArray = false;
			t_Size.PrefersInline = false;
			t_Size.ArrayCountForMember = t_Content.VariableName;

			AddStruct("String", { t_Content, t_Size },
			{
				{ "As", "std::string" },
				{ "AsHeader", "String" }
			}, BlobPointerSize::Size16);

			m_Elements["String"]->IsBaseASDF = true;
		}

		// PointerSize
		{
			AddEnum("PointerSize",
			{
				{ "Size16", 0 },
				{ "Size32", 1 },
				{ "Size64", 2 },
			}, {});

			m_Elements["PointerSize"]->IsBaseASDF = true;
		}

		// Resource
		{
			StructElement::Member t_Magic;
			t_Magic.VariableName = "Magic";
			t_Magic.Type = "uint32_t";
			t_Magic.DefaultValue = std::to_string(0x45DF0000);

			StructElement::Member t_Version;
			t_Version.VariableName = "Version";
			t_Version.Type = "uint32_t";
			t_Version.DefaultValue = std::to_string((uint32_t)(ResourceVersion::Major << 16) + (uint32_t)(ResourceVersion::Minor << 8) + (uint32_t)(ResourceVersion::Patch << 0));

			StructElement::Member t_PointerSize;
			t_PointerSize.VariableName = "PointerSize";
			t_PointerSize.Type = "PointerSize";
			t_PointerSize.DefaultValue = "Size16";

			StructElement::Member t_ExternalFiles;
			t_ExternalFiles.VariableName = "ExternalFiles";
			t_ExternalFiles.Type = "String";
			t_ExternalFiles.IsArray = true;

			StructElement::Member t_FileCount;
			t_FileCount.VariableName = "ExternalFileCount";
			t_FileCount.Type = "uint32_t";
			t_FileCount.IsArray = false;
			t_FileCount.PrefersInline = false;
			t_FileCount.ArrayCountForMember = t_ExternalFiles.VariableName;

			AddStruct("Resource", { t_Magic, t_Version, t_PointerSize, t_ExternalFiles, t_FileCount }, {}, BlobPointerSize::Size16);

			m_Elements["Resource"]->IsBaseASDF = true;
		}
	}

	TypeInfo::~TypeInfo()
	{
		for (auto t_Element : m_Elements)
			delete t_Element.second;
		m_Elements.clear();
	}

	void TypeInfo::SetIncludeDirectory(std::string_view a_IncludeDirectory)
	{
		m_IncludeDirectory = a_IncludeDirectory;
	}

	const TypeInfo::PlainElement& TypeInfo::AddPlain(std::string_view a_Name, std::string_view a_DefaultValueDenotation, std::string_view a_HeaderName)
	{
		std::string t_Name(a_Name);
		m_ErrorHandler.AssertError(m_Elements.find(t_Name) == m_Elements.end(), "Duplicate plain element found (%s)!", a_Name.data());

		PlainElement* t_Element = new PlainElement(a_Name, a_DefaultValueDenotation);
		t_Element->TypeArguments["Header"] = a_HeaderName;
		m_Elements[t_Name] = t_Element;
		return *t_Element;
	}

	const TypeInfo::EnumElement& TypeInfo::AddEnum(std::string_view a_Name, std::vector<EnumElement::Value>&& a_Values, std::map<std::string, std::string>&& a_ArgMap, std::string_view a_DefaultValueDenotation)
	{
		std::string t_Name(a_Name);
		m_ErrorHandler.AssertError(m_Elements.find(t_Name) == m_Elements.end(), "Duplicate enum element found (%s)!", a_Name.data());

		EnumElement* t_Element = new EnumElement(a_Name, std::move(a_Values), a_DefaultValueDenotation);
		t_Element->TypeArguments = a_ArgMap;
		m_Elements[t_Name] = t_Element;
		return *t_Element;
	}

	const TypeInfo::StructElement& TypeInfo::AddStruct(std::string_view a_Name, std::vector<StructElement::Member>&& a_Members, std::map<std::string, std::string>&& a_ArgMap, BlobPointerSize a_PointerSize, const TypeInfo::StructElement* a_Parent)
	{
		std::string t_Name(a_Name);
		m_ErrorHandler.AssertError(m_Elements.find(t_Name) == m_Elements.end(), "Duplicate struct element found (%s)!", a_Name.data());

		StructElement* t_Element = new StructElement(a_Name, std::move(a_Members));
		t_Element->TypeArguments = a_ArgMap;
		t_Element->Parent = a_Parent;
		t_Element->PointerSize = a_PointerSize;
		m_Elements[t_Name] = t_Element;
		return *t_Element;
	}
	
	const TypeInfo::BaseElement* TypeInfo::GetElement(std::string_view a_Type) const
	{
		auto t_Index = m_Elements.find(std::string(a_Type));
		if (t_Index == m_Elements.end())
			return nullptr;

		return t_Index->second;
	}

	std::string TypeInfo::GetElementFile(const TypeInfo::BaseElement& a_Element, std::string_view a_Extension) const
	{
		// If we don't have a header specified, but we're a struct, we can construct it
		if (a_Element.IsBaseASDF == false && a_Element.HeaderName.empty())
		{
			// TODO: Make customisable
			std::string t_Name = a_Element.Name;
			for (size_t i = 0; i < t_Name.size(); i++)
			{
				if (t_Name[i] < 'A' || t_Name[i] > 'Z')
					continue;

				t_Name[i] = tolower(t_Name[i]);

				bool t_NextIsLowerCase = i + 1 >= t_Name.size() || (t_Name[i + 1] < 'A' || t_Name[i + 1] > 'Z');
				if (i != 0 && t_NextIsLowerCase)
					t_Name = t_Name.substr(0, i) + "_" + t_Name.substr(i);
			}

			fs::path t_File(m_IncludeDirectory);
			t_File.append(t_Name + std::string(a_Extension));
			return t_File.generic_string();
		}

		return a_Element.HeaderName;
	}

	std::string TypeInfo::GetElementFile(std::string_view a_Type, std::string_view a_Extension) const
	{
		const TypeInfo::BaseElement* t_Element = GetElement(a_Type);
		if (t_Element == nullptr)
			return "";

		return GetElementFile(*t_Element, a_Extension);
	}

	std::string TypeInfo::GetPointerTypeName(TypeInfo::BlobPointerSize a_PointerSize) const
	{
		std::string t_Type;
		switch (a_PointerSize)
		{
		case TypeInfo::BlobPointerSize::Size16:
			t_Type = "uint16_t";
			break;

		case TypeInfo::BlobPointerSize::Size32:
			t_Type = "uint32_t";
			break;

		case TypeInfo::BlobPointerSize::Size64:
			t_Type = "uint64_t";
			break;

		default:
			GetErrorHandler().AssertFatal(false, "Unable to get relative pointer name, because the size is unknown.");
			break;
		}

		return t_Type;
	}

	size_t TypeInfo::GetPointerByteSize(TypeInfo::BlobPointerSize a_PointerSize) const
	{
		switch (a_PointerSize)
		{
		case TypeInfo::BlobPointerSize::Size16:
			return sizeof(uint16_t);

		case TypeInfo::BlobPointerSize::Size32:
			return sizeof(uint32_t);

		case TypeInfo::BlobPointerSize::Size64:
			return sizeof(uint64_t);

		default:
			GetErrorHandler().AssertFatal(false, "Unable to get relative pointer name, because the size is unknown.");
			break;
		}

		return 0;
	}

	const TypeInfo::StructElement::Member* TypeInfo::StructElement::GetMember(std::string_view a_Name) const
	{
		for (auto& t_Member : Members)
			if (t_Member.VariableName == a_Name)
				return &t_Member;
		return nullptr;
	}
}
