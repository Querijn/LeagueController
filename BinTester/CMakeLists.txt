cmake_minimum_required(VERSION 3.13)
project(BinTester)

set(BINTEST_SRC "" CACHE INTERNAL "BinTester source files")

ADD_SRC(BINTEST_SRC		"\\"					""										"src/main.cpp")
add_executable(BinTester ${BINTEST_SRC})

target_include_directories(BinTester PRIVATE "src")
target_link_libraries(BinTester GameOverlay LeaconCommon LeagueLib)

set_target_properties(BinTester PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS ON)
if (WIN32)
	target_compile_definitions(BinTester PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(BinTester PUBLIC NOMINMAX)
endif ()
target_compile_definitions(BinTester PUBLIC LEAGUE_DEBUG_FOLDER="${CMAKE_SOURCE_DIR}")
