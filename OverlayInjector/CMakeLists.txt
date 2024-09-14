cmake_minimum_required(VERSION 3.13)
project(LeagueControllerLauncher)

set(GAMEOVERLAY_SRC "" CACHE INTERNAL "LeagueControllerLauncher source files")
ADD_SRC(GAMEOVERLAY_SRC		"\\"		""						"src/main.cpp")
ADD_SRC(GAMEOVERLAY_SRC		"\\"		"src/process.hpp"		"src/process.cpp")

find_package(OpenSSL REQUIRED)
add_executable(LeagueControllerLauncher ${GAMEOVERLAY_SRC})
add_dependencies(LeagueControllerLauncher LeagueController) # Ensure we build LeagueController before the injector
target_link_directories(LeagueControllerLauncher PRIVATE ${OPENSSL_INCLUDE_DIR}/../lib) # TODO: lol fix this
target_link_libraries(LeagueControllerLauncher PRIVATE libssl openssl libcrypto ControllerManager LeaconCommon GameOverlay LeaconIPC)
target_include_directories(LeagueControllerLauncher PUBLIC ${OPENSSL_INCLUDE_DIR})
message(${OPENSSL_INCLUDE_DIR}/../lib)
set_target_properties(LeagueControllerLauncher PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF)
target_compile_definitions(LeagueControllerLauncher PUBLIC LEAGUECONTROLLER_OUTPUT_FOLDER="${CMAKE_BINARY_DIR}")
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LeagueControllerLauncher)
