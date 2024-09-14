#include "asdf/serialisation_parser.hpp"

#include "hash_helper.hpp"

namespace ASDF
{
	SerialisationParser::SerialisationParser(TypeInfo& a_Info) :
		m_Info(a_Info), m_ErrorHandler(a_Info.GetErrorHandler())
	{
	}

	void SerialisationParser::HandleElement(std::vector<SerialisationParser::SerialisationItem>& a_Items, const TypeInfo::BaseElement* a_Element, const TypeInfo::StructElement::Member* a_Member, std::string_view a_HintType, bool a_ExpectingArrayCount)
	{
		m_ErrorHandler.AssertError(a_Element, "Serialisation order parsing could not identify type %s.", a_HintType.data());
		if (a_Element == nullptr)
			return;

		SerialisationParser::SerialisationItem t_Item;
		t_Item.Element = a_Element;
		t_Item.Member = a_Member;

		if (a_Member)
		{
			// Don't handle array count members. Do these when handling arrays (so that you can force an order).
			if (a_Member->ArrayCountForMember.empty() == false && a_ExpectingArrayCount == false)
				return;

			if (a_Member->IsArray)
			{
				// Find the array count now
				const TypeInfo::StructElement::Member* t_CountMember = nullptr;
				for (auto& t_Child : a_Member->Parent->Members)
				{
					if (t_Child.ArrayCountForMember == a_Member->VariableName)
					{
						t_CountMember = &t_Child;
						break;
					}
				}

				m_ErrorHandler.AssertWarning(t_CountMember, "Could not find the count member for %s", a_Member->VariableName.c_str());
				if (t_CountMember)
					HandleElement(a_Items, m_Info.GetElement(t_CountMember->Type), t_CountMember, t_CountMember->Type, true);
			}
		}

		switch (a_Element->Type)
		{

			// Make sure members of this struct also have their serialisation order.
		case TypeInfo::ElementType::StructType:
		{
			const TypeInfo::StructElement* t_Struct = static_cast<const TypeInfo::StructElement*>(a_Element);
			GetSerialisationOrder(*t_Struct);
			break;
		}
		}

		a_Items.push_back(t_Item);
	}

	const std::vector<SerialisationParser::SerialisationItem>& SerialisationParser::GetSerialisationOrder(const TypeInfo::StructElement& a_Element)
	{
		// See if it already exists
		auto t_ExistingOrderIndex = m_Orders.find(&a_Element);
		if (t_ExistingOrderIndex != m_Orders.end())
			return t_ExistingOrderIndex->second;

		std::vector<SerialisationItem>& t_Items = m_Orders[&a_Element];

		for (auto& t_Member : a_Element.Members)
			HandleElement(t_Items, m_Info.GetElement(t_Member.Type), &t_Member, t_Member.Type, false);

		// TODO: Randomisation here

		return t_Items;
	}

	size_t SerialisationParser::GetMaxPackedSize(const TypeInfo::BaseElement& a_Element) const
	{
		switch (a_Element.Type)
		{
		case TypeInfo::ElementType::EnumType:
			return GetMaxPackedSize((const TypeInfo::EnumElement&)a_Element);
		case TypeInfo::ElementType::PlainType:
			return GetMaxPackedSize((const TypeInfo::PlainElement&)a_Element);
		case TypeInfo::ElementType::StructType:
			return GetMaxPackedSize((const TypeInfo::StructElement&)a_Element);

		default:
			m_ErrorHandler.AssertFatal(false, "Unable to determine element!");
			return 0;
		}
	}
	
	size_t SerialisationParser::GetOwnPackedSize(const TypeInfo::BaseElement& a_Element) const
	{
		switch (a_Element.Type)
		{
		case TypeInfo::ElementType::EnumType:
			return GetOwnPackedSize((const TypeInfo::EnumElement&)a_Element);
		case TypeInfo::ElementType::PlainType:
			return GetOwnPackedSize((const TypeInfo::PlainElement&)a_Element);
		case TypeInfo::ElementType::StructType:
			return GetOwnPackedSize((const TypeInfo::StructElement&)a_Element);

		default:
			m_ErrorHandler.AssertFatal(false, "Unable to determine element!");
			return 0;
		}
	}

	size_t SerialisationParser::GetOwnPackedSize(const TypeInfo::StructElement& a_Element) const
	{
		size_t t_Size = 0;

		for (auto& t_Member : a_Element.Members)
		{
			auto t_Element = m_Info.GetElement(t_Member.Type);
			m_ErrorHandler.AssertFatal(t_Element, "Unable to find element '%s'! Cannot get the size of %s.%s", t_Member.Type, a_Element.Name, t_Member.VariableName);

			if (t_Member.IsPointer || t_Member.IsArray)
			{
				switch (a_Element.PointerSize)
				{
				case TypeInfo::BlobPointerSize::Size16:
					t_Size += sizeof(uint16_t);
					break;

				case TypeInfo::BlobPointerSize::Size32:
					t_Size += sizeof(uint32_t);
					break;

				case TypeInfo::BlobPointerSize::Size64:
					t_Size += sizeof(uint64_t);
					break;
				}
				continue;
			}

			t_Size += GetOwnPackedSize(*t_Element);
		}

		return t_Size;
	}
	
	size_t SerialisationParser::GetMaxPackedSize(const TypeInfo::StructElement& a_Element) const
	{
		if (a_Element.Parent != nullptr)
			return GetMaxPackedSize(*a_Element.Parent);

		if (a_Element.Extensions.empty())
			return GetOwnPackedSize(a_Element);

		size_t t_Size = 0; // GetOwnPackedSize(a_Element); // Technically should be GetOwnPackedSize(a_Element), but any extension should always be as large or larger.
		for (auto t_Extension : a_Element.Extensions)
		{
			size_t t_ExtensionSize = GetOwnPackedSize(*t_Extension);
			if (t_Size < t_ExtensionSize)
				t_Size = t_ExtensionSize;
		}

		return t_Size;
	}
	
	size_t SerialisationParser::GetOwnPackedSize(const TypeInfo::EnumElement& a_Element) const
	{
		return sizeof(uint32_t); // If this changes, DataParser::OutputBlobWriteEnum needs to reflect this change too!
	}

	size_t SerialisationParser::GetOwnPackedSize(const TypeInfo::PlainElement& a_Element) const
	{
		switch (Hash::CRC32(a_Element.Name))
		{
		case COMPILE_TIME_CRC32_STR("uint8_t"):
		case COMPILE_TIME_CRC32_STR("int8_t"):
		case COMPILE_TIME_CRC32_STR("bool"):
		case COMPILE_TIME_CRC32_STR("char"):
			return sizeof(uint8_t);

		case COMPILE_TIME_CRC32_STR("int16_t"):
		case COMPILE_TIME_CRC32_STR("uint16_t"):
			return sizeof(uint16_t);

		case COMPILE_TIME_CRC32_STR("int32_t"):
		case COMPILE_TIME_CRC32_STR("uint32_t"):
			return sizeof(uint32_t);

		case COMPILE_TIME_CRC32_STR("int64_t"):
		case COMPILE_TIME_CRC32_STR("uint64_t"):
			return sizeof(uint64_t);

		case COMPILE_TIME_CRC32_STR("float"):
			return sizeof(float);

		case COMPILE_TIME_CRC32_STR("double"):
			return sizeof(double);

		default:
			m_ErrorHandler.AssertFatal(false, "Unable to figure out size of plain element %s", a_Element.Name);
			return 0;
		}
	}

	size_t SerialisationParser::GetMaxPackedSize(const TypeInfo::EnumElement& a_Element) const { return GetOwnPackedSize(a_Element); }
	size_t SerialisationParser::GetMaxPackedSize(const TypeInfo::PlainElement& a_Element) const { return GetOwnPackedSize(a_Element); }
}