cmake_minimum_required(VERSION 3.17)
cmake_policy(SET CMP0079 NEW)
project(SkermMathParser)

set(SKERM_MATH_SRCS "" CACHE INTERNAL "Skerm math sources")
function(ADD_SRC var folder inc src)
	source_group(${folder} FILES ${inc} ${src})
	set(${var} ${${var}} ${inc} ${src} CACHE INTERNAL "" FORCE)
endfunction(ADD_SRC)

# General properties
ADD_SRC(SKERM_MATH_SRCS	"Parser"		"inc/skerm/math_parser.hpp"		"src/math_parser.cpp")
ADD_SRC(SKERM_MATH_SRCS	"Tokeniser"		"inc/skerm/tokeniser.hpp"		"src/tokeniser.cpp")
ADD_SRC(SKERM_MATH_SRCS	"Tokens"		"inc/skerm/tokens.hpp"			"src/tokens.cpp")

if (EMSCRIPTEN)
	add_executable(SkermMathParser ${SKERM_MATH_SRCS})
else()
	add_library(SkermMathParser STATIC ${SKERM_MATH_SRCS})
endif()
target_include_directories(SkermMathParser PUBLIC inc)
target_include_directories(SkermMathParser PRIVATE src)

set_target_properties(SkermMathParser PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF)

if (EMSCRIPTEN)
	set(SPEK_FLAGS "" CACHE INTERNAL "Spek LINK FLAGS")
	if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
		set(${SPEK_FLAGS} ${SPEK_FLAGS} "-g4-s SAFE_HEAP=1 -s WARN_UNALIGNED=1 -fsanitize=undefined")
	else ()
		set(${SPEK_FLAGS} ${SPEK_FLAGS} "-O2 --closure 1")
	endif ()
	set_target_properties(SkermMathParser PROPERTIES LINK_FLAGS ${SPEK_FLAGS} "-s DISABLE_EXCEPTION_CATCHING=2 -s FULL_ES2=1 --bind -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS=\"['_skerm_reduce_equation_string']\"")
endif ()