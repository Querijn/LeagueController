cmake_minimum_required(VERSION 3.13)
project(DummyWrapper)

set(DUMMYWRAPPER_SRC "" CACHE INTERNAL "DummyWrapper source files")
ADD_SRC(DUMMYWRAPPER_SRC		"\\"		""		"src/main.cpp")

add_executable(DummyWrapper ${DUMMYWRAPPER_SRC})
add_dependencies(DummyWrapper LeagueController LeagueControllerLauncher) # Ensure we build LeagueController/Injector before the wrapper
target_link_libraries(DummyWrapper ControllerManager LeaconCommon GameOverlay LeaconIPC)
set_target_properties(DummyWrapper PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 17
						CXX_EXTENSIONS OFF)
target_compile_definitions(DummyWrapper PUBLIC LEAGUECONTROLLER_OUTPUT_FOLDER="${CMAKE_BINARY_DIR}")
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DummyWrapper)
