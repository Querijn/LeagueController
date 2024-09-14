cmake_minimum_required(VERSION 3.13)
project(LeagueInternals)

set(LEAHACK_SRC "" CACHE INTERNAL "LeagueInternals source files")

ADD_SRC(LEAHACK_SRC		"GameObject\\"			"inc/league_internals/game_object.hpp"			"src/game_object.cpp")
ADD_SRC(LEAHACK_SRC		"FlagDuo\\"				"inc/league_internals/flag_duo.hpp"				"src/flag_duo.cpp")
ADD_SRC(LEAHACK_SRC		"Offsets\\"				"inc/league_internals/offsets.hpp"				"src/offsets.cpp")
ADD_SRC(LEAHACK_SRC		"Task\\"				"inc/league_internals/task_base.hpp"			"src/task_base.cpp")
ADD_SRC(LEAHACK_SRC		"Offsets\\"				"inc/league_internals/offset_loader.hpp"		"src/offset_loader.cpp")

ADD_SRC(LEAHACK_SRC		"Task\\HudManager"		"inc/league_internals/task_hud_manager.hpp"		"src/task_hud_manager.cpp")
ADD_SRC(LEAHACK_SRC		"Task\\RenderFlag"		"inc/league_internals/task_rendering_flag.hpp"	"src/task_rendering_flag.cpp")
ADD_SRC(LEAHACK_SRC		"Task\\SpellBook"		"inc/league_internals/task_spellbook.hpp"		"src/task_spellbook.cpp")
ADD_SRC(LEAHACK_SRC		"Task\\GameObject"		"inc/league_internals/task_gameobject.hpp"		"src/task_gameobject.cpp")

add_library(LeagueInternals STATIC ${LEAHACK_SRC})
target_include_directories(LeagueInternals PUBLIC "inc")
target_include_directories(LeagueInternals PRIVATE "src")
target_link_libraries(LeagueInternals LeaconCommon MemoryHelper minhook GameOverlay LeaconIPC)

set_target_properties(LeagueInternals PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF)
if (WIN32)
	target_compile_definitions(LeagueInternals PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(LeagueInternals PUBLIC NOMINMAX)
endif ()