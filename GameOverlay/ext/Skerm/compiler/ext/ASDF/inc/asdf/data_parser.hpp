#pragma once

#include <asdf/type_info.hpp>

#include <string>
#include <map>
#include <memory>

namespace ASDF
{
	class SerialisationParser;
#if !ASDF_DEBUG_DATA_PARSING
	struct MapIndex
	{
		MapIndex() : CRC(0) {}
		MapIndex(const std::string& a_Index);

		uint32_t CRC;
	};

	struct MapIndexCompare
	{
		bool operator()(const MapIndex& a, const MapIndex& b) const
		{
			return a.CRC < b.CRC;
		}
	};
#endif

	class DataParser
	{
	public:
		DataParser(TypeInfo& a_Info);

		struct ElementData;

#if defined(ASDF_DEBUG_DATA_PARSING)
		using SubData = std::map<std::string, std::shared_ptr<ElementData>>;
#else
		using SubData = std::map<const MapIndex, std::shared_ptr<ElementData>, MapIndexCompare>;
#endif
		struct ElementData
		{
			ElementData Copy() const;

			// TODO: Rework this into a std variant
			class ValueT
			{
			public:
				enum class Type : uint8_t
				{
					Unset,
					Float,
					Double
				};

				ValueT() :
					m_Value(""), m_Type(Type::Unset), m_IsStringSet(false), m_HasNativeValue(false), m_ExplicitlySet(false)
				{}

				ValueT(std::string_view a_ValueAsString, Type a_Type, bool a_IsArray) :
					m_Value(a_ValueAsString), m_Type(a_Type), m_IsStringSet(true), m_HasNativeValue(false), m_ExplicitlySet(true)
				{}

				ValueT(float a_Value, Type a_Type, bool a_IsArray) :
					m_FloatValue(a_Value), m_Type(a_Type), m_IsStringSet(false), m_HasNativeValue(true), m_ExplicitlySet(true)
				{}

				ValueT(double a_Value, Type a_Type, bool a_IsArray) :
					m_DoubleValue(a_Value), m_Type(a_Type), m_IsStringSet(false), m_HasNativeValue(true), m_ExplicitlySet(true)
				{}

				ValueT& operator=(double a_Value)
				{
					m_Type = Type::Double;
					m_DoubleValue = a_Value;
					m_IsStringSet = false;
					m_HasNativeValue = true;
					m_ExplicitlySet = true;

					return *this;
				}

				ValueT& operator=(float a_Value)
				{
					m_Type = Type::Float;
					m_FloatValue = a_Value;
					m_IsStringSet = false;
					m_HasNativeValue = true;
					m_ExplicitlySet = true;

					return *this;
				}

				ValueT& operator=(const std::string& a_ValueAsString)
				{
					m_Type = Type::Unset;
					m_Value = a_ValueAsString;
					m_IsStringSet = true;
					m_HasNativeValue = false;
					m_ExplicitlySet = true;

					return *this;
				}

				std::string GetString()
				{
					if (m_IsStringSet)
						return m_Value;

					// TODO: Assert for HasNativeValue
					switch (m_Type)
					{
					case Type::Double:
						m_Value = std::to_string(m_DoubleValue);
						break;
					case Type::Float:
						m_Value = std::to_string(m_FloatValue);
						break;
					default:
						return "";
					}
					m_IsStringSet = true;
					return m_Value;
				}

				std::string GetString() const 
				{
					if (m_IsStringSet)
						return m_Value;

					// TODO: Assert for HasNativeValue
					switch (m_Type)
					{
					case Type::Double:
						return std::to_string(m_DoubleValue);
					case Type::Float:
						return std::to_string(m_FloatValue);
					default:
						return "";
					}
				}
				char GetChar() const { return m_Value[0]; }
				float GetFloat() const { return m_FloatValue; }
				double GetDouble() const { return m_DoubleValue; }

				bool HasString() const { return m_IsStringSet; }
				bool HasNativeValue() const { return m_HasNativeValue; }
				bool IsExplicitlySet() const { return m_ExplicitlySet; }

			private:
				// TODO: union
				std::string m_Value; // 24 (24)

				union
				{
					double m_DoubleValue; // 32 (8)
					float m_FloatValue;
				};

				Type m_Type; // 33
				bool m_IsStringSet : 1;  // 34
				bool m_HasNativeValue : 1; // 35
				bool m_IsArray : 1; // 36
				bool m_ExplicitlySet : 1; // 37
			};

			const TypeInfo::BaseElement* Element = nullptr;
			bool IsArray = false;
			bool IsPointer = false;
			ValueT Value;

			SubData SubElements;

			void AddToArray(const ElementData& a_DataToCopy);
			std::shared_ptr<ElementData> GetSubElement(std::string_view a_Name);
		};

		struct OutputBlobOutput
		{
		#if ASDF_DEBUG_DATA_PARSING
			std::string DebugData;
		#endif

			std::vector<uint8_t> Data;
		};

		ElementData ParseStruct(const ASDF::TypeInfo::StructElement& a_Struct) const;
		ElementData ParsePlain(const ASDF::TypeInfo::PlainElement& a_Plain) const;

		const ElementData ParseXML(std::string_view a_File);
		void OutputBlob(std::string_view a_OutputFile, const ElementData& a_ElementData, SerialisationParser& a_SerialisationParser);
		void OutputBlobToMemory(OutputBlobOutput& a_Output, const ElementData& a_ElementData, SerialisationParser& a_SerialisationParser);

	private:
		ErrorHandler& m_ErrorHandler;
		TypeInfo& m_Info;
	};

#define MakeElementData std::make_shared<DataParser::ElementData>
}