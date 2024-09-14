cmake_minimum_required(VERSION 3.13)
project(LeaconIPC)

set(LEACON_IPC_SRC "" CACHE INTERNAL "Leacon IPC source files")
ADD_SRC(LEACON_IPC_SRC		"Connection\\"		"inc/league_controller/connection.hpp"		"src/connection.cpp")
ADD_SRC(LEACON_IPC_SRC		"State\\"			"inc/league_controller/state_base.hpp"		"src/state_base.cpp")

set(LIBIPC_BUILD_DEMOS OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(LIBIPC_BUILD_TESTS OFF CACHE BOOL "Unnecessary for this project" FORCE)
add_subdirectory("ext/cpp-ipc")
set_target_properties(ipc PROPERTIES FOLDER CMakePredefinedTargets)

add_library(LeaconIPC STATIC ${LEACON_IPC_SRC})
target_include_directories(LeaconIPC PUBLIC "inc")
target_include_directories(LeaconIPC PRIVATE "src")
target_link_libraries(LeaconIPC LeaconCommon ipc)
set_target_properties(LeaconIPC PROPERTIES 
								VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
								CXX_STANDARD 17
								CXX_EXTENSIONS OFF)
if (WIN32)
	target_compile_definitions(LeaconIPC PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(LeaconIPC PUBLIC NOMINMAX)
endif ()