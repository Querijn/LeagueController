#pragma once

// Automatically generated ASDF file
// Editing this is futile and lame
// edit struct_header.mst instead

#include <skerm/math_token_type.hpp>
#include <skerm/tokens.hpp>
#include <skerm/math_base_token.hpp>
#include <vector>
#include <skerm/base_types.hpp>

namespace Skerm
{
	namespace ASDF
	{
	#pragma pack(push, 1)
		struct MathEquationTokenPacked
		{
			using TokensPtr = ASDF::RelPtr<MathBaseTokenPacked, uint16_t, 2>;

			MathTokenTypePacked Type;
			uint8_t TokenCount;
			ASDF::Array<TokensPtr> Tokens;
		};
	#pragma pack(pop)

		using MathEquationToken = Skerm::EquationToken;
	}
}
