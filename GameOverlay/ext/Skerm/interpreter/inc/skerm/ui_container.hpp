#pragma once

// Automatically generated ASDF file
// Editing this is futile and lame
// edit struct_header.mst instead

#include <skerm/math_equation_token.hpp>
#include <skerm/ui_anchor.hpp>
#include <skerm/ui_type.hpp>
#include <skerm/ui_container.hpp>
#include <vector>
#include <skerm/base_types.hpp>

namespace Skerm
{
	namespace ASDF
	{
	#pragma pack(push, 1)
		struct UIContainerPacked
		{
			using ChildrenPtr = ASDF::RelPtr<UIContainerPacked, uint16_t, 2>;

			UITypePacked Type;
			MathEquationTokenPacked Left;
			MathEquationTokenPacked Right;
			MathEquationTokenPacked Top;
			MathEquationTokenPacked Bottom;
			MathEquationTokenPacked Color;
			UIAnchorPacked Anchor;
			StringPacked Name;
			uint32_t ChildCount;
			ASDF::Array<ChildrenPtr> Children;
		};
	#pragma pack(pop)

		struct UIContainer
		{
			UIContainer();
			UIContainer(const UIContainerPacked& a_Packed);
			UIContainer(const UIContainer& a_Copy);
			UIContainer& operator=(const UIContainer& a_Copy);

			UIType Type;
			Skerm::EquationToken Left;
			Skerm::EquationToken Right;
			Skerm::EquationToken Top;
			Skerm::EquationToken Bottom;
			Skerm::EquationToken Color;
			UIAnchor Anchor;
			std::string Name;
			uint32_t ChildCount;
			std::vector<std::shared_ptr<UIContainer>> Children;
		};
		
		void Convert(UIContainer& a_Target, const UIContainerPacked& a_Other);
	}
}
