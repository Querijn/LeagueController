#pragma once

// Automatically generated ASDF file
// Editing this is futile and lame
// edit struct_header.mst instead

#include <skerm/ui_container.hpp>
#include <vector>
#include <skerm/base_types.hpp>

namespace Skerm
{
	namespace ASDF
	{
	#pragma pack(push, 1)
		struct UINamespacePacked
		{
			using ElementsPtr = ASDF::RelPtr<UIContainerPacked, uint16_t, 2>;

			StringPacked Name;
			uint32_t Count;
			ASDF::Array<ElementsPtr> Elements;
		};
	#pragma pack(pop)

		struct UINamespace
		{
			UINamespace();
			UINamespace(const UINamespacePacked& a_Packed);
			UINamespace(const UINamespace& a_Copy);
			UINamespace& operator=(const UINamespace& a_Copy);

			std::string Name;
			uint32_t Count;
			std::vector<std::shared_ptr<UIContainer>> Elements;
		};
		
		void Convert(UINamespace& a_Target, const UINamespacePacked& a_Other);
	}
}
