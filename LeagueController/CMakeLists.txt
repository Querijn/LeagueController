cmake_minimum_required(VERSION 3.13)
project(LeagueController)

set(LEACON_SRC "" CACHE INTERNAL "LeagueController source files")

# LeagueLib
add_subdirectory("ext/leaguelib")
set_target_properties(LeagueLib PROPERTIES FOLDER Libraries)
set_target_properties(SpekUtil PROPERTIES FOLDER Libraries)
set_target_properties(SpekFile PROPERTIES FOLDER Libraries)
set_target_properties(LeaconCommon PROPERTIES FOLDER Libraries)

set(LEACON_SRC "" CACHE INTERNAL "LeagueController source files")

ADD_SRC(LEACON_SRC		"\\"					""										"src/main.cpp")
ADD_SRC(LEACON_SRC		"Setup\\"				"src/setup.hpp"							"src/setup.cpp")
ADD_SRC(LEACON_SRC		"DebugRender\\"			"src/debug_render.hpp"					"src/debug_render.cpp")
ADD_SRC(LEACON_SRC		"Controller\\"			"src/controller.hpp"					"src/controller.cpp")
ADD_SRC(LEACON_SRC		"Cursor\\"				"src/cursor.hpp"						"src/cursor.cpp")
ADD_SRC(LEACON_SRC		"Order\\"				"src/issue_order.hpp"					"src/issue_order.cpp")
ADD_SRC(LEACON_SRC		"Champ\\"				"src/champion_data.hpp"					"src/champion_data.cpp")
ADD_SRC(LEACON_SRC		"File\\"				"src/filesystem_embedded.hpp"			"src/filesystem_embedded.cpp")
ADD_SRC(LEACON_SRC		"File\\"				"src/files_embedded.hpp"				"src/files_embedded.cpp")
ADD_SRC(LEACON_SRC		"Item\\"				"src/item.hpp"							"src/item.cpp")
ADD_SRC(LEACON_SRC		"Script\\"				"src/script.hpp"						"src/script.cpp")
ADD_SRC(LEACON_SRC		"Script\\Lua"			"src/lua_wrapper.hpp"					"")
ADD_SRC(LEACON_SRC		"Script\\Debug"			"src/script_debug.hpp"					"src/script_debug.cpp")
ADD_SRC(LEACON_SRC		"Script\\Require"		"src/script_require.hpp"				"src/script_require.cpp")
ADD_SRC(LEACON_SRC		"Script\\Math"			"src/script_math.hpp"					"src/script_math.cpp")
ADD_SRC(LEACON_SRC		"Script\\Output"		"src/script_output.hpp"					"src/script_output.cpp")
ADD_SRC(LEACON_SRC		"Script\\Game"			"src/script_game.hpp"					"src/script_game.cpp")
ADD_SRC(LEACON_SRC		"Script\\UI"			"src/script_ui.hpp"						"src/script_ui.cpp")
ADD_SRC(LEACON_SRC		"Script\\Controller"	"src/script_controller.hpp"				"src/script_controller.cpp")
ADD_SRC(LEACON_SRC		"Script\\Mapper"		"src/script_controller_mapper.hpp"		"src/script_controller_mapper.cpp")

add_library(LeagueController SHARED ${LEACON_SRC})
# add_executable(LeagueController ${LEACON_SRC})

if (LEACON_SUBMISSION)
	add_subdirectory("ext/luajit")

	set_target_properties(buildvm PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	set_target_properties(minilua PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	set_target_properties(lua PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	set_target_properties(lua-static PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	set_target_properties(liblua-static PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	set_target_properties(liblua-shared PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	
	target_compile_definitions(LeagueController PUBLIC -DLUA_USE_ASSERT)
	target_compile_definitions(LeagueController PUBLIC -DLUA_USE_APICHECK)
	target_compile_definitions(LeagueController PUBLIC -DLEACON_LUAJIT)
	target_link_libraries(LeagueController liblua-static)
else ()
	add_subdirectory("ext/lua")

	target_compile_definitions(LeagueController PUBLIC LEACON_LUA)
	target_include_directories(LeagueController PRIVATE "ext/lua/lua-5.4.4/include")
	target_link_libraries(LeagueController lua_static)

	set_target_properties(lua_static PROPERTIES FOLDER CMakePredefinedTargets/Lua)
	set_target_properties(luac PROPERTIES FOLDER CMakePredefinedTargets/Lua)
endif ()

target_include_directories(LeagueController PRIVATE "ext")
target_include_directories(LeagueController PRIVATE "src")
target_link_libraries(LeagueController GameOverlay LeaconCommon LeagueInternals ControllerManager LeagueLib LeaconIPC)

set_target_properties(LeagueController PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS ON)
if (WIN32)
	target_compile_definitions(LeagueController PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(LeagueController PUBLIC NOMINMAX)
endif ()
target_compile_definitions(LeagueController PUBLIC LEAGUE_DEBUG_FOLDER="${CMAKE_SOURCE_DIR}")
