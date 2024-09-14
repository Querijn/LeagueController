#include <skerm/asdf_interpreter.hpp>
#include <skerm/ui_color.hpp>

namespace Skerm
{
	namespace ASDF
	{
		UIColor::UIColor() :
			R(),
			G(),
			B(),
			A()
		{}

		UIColor::UIColor(const UIColorPacked& inPacked)
		{
			Convert(R, inPacked.R);
			Convert(G, inPacked.G);
			Convert(B, inPacked.B);
			Convert(A, inPacked.A);
		}

		UIColor::UIColor(const UIColor& inCopy)
		{
			R = inCopy.R;
			G = inCopy.G;
			B = inCopy.B;
			A = inCopy.A;
		}

		UIColor& UIColor::operator=(const UIColor& inCopy)
		{
			R = inCopy.R;
			G = inCopy.G;
			B = inCopy.B;
			A = inCopy.A;

			return *this;
		}

		void Convert(UIColor& inTarget, const UIColorPacked& inOther)
		{
			Convert(inTarget.R, inOther.R);
			Convert(inTarget.G, inOther.G);
			Convert(inTarget.B, inOther.B);
			Convert(inTarget.A, inOther.A);
		}
	}
}
