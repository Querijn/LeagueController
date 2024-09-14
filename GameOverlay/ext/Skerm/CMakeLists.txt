cmake_minimum_required(VERSION 3.5)
project(Skerm)

function(ADD_SRC var folder inc src)
	source_group(${folder} FILES ${inc} ${src})
	set(${var} ${${var}} ${inc} ${src} CACHE INTERNAL "" FORCE)
endfunction(ADD_SRC)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(math_parser)
add_subdirectory(compiler)
add_subdirectory(interpreter)

set_target_properties(SkermCompilerApp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
