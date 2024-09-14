cmake_minimum_required(VERSION 3.5)
project(SkermInterpreter)

set(SKERM_INTERPRETER_SRCS "" CACHE INTERNAL "Skerm interpreter sources")
function(ADD_SRC var folder inc src)
	source_group(${folder} FILES ${inc} ${src})
	set(${var} ${${var}} ${inc} ${src} CACHE INTERNAL "" FORCE)
endfunction(ADD_SRC)

# Generated files
ADD_SRC(SKERM_INTERPRETER_SRCS "Types"		"inc/skerm/base_types.hpp"					"src/skerm/base_types.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "ASDF"		"inc/skerm/asdf_interpreter.hpp"			"src/skerm/asdf_interpreter.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_addition_token.hpp"			"src/skerm/math_addition_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_base_token.hpp"			"src/skerm/math_base_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_division_token.hpp"			"src/skerm/math_division_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_equation_token.hpp"			"src/skerm/math_equation_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_multiplication_token.hpp"			"src/skerm/math_multiplication_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_struct_token.hpp"			"src/skerm/math_struct_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_subtraction_token.hpp"			"src/skerm/math_subtraction_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_token_type.hpp"			"src/skerm/math_token_type.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/math_variable_token.hpp"			"src/skerm/math_variable_token.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/ui_anchor.hpp"			"src/skerm/ui_anchor.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/ui_container.hpp"			"src/skerm/ui_container.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/ui_image.hpp"			"src/skerm/ui_image.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/ui_namespace.hpp"			"src/skerm/ui_namespace.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/ui_text.hpp"			"src/skerm/ui_text.cpp")
ADD_SRC(SKERM_INTERPRETER_SRCS "Data"		"inc/skerm/ui_type.hpp"			"src/skerm/ui_type.cpp")

add_library(SkermInterpreter STATIC ${SKERM_INTERPRETER_SRCS})

target_include_directories(SkermInterpreter PUBLIC inc)
target_include_directories(SkermInterpreter PRIVATE src)
target_link_libraries(SkermInterpreter SkermMathParser)

set_target_properties(SkermInterpreter PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF)