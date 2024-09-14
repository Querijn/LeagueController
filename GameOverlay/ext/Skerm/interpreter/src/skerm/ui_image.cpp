#include <skerm/asdf_interpreter.hpp>
#include <skerm/ui_image.hpp>

namespace Skerm
{
	namespace ASDF
	{
		UIImage::UIImage() :
			Source(),
			Sprite()
		{}

		UIImage::UIImage(const UIImagePacked& a_Packed)
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
			Convert(Source, a_Packed.Source);
			Convert(Sprite, a_Packed.Sprite);
		}

		UIImage::UIImage(const UIImage& a_Copy)
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
			Source = a_Copy.Source;
			Sprite = a_Copy.Sprite;
		}

		UIImage& UIImage::operator=(const UIImage& a_Copy)
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
			Source = a_Copy.Source;
			Sprite = a_Copy.Sprite;

			return *this;
		}

		void Convert(UIImage& a_Target, const UIImagePacked& a_Other)
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
			Convert(a_Target.Source, a_Other.Source);
			Convert(a_Target.Sprite, a_Other.Sprite);
		}
	}
}
