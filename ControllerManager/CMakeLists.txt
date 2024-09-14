cmake_minimum_required(VERSION 3.13)
project(ControllerManager)

set(CONMAN_SRC "" CACHE INTERNAL "ControllerManager source files")
ADD_SRC(CONMAN_SRC	"Input"						"inc/league_controller/controller_input.hpp"					"src/controller_input.cpp")
ADD_SRC(CONMAN_SRC	"Input"						"inc/league_controller/controller_input_definition.hpp"			"")
ADD_SRC(CONMAN_SRC	"Controller"				"inc/league_controller/controller.hpp"							"src/controller.cpp")
ADD_SRC(CONMAN_SRC	"Listener"					"inc/league_controller/controller_listener.hpp"					"src/controller_listener.cpp")
ADD_SRC(CONMAN_SRC	"Manager"					"inc/league_controller/controller_manager.hpp"					"src/controller_manager.cpp")
ADD_SRC(CONMAN_SRC	"RawInput"					"inc/league_controller/controller_windows_rawinput.hpp"			"src/controller_windows_rawinput.cpp")
ADD_SRC(CONMAN_SRC	"XInput"					"inc/league_controller/controller_windows_xinput.hpp"			"src/controller_windows_xinput.cpp")

ADD_SRC(CONMAN_SRC	"UsageType"					"src/controller_usage_type.hpp"									"src/controller_usage_type.cpp")
ADD_SRC(CONMAN_SRC	"UsageType"					"src/controller_usage_type_definition.hpp"						"")

add_library(ControllerManager ${CONMAN_SRC})
target_include_directories(ControllerManager PUBLIC "inc")
target_include_directories(ControllerManager PRIVATE "src")
target_link_libraries(ControllerManager LeaconCommon SpekUtil)
set_target_properties(ControllerManager PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF)

if (WIN32)
	target_compile_definitions(ControllerManager PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(ControllerManager PUBLIC NOMINMAX)
endif ()
