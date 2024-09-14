#pragma once

// Automatically generated ASDF file
// Editing this is futile and lame
// edit struct_header.mst instead

#include <vector>
#include <skerm/base_types.hpp>

namespace Skerm
{
	namespace ASDF
	{
	#pragma pack(push, 1)
		struct UIColorPacked
		{
			

			float R = 1;
			float G = 1;
			float B = 1;
			float A = 1;
		};
	#pragma pack(pop)

		struct UIColor
		{
			UIColor();
			UIColor(const UIColorPacked& inPacked);
			UIColor(const UIColor& inCopy);
			UIColor& operator=(const UIColor& inCopy);

			float R = 1;
			float G = 1;
			float B = 1;
			float A = 1;
		};
		
		void Convert(UIColor& inTarget, const UIColorPacked& inOther);
	}
}
