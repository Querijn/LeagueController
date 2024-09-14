#include "lua_wrapper.hpp"
#include "script_output.hpp"
#include "controller.hpp"
#include "cursor.hpp"
#include "game_overlay/input.hpp"

#include <game_overlay/log.hpp>

namespace LeagueController
{
	extern GameOverlay::LogCategory g_scriptLog;

	void AddOutputScriptSystem(sol::state& state)
	{
		Leacon_Profiler;
		state["MouseButton"] = state.create_table();
		state["MouseButton"]["Left"] = (int)MouseButton::Left;
		state["MouseButton"]["Right"] = (int)MouseButton::Right;

		state["Keyboard"] = state.create_table();
		state["Keyboard"]["KeyNone"] = Keyboard::KeyNone;

		state["Keyboard"]["KeyQ"] = Keyboard::KeyQ;
		state["Keyboard"]["KeyW"] = Keyboard::KeyW;
		state["Keyboard"]["KeyE"] = Keyboard::KeyE;
		state["Keyboard"]["KeyR"] = Keyboard::KeyR;
		state["Keyboard"]["KeyT"] = Keyboard::KeyT;
		state["Keyboard"]["KeyY"] = Keyboard::KeyY;
		state["Keyboard"]["KeyU"] = Keyboard::KeyU;
		state["Keyboard"]["KeyI"] = Keyboard::KeyI;
		state["Keyboard"]["KeyO"] = Keyboard::KeyO;
		state["Keyboard"]["KeyP"] = Keyboard::KeyP;
		state["Keyboard"]["KeyA"] = Keyboard::KeyA;
		state["Keyboard"]["KeyS"] = Keyboard::KeyS;
		state["Keyboard"]["KeyD"] = Keyboard::KeyD;
		state["Keyboard"]["KeyF"] = Keyboard::KeyF;
		state["Keyboard"]["KeyG"] = Keyboard::KeyG;
		state["Keyboard"]["KeyH"] = Keyboard::KeyH;
		state["Keyboard"]["KeyJ"] = Keyboard::KeyJ;
		state["Keyboard"]["KeyK"] = Keyboard::KeyK;
		state["Keyboard"]["KeyK"] = Keyboard::KeyK;
		state["Keyboard"]["KeyL"] = Keyboard::KeyL;
		state["Keyboard"]["KeyZ"] = Keyboard::KeyZ;
		state["Keyboard"]["KeyX"] = Keyboard::KeyX;
		state["Keyboard"]["KeyC"] = Keyboard::KeyC;
		state["Keyboard"]["KeyV"] = Keyboard::KeyV;
		state["Keyboard"]["KeyB"] = Keyboard::KeyB;
		state["Keyboard"]["KeyN"] = Keyboard::KeyN;
		state["Keyboard"]["KeyM"] = Keyboard::KeyM;

		state["Keyboard"]["Key0"] = Keyboard::Key0;
		state["Keyboard"]["Key1"] = Keyboard::Key1;
		state["Keyboard"]["Key2"] = Keyboard::Key2;
		state["Keyboard"]["Key3"] = Keyboard::Key3;
		state["Keyboard"]["Key4"] = Keyboard::Key4;
		state["Keyboard"]["Key5"] = Keyboard::Key5;
		state["Keyboard"]["Key6"] = Keyboard::Key6;
		state["Keyboard"]["Key7"] = Keyboard::Key7;
		state["Keyboard"]["Key8"] = Keyboard::Key8;
		state["Keyboard"]["Key9"] = Keyboard::Key9;

		state["Keyboard"]["KeyCtrl"] = Keyboard::KeyCtrl;
		state["Keyboard"]["KeyShift"] = Keyboard::KeyShift;

		state.set_function("ShowCrosshair", &LeagueController::ShowCrosshair);
		state.set_function("SetCrosshairPos", &LeagueController::SetCrosshairPos);

		state.set_function("SetMouseTarget", &SetMouseTarget);
		state.set_function("SetInstantMouseTarget", &SetInstantMouseTarget);
		state.set_function("FeignMouseClick", &FeignMouseClick);
		state.set_function("FeignKeyPressOnMouse", &FeignKeyPressOnMouse);
		state.set_function("FeignKeyPress", &FeignKeyPress);
		state.set_function("FeignKeyRelease", &FeignKeyRelease);
	}
	void DestroyScriptOutput()
	{
		Leacon_Profiler;
	}
}