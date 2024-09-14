cmake_minimum_required(VERSION 3.13)
project(LeaconCommon)

set(LEACON_COMMON_SRC "" CACHE INTERNAL "Leacon Common source files")
ADD_SRC(LEACON_COMMON_SRC		"Types\\"					"inc/league_controller/types.hpp"				"src/types.cpp")
ADD_SRC(LEACON_COMMON_SRC		"Duration\\"				"inc/league_controller/duration.hpp"			"src/duration.cpp")
ADD_SRC(LEACON_COMMON_SRC		"SharedMem\\"				"inc/league_controller/shared_memory.hpp"		"src/shared_memory.cpp")
ADD_SRC(LEACON_COMMON_SRC		"Settings\\"				"inc/league_controller/base64.h"				"src/base64.cpp")
ADD_SRC(LEACON_COMMON_SRC		"Settings\\"				"inc/league_controller/settings.hpp"			"src/settings.cpp")
ADD_SRC(LEACON_COMMON_SRC		"Allocator\\"				"inc/league_controller/allocator.hpp"			"src/allocator.cpp")
ADD_SRC(LEACON_COMMON_SRC		"LauncherData\\"			"inc/league_controller/launcher_data.hpp"		"")
ADD_SRC(LEACON_COMMON_SRC		"\\"						"inc/league_controller/config.hpp"				"")

ADD_SRC(LEACON_COMMON_SRC		"SecureString\\"			"inc/league_controller/secure_string.hpp"		"src/secure_string.cpp")
ADD_SRC(LEACON_COMMON_SRC		"Obfuscation\\"				"inc/league_controller/obfuscation.hpp"			"src/obfuscation.cpp")

add_library(LeaconCommon STATIC ${LEACON_COMMON_SRC})
target_include_directories(LeaconCommon PUBLIC "ext/glm")
target_include_directories(LeaconCommon PUBLIC "ext/mini/src")
target_include_directories(LeaconCommon PUBLIC "inc")
target_include_directories(LeaconCommon PRIVATE "src")
if (LEACON_PROFILE)
	target_link_libraries(LeaconCommon SpekUtil Tracy::TracyClient)
endif ()
set_target_properties(LeaconCommon PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF)
if (WIN32)
	target_compile_definitions(LeaconCommon PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(LeaconCommon PUBLIC NOMINMAX)
endif ()