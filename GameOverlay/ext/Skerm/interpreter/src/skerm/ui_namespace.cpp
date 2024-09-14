#include <skerm/asdf_interpreter.hpp>
#include <skerm/ui_namespace.hpp>

namespace Skerm
{
	namespace ASDF
	{
		UINamespace::UINamespace() :
			Name(),
			Count(),
			Elements()
		{}

		UINamespace::UINamespace(const UINamespacePacked& a_Packed)
		{
			Convert(Name, a_Packed.Name);
			Convert(Count, a_Packed.Count);
			Elements.resize(a_Packed.Count);
			for (uint32_t i = 0; i < a_Packed.Count; i++)
				Convert(Elements[i], a_Packed.Elements[i]);
		}

		UINamespace::UINamespace(const UINamespace& a_Copy)
		{
			Name = a_Copy.Name;
			Count = a_Copy.Count;
			Elements = a_Copy.Elements;
		}

		UINamespace& UINamespace::operator=(const UINamespace& a_Copy)
		{
			Name = a_Copy.Name;
			Count = a_Copy.Count;
			Elements = a_Copy.Elements;

			return *this;
		}

		void Convert(UINamespace& a_Target, const UINamespacePacked& a_Other)
		{
			Convert(a_Target.Name, a_Other.Name);
			Convert(a_Target.Count, a_Other.Count);
			a_Target.Elements.resize(a_Other.Count);
			for (uint32_t i = 0; i < a_Other.Count; i++)
				Convert(a_Target.Elements[i], a_Other.Elements[i]);
		}
	}
}
