cmake_minimum_required(VERSION 3.5)
project(ASDF)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/HELPER.txt)

set(ASDF_LIB_DIRS "" CACHE INTERNAL "ASDF library directories")
set(ASDF_LIBRARIES "" CACHE INTERNAL "ASDF library")
set(ASDF_SOURCES "" CACHE INTERNAL "ASDF sources")

option(ASDF_LIBRARY					"Use ASDF as library" ON)
option(ASDF_DEBUG_DATA_PARSING		"Store Data Parser debug data" OFF)

# Makes sure we can use filters in our projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(ext/mstch)
include_directories(${mstch_INCLUDE_DIR})

add_subdirectory(ext/pugixml)
include_directories(ext/pugixml/src)

include_directories(ext/dirent)

ADD_ASDF_SOURCE(ASDF_SOURCES	"Application"			"inc/asdf/config.hpp"					"src/main.cpp")

ADD_ASDF_SOURCE(ASDF_SOURCES	"Hash"					"src/hash_helper.hpp"					"")
ADD_ASDF_SOURCE(ASDF_SOURCES	"ErrorHandling"			"inc/asdf/error_handling.hpp"			"src/error_handling.cpp")
ADD_ASDF_SOURCE(ASDF_SOURCES	"SchemaParser"			"inc/asdf/schema_parser.hpp"			"src/schema_parser.cpp")
ADD_ASDF_SOURCE(ASDF_SOURCES	"DataParser"			"inc/asdf/data_parser.hpp"				"src/data_parser.cpp")
ADD_ASDF_SOURCE(ASDF_SOURCES	"SerialisationParser"	"inc/asdf/serialisation_parser.hpp"		"src/serialisation_parser.cpp")
ADD_ASDF_SOURCE(ASDF_SOURCES	"TypeInfo"				"inc/asdf/type_info.hpp"				"src/type_info.cpp")
ADD_ASDF_SOURCE(ASDF_SOURCES	"Compiler"				"inc/asdf/compiler.hpp"					"src/compiler.cpp")

if (ASDF_LIBRARY)
	add_library(ASDF ${ASDF_SOURCES})
	target_compile_definitions(ASDF PUBLIC ASDF_LIBRARY)

else (ASDF_LIBRARY)
	add_executable(ASDF ${ASDF_SOURCES})
	target_compile_definitions(ASDF PUBLIC ASDF_APPLICATION)

endif ()

if (ASDF_DEBUG_DATA_PARSING)
	target_compile_definitions(ASDF PUBLIC ASDF_DEBUG_DATA_PARSING)
endif ()

# Local includes
target_include_directories(ASDF PUBLIC "inc")
target_include_directories(ASDF PRIVATE "src")

set_target_properties(ASDF PROPERTIES 
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF)
target_link_libraries(ASDF ${ASDF_LIBRARIES} pugixml mstch)
