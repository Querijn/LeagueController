#include <league_controller/controller_input.hpp>

namespace LeagueController
{
	const char* GetControllerInputName(ControllerInput a_Input)
	{
		switch (a_Input)
		{
#define DEFINE_ENUM(a, b, c) case ControllerInput::a: return #c;
#define DEFINE_ENUM_ALT(a, b, c)
#define DEFINE_ENUM_INFO(a, b)
#include <league_controller/controller_input_definition.hpp>
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		}

		return "Unknown";
	}

	ControllerInput* GetControllerInputList()
	{
		static ControllerInput t_List[] =
		{
#define DEFINE_ENUM(a, b, c) ControllerInput::a,
#define DEFINE_ENUM_ALT(a, b, c)
#define DEFINE_ENUM_INFO(a, b)
#include <league_controller/controller_input_definition.hpp>
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		};

		return t_List;
	}

	const char** GetControllerInputNameList()
	{
		static const char* t_List[] =
		{
#define DEFINE_ENUM(a, b, c) #c,
#define DEFINE_ENUM_ALT(a, b, c)
#define DEFINE_ENUM_INFO(a, b)
#include <league_controller/controller_input_definition.hpp>
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		};

		return t_List;
	}

	// TODO: Also make switch hash alternative?
	ControllerInput GetControllerInputByName(std::string_view a_Name)
	{
#define DEFINE_ENUM(a, b, c) if (#c == a_Name) return ControllerInput::a;
#define DEFINE_ENUM_ALT(a, b, c)
#define DEFINE_ENUM_INFO(a, b)
#include <league_controller/controller_input_definition.hpp>
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO

		return ControllerInput::Max;
	}

	size_t GetControllerInputUniqueCount()
	{
		return 0
#define DEFINE_ENUM(a, b, c) + 1
#define DEFINE_ENUM_ALT(a, b, c)
#define DEFINE_ENUM_INFO(a, b)

#include <league_controller/controller_input_definition.hpp>

#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		;
	}

	size_t GetControllerInputCount()
	{
		return (size_t)ControllerInput::Max;
	}
}