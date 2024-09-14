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
		enum MathTokenType : uint32_t
		{
			MultiplicationTokenType,
			DivisionTokenType,
			AdditionTokenType,
			SubtractionTokenType,
			ValueVariableTokenType,
			EquationTokenType,
			StructTokenType
		};
		using MathTokenTypePacked = MathTokenType;
	#pragma pack(pop)
	}
}
