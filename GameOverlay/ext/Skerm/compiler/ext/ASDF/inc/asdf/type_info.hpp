#pragma once

#include <asdf/error_handling.hpp>

#include <string_view>
#include <map>
#include <vector>

namespace ASDF
{
	struct ResourceVersion
	{
		static const uint8_t Major = 0;
		static const uint8_t Minor = 1;
		static const uint8_t Patch = 0;
	};

	class TypeInfo
	{
	public:
		TypeInfo(ErrorHandler& a_ErrorHandler);
		~TypeInfo();

		void SetIncludeDirectory(std::string_view a_IncludeDirectory);

		enum class ElementType
		{
			StructType = 0,
			EnumType = 1,
			PlainType = 2
		};

		enum class BlobPointerSize
		{
			Size16,
			Size32,
			Size64
		};

		struct BaseElement
		{
			BaseElement(std::string_view a_Name, ElementType a_Type, std::string_view a_DefaultValue) :
				Name(a_Name), Type(a_Type), DefaultValue(a_DefaultValue), IsBaseASDF(a_Type == ElementType::PlainType)
			{}
			virtual ~BaseElement() {}

			std::string Name;
			ElementType Type;

			std::string HeaderName;
			std::string DefaultValue;

			std::map<std::string, std::string> TypeArguments;

			bool IsBaseASDF;
		};

		struct StructElement : public BaseElement
		{
			struct Member
			{
				std::string Type;
				std::string VariableName;
				std::string DefaultValue;

				bool IsArray = false;
				bool IsPointer = false;
				bool PrefersInline = false;
				bool CameFromParent = false;
				std::string ArrayCountForMember;

				const StructElement* Parent = nullptr;
			};

			StructElement(std::string_view a_Name, std::vector<Member>&& a_Members) :
				BaseElement(a_Name, ElementType::StructType, ""),
				Members(a_Members)
			{
				for (auto& t_Member : Members)
					t_Member.Parent = this;
			}

			std::vector<Member> Members;

			BlobPointerSize PointerSize = BlobPointerSize::Size16;
			const StructElement* Parent = nullptr;
			std::vector<const StructElement*> Extensions;

			const Member* GetMember(std::string_view a_Name) const;
		};

		struct EnumElement : public BaseElement
		{
			struct Value
			{
				std::string ValueName;
				int64_t RepresentingNumber;
			};

			EnumElement(std::string_view a_Name, std::vector<Value>&& a_Values, std::string_view a_DefaultValue = "") :
				BaseElement(a_Name, ElementType::EnumType, a_DefaultValue),
				Values(a_Values)
			{}

			std::vector<Value> Values;
		};

		struct PlainElement : public BaseElement
		{
			PlainElement(std::string_view a_Name, std::string_view a_DefaultValue) :
				BaseElement(a_Name, ElementType::PlainType, a_DefaultValue)
			{}
		};

		const PlainElement& AddPlain(std::string_view a_Name, std::string_view a_DefaultValueDenotation, std::string_view a_HeaderName = "");
		const EnumElement& AddEnum(std::string_view a_Name, std::vector<EnumElement::Value>&& a_Values, std::map<std::string, std::string>&& a_ArgMap, std::string_view a_DefaultValueDenotation = "");
		const StructElement& AddStruct(std::string_view a_Name, std::vector<StructElement::Member>&& a_Members, std::map<std::string, std::string>&& a_ArgMap, BlobPointerSize a_PointerSize, const TypeInfo::StructElement* a_Parent = nullptr);

		const BaseElement* GetElement(std::string_view a_Type) const;
		std::string GetElementFile(const BaseElement& a_Element, std::string_view a_Extension = ".hpp") const;
		std::string GetElementFile(std::string_view a_Type, std::string_view a_Extension = ".hpp") const;
		std::string GetPointerTypeName(TypeInfo::BlobPointerSize a_PointerSize) const;
		size_t GetPointerByteSize(TypeInfo::BlobPointerSize a_PointerSize) const;

		ErrorHandler& GetErrorHandler() const { return m_ErrorHandler; }

	private:
		ErrorHandler& m_ErrorHandler;
		std::map<std::string, BaseElement*> m_Elements;
		std::string m_IncludeDirectory;
	};
}