#pragma once

#include <asdf/type_info.hpp>
#include <asdf/schema_parser.hpp>
#include <asdf/serialisation_parser.hpp>
#include <asdf/data_parser.hpp>
#include <asdf/error_handling.hpp>

namespace ASDF
{
	struct Compiler
	{
		Compiler();

		ErrorHandler Errors;
		TypeInfo Info;
		SchemaParser Schema;
		DataParser Data;
		SerialisationParser Serialisation;
	};
}