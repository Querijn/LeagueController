cmake_minimum_required(VERSION 3.13)
project(MemoryHelper)

# Capstone
set(CAPSTONE_X86_SUPPORT ON CACHE BOOL "Required for this project" FORCE)
set(CAPSTONE_ARM_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_ARM64_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_M68K_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_MIPS_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_PPC_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_SPARC_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_SYSZ_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_XCORE_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_TMS320C64X_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_M680X_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)
set(CAPSTONE_EVM_SUPPORT OFF CACHE BOOL "Unnecessary for this project" FORCE)

set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "Only build static" FORCE)
set(CAPSTONE_BUILD_STATIC ON CACHE BOOL "Only build static" FORCE)
set(CAPSTONE_BUILD_STATIC_RUNTIME OFF CACHE BOOL "Only build static" FORCE)
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "Only build static" FORCE)
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "Only build static" FORCE)
set(CAPSTONE_BUILD_DIET OFF CACHE BOOL "Build all details" FORCE)

add_subdirectory("ext/capstone-4.0.2")
set_target_properties(capstone PROPERTIES FOLDER CMakePredefinedTargets)

set(MEMHELPER_SRC "" CACHE INTERNAL "MemoryHelper source files")

ADD_SRC(MEMHELPER_SRC		"Address"			"inc/memory_helper/address.hpp"					"")
ADD_SRC(MEMHELPER_SRC		"Instruction"		"inc/memory_helper/instruction.hpp"				"src/instruction.cpp")
ADD_SRC(MEMHELPER_SRC		"Helpers"			"inc/memory_helper/helper_functions.hpp"		"src/helper_functions.cpp")
ADD_SRC(MEMHELPER_SRC		"\\"				""												"src/main.cpp")

# add_executable(MemoryHelper ${MEMHELPER_SRC})
add_library(MemoryHelper STATIC ${MEMHELPER_SRC})
target_include_directories(MemoryHelper PUBLIC "inc")
target_include_directories(MemoryHelper PUBLIC "ext/capstone-4.0.2/include")
target_include_directories(MemoryHelper PRIVATE "src")
target_link_libraries(MemoryHelper SpekUtil capstone)

set_target_properties(MemoryHelper PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 17
						CXX_EXTENSIONS OFF)
if (WIN32)
	target_compile_definitions(MemoryHelper PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(MemoryHelper PUBLIC NOMINMAX)
endif ()