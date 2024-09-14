cmake_minimum_required(VERSION 3.17)
cmake_policy(SET CMP0079 NEW)
project(SkermCompiler)

option(SKERM_BUILD_COMPILER_AS_LIBRARY "Build SkermCompiler as a library" OFF)

# ASDF
add_subdirectory(ext/ASDF)
set_target_properties(ASDF PROPERTIES FOLDER Compiler)
set_target_properties(mstch PROPERTIES FOLDER Compiler)
set_target_properties(pugixml PROPERTIES FOLDER Compiler)

# General properties
set(SKERM_COMPILER_SRCS "" CACHE INTERNAL "Skerm compiler sources")
ADD_SRC(SKERM_COMPILER_SRCS	"UIFile"		"inc/skerm/source_file.hpp"		"src/source_file.cpp")
ADD_SRC(SKERM_COMPILER_SRCS	"Compiler"		"inc/skerm/compiler.hpp"		"src/compiler.cpp")
ADD_SRC(SKERM_COMPILER_SRCS	"\\"			""								"src/main.cpp")

add_library(SkermCompilerLib STATIC ${SKERM_COMPILER_SRCS})
target_compile_definitions(SkermCompilerLib PUBLIC SKERM_BUILD_COMPILER_AS_LIBRARY)

add_executable(SkermCompilerApp ${SKERM_COMPILER_SRCS})
target_compile_definitions(SkermCompilerApp PUBLIC SKERM_BUILD_COMPILER_AS_APPLICATION)

# Just a function to make sure that code is not duplicated between SkermCompilerApp and SkermCompilerLib
function(SETUP_COMPILER project)
	target_include_directories(${project} PUBLIC inc)

	set_target_properties(${project} PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS OFF)
	target_link_libraries(${project} ASDF SkermMathParser)
endfunction(SETUP_COMPILER)

SETUP_COMPILER(SkermCompilerLib)
SETUP_COMPILER(SkermCompilerApp)
