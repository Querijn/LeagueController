#pragma once

#include <string_view>

namespace LeagueController
{
	enum class ControllerInput
	{
#define DEFINE_ENUM(a, b, c) a = b,
#define DEFINE_ENUM_ALT(a, b, c) a = b,
#define DEFINE_ENUM_INFO(a, b) a = b,
#include <league_controller/controller_input_definition.hpp>
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
	};

	const char* GetControllerInputName(ControllerInput a_Input);
	ControllerInput* GetControllerInputList();
	const char** GetControllerInputNameList();
	ControllerInput GetControllerInputByName(std::string_view a_Name);
	size_t GetControllerInputCount();
	size_t GetControllerInputUniqueCount();
}