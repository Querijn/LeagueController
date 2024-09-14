#include "lua_wrapper.hpp"
#include "script_controller.hpp"
#include "controller.hpp"

#include <game_overlay/log.hpp>
#include <game_overlay/window.hpp>
#include <game_overlay/overlay.hpp>

#include <league_internals/offsets.hpp>
#include <league_internals/game_object.hpp>

// ControllerManager
#include <league_controller/controller_input.hpp>
#include <league_controller/controller.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller_manager.hpp>

#include <Windows.h>

namespace LeagueController
{
	extern GameOverlay::LogCategory	g_scriptLog;
	static bool						g_lizardModeEnabled = false;
	static bool						g_isSteamDeck = false;
	static glm::vec2 				g_mousePos;
	static f32 						g_mouseSpeed = 0.5f;
	static f32						g_edgeOffset = 0.03f;

	void SetLizardMode(bool enabled) { g_lizardModeEnabled = enabled; }
	bool IsLizardModeEnabled() { return g_lizardModeEnabled; }

	void DestroyScriptController()
	{
	}

	static bool LocalIsButtonDown(int button) { return IsButtonDown((ControllerInput)button); }
	static bool LocalIsButtonUp(int button) { return IsButtonUp((ControllerInput)button); }

	void AddControllerScriptSystem(sol::state& state)
	{
		Leacon_Profiler;
		state.set_function("IsButtonDown", LocalIsButtonDown);
		state.set_function("IsButtonUp", LocalIsButtonUp);
		state.set_function("GetControllerInputName", GetControllerInputName);
		state.set_function("SetLizardMode", SetLizardMode);
		state.set_function("IsLizardModeEnabled", IsLizardModeEnabled);

		state.set_function("GetRightAnalogDir", [](bool make3D, bool normalize)
		{
			Leacon_Profiler;
			glm::vec3 dir;
			bool hasDir = GetRightAnalogDir(dir, make3D);
			return std::make_tuple(hasDir, dir, normalize);
		});

		state.set_function("GetLeftAnalogDir", [](bool make3D, bool normalize)
		{
			Leacon_Profiler;
			glm::vec3 dir;
			bool hasDir = GetLeftAnalogDir(dir, make3D, normalize);
			return std::make_tuple(hasDir, dir);
		});

		state["ControllerInput"] = state.create_table();
#define xstr(s) str(s)
#define str(s) #s
#define DEFINE_ENUM(a, b, c)		state["ControllerInput"][str(c)] = (int)ControllerInput::a;
#define DEFINE_ENUM_ALT(a, b, c)	state["ControllerInput"][str(c)] = (int)ControllerInput::a;
#define DEFINE_ENUM_INFO(a, b)
#include <league_controller/controller_input_definition.hpp>
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
	}


	static void UpdateLizardMode(Duration dt)
	{
		Leacon_Profiler;
		if (g_isSteamDeck) // SteamDeck does this on its own
			return;

		static bool wasLizardModeActive = false;
		if (g_lizardModeEnabled == false)
		{
			wasLizardModeActive = false;
			return;
		}

		if (wasLizardModeActive == false) // Check if enabled this frame
		{
			g_mousePos = glm::vec2(GameOverlay::GetWidth() * 0.5f, GameOverlay::GetHeight() * 0.5f);
			wasLizardModeActive = true;
			GameOverlay_LogInfo(g_scriptLog, "Enabled lizard mode");
		}
		
		if (GameOverlay::IsWindowFocussed() == false)
			return;

		glm::vec3 dir;
		if (GetRightAnalogDir(dir) == false)
			return;

		// TODO:
		/*POINT p;
		if (GetCursorPos(&p))
		{
			g_mousePos.x = p.x;
			g_mousePos.y = p.y;
		}*/

		dir.y = -dir.y;
		g_mousePos += (glm::vec2)dir * ((g_mouseSpeed * (f32)GameOverlay::GetWidth()) * (f32)dt.ToSecF64());
		
		f32 negEdgeOffset = 1.0f - g_edgeOffset;
		f32 width = GameOverlay::GetWidth();
		f32 height = GameOverlay::GetHeight();
		if (g_mousePos.x < width * g_edgeOffset)
			g_mousePos.x = width * g_edgeOffset;
		else if (g_mousePos.x >= width * negEdgeOffset)
			g_mousePos.x = width * negEdgeOffset;

		if (g_mousePos.y < height * g_edgeOffset)
			g_mousePos.y = height * g_edgeOffset;
		else if (g_mousePos.y >= height * negEdgeOffset)
			g_mousePos.y = height * negEdgeOffset;

		int screenX, screenY;
		if (GameOverlay::GetClientGlobalPosition(screenX, screenY))
			SetCursorPos(g_mousePos.x + screenX, g_mousePos.y + screenY);
	}

	void UpdateScriptController(Duration dt)
	{
		Leacon_Profiler;
		UpdateLizardMode(dt);
	}
}