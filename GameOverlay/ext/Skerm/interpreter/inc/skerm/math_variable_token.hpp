#pragma once

// Automatically generated ASDF file
// Editing this is futile and lame
// edit struct_header.mst instead

#include <skerm/math_token_type.hpp>
#include <skerm/tokens.hpp>
#include <vector>
#include <skerm/base_types.hpp>

namespace Skerm
{
	namespace ASDF
	{
	#pragma pack(push, 1)
		struct MathVariableTokenPacked
		{
			

			MathTokenTypePacked Type;
			double Value = 1.0;
			StringPacked Variable;
		};
	#pragma pack(pop)

		using MathVariableToken = Skerm::ValueVariableToken;
	}
}
