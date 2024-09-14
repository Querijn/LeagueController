#pragma once

#include <asdf/type_info.hpp>

#include <vector>
#include <map>

namespace ASDF
{
	class SerialisationParser
	{
	public:
		SerialisationParser(TypeInfo& a_Info);

		struct SerialisationItem
		{
			const TypeInfo::BaseElement* Element = nullptr;
			const TypeInfo::StructElement::Member* Member = nullptr;
		};

		const std::vector<SerialisationItem>& GetSerialisationOrder(const TypeInfo::StructElement& a_Element);
		size_t GetOwnPackedSize(const TypeInfo::BaseElement& a_Element) const;
		size_t GetMaxPackedSize(const TypeInfo::BaseElement& a_Element) const;

		size_t GetOwnPackedSize(const TypeInfo::StructElement& a_Element) const;
		size_t GetMaxPackedSize(const TypeInfo::StructElement& a_Element) const;

		size_t GetOwnPackedSize(const TypeInfo::EnumElement& a_Element) const;
		size_t GetMaxPackedSize(const TypeInfo::EnumElement& a_Element) const;

		size_t GetOwnPackedSize(const TypeInfo::PlainElement& a_Element) const;
		size_t GetMaxPackedSize(const TypeInfo::PlainElement& a_Element) const;

	private:
		ErrorHandler& m_ErrorHandler;
		TypeInfo& m_Info;

		std::map<const TypeInfo::StructElement*, std::vector<SerialisationItem>> m_Orders;

		void HandleElement(std::vector<SerialisationParser::SerialisationItem>& t_Items, const TypeInfo::BaseElement* a_Element, const TypeInfo::StructElement::Member* a_Member, std::string_view a_HintType, bool a_ExpectingArrayCount = false);
	};
}