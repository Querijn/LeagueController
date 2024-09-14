cmake_minimum_required(VERSION 3.13)
project(GameOverlay)

# MinHook
add_subdirectory("ext/minhook")
set_target_properties(minhook PROPERTIES FOLDER CMakePredefinedTargets)

# Sokol
add_subdirectory("ext/sokol")
set_target_properties(Sokol PROPERTIES FOLDER CMakePredefinedTargets)

# Skerm
add_subdirectory("ext/skerm")
set_target_properties(SkermCompilerLib PROPERTIES FOLDER Libraries/Skerm)
set_target_properties(SkermCompilerApp PROPERTIES FOLDER Libraries/Skerm)
set_target_properties(SkermInterpreter PROPERTIES FOLDER Libraries/Skerm)
set_target_properties(SkermMathParser PROPERTIES FOLDER Libraries/Skerm)

set(OVERLAY_SRC "" CACHE INTERNAL "GameOverlay source files")

ADD_SRC(OVERLAY_SRC		"\\"					"inc/game_overlay/overlay.hpp"						"src/overlay.cpp")
ADD_SRC(OVERLAY_SRC		"Log\\"					"inc/game_overlay/log.hpp"							"src/log.cpp")
ADD_SRC(OVERLAY_SRC		"Window\\"				"inc/game_overlay/window.hpp"						"src/window.cpp")
ADD_SRC(OVERLAY_SRC		"Input\\"				"inc/game_overlay/input.hpp"						"src/input.cpp")
ADD_SRC(OVERLAY_SRC		"Image"					"inc/game_overlay/image.hpp"						"src/image.cpp")
ADD_SRC(OVERLAY_SRC		"ImageInstance"			"inc/game_overlay/image_instance.hpp"				"src/image_instance.cpp")
ADD_SRC(OVERLAY_SRC		"Font"					"inc/game_overlay/font.hpp"							"src/font.cpp")
ADD_SRC(OVERLAY_SRC		"Skerm"					"inc/game_overlay/skerm.hpp"						"src/skerm.cpp")
ADD_SRC(OVERLAY_SRC		"RadialMenu"			"inc/game_overlay/radial_menu.hpp"					"src/radial_menu.cpp")
ADD_SRC(OVERLAY_SRC		"SimpleInput"			"inc/game_overlay/simple_input.hpp"					"src/simple_input.cpp")
ADD_SRC(OVERLAY_SRC		"DebugRenderer"			"inc/game_overlay/debug_renderer.hpp"				"src/debug_renderer.cpp")
ADD_SRC(OVERLAY_SRC		"Surface"				"inc/game_overlay/surface.hpp"						"src/surface.cpp")

ADD_SRC(OVERLAY_SRC		"Assert\\"				"src/ppk_assert.h"									"src/ppk_assert.cpp")
ADD_SRC(OVERLAY_SRC		"Common\\"				"src/common.hpp"									"")
ADD_SRC(OVERLAY_SRC		"Renderer\\DX11"		"src/renderer_vtable_dx11.hpp"						"")
ADD_SRC(OVERLAY_SRC		"Renderer\\DX11"		"src/renderer_dx11.hpp"								"src/renderer_dx11.cpp")
ADD_SRC(OVERLAY_SRC		"Renderer"				"src/renderer.hpp"									"src/renderer.cpp")
ADD_SRC(OVERLAY_SRC		"Schrift"				"src/schrift.h"										"src/schrift.c")
ADD_SRC(OVERLAY_SRC		"Shader\\Image"			"src/shaders/image.hpp"								"shaders/image.glsl")
ADD_SRC(OVERLAY_SRC		"Shader\\SkermImage"	"src/shaders/skerm_image.hpp"						"shaders/skerm_image.glsl")
ADD_SRC(OVERLAY_SRC		"Shader\\Container"		"src/shaders/skerm_container.hpp"					"shaders/skerm_container.glsl")
ADD_SRC(OVERLAY_SRC		"Shader\\Text"			"src/shaders/text.hpp"								"shaders/text.glsl")

# ImGUI
ADD_SRC(OVERLAY_SRC		"ImGui"					"ext/imgui/imgui.h"									"ext/imgui/imgui.cpp")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imgui_demo.cpp")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imgui_draw.cpp")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imgui_internal.h")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imgui_tables.cpp")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imgui_widgets.cpp")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imstb_rectpack.h")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imstb_textedit.h")
ADD_SRC(OVERLAY_SRC		"ImGui"					""													"ext/imgui/imstb_truetype.h")
ADD_SRC(OVERLAY_SRC		"ImGui\\Win32"			"ext/imgui/backends/imgui_impl_win32.h"				"ext/imgui/backends/imgui_impl_win32.cpp")
ADD_SRC(OVERLAY_SRC		"ImGui\\DX11"			"ext/imgui/backends/imgui_impl_dx11.h"				"ext/imgui/backends/imgui_impl_dx11.cpp")

add_library(GameOverlay STATIC ${OVERLAY_SRC})
target_include_directories(GameOverlay PUBLIC "inc")
target_include_directories(GameOverlay PUBLIC "ext/imgui")
target_include_directories(GameOverlay PRIVATE "src")
target_include_directories(GameOverlay PRIVATE "ext")
target_include_directories(GameOverlay PRIVATE "ext/minhook/include")
target_include_directories(GameOverlay PRIVATE "ext/imgui/backends")
target_link_libraries(GameOverlay minhook SpekFile LeaconCommon Sokol SkermInterpreter SkermCompilerLib)

set_target_properties(GameOverlay PROPERTIES 
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS OFF)
if (WIN32)
	target_compile_definitions(GameOverlay PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(GameOverlay PUBLIC NOMINMAX)
	target_compile_definitions(GameOverlay PUBLIC SOKOL_NO_ENTRY)

	# Build shaders before this project.
	# add_custom_target(GameOverlayShaders COMMAND cmd /c "${CMAKE_CURRENT_SOURCE_DIR}/GenerateShader.bat ${CMAKE_CURRENT_SOURCE_DIR}")
	# set_target_properties(GameOverlayShaders PROPERTIES FOLDER Libraries)
	# add_dependencies(GameOverlay GameOverlayShaders)
endif ()
target_compile_definitions(GameOverlay PUBLIC GAMEOVERLAY_DEBUG_FOLDER="${CMAKE_SOURCE_DIR}")