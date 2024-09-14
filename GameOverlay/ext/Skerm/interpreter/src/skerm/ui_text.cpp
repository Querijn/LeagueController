#include <skerm/asdf_interpreter.hpp>
#include <skerm/ui_text.hpp>

namespace Skerm
{
	namespace ASDF
	{
		UIText::UIText() :
			Content(),
			FontFile(),
			FontSize()
		{}

		UIText::UIText(const UITextPacked& a_Packed)
		{
			Convert(Type, a_Packed.Type);
			Convert(Left, a_Packed.Left);
			Convert(Right, a_Packed.Right);
			Convert(Top, a_Packed.Top);
			Convert(Bottom, a_Packed.Bottom);
			Convert(Color, a_Packed.Color);
			Convert(Anchor, a_Packed.Anchor);
			Convert(Name, a_Packed.Name);
			Convert(ChildCount, a_Packed.ChildCount);
			Children.resize(a_Packed.ChildCount);
			for (uint32_t i = 0; i < a_Packed.ChildCount; i++)
				Convert(Children[i], a_Packed.Children[i]);
			Convert(Content, a_Packed.Content);
			Convert(FontFile, a_Packed.FontFile);
			Convert(FontSize, a_Packed.FontSize);
		}

		UIText::UIText(const UIText& a_Copy)
		{
			Type = a_Copy.Type;
			Left = a_Copy.Left;
			Right = a_Copy.Right;
			Top = a_Copy.Top;
			Bottom = a_Copy.Bottom;
			Color = a_Copy.Color;
			Anchor = a_Copy.Anchor;
			Name = a_Copy.Name;
			ChildCount = a_Copy.ChildCount;
			Children = a_Copy.Children;
			Content = a_Copy.Content;
			FontFile = a_Copy.FontFile;
			FontSize = a_Copy.FontSize;
		}

		UIText& UIText::operator=(const UIText& a_Copy)
		{
			Type = a_Copy.Type;
			Left = a_Copy.Left;
			Right = a_Copy.Right;
			Top = a_Copy.Top;
			Bottom = a_Copy.Bottom;
			Color = a_Copy.Color;
			Anchor = a_Copy.Anchor;
			Name = a_Copy.Name;
			ChildCount = a_Copy.ChildCount;
			Children = a_Copy.Children;
			Content = a_Copy.Content;
			FontFile = a_Copy.FontFile;
			FontSize = a_Copy.FontSize;

			return *this;
		}

		void Convert(UIText& a_Target, const UITextPacked& a_Other)
		{
			Convert(a_Target.Type, a_Other.Type);
			Convert(a_Target.Left, a_Other.Left);
			Convert(a_Target.Right, a_Other.Right);
			Convert(a_Target.Top, a_Other.Top);
			Convert(a_Target.Bottom, a_Other.Bottom);
			Convert(a_Target.Color, a_Other.Color);
			Convert(a_Target.Anchor, a_Other.Anchor);
			Convert(a_Target.Name, a_Other.Name);
			Convert(a_Target.ChildCount, a_Other.ChildCount);
			a_Target.Children.resize(a_Other.ChildCount);
			for (uint32_t i = 0; i < a_Other.ChildCount; i++)
				Convert(a_Target.Children[i], a_Other.Children[i]);
			Convert(a_Target.Content, a_Other.Content);
			Convert(a_Target.FontFile, a_Other.FontFile);
			Convert(a_Target.FontSize, a_Other.FontSize);
		}
	}
}
