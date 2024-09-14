#pragma once

// Automatically generated ASDF file
// Editing this is futile and lame
// edit struct_header.mst instead


#include <cstdint>

namespace Skerm
{
	namespace ASDF
	{
	#pragma pack(push, 1)
		enum UIType : uint32_t
		{
			ContainerType,
			ImageType,
			TextType
		};
		using UITypePacked = UIType;
	#pragma pack(pop)
	}
}
