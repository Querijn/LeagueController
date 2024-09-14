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
		struct UITextPacked
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
			StringPacked Content;
			StringPacked FontFile;
			MathEquationTokenPacked FontSize;
		};
	#pragma pack(pop)

		struct UIText : public UIContainer
		{
			UIText();
			UIText(const UITextPacked& a_Packed);
			UIText(const UIText& a_Copy);
			UIText& operator=(const UIText& a_Copy);

			std::string Content;
			std::string FontFile;
			Skerm::EquationToken FontSize;
		};
		
		void Convert(UIText& a_Target, const UITextPacked& a_Other);
	}
}
