#pragma once

#include <asdf/compiler.hpp>

namespace ASDF
{
	Compiler::Compiler() :
		Errors(),
		Info(Errors),
		Schema(Info),
		Data(Info),
		Serialisation(Info)
	{
	}
}