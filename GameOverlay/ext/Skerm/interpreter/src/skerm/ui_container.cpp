#include <skerm/asdf_interpreter.hpp>
#include <skerm/ui_container.hpp>

namespace Skerm
{
	namespace ASDF
	{
		UIContainer::UIContainer() :
			Type(),
			Left(),
			Right(),
			Top(),
			Bottom(),
			Color(),
			Anchor(),
			Name(),
			ChildCount(),
			Children()
		{}

		UIContainer::UIContainer(const UIContainerPacked& a_Packed)
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
		}

		UIContainer::UIContainer(const UIContainer& a_Copy)
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
		}

		UIContainer& UIContainer::operator=(const UIContainer& a_Copy)
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

			return *this;
		}

		void Convert(UIContainer& a_Target, const UIContainerPacked& a_Other)
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
		}
	}
}
