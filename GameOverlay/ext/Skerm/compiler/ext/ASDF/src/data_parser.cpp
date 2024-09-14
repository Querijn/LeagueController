#include "asdf/data_parser.hpp"
#include "asdf/serialisation_parser.hpp"
#include "asdf/error_handling.hpp"
#include "hash_helper.hpp"

#include <pugixml.hpp>

#include <filesystem>
#include <fstream>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ASDF
{
	namespace fs = std::filesystem;

#if ASDF_DEBUG_DATA_PARSING
	struct PrivateOutputBlobState;
	class PrivateDebugData
	{
	public:
		PrivateDebugData(const DataParser& a_DataParser, const DataParser::ElementData& a_Input, PrivateOutputBlobState& a_State, std::string& a_Output) :
			m_DataParser(a_DataParser),
			m_Input(a_Input),
			m_State(a_State),
			m_OutputTarget(a_Output)
		{}

		struct StructureData
		{
			std::string ElementName;

			bool IsArray;
			bool IsPointer;

			std::vector<DataParser::ElementData::ValueT> Values;
		};

		struct WrittenData
		{
			std::string Type;
			size_t ByteSize;
			DataParser::ElementData::ValueT Value;

			std::string ElementStackEntry;
		};

		using WrittenDataIndex = size_t;

		void PushElementStack(std::string_view a_Name) { m_ElementStack.push_back(a_Name.data()); }
		void PopElementStack() { m_ElementStack.pop_back(); }
		bool ElementStackIsEmpty() const { return m_ElementStack.empty(); }

		WrittenDataIndex WriteData(std::string_view a_Type, size_t a_ByteCount, DataParser::ElementData::ValueT a_Value)
		{
			if (a_ByteCount == 0)
				return ~0;

			std::string t_Stack = "";
			bool t_First = true;
			for (auto& t_Element : m_ElementStack)
			{
				if (t_First)
					t_First = false;
				else
					t_Stack += " -> ";

				t_Stack += t_Element;
			}

			m_WrittenData.emplace_back(WrittenData { a_Type.data(), a_ByteCount, a_Value, t_Stack });
			return m_WrittenData.size() - 1;
		}

		WrittenData& GetWrittenDataByIndex(WrittenDataIndex a_Index)
		{
			return m_WrittenData[a_Index];
		}

		void GenerateJSON();

	private:
		void GenerateInputJSON(const DataParser::ElementData& a_Root);

		const PrivateOutputBlobState& m_State;
		const DataParser& m_DataParser;
		const DataParser::ElementData& m_Input;
		std::string& m_OutputTarget;

		std::vector<WrittenData> m_WrittenData;
		std::vector<std::string> m_ElementStack;
	};

#define OUTPUT_DEBUG_DATA_RET(a_Index, a_TypeName, a_ByteCount, a_Value) do { a_Index = a_State.DebugData.WriteData(a_TypeName, a_ByteCount, a_Value); } while (0)
#define OUTPUT_DEBUG_DATA_RET_COND_PTR(a_Index, a_TypeName, a_ByteCount, a_Value) do { auto t_Result = a_State.DebugData.WriteData(a_TypeName, a_ByteCount, a_Value); if (a_Index != nullptr) *a_Index = t_Result; } while (0)
#define OUTPUT_DEBUG_DATA(a_TypeName, a_ByteCount, a_Value) a_State.DebugData.WriteData(a_TypeName, a_ByteCount, a_Value)
#define OUTPUT_DEBUG_DATA_A(a_State, a_TypeName, a_ByteCount, a_Value) a_State.DebugData.WriteData(a_TypeName, a_ByteCount, a_Value)
#define PUSH_ELEMENT_STACK(a_Name) a_State.DebugData.PushElementStack(a_Name)
#define PUSH_ELEMENT_STACK_A(a_State, a_Name) a_State.DebugData.PushElementStack(a_Name)
#define POP_ELEMENT_STACK() a_State.DebugData.PopElementStack()
#define POP_ELEMENT_STACK_A(a_State) a_State.DebugData.PopElementStack()

#else

#define OUTPUT_DEBUG_DATA_RET(a_Index, a_TypeName, a_ByteCount, a_Value) do { } while(0)
#define OUTPUT_DEBUG_DATA_RET_COND_PTR(a_Index, a_TypeName, a_ByteCount, a_Value) do { } while(0)
#define OUTPUT_DEBUG_DATA(a_TypeName, a_ByteCount, a_Value) do { } while(0)
#define OUTPUT_DEBUG_DATA_A(a_State, a_TypeName, a_ByteCount, a_Value) do { } while(0)
#define PUSH_ELEMENT_STACK(a_Name) do { } while(0)
#define PUSH_ELEMENT_STACK_A(a_State, a_Name) do { } while(0)
#define POP_ELEMENT_STACK() do { } while(0)
#define POP_ELEMENT_STACK_A(a_State) do { } while(0)

	MapIndex::MapIndex(const std::string& a_Index)
		: CRC(Hash::CRC32(a_Index))
	{
	}

#endif

	enum class OutputBlobScope
	{
		GlobalScope,
		ElementScope,
		ArrayScope
	};

	struct PrivateOutputBlobState
	{
		PrivateOutputBlobState(const DataParser& a_DataParser, const DataParser::ElementData& a_ElementData, DataParser::OutputBlobOutput& a_Parent, TypeInfo& a_Info, SerialisationParser& a_SerParser) :
			Parent(a_Parent),
		#if ASDF_DEBUG_DATA_PARSING
			DebugData(a_DataParser, a_ElementData, *this, a_Parent.DebugData),
		#endif
			Data(a_Parent.Data),
			Info(a_Info),
			SerParser(a_SerParser)
		{ }

		DataParser::OutputBlobOutput& Parent;

	#if ASDF_DEBUG_DATA_PARSING
		PrivateDebugData DebugData;
	#endif

		std::vector<uint8_t>& Data;

		struct ArrayState
		{
			size_t CurrentIndex;
			size_t ElementCount = 0;
		};

		struct PointerData
		{
			size_t Offset;
			TypeInfo::BlobPointerSize PointerSize;

		#if ASDF_DEBUG_DATA_PARSING
			PrivateDebugData::WrittenDataIndex DebugDataIndex;
		#endif
		};

		using PointerMap = std::vector<std::pair<const DataParser::ElementData*, PointerData>>;
		using ArrayStateMap = std::map<const DataParser::ElementData*, ArrayState>;

		PointerMap Pointers;
		SerialisationParser& SerParser;
		TypeInfo& Info;
		OutputBlobScope Scope = OutputBlobScope::GlobalScope;
		ArrayStateMap Arrays;
	};

	void OutputBlobElementData(const DataParser::ElementData& a_ElementData, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State, PrivateOutputBlobState::PointerMap::iterator* a_PointerIteratorHint = nullptr);

	DataParser::DataParser(TypeInfo & a_Info) :
		m_Info(a_Info), m_ErrorHandler(a_Info.GetErrorHandler())
	{
	}

	pugi::xml_node GetArrayCountElement(const pugi::xml_object_range<pugi::xml_named_node_iterator>& a_Source, std::string_view a_Name)
	{
		for (auto& t_Item : a_Source)
		{
			auto t_Name = t_Item.attribute("IsArrayCountFor").as_string();
			if (a_Name == t_Name)
				return t_Item;
		}

		return pugi::xml_node();
	}

	void ParseXMLElement(pugi::xml_node& a_RootXML, std::shared_ptr<DataParser::ElementData> a_ElementData, DataParser::SubData& a_FileData, TypeInfo& a_Info, const TypeInfo::BaseElement* a_ElementHint = nullptr)
	{
		auto& t_ErrorHandler = a_Info.GetErrorHandler();
		const char* t_RootElementType = nullptr;
		const TypeInfo::BaseElement* t_RootElement = a_ElementHint;

		if (a_ElementHint == nullptr)
		{
			t_RootElementType = a_RootXML.attribute("Type").as_string();
			t_RootElement = a_Info.GetElement(t_RootElementType);
		}
		else
		{
			t_RootElementType = a_ElementHint->Name.c_str();
		}

		if (a_ElementData)
			a_ElementData->Element = t_RootElement;

		t_ErrorHandler.AssertError(t_RootElement, "DataParser::ParseXML could not find declaration of %s!", t_RootElementType);
		if (t_RootElement == nullptr)
			return;

		// Just put enums down as uint32_ts
		if (t_RootElement->Type == TypeInfo::ElementType::EnumType)
		{
			auto t_ElementData = MakeElementData();
			t_ElementData->Element = t_RootElement;
			t_ElementData->Value = a_RootXML.attribute("Value").as_string();
			a_FileData[t_RootElement->Name] = t_ElementData;
			return;
		}

		if (t_RootElement->Type == TypeInfo::ElementType::StructType)
		{
			const TypeInfo::StructElement* t_Struct = static_cast<const TypeInfo::StructElement*>(t_RootElement);
			for (auto& t_MemberXMLNode : a_RootXML.children("Item"))
			{
				std::string t_MemberName = t_MemberXMLNode.attribute("Name").as_string();
				std::string t_MemberType = t_MemberXMLNode.attribute("Type").as_string();

				const TypeInfo::StructElement::Member* t_StructMember = t_Struct->GetMember(t_MemberName.c_str());
				t_ErrorHandler.AssertError(t_StructMember, "One of the item declarations was missing for member %s of %s.", t_MemberName.c_str(), t_RootElement->Name.c_str());
				if (t_StructMember == nullptr)
					continue;

				// Skip if it's an array count - arrays are handled below, they need their count, don't encode counts on their own
				if (t_StructMember->ArrayCountForMember.size() != 0)
					continue;

				// If it's an array, it needs another member to function correctly, as the size of the data depends on the count of the array.
				if (t_StructMember->IsArray)
				{
					auto& t_ArrayItem = t_MemberXMLNode;
					auto t_Children = t_ArrayItem.children("Item");
					size_t t_Count = std::distance(t_Children.begin(), t_Children.end());
					bool t_HandledAsInline = false;

					// Here we check inline size for specific types that allow it
					if (t_StructMember->PrefersInline)
					{
						auto t_ArrayElement = a_Info.GetElement(t_StructMember->Type);
						t_ErrorHandler.AssertError(t_ArrayElement, "The type for %s is undefined!", t_MemberName.c_str());
						t_ErrorHandler.AssertWarning(t_ArrayElement->Type == TypeInfo::ElementType::PlainType, "Found an element (%s) that prefers inline, but we can't inline non-plain types!", t_MemberName.c_str());

						if (t_ArrayElement->Type == TypeInfo::ElementType::PlainType)
						{
							std::string t_Value = t_ArrayItem.attribute("Value").as_string();
							
							if (t_StructMember->Type == "char")
							{
								t_Count = t_Value.length() + 1;
								auto t_Data = MakeElementData();
								t_Data->IsArray = true;
								t_Data->Element = t_ArrayElement;
								t_Data->Value = t_Value + '\0';
								a_FileData[t_StructMember->VariableName] = t_Data;

								t_HandledAsInline = true;
							}
							else // TODO: Others
								t_ErrorHandler.AssertWarning(false, "Cannot determine %s's inline size!", t_StructMember->Type);
						}
					}

					// Count
					const TypeInfo::StructElement::Member* t_CountMember = nullptr;
					{
						std::string t_CountString = std::to_string(t_Count);

						const TypeInfo::BaseElement* t_CountDeclaration = nullptr;
						for (auto& t_Member : t_Struct->Members)
						{
							if (t_Member.ArrayCountForMember == t_StructMember->VariableName)
							{
								t_CountDeclaration = a_Info.GetElement(t_Member.Type);
								t_CountMember = &t_Member;
								break;
							}
						}

						t_ErrorHandler.AssertError(t_CountMember, "Could not find the count item for the array %s", t_StructMember->VariableName.c_str());
						t_ErrorHandler.AssertError(t_CountDeclaration, "Could not find the count element for the array %s", t_StructMember->VariableName.c_str());
						if (t_CountDeclaration == nullptr)
							continue;

						auto t_CountElementData = MakeElementData();
						t_CountElementData->Element = t_CountDeclaration;
						t_CountElementData->Value = t_CountString;
						a_FileData[t_CountMember->VariableName] = t_CountElementData;
					}

					// Elements (but only if we have not already handled it and we have some)
					if (t_HandledAsInline == false && t_Count != 0)
					{
						auto t_ArrayElement = a_Info.GetElement(t_StructMember->Type);
						t_ErrorHandler.AssertError(t_ArrayElement, "The type for %s is undefined!", t_MemberName.c_str());

						auto t_ElementData = MakeElementData();
						t_ElementData->IsArray = true;
						t_ElementData->Element = t_ArrayElement;

						int t_ArrayOffset = 0;
						for (auto& t_ArrayChild : t_Children)
						{
							auto& t_SubData = t_ElementData->SubElements[std::to_string(t_ArrayOffset++)];
							t_SubData = MakeElementData();
							ParseXMLElement(t_ArrayChild, t_SubData, t_SubData->SubElements, a_Info, t_ArrayElement);
						}
						a_FileData[t_StructMember->VariableName] = t_ElementData;
					}
					continue;
				}
				else if (t_StructMember->IsPointer)
				{
					auto t_TargetElement = a_Info.GetElement(t_StructMember->Type);
					t_ErrorHandler.AssertError(t_TargetElement, "The type for %s is undefined!", t_MemberName.c_str());

					auto t_ElementData = MakeElementData();
					t_ElementData->IsArray = true;
					t_ElementData->Element = t_TargetElement;

					a_FileData[t_StructMember->VariableName] = t_ElementData;
				}

				// Other type handling
				auto t_Data = MakeElementData();
				t_Data->Element = a_Info.GetElement(t_MemberXMLNode.attribute("Type").as_string());
				t_ErrorHandler.AssertError(t_Data->Element, "The type for %s is undefined!", t_MemberName.c_str());
				ParseXMLElement(t_MemberXMLNode, t_Data, t_Data->SubElements, a_Info);
				a_FileData[t_StructMember->VariableName] = t_Data;
			}
			return;
		}

		if (t_RootElement->Type == TypeInfo::ElementType::PlainType)
		{
			auto t_ValueNode = a_RootXML.attribute("Value");
			t_ErrorHandler.AssertError(t_ValueNode.empty() == false, "Did not expect an empty value from %s", t_RootElementType);

			a_ElementData->Element = t_RootElement;
			a_ElementData->Value = t_ValueNode.as_string();
			return;
		}
	}

	const DataParser::ElementData DataParser::ParseXML(std::string_view a_File)
	{
		pugi::xml_document t_Document;
		pugi::xml_parse_result t_Result = t_Document.load_file(a_File.data());
		m_ErrorHandler.AssertFatal(t_Result, "XML Data file '%s' failed to load.", a_File.data());

		pugi::xml_node t_Root = t_Document.child("Item");
		m_ErrorHandler.AssertError(t_Root.empty() == false, "No root found in '%s'", a_File.data());

		auto t_ResultData = MakeElementData();
		ParseXMLElement(t_Root, t_ResultData, t_ResultData->SubElements, m_Info);
		return *t_ResultData.get();
	}

	void ParsePlainInternal(const ASDF::TypeInfo::PlainElement& a_Plain, DataParser::ElementData& a_Data, TypeInfo& a_Info)
	{
		a_Data.Element = &a_Plain;
		a_Data.Value = a_Plain.DefaultValue;
	}

	void ParseEnumInternal(const ASDF::TypeInfo::EnumElement& a_Enum, DataParser::ElementData& a_Data, TypeInfo& a_Info)
	{
		a_Data.Element = &a_Enum;
		a_Data.Value = a_Enum.DefaultValue;
	}

	void ParseStructInternal(const ASDF::TypeInfo::StructElement& a_Struct, DataParser::ElementData& a_Data, TypeInfo& a_Info)
	{
		a_Data.Element = &a_Struct;

		std::vector<const TypeInfo::StructElement::Member*> t_CountElements;

		for (auto& t_Member : a_Struct.Members)
		{
			auto* t_Element = a_Info.GetElement(t_Member.Type);
			a_Info.GetErrorHandler().AssertError(t_Element, "Could not determine member type '%s' of '%s' in '%s'", t_Member.Type.c_str(), t_Member.VariableName.c_str(), a_Struct.Name.c_str());
			if (t_Element == nullptr)
				continue;

			auto& t_SubElementData = a_Data.SubElements[t_Member.VariableName];
			t_SubElementData = MakeElementData();
			t_SubElementData->Element = t_Element;
			t_SubElementData->IsPointer = t_Member.IsPointer;
			t_SubElementData->IsArray = t_Member.IsArray;

			if (t_Member.IsArray && t_Member.ArrayCountForMember.size() != 0)
				t_CountElements.push_back(&t_Member);

			if (t_Member.IsPointer || t_Member.IsArray)
				continue;

			switch (t_Element->Type)
			{
			case TypeInfo::ElementType::StructType:
				ParseStructInternal(*(TypeInfo::StructElement*)t_Element, *t_SubElementData, a_Info);
				break;

			case TypeInfo::ElementType::PlainType:
				ParsePlainInternal(*(TypeInfo::PlainElement*)t_Element, *t_SubElementData, a_Info);
				break;

			case TypeInfo::ElementType::EnumType:
				ParseEnumInternal(*(TypeInfo::EnumElement*)t_Element, *t_SubElementData, a_Info);
				break;
			}
		}

		// Correctly give all arrays their sub elements
		for (auto& t_CountElement : t_CountElements)
		{
			auto t_Array = a_Data.SubElements.find(t_CountElement->ArrayCountForMember);
			if (t_Array == a_Data.SubElements.end())
				continue;

			auto t_Index = a_Data.SubElements.find(t_CountElement->VariableName);
			
			assert(t_Index != a_Data.SubElements.end());
		}
	}

	DataParser::ElementData DataParser::ParseStruct(const ASDF::TypeInfo::StructElement& a_Struct) const
	{
		auto t_StructOutput = DataParser::ElementData();
		ParseStructInternal(a_Struct, t_StructOutput, m_Info);
		return t_StructOutput;
	}

	DataParser::ElementData DataParser::ParsePlain(const ASDF::TypeInfo::PlainElement& a_Plain) const
	{
		auto t_Output = DataParser::ElementData();
		ParsePlainInternal(a_Plain, t_Output, m_Info);
		return t_Output;
	}

	// This macro creates one function for every number type, so that I can call it from WriteNumber below
#define NUM_CONVERTER(a_Number, a_Function) static void ConvertNumber(std::string_view a_Value, a_Number& a_Result) { try { a_Result = static_cast<a_Number>(a_Function(a_Value.data())); } catch (...) { a_Result = a_Number(); } }
	NUM_CONVERTER(uint8_t, std::stoul)
	NUM_CONVERTER(uint16_t, std::stoul)
	NUM_CONVERTER(uint32_t, std::stoul)
	NUM_CONVERTER(uint64_t, std::stoull)

	NUM_CONVERTER(int8_t, std::stol)
	NUM_CONVERTER(int16_t, std::stol)
	NUM_CONVERTER(int32_t, std::stol)
	NUM_CONVERTER(int64_t, std::stoll)

	template<typename a_Number>
	void WriteNumber(std::string_view a_Type, const DataParser::ElementData::ValueT& a_Value, PrivateOutputBlobState& a_State, std::string a_TypeName = ""
	#if ASDF_DEBUG_DATA_PARSING
		, PrivateDebugData::WrittenDataIndex* a_WrittenDataLocation = nullptr
	#endif
		)
	{
		a_Number t_Result;
		ConvertNumber(a_Value.GetString(), t_Result);

		const uint8_t* t_Data = reinterpret_cast<const uint8_t*>(&t_Result);
		a_State.Data.insert(a_State.Data.end(), t_Data, t_Data + sizeof(a_Number));
		OUTPUT_DEBUG_DATA_RET_COND_PTR(a_WrittenDataLocation, a_TypeName.empty() ? a_Type.data() : a_TypeName.c_str(), sizeof(a_Number), a_Value);
	}

	inline void OutputBlobSingleType(std::string_view a_Type, const DataParser::ElementData::ValueT& a_Value, PrivateOutputBlobState& a_State, std::string a_TypeName = ""
	#if ASDF_DEBUG_DATA_PARSING
		, PrivateDebugData::WrittenDataIndex* a_WrittenDataLocation = nullptr
	#endif
		)
	{
#define NEW_NUMBER_CASE(a_Number, a_Function, a_NewFunction) case COMPILE_TIME_CRC32_STR(#a_Number):\
		{\
			bool t_FailedWrite = false;\
			a_Number t_Result;\
			if (a_Value.HasNativeValue() == false)\
			{\
				try { t_Result = static_cast<a_Number>(a_Function(a_Value.GetString())); }\
				catch (...) { t_FailedWrite = true; t_Result = a_Number(); }\
			}\
			else t_Result = a_Value.a_NewFunction();\
			const uint8_t* t_Data = reinterpret_cast<const uint8_t*>(&t_Result);\
			a_State.Data.insert(a_State.Data.end(), t_Data, t_Data + sizeof(a_Number));\
			OUTPUT_DEBUG_DATA_RET_COND_PTR(a_WrittenDataLocation, a_TypeName.empty() ? a_Type.data() : a_TypeName.c_str(), sizeof(a_Number), a_Value);\
			break;\
		}

		switch (Hash::CRC32(a_Type))
		{
		#if ASDF_DEBUG_DATA_PARSING

		case COMPILE_TIME_CRC32_STR("uint8_t"): WriteNumber<uint8_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;
		case COMPILE_TIME_CRC32_STR("uint16_t"): WriteNumber<uint16_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;
		case COMPILE_TIME_CRC32_STR("uint32_t"): WriteNumber<uint32_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;
		case COMPILE_TIME_CRC32_STR("uint64_t"): WriteNumber<uint64_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;

		case COMPILE_TIME_CRC32_STR("int8_t"): WriteNumber<int8_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;
		case COMPILE_TIME_CRC32_STR("int16_t"): WriteNumber<int16_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;
		case COMPILE_TIME_CRC32_STR("int32_t"): WriteNumber<int32_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;
		case COMPILE_TIME_CRC32_STR("int64_t"): WriteNumber<int64_t>(a_Type, a_Value, a_State, a_TypeName, a_WrittenDataLocation); break;

		#else

		case COMPILE_TIME_CRC32_STR("uint8_t"): WriteNumber<uint8_t>(a_Type, a_Value, a_State, a_TypeName); break;
		case COMPILE_TIME_CRC32_STR("uint16_t"): WriteNumber<uint16_t>(a_Type, a_Value, a_State, a_TypeName); break;
		case COMPILE_TIME_CRC32_STR("uint32_t"): WriteNumber<uint32_t>(a_Type, a_Value, a_State, a_TypeName); break;
		case COMPILE_TIME_CRC32_STR("uint64_t"): WriteNumber<uint64_t>(a_Type, a_Value, a_State, a_TypeName); break;

		case COMPILE_TIME_CRC32_STR("int8_t"): WriteNumber<int8_t>(a_Type, a_Value, a_State, a_TypeName); break;
		case COMPILE_TIME_CRC32_STR("int16_t"): WriteNumber<int16_t>(a_Type, a_Value, a_State, a_TypeName); break;
		case COMPILE_TIME_CRC32_STR("int32_t"): WriteNumber<int32_t>(a_Type, a_Value, a_State, a_TypeName); break;
		case COMPILE_TIME_CRC32_STR("int64_t"): WriteNumber<int64_t>(a_Type, a_Value, a_State, a_TypeName); break;

		#endif

		NEW_NUMBER_CASE(float, std::stof, GetFloat)
		NEW_NUMBER_CASE(double, std::stod, GetDouble)

		case COMPILE_TIME_CRC32_STR("char"):
		{
			a_State.Data.push_back(a_Value.GetChar());
			OUTPUT_DEBUG_DATA_RET_COND_PTR(a_WrittenDataLocation, a_TypeName.empty() ? a_Type.data() : a_TypeName.c_str(), sizeof(char), a_Value);
			break;
		}

		default:
			// __debugbreak();
			break;
		}
	}

	size_t OutputBlobRelativePointer(size_t a_Offset, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State
	#if ASDF_DEBUG_DATA_PARSING
		, PrivateDebugData::WrittenDataIndex* a_WrittenDataLocation
	#endif
		)
	{
		auto t_Size = a_State.Data.size();
		std::string t_Type = a_State.Info.GetPointerTypeName(a_PointerSize);

		OutputBlobSingleType(t_Type, DataParser::ElementData::ValueT(std::to_string(a_Offset), DataParser::ElementData::ValueT::Type::Unset, false), a_State, t_Type + " (ptr)"
		#if ASDF_DEBUG_DATA_PARSING
			, a_WrittenDataLocation
		#endif
			);

		return t_Size;
	}

	void OutputBlobFillDefaultData(const TypeInfo::BaseElement* a_Element, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State, bool a_IsArray)
	{
		auto t_ErrorHandler = a_State.Info.GetErrorHandler();
		t_ErrorHandler.AssertFatal(a_Element, "Unable to add default data, element does not exist.");

		// In this case, we have to write a nullptr
		if (a_IsArray)
		{
			t_ErrorHandler.AssertFatal(a_State.Scope == OutputBlobScope::ElementScope, "Expected to be in element scope when writing array default data!");
			// It's a null pointer for now, this will contain the data later (Store pointer location)

			OutputBlobRelativePointer(0, a_PointerSize, a_State
			#if ASDF_DEBUG_DATA_PARSING
				, nullptr
			#endif
				);
			return;
		}

		switch (a_Element->Type)
		{
		case TypeInfo::ElementType::StructType:
		{
			auto* t_Struct = static_cast<const TypeInfo::StructElement*>(a_Element);
			auto& t_Order = a_State.SerParser.GetSerialisationOrder(*t_Struct);
			t_ErrorHandler.AssertFatal(t_Order.size() != 0, "Expected struct ");

			for (auto& t_SerialisationItem : t_Order)
			{
				PUSH_ELEMENT_STACK(t_SerialisationItem.Member->VariableName);

				auto t_TempScope = a_State.Scope;
				a_State.Scope = OutputBlobScope::ElementScope;
				OutputBlobFillDefaultData(t_SerialisationItem.Element, t_Struct->PointerSize, a_State, t_SerialisationItem.Member->IsArray);
				a_State.Scope = t_TempScope;

				POP_ELEMENT_STACK();
			}
			break;
		}

		case TypeInfo::ElementType::EnumType:
		{
			auto* t_Enum = static_cast<const TypeInfo::EnumElement*>(a_Element);
			auto t_Data = DataParser::ElementData::ValueT(std::to_string(t_Enum->Values.begin()->RepresentingNumber), DataParser::ElementData::ValueT::Type::Unset, false);
			OutputBlobSingleType("uint32_t", t_Data, a_State);
			break;
		}

		case TypeInfo::ElementType::PlainType:
		{
			auto t_Data = DataParser::ElementData::ValueT(a_Element->DefaultValue, DataParser::ElementData::ValueT::Type::Unset, false);
			OutputBlobSingleType(a_Element->Name, t_Data, a_State);
			break;
		}
		}
	}

	void OutputBlobWriteEnum(const DataParser::ElementData& a_ElementData, PrivateOutputBlobState& a_State, ErrorHandler& a_ErrorHandler)
	{
		auto& t_Enum = *static_cast<const TypeInfo::EnumElement*>(a_ElementData.Element);
		int64_t t_RepresentingNumberValue = 0;
		int64_t t_Result = -1;

		bool t_IsNumber = a_ElementData.Value.HasString() == false;
		if (t_IsNumber)
		{
			try
			{
				t_RepresentingNumberValue = std::atoi(a_ElementData.Value.GetString().c_str());
			}
			catch (...)
			{
				t_RepresentingNumberValue = -1;
			}

			for (size_t i = 0; i < t_Enum.Values.size(); i++)
			{
				if (t_RepresentingNumberValue == t_Enum.Values[i].RepresentingNumber)
				{
					t_Result = t_RepresentingNumberValue;
					break;
				}
			}
		}

		// If it's not a number, it's a value by name
		else for (size_t i = 0; i < t_Enum.Values.size(); i++)
		{
			if (t_Enum.Values[i].ValueName == a_ElementData.Value.GetString())
			{
				t_Result = t_Enum.Values[i].RepresentingNumber;
				break;
			}
		}

		a_ErrorHandler.AssertError(t_Result >= 0, "Could not determine value of enum element '%s': %s", a_ElementData.Element->Name.c_str(), a_ElementData.Value.GetString().c_str());
		if (t_Result < 0 && t_Enum.Values.empty() == false)
			t_Result = t_Enum.Values[0].RepresentingNumber;

		std::string t_Value = std::to_string(t_Result);

		auto t_Scope = a_State.Scope;
		a_State.Scope = OutputBlobScope::ElementScope;
		auto t_Data = DataParser::ElementData::ValueT(t_Value, DataParser::ElementData::ValueT::Type::Unset, false);
		OutputBlobSingleType("uint32_t", t_Data, a_State); // If this changes, SerialisationParser::GetOwnPackedSize() needs to reflect this change too!
		a_State.Scope = t_Scope;
	}

	std::vector<std::string> OutputBlobSplitArray(std::string_view a_Type, std::string_view a_Value, PrivateOutputBlobState& a_State)
	{
		std::vector<std::string> t_Result;
		switch (Hash::CRC32(a_Type))
		{
		case COMPILE_TIME_CRC32_STR("char"): // Basically a string
			if (a_Value.empty())
				break;

			t_Result.reserve(a_Value.size());
			for (char t_Character : a_Value)
			{
				char t_Data[2] = { t_Character, 0 };
				t_Result.push_back(std::string(t_Data));
			}
			t_Result.push_back(std::string({ 0 }));
			break;

		default:
			a_State.Info.GetErrorHandler().AssertFatal(false, "Unable to split array for type '%s'.", a_Type.data());
			break;
		}

		return t_Result;
	}

	void OutputBlobWriteStruct(const DataParser::ElementData& a_ElementData, PrivateOutputBlobState& a_State, PrivateOutputBlobState::PointerMap::iterator* a_PointerIteratorHint = nullptr)
	{
		auto& t_ErrorHandler = a_State.Info.GetErrorHandler();
		auto* t_Element = a_ElementData.Element;

		auto* t_Struct = static_cast<const TypeInfo::StructElement*>(t_Element);
		auto& t_Order = a_State.SerParser.GetSerialisationOrder(*t_Struct);
		t_ErrorHandler.AssertFatal(t_Order.empty() == false, "Expected struct order!");

		// HACK: This allows you to set value directly onto the string struct.
		DataParser::ElementData t_ElementData = a_ElementData;
		if (t_Struct->Name == "String" && t_ElementData.Value.HasString())
		{
			std::string t_Value = t_ElementData.Value.GetString();
			t_ElementData.GetSubElement("Content")->Value = t_Value;
			t_ElementData.GetSubElement("Size")->Value = std::to_string(t_Value.empty() ? 0 : t_Value.size() + 1);
		}

		for (auto& t_SerialisationItem : t_Order)
		{
			std::string t_Name = t_SerialisationItem.Member->VariableName;
			PUSH_ELEMENT_STACK(t_Name);
			t_ErrorHandler.AssertFatal(t_SerialisationItem.Member, "%s has a serialisation order, but it's missing a member (Element name: %s).", t_Element->Name.c_str(), t_SerialisationItem.Element ? t_SerialisationItem.Element->Name.c_str() : "???");

			// Are we an array count? It might mismatch the actual count or the array might be not in data. We need to handle those cases.
			if (t_SerialisationItem.Member->ArrayCountForMember.empty() == false)
			{
				const SerialisationParser::SerialisationItem* t_ArrayItem = nullptr;
				for (auto& t_PotentialArrayItem : t_Order)
				{
					if (t_PotentialArrayItem.Member->VariableName == t_SerialisationItem.Member->ArrayCountForMember)
					{
						t_ErrorHandler.AssertWarning(t_PotentialArrayItem.Member->IsArray, "Found the array for the array count '%s' (%s), but it's not marked as an array?", t_PotentialArrayItem.Member->VariableName.c_str(), t_SerialisationItem.Member->ArrayCountForMember.c_str());
						t_ArrayItem = &t_PotentialArrayItem;
						break;
					}
				}

				t_ErrorHandler.AssertFatal(t_ArrayItem, "Could not find array for '%s' (was looking for '%s').", t_SerialisationItem.Member->VariableName.c_str(), t_SerialisationItem.Member->ArrayCountForMember.c_str());

				auto t_ArrayDataIndex = t_ElementData.SubElements.find(t_ArrayItem->Member->VariableName);
				t_ErrorHandler.AssertFatal(t_ArrayItem->Member, "%s has a serialisation order, but it's missing a member (Element name: %s).", t_Element->Name.c_str(), t_SerialisationItem.Element ? t_SerialisationItem.Element->Name.c_str() : "???");
				bool t_ArrayDataExists = t_ArrayDataIndex != t_ElementData.SubElements.end();
				if (t_ArrayDataExists)
				{
					// Array can be stored in value, or inline. Currently the value length if inline
					size_t t_InlineSize = t_ArrayDataIndex->second->Value.HasString() ? OutputBlobSplitArray(t_ArrayDataIndex->second->Element->Name, t_ArrayDataIndex->second->Value.GetString(), a_State).size() : 0;
					size_t t_RegularSize = t_ArrayDataIndex->second->SubElements.size();
					std::string t_ArraySize = std::to_string(t_InlineSize == 0 ? t_RegularSize : t_InlineSize);

					auto t_Data = DataParser::ElementData::ValueT(t_ArraySize, DataParser::ElementData::ValueT::Type::Unset, false);
					OutputBlobSingleType(t_SerialisationItem.Element->Name, t_Data, a_State);
				}
				else // Array doesn't have data
				{
					auto t_Data = DataParser::ElementData::ValueT("0", DataParser::ElementData::ValueT::Type::Unset, false);
					OutputBlobSingleType(t_SerialisationItem.Element->Name, t_Data, a_State);
				}
			}
			else
			{
				auto t_DataIndex = t_ElementData.SubElements.find(t_Name); // Either use the already existing array element index or find by name
				bool t_DataExists = t_DataIndex != t_ElementData.SubElements.end();
				t_ErrorHandler.AssertWarning(t_DataExists, "%s has an item in its serialisation order that is not in the data. (%s) Will fill with default items.", t_Element->Name.c_str(), t_Name.c_str());

				auto t_TempScope = a_State.Scope;
				a_State.Scope = OutputBlobScope::ElementScope;
				if (t_DataExists)
					OutputBlobElementData(*t_DataIndex->second, t_Struct->PointerSize, a_State);
				else
					OutputBlobFillDefaultData(t_SerialisationItem.Element, t_Struct->PointerSize, a_State, t_SerialisationItem.Member->IsArray);
				a_State.Scope = t_TempScope;
			}

			POP_ELEMENT_STACK();
		}
	}

	void PushNullPointer(const DataParser::ElementData& a_ElementData, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State)
	{
		// It's a null pointer for now, this will contain the data later (Store pointer location)
	#if ASDF_DEBUG_DATA_PARSING
		PrivateDebugData::WrittenDataIndex t_WrittenDataLocation;
		size_t t_Offset = OutputBlobRelativePointer(0, a_PointerSize, a_State, &t_WrittenDataLocation);
		PrivateOutputBlobState::PointerData t_Data{ t_Offset, a_PointerSize, t_WrittenDataLocation };
	#else 
		size_t t_Offset = OutputBlobRelativePointer(0, a_PointerSize, a_State);
		PrivateOutputBlobState::PointerData t_Data{ t_Offset, a_PointerSize };
	#endif

		a_State.Pointers.push_back(std::make_pair(&a_ElementData, t_Data));
	}

	void OutputBlobElementScope(const DataParser::ElementData& a_ElementData, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State, PrivateOutputBlobState::PointerMap::iterator* a_PointerIteratorHint = nullptr)
	{
		auto& t_ErrorHandler = a_State.Info.GetErrorHandler();
		auto* t_Element = a_ElementData.Element;
		t_ErrorHandler.AssertFatal(t_Element, "Unable to output element to blob, element data contained a null element.");

		// These need to write a pointer
		if (a_ElementData.IsPointer || a_ElementData.IsArray)
		{
			PushNullPointer(a_ElementData, a_PointerSize, a_State);
		}

		// Init array data
		if (a_ElementData.IsArray)
		{
			auto& t_Array = a_State.Arrays[&a_ElementData];
			t_Array.CurrentIndex = 0;
			t_Array.ElementCount = a_ElementData.SubElements.size();
		}
		else switch(t_Element->Type)
		{
		case TypeInfo::ElementType::StructType:
			OutputBlobWriteStruct(a_ElementData, a_State, a_PointerIteratorHint);
			break;

			// Write enums as uint32_t
		case TypeInfo::ElementType::EnumType:
			OutputBlobWriteEnum(a_ElementData, a_State, t_ErrorHandler);
			break;

		case TypeInfo::ElementType::PlainType:
			OutputBlobSingleType(t_Element->Name, a_ElementData.Value, a_State);
			break;
		}
	}

	void OutputBlobGlobalScopePointer(PrivateOutputBlobState& a_State, PrivateOutputBlobState::PointerMap::iterator& a_PointerIteratorHint)
	{
		auto& t_ErrorHandler = a_State.Info.GetErrorHandler();

		uint8_t* t_Pointer = a_State.Data.data() + a_PointerIteratorHint->second.Offset;
		size_t t_PointerLocation = reinterpret_cast<size_t>(t_Pointer);
		size_t t_DataLocation = reinterpret_cast<size_t>(a_State.Data.data()) + a_State.Data.size();
		size_t t_RelativePosition = t_DataLocation - t_PointerLocation;

		t_ErrorHandler.AssertError(*t_Pointer == 0, "Expected data to point to relative null pointer!");

	#if ASDF_DEBUG_DATA_PARSING
		PrivateDebugData::WrittenData& t_Data = a_State.DebugData.GetWrittenDataByIndex(a_PointerIteratorHint->second.DebugDataIndex);
		t_Data.Value = (double)t_RelativePosition; // TODO: Maybe I should support uints
	#endif

		switch (a_PointerIteratorHint->second.PointerSize)
		{
		case TypeInfo::BlobPointerSize::Size16:
		{
			t_ErrorHandler.AssertError(t_RelativePosition <= std::numeric_limits<uint16_t>::max(), "Unable to contain offset in this pointer!"); // TODO: More info
			uint16_t* t_Pointer16 = reinterpret_cast<uint16_t*>(t_Pointer);
			*t_Pointer16 = (uint16_t)t_RelativePosition;
			break;
		}

		case TypeInfo::BlobPointerSize::Size32:
		{
			t_ErrorHandler.AssertError(t_RelativePosition <= std::numeric_limits<uint32_t>::max(), "Unable to contain offset in this pointer!"); // TODO: More info
			uint32_t* t_Pointer32 = reinterpret_cast<uint32_t*>(t_Pointer);
			*t_Pointer32 = (uint32_t)t_RelativePosition;
			break;
		}

		case TypeInfo::BlobPointerSize::Size64:
		{
			t_ErrorHandler.AssertError(t_RelativePosition <= std::numeric_limits<uint64_t>::max(), "Unable to contain offset in this pointer!"); // TODO: More info
			uint64_t* t_Pointer32 = reinterpret_cast<uint64_t*>(t_Pointer);
			*t_Pointer32 = (uint64_t)t_RelativePosition;
			break;
		}

		default:
			a_State.Info.GetErrorHandler().AssertFatal(false, "Unable to write relative pointer, because the size is unknown.");
		}
	}

	void OutputBlobGlobalScope(const DataParser::ElementData& a_ElementData, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State, PrivateOutputBlobState::PointerMap::iterator* a_PointerIteratorHint = nullptr)
	{
		auto& t_ErrorHandler = a_State.Info.GetErrorHandler();
		auto* t_Element = a_ElementData.Element;
		t_ErrorHandler.AssertFatal(t_Element, "Unable to output element to blob, element data contained a null element.");

		t_ErrorHandler.AssertFatal((a_ElementData.IsArray && a_PointerIteratorHint != nullptr) || a_ElementData.IsArray == false, "We found an array (%s) without a pointer pointing towards it.", a_ElementData.Element->Name.c_str());
		DataParser::SubData::const_iterator t_ArraySubElement = a_ElementData.SubElements.end();

		// Since we're in global scope, we now write the current location (relatively) to the location of the pointer.
		if (a_PointerIteratorHint)
		{
			// Check if empty array
			bool t_IsEmptyArray = a_ElementData.IsArray && a_ElementData.SubElements.empty() && a_ElementData.Value.GetString().empty();
			if (t_IsEmptyArray == false)
				OutputBlobGlobalScopePointer(a_State, *a_PointerIteratorHint);
		}

		if (a_ElementData.IsArray)
		{
			// If it's an array, we have array info
			auto t_ArrayInfoIndex = a_State.Arrays.find(&a_ElementData);
			if (t_ArrayInfoIndex != a_State.Arrays.end())
			{
				// If we have no elements, it has to be a value.. But only if the array specified it.
				if (a_ElementData.SubElements.empty() && a_ElementData.Value.GetString().empty() == false)
				{
					// TODO: t_ErrorHandler.AssertWarning(Member->PrefersInline, "etc etc");
					const std::string& t_Type = a_ElementData.Element->Name;
					const std::string& t_CompleteValue = a_ElementData.Value.GetString();
					std::vector<std::string> t_ValueArray = OutputBlobSplitArray(t_Type, t_CompleteValue, a_State);
					for (std::string& t_Value : t_ValueArray)
					{
						auto t_Data = DataParser::ElementData::ValueT(t_Value, DataParser::ElementData::ValueT::Type::Unset, false);
						OutputBlobSingleType(t_Type, t_Data, a_State);
					}
				}

				// Go through all the array elements and add them.
				else while (t_ArrayInfoIndex->second.CurrentIndex != a_ElementData.SubElements.size())
				{
					size_t t_BeginOffset = a_State.Data.size();

					auto t_ArrayElementPair = a_ElementData.SubElements.find(std::to_string(t_ArrayInfoIndex->second.CurrentIndex));
					t_ErrorHandler.AssertFatal(t_ArrayElementPair != a_ElementData.SubElements.end(), "Could not find array index %d", t_ArrayInfoIndex->second.CurrentIndex);

					// Between every array, we need to pad, because they need to remain the same sequential size for random access.
					// TODO: Add option for non-random access arrays (just iterating)
					auto t_TempScope = a_State.Scope;
					a_State.Scope = OutputBlobScope::ArrayScope; // Write out each array element
					OutputBlobElementData(*t_ArrayElementPair->second, a_PointerSize, a_State);
					a_State.Scope = t_TempScope;
					size_t t_EndOffset = a_State.Data.size();

					size_t t_Size = t_ArrayElementPair->second->IsPointer == false ? a_State.SerParser.GetMaxPackedSize(*t_Element) : a_State.Info.GetPointerByteSize(a_PointerSize);
					size_t t_PadAmount = (t_EndOffset - t_BeginOffset) - t_Size;
					for (size_t i = 0; i < t_PadAmount; i++)
						a_State.Data.push_back(0xCD);
					if (t_PadAmount != 0)
						OUTPUT_DEBUG_DATA("<padding>", t_PadAmount, DataParser::ElementData::ValueT("", DataParser::ElementData::ValueT::Type::Unset, false));

					t_BeginOffset += t_Size;

					t_ArrayInfoIndex->second.CurrentIndex++;
				}
			}
		}

		// If we're writing out pointers in an array in global scope, we need to explicitly write out these pointers still.
		else if (a_ElementData.IsPointer && a_State.Scope == OutputBlobScope::ArrayScope)
		{
			// It's a null pointer for now, this will contain the data later (Store pointer location)
			PushNullPointer(a_ElementData, a_PointerSize, a_State);
		}

		else switch (t_Element->Type)
		{
		// We're in global scope, meaning that we have to write out structs with their serialisation order
		case TypeInfo::ElementType::StructType:
			OutputBlobWriteStruct(a_ElementData, a_State, a_PointerIteratorHint);
			break;

		// Write enums as uint32_t
		case TypeInfo::ElementType::EnumType:
			OutputBlobWriteEnum(a_ElementData, a_State, t_ErrorHandler);
			return;

		case TypeInfo::ElementType::PlainType:
			OutputBlobSingleType(t_Element->Name, a_ElementData.Value, a_State);
			break;
		}
	}

	void OutputBlobElementData(const DataParser::ElementData& a_ElementData, TypeInfo::BlobPointerSize a_PointerSize, PrivateOutputBlobState& a_State, PrivateOutputBlobState::PointerMap::iterator* a_PointerIteratorHint)
	{
		auto& t_ErrorHandler = a_State.Info.GetErrorHandler();
		switch (a_State.Scope)
		{
		
		// We're inside an element. We can't write structs or arrays, so we make pointers to those.
		case OutputBlobScope::ElementScope:
			OutputBlobElementScope(a_ElementData, a_PointerSize, a_State, a_PointerIteratorHint);
			break;

		// We're in global scope. We can write anything here, although we're expecting a pointer that points to them, or it being the "main struct" (which is at "0")
		// Array scope is the same (but we're writing array elements)
		case OutputBlobScope::GlobalScope:
		case OutputBlobScope::ArrayScope:
			OutputBlobGlobalScope(a_ElementData, a_PointerSize, a_State, a_PointerIteratorHint);
			break;

		default:
			
			break;
		}
	}

	void DataParser::OutputBlobToMemory(OutputBlobOutput& a_Output, const ElementData& a_ElementData, SerialisationParser& a_SerialisationParser)
	{
		PrivateOutputBlobState t_State(*this, a_ElementData, a_Output, m_Info, a_SerialisationParser);

		// Determine pointer size
		TypeInfo::BlobPointerSize t_GlobalPointerSize = TypeInfo::BlobPointerSize::Size16;
		if (a_ElementData.Element->Type == TypeInfo::ElementType::StructType)
			t_GlobalPointerSize = ((TypeInfo::StructElement*)a_ElementData.Element)->PointerSize;

		// Write base element
		PUSH_ELEMENT_STACK_A(t_State, "Root." + a_ElementData.Element->Name);
		OutputBlobElementData(a_ElementData, t_GlobalPointerSize, t_State);
		POP_ELEMENT_STACK_A(t_State);

		while (t_State.Pointers.size() != 0)
		{
			// We need to copy the existing current pointer set over because new ones are being added to t_State.Pointers.
			PrivateOutputBlobState::PointerMap t_Pointers = t_State.Pointers;
			t_State.Pointers.clear();

			size_t t_BeginOffset = t_State.Data.size();

			for (auto t_Pointer = t_Pointers.begin(); t_Pointer != t_Pointers.end(); t_Pointer++)
			{
				PUSH_ELEMENT_STACK_A(t_State, "Pointer." + t_Pointer->first->Element->Name);
				OutputBlobElementData(*t_Pointer->first, t_Pointer->second.PointerSize, t_State, &t_Pointer); // Might add new pointers to t_State.Pointers
				POP_ELEMENT_STACK_A(t_State);
			}
		}

	#if ASDF_DEBUG_DATA_PARSING
		m_ErrorHandler.AssertFatal(t_State.DebugData.ElementStackIsEmpty(), "Expected element stack to be 0 after formatting data for '%s'!", a_ElementData.Element->Name.c_str());
		t_State.DebugData.GenerateJSON();
	#endif
	}

	void DataParser::OutputBlob(std::string_view a_OutputFile, const ElementData& a_ElementData, SerialisationParser& a_SerialisationParser)
	{
		OutputBlobOutput t_State;
		OutputBlobToMemory(t_State, a_ElementData, a_SerialisationParser);

		fs::path t_OutputPath(a_OutputFile);
		fs::create_directories(t_OutputPath.parent_path());

		std::ofstream t_OutputFile(a_OutputFile.data(), std::ios::binary);
		t_OutputFile.write(reinterpret_cast<const char*>(t_State.Data.data()), t_State.Data.size());
		t_OutputFile.close();

	#if ASDF_DEBUG_DATA_PARSING
		t_OutputPath = fs::path(std::string(a_OutputFile) + ".debug.json");
		std::ofstream t_DebugFile(t_OutputPath, std::ios::binary);
		t_DebugFile.write(t_State.DebugData.data(), t_State.DebugData.size());
	#endif
	}

	DataParser::ElementData DataParser::ElementData::Copy() const
	{
		ElementData t_Data;
		t_Data.Element = Element;
		t_Data.IsArray = IsArray;
		t_Data.IsPointer = IsPointer;
		t_Data.Value = Value;

		for (auto& t_SubData : SubElements)
		{
			ElementData t_Copy = t_SubData.second->Copy();
			t_Data.SubElements[t_SubData.first] = MakeElementData(t_Copy);
		}

		return t_Data;
	}
	
	void DataParser::ElementData::AddToArray(const ElementData& a_DataToCopy)
	{
		if (IsArray == false)
			return;

		std::string t_Index = std::to_string(SubElements.size());
		SubElements[t_Index] = MakeElementData(a_DataToCopy);
		SubElements[t_Index]->IsPointer = IsPointer; // Make sure the sub-element knows it's supposed to be a pointer
	}
	
	std::shared_ptr<DataParser::ElementData> DataParser::ElementData::GetSubElement(std::string_view a_Name)
	{
auto t_Index = SubElements.find(std::string(a_Name.data()));
return t_Index != SubElements.end() ? t_Index->second : nullptr;
	}

#if ASDF_DEBUG_DATA_PARSING
	void OutputOldDebugJSONString(std::string& a_Target, const DataParser::ElementData& a_Root, std::string_view a_NameSuggestion = "", std::string a_TabCount = "")
	{
		a_Target += a_TabCount + " " + a_Root.Element->Name;
		a_Target += (a_NameSuggestion.empty() ? " \\\"Root\\\"" : std::string(" \\\"") + a_NameSuggestion.data() + "\\\"");

		if (a_Root.IsArray)
			a_Target += " (Array)";
		else if (a_Root.IsPointer)
			a_Target += " (Pointer)";

		a_Target += "\\n";

		for (auto& t_Child : a_Root.SubElements)
		{
			if (t_Child.second->SubElements.empty() == false)
				OutputOldDebugJSONString(a_Target, *t_Child.second, t_Child.first, a_TabCount + " |");
			else
				a_Target += a_TabCount + " | " + t_Child.second->Element->Name + " \\\"" + t_Child.first + "\\\" (Value = \\\"" + t_Child.second->Value.GetString() + "\\\")\\n";
		}

		if (a_TabCount.empty())
			a_Target += "\\n\\n";
	}

	void PrivateDebugData::GenerateInputJSON(const DataParser::ElementData& a_Root)
	{
		// If char array, handle as string
		if (a_Root.IsArray && a_Root.Value.GetString().empty() == false)
		{
			m_OutputTarget += "\"" + a_Root.Value.GetString() + "\"";
			return;
		}

		// If string, value might be set directly on the element struct.
		else if (a_Root.Element->Name == "String")
		{
			if (a_Root.Value.GetString().empty() == false)
			{
				m_OutputTarget += "\"" + a_Root.Value.GetString() + "\"";
				return;
			}
			else // if empty string
			{
				auto t_ContentIndex = a_Root.SubElements.find("Content");
				if (t_ContentIndex == a_Root.SubElements.end()) // Content is missing
				{
					m_OutputTarget += "\"\"";
					return;
				}

				// Or size is 0
				auto t_SizeIndex = a_Root.SubElements.find("Size");
				if (t_SizeIndex != a_Root.SubElements.end() && t_SizeIndex->second->Value.GetDouble() == 0)
				{
					m_OutputTarget += "\"\"";
					return;
				}
			}
		}

		if (a_Root.IsArray)
			m_OutputTarget += "[";

		size_t t_ElementCount = a_Root.IsArray ? a_Root.SubElements.size() : 1;
		bool t_FirstArrayElement = true;

		for (size_t i = 0; i < t_ElementCount; i++)
		{
			if (t_FirstArrayElement)
				t_FirstArrayElement = false;
			else
				m_OutputTarget += ",";

			const DataParser::ElementData* t_Data = &a_Root;
			if (a_Root.IsArray)
			{
				auto t_ElemLocation = a_Root.SubElements.find(std::to_string(i));
				t_Data = t_ElemLocation->second.get();
			}

			switch (t_Data->Element->Type)
			{

			case TypeInfo::ElementType::StructType:
			{
				m_OutputTarget += "{";

				auto t_Struct = (const TypeInfo::StructElement*)t_Data->Element;
				bool t_FirstStructMember = true;
				for (auto& t_Member : t_Struct->Members)
				{
					// Exclude members not explicitly set.
					auto t_ValueLocation = t_Data->SubElements.find(t_Member.VariableName);
					if (t_ValueLocation != t_Data->SubElements.end())
					{
						auto t_Element = m_State.Info.GetElement(t_Member.Type);
						if (t_Element->Type == TypeInfo::ElementType::EnumType || t_Element->Type == TypeInfo::ElementType::PlainType)
						{
							if (!t_ValueLocation->second->Value.IsExplicitlySet())
								continue;
						}
					}

					if (t_FirstStructMember == false)
						m_OutputTarget += ", ";
					else
						t_FirstStructMember = false;

					m_OutputTarget += "\"" + t_Member.VariableName + "\": ";

					if (t_ValueLocation != t_Data->SubElements.end())
						GenerateInputJSON(*t_ValueLocation->second);
					else if (t_Member.IsArray)
						m_OutputTarget += "[]";
					else
					{
						auto t_Element = m_State.Info.GetElement(t_Member.Type);
						if (t_Element->Type == TypeInfo::ElementType::StructType)
							GenerateInputJSON(m_DataParser.ParseStruct((const TypeInfo::StructElement&)*t_Element));
						else
							__debugbreak();
					}
				}

				m_OutputTarget += "}";
				break;
			}

			case TypeInfo::ElementType::PlainType:
			case TypeInfo::ElementType::EnumType:
			{
				if (t_Data->Value.HasNativeValue())
					m_OutputTarget += t_Data->Value.GetString();
				else
				{
					std::string t_String = t_Data->Value.GetString();
					size_t t_StringLen = t_String.size();

					// Either it's longer than 1 (not a single character) or alphanumerical
					if (t_StringLen > 1 ||
						(t_String[0] >= 'A' && t_String[0] <= 'Z') ||
						(t_String[0] >= 'a' && t_String[0] <= 'z') ||
						(t_String[0] >= '0' && t_String[0] <= '9'))
						m_OutputTarget += "\"" + t_String + "\"";
					else if (t_StringLen == 1 || t_StringLen)
						m_OutputTarget += std::to_string((uint32_t)t_String[0]); // output the character as number
				}
				break;
			}

			}
		}

		if (a_Root.IsArray)
			m_OutputTarget += "]";
	}

	void PrivateDebugData::GenerateJSON()
	{
		m_OutputTarget = "{";

		m_OutputTarget += "\"input\": ";

		GenerateInputJSON(m_Input);

		m_OutputTarget += ", \"written_data\": [";

		bool t_FirstStructMember = true;
		for (auto& t_WrittenData : m_WrittenData)
		{
			if (t_FirstStructMember == false)
				m_OutputTarget += ", ";
			else
				t_FirstStructMember = false;

			m_OutputTarget += "{";

			m_OutputTarget += "\"type\": \"" + t_WrittenData.Type + "\", ";
			m_OutputTarget += "\"size\": " + std::to_string(t_WrittenData.ByteSize) + ", ";
			m_OutputTarget += "\"stack\": \"" + t_WrittenData.ElementStackEntry + "\", ";

			if (t_WrittenData.Type == "char")
			{
				std::string t_String = t_WrittenData.Value.GetString();
				if ((t_String[0] >= 'A' && t_String[0] <= 'Z') ||
					(t_String[0] >= 'a' && t_String[0] <= 'z') ||
					(t_String[0] >= '0' && t_String[0] <= '9'))
					m_OutputTarget += "\"value\": \"" + t_String + "\"";
				else
					m_OutputTarget += "\"value\": " + std::to_string((uint32_t)(t_WrittenData.Value.GetChar()));
			}
			else
			{
				m_OutputTarget += "\"value\": \"" + t_WrittenData.Value.GetString() + "\"";
			}

			m_OutputTarget += "}";
		}

		m_OutputTarget += "], \"format_string\": \"";

		OutputOldDebugJSONString(m_OutputTarget, m_Input);

		m_OutputTarget += "\"}";
	}
#endif
}
