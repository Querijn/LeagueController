cmake_minimum_required(VERSION 3.0)
project(LeagueController)

# Leacon Options
option(LEACON_SUBMISSION "Build LeagueController in submission mode" ON)
option(LEACON_PROFILE	 "Build LeagueController with a profiler" OFF)

# Ensure x64
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/Install)

# Some forced settings
set(LUA_ENABLE_SHARED OFF CACHE BOOL "" FORCE)

# Folder functionality
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
function(ADD_SRC var folder inc src)
	source_group(${folder} FILES ${inc} ${src})
	set(${var} ${${var}} ${inc} ${src} CACHE INTERNAL "")
endfunction(ADD_SRC)

if (LEACON_SUBMISSION)
	add_definitions(-DLEACON_SUBMISSION)
endif (LEACON_SUBMISSION)
	
if (LEACON_PROFILE)
	# Tracy
	add_definitions(-DTRACY_ENABLE=1)
	option(TRACY_ENABLE "" ON)
	option(TRACY_ON_DEMAND "" ON)
	add_subdirectory(ext/tracy)
	set_target_properties(TracyClient PROPERTIES FOLDER CMakePredefinedTargets)
endif (LEACON_PROFILE)

# Projects
add_subdirectory("MemoryHelper")
add_subdirectory("Common")
add_subdirectory("IPC")
add_subdirectory("ControllerManager")
add_subdirectory("GameOverlay")
add_subdirectory("LeagueInternals")
add_subdirectory("OverlayInjector")
add_subdirectory("DummyWrapper")
add_subdirectory("LeagueController")
add_subdirectory("BinTester")

# Project folders
set_target_properties(MemoryHelper PROPERTIES FOLDER Libraries)
set_target_properties(LeaconIPC PROPERTIES FOLDER Libraries)
set_target_properties(DummyWrapper PROPERTIES FOLDER Libraries)
set_target_properties(ASDF PROPERTIES FOLDER Libraries)
set_target_properties(mstch PROPERTIES FOLDER CMakePredefinedTargets)
set_target_properties(pugixml PROPERTIES FOLDER CMakePredefinedTargets)
add_dependencies(LeagueControllerLauncher LeagueController)

# This reloads the data embeds if necessary
if (LEACON_SUBMISSION)
	add_custom_target(LeaconDataEmbedder COMMAND cmd /c "node ${CMAKE_CURRENT_SOURCE_DIR}/DataEmbedder/index.js" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_target_properties(LeaconDataEmbedder PROPERTIES FOLDER Libraries)
	add_dependencies(LeagueController LeaconDataEmbedder)
endif (LEACON_SUBMISSION)