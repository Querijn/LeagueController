#include <game_overlay/log.hpp>
#include <game_overlay/window.hpp>
#include <game_overlay/input.hpp>
#include <game_overlay/radial_menu.hpp>

#include "common.hpp"
#include "renderer.hpp"

#include "imgui.h"

#include <cstdio>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <map>
#include <vector>
#include <chrono>

#include "MinHook.h"

#include <spek/util/duration.hpp>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma warning(disable: 4644) // Usage of the macro-based offsetof pattern in constant expressions is non-standard; use offsetof defined in the C++ standard library instead
// Which is funny because it comes from dinput.h

typedef HRESULT(WINAPI* FuncGetDeviceData)(IDirectInputDevice8* device, DWORD objectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD inOut, DWORD flags);
typedef HRESULT(WINAPI* FuncGetDeviceState)(IDirectInputDevice8* device, DWORD dataSize, LPVOID data);

namespace GameOverlay
{
	namespace Input
	{
		void OnKeyDown(u32 virtualKey);
		void OnKeyUp(u32 virtualKey);
		void OnMouseDown(u32 button);
		void OnMouseUp(u32 button);
		void OnMouseScroll(f32 scrollDelta);
	}

	using MilliSec = std::chrono::milliseconds;
	using Time = std::chrono::system_clock::time_point;
	using Clock = std::chrono::system_clock;

	LogCategory g_inputLog("Input");

	struct FakeMouseInput
	{
		int button;
		bool down;
		int x;
		int y;
	};

	FuncGetDeviceData			g_originalGetDeviceData = NULL;
	FuncGetDeviceState			g_originalGetDeviceState = NULL;
	void*						g_getDeviceDataHook = NULL;
	void*						g_getDeviceStateHook = NULL;
	std::vector<FakeMouseInput> g_mouseInputs;

	// Data to fake
	Time						g_time;
	DWORD						g_timeStampOffset = 0;
	DWORD						g_nextSequence = 0;
	UINT_PTR					g_appData = ~0;
	IDirectInputDevice8*		g_mouseDevice = nullptr;

	enum class InputType
	{
		Unknown = 0,
		Mouse = 1,
		Keyboard = 2,
	};

	static const char* GetInputTypeName(InputType a)
	{
		switch (a)
		{
		case InputType::Keyboard: return "Keyboard";
		case InputType::Mouse: return "Mouse";
		default: return "UNKNOWN INPUTDEVICE TYPE";
		}
	}

	struct InputDevice
	{
		InputDevice()
		{
			x = 0;
			y = 0;
			memset(buttons, 0, sizeof(buttons));
		}

		InputType type = InputType::Unknown;

		int x, y;
		bool buttons[256];
	};

	static std::map<IDirectInputDevice8*, InputDevice> g_inputDeviceMap;
	static InputDevice* GetInputDevice(IDirectInputDevice8* device)
	{
		static int iterator = 0;
		auto index = g_inputDeviceMap.find(device);
		if (index == g_inputDeviceMap.end())
			return nullptr;
		return &index->second;
	}

	static const char* GetKeyName(DWORD inputKey)
	{
		Leacon_Profiler;
		const char* key;
		switch (inputKey)
		{
		case DIK_ESCAPE: key = "ESCAPE"; break;
		case DIK_1: key = "1"; break;
		case DIK_2: key = "2"; break;
		case DIK_3: key = "3"; break;
		case DIK_4: key = "4"; break;
		case DIK_5: key = "5"; break;
		case DIK_6: key = "6"; break;
		case DIK_7: key = "7"; break;
		case DIK_8: key = "8"; break;
		case DIK_9: key = "9"; break;
		case DIK_0: key = "0"; break;
		case DIK_MINUS: key = "MINUS"; break;
		case DIK_EQUALS: key = "EQUALS"; break;
		case DIK_BACK: key = "BACK"; break;
		case DIK_TAB: key = "TAB"; break;
		case DIK_Q: key = "Q"; break;
		case DIK_W: key = "W"; break;
		case DIK_E: key = "E"; break;
		case DIK_R: key = "R"; break;
		case DIK_T: key = "T"; break;
		case DIK_Y: key = "Y"; break;
		case DIK_U: key = "U"; break;
		case DIK_I: key = "I"; break;
		case DIK_O: key = "O"; break;
		case DIK_P: key = "P"; break;
		case DIK_LBRACKET: key = "LBRACKET"; break;
		case DIK_RBRACKET: key = "RBRACKET"; break;
		case DIK_RETURN: key = "RETURN"; break;
		case DIK_LCONTROL: key = "LCONTROL"; break;
		case DIK_A: key = "A"; break;
		case DIK_S: key = "S"; break;
		case DIK_D: key = "D"; break;
		case DIK_F: key = "F"; break;
		case DIK_G: key = "G"; break;
		case DIK_H: key = "H"; break;
		case DIK_J: key = "J"; break;
		case DIK_K: key = "K"; break;
		case DIK_L: key = "L"; break;
		case DIK_SEMICOLON: key = "SEMICOLON"; break;
		case DIK_APOSTROPHE: key = "APOSTROPHE"; break;
		case DIK_GRAVE: key = "GRAVE"; break;
		case DIK_LSHIFT: key = "LSHIFT"; break;
		case DIK_BACKSLASH: key = "BACKSLASH"; break;
		case DIK_Z: key = "Z"; break;
		case DIK_X: key = "X"; break;
		case DIK_C: key = "C"; break;
		case DIK_V: key = "V"; break;
		case DIK_B: key = "B"; break;
		case DIK_N: key = "N"; break;
		case DIK_M: key = "M"; break;
		case DIK_COMMA: key = "COMMA"; break;
		case DIK_PERIOD: key = "PERIOD"; break;
		case DIK_SLASH: key = "SLASH"; break;
		case DIK_RSHIFT: key = "RSHIFT"; break;
		case DIK_MULTIPLY: key = "MULTIPLY"; break;
		case DIK_LMENU: key = "LMENU"; break;
		case DIK_SPACE: key = "SPACE"; break;
		case DIK_CAPITAL: key = "CAPITAL"; break;
		case DIK_F1: key = "F1"; break;
		case DIK_F2: key = "F2"; break;
		case DIK_F3: key = "F3"; break;
		case DIK_F4: key = "F4"; break;
		case DIK_F5: key = "F5"; break;
		case DIK_F6: key = "F6"; break;
		case DIK_F7: key = "F7"; break;
		case DIK_F8: key = "F8"; break;
		case DIK_F9: key = "F9"; break;
		case DIK_F10: key = "F10"; break;
		case DIK_NUMLOCK: key = "NUMLOCK"; break;
		case DIK_SCROLL: key = "SCROLL"; break;
		case DIK_NUMPAD7: key = "NUMPAD7"; break;
		case DIK_NUMPAD8: key = "NUMPAD8"; break;
		case DIK_NUMPAD9: key = "NUMPAD9"; break;
		case DIK_SUBTRACT: key = "SUBTRACT"; break;
		case DIK_NUMPAD4: key = "NUMPAD4"; break;
		case DIK_NUMPAD5: key = "NUMPAD5"; break;
		case DIK_NUMPAD6: key = "NUMPAD6"; break;
		case DIK_ADD: key = "ADD"; break;
		case DIK_NUMPAD1: key = "NUMPAD1"; break;
		case DIK_NUMPAD2: key = "NUMPAD2"; break;
		case DIK_NUMPAD3: key = "NUMPAD3"; break;
		case DIK_NUMPAD0: key = "NUMPAD0"; break;
		case DIK_DECIMAL: key = "DECIMAL"; break;
		case DIK_OEM_102: key = "OEM_102"; break;
		case DIK_F11: key = "F11"; break;
		case DIK_F12: key = "F12"; break;
		case DIK_F13: key = "F13"; break;
		case DIK_F14: key = "F14"; break;
		case DIK_F15: key = "F15"; break;
		case DIK_KANA: key = "KANA"; break;
		case DIK_ABNT_C1: key = "ABNT_C1"; break;
		case DIK_CONVERT: key = "CONVERT"; break;
		case DIK_NOCONVERT: key = "NOCONVERT"; break;
		case DIK_YEN: key = "YEN"; break;
		case DIK_ABNT_C2: key = "ABNT_C2"; break;
		case DIK_NUMPADEQUALS: key = "NUMPADEQUALS"; break;
		case DIK_PREVTRACK: key = "PREVTRACK"; break;
		case DIK_AT: key = "AT"; break;
		case DIK_COLON: key = "COLON"; break;
		case DIK_UNDERLINE: key = "UNDERLINE"; break;
		case DIK_KANJI: key = "KANJI"; break;
		case DIK_STOP: key = "STOP"; break;
		case DIK_AX: key = "AX"; break;
		case DIK_UNLABELED: key = "UNLABELED"; break;
		case DIK_NEXTTRACK: key = "NEXTTRACK"; break;
		case DIK_NUMPADENTER: key = "NUMPADENTER"; break;
		case DIK_RCONTROL: key = "RCONTROL"; break;
		case DIK_MUTE: key = "MUTE"; break;
		case DIK_CALCULATOR: key = "CALCULATOR"; break;
		case DIK_PLAYPAUSE: key = "PLAYPAUSE"; break;
		case DIK_MEDIASTOP: key = "MEDIASTOP"; break;
		case DIK_VOLUMEDOWN: key = "VOLUMEDOWN"; break;
		case DIK_VOLUMEUP: key = "VOLUMEUP"; break;
		case DIK_WEBHOME: key = "WEBHOME"; break;
		case DIK_NUMPADCOMMA: key = "NUMPADCOMMA"; break;
		case DIK_DIVIDE: key = "DIVIDE"; break;
		case DIK_SYSRQ: key = "SYSRQ"; break;
		case DIK_RMENU: key = "RMENU"; break;
		case DIK_PAUSE: key = "PAUSE"; break;
		case DIK_HOME: key = "HOME"; break;
		case DIK_UP: key = "UP"; break;
		case DIK_PRIOR: key = "PRIOR"; break;
		case DIK_LEFT: key = "LEFT"; break;
		case DIK_RIGHT: key = "RIGHT"; break;
		case DIK_END: key = "END"; break;
		case DIK_DOWN: key = "DOWN"; break;
		case DIK_NEXT: key = "NEXT"; break;
		case DIK_INSERT: key = "INSERT"; break;
		case DIK_DELETE: key = "DELETE"; break;
		case DIK_LWIN: key = "LWIN"; break;
		case DIK_RWIN: key = "RWIN"; break;
		case DIK_APPS: key = "APPS"; break;
		case DIK_POWER: key = "POWER"; break;
		case DIK_SLEEP: key = "SLEEP"; break;
		case DIK_WAKE: key = "WAKE"; break;
		case DIK_WEBSEARCH: key = "WEBSEARCH"; break;
		case DIK_WEBFAVORITES: key = "WEBFAVORITES"; break;
		case DIK_WEBREFRESH: key = "WEBREFRESH"; break;
		case DIK_WEBSTOP: key = "WEBSTOP"; break;
		case DIK_WEBFORWARD: key = "WEBFORWARD"; break;
		case DIK_WEBBACK: key = "WEBBACK"; break;
		case DIK_MYCOMPUTER: key = "MYCOMPUTER"; break;
		case DIK_MAIL: key = "MAIL"; break;
		case DIK_MEDIASELECT: key = "MEDIASELECT"; break;
		default: key = "<UNKNOWN>"; break;
		}

		return key;
	}

	template<typename MouseStateType>
	static void CreateMouseEvents(InputDevice& inputDevice, const MouseStateType& state)
	{
		Leacon_Profiler;
		if (state.lX != 0 || state.lY != 0)
		{
			if (GetMousePos(inputDevice.x, inputDevice.y))
			{
				// _sapp.mouse.x = (f32)inputDevice.x;
				// _sapp.mouse.y = (f32)inputDevice.y;
				// _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID);
			}
		}

		if (!!state.rgbButtons[0] != inputDevice.buttons[0])
		{
			inputDevice.buttons[0] = !!state.rgbButtons[0];
			// sapp_event_type eventType = inputDevice.buttons[0] ? SAPP_EVENTTYPE_MOUSE_DOWN : SAPP_EVENTTYPE_MOUSE_UP;
			// _sapp_win32_mouse_event(eventType, SAPP_MOUSEBUTTON_LEFT);
		}

		if (!!state.rgbButtons[1] != inputDevice.buttons[1])
		{
			inputDevice.buttons[1] = !!state.rgbButtons[1];
			// sapp_event_type eventType = inputDevice.buttons[1] ? SAPP_EVENTTYPE_MOUSE_DOWN : SAPP_EVENTTYPE_MOUSE_UP;
			// _sapp_win32_mouse_event(eventType, SAPP_MOUSEBUTTON_RIGHT);
		}

		if (!!state.rgbButtons[2] != inputDevice.buttons[2])
		{
			inputDevice.buttons[2] = !!state.rgbButtons[2];
			// sapp_event_type eventType = inputDevice.buttons[2] ? SAPP_EVENTTYPE_MOUSE_DOWN : SAPP_EVENTTYPE_MOUSE_UP;
			// _sapp_win32_mouse_event(eventType, SAPP_MOUSEBUTTON_MIDDLE);
		}
	}

	static HRESULT WINAPI HookedGetDeviceState(IDirectInputDevice8* device, DWORD dataSize, LPVOID data)
	{
		Leacon_Profiler;
		GameOverlay_LogInfoOnce(g_inputLog, "Received GetDeviceState call.");

		HRESULT result = g_originalGetDeviceState(device, dataSize, data);
		if (result != DI_OK)
			return result;

		InputDevice& inputDevice = g_inputDeviceMap[device];
		if (g_mouseDevice)
			g_mouseDevice = device;

		// Determine type of device based on size of state struct
		switch (dataSize)
		{
		case sizeof(DIMOUSESTATE) :
			inputDevice.type = InputType::Mouse;
			// GameOverlay_LogDebug(g_inputLog, "GetDeviceState: DIMOUSESTATE");
			CreateMouseEvents(inputDevice, *(DIMOUSESTATE*)data);
			break;

		case sizeof(DIMOUSESTATE2) :
			// GameOverlay_LogDebug(g_inputLog, "GetDeviceState: DIMOUSESTATE2");
			g_inputDeviceMap[device].type = InputType::Mouse;
			CreateMouseEvents(inputDevice, *(DIMOUSESTATE2*)data);
			break;

		case sizeof(unsigned char) * 256:
			// GameOverlay_LogDebug(g_inputLog, "GetDeviceState: Keyboard");
			g_inputDeviceMap[device].type = InputType::Keyboard;
			break;

		default:
			GameOverlay_LogError(g_inputLog, "HookedGetDeviceState: Unknown size %u", dataSize);
		}

		return result;
	}

	InputDevice* InitDeviceData(IDirectInputDevice8* device)
	{
		DIDEVCAPS caps;
		caps.dwSize = sizeof(DIDEVCAPS);
		if (SUCCEEDED(device->GetCapabilities(&caps)) == false)
		{
			GameOverlay_LogError(g_inputLog, "Failed to get device capabilities for 0x%x.", device);
			return nullptr;
		}
		
		InputDevice& inputDevice = g_inputDeviceMap[device];
		if (caps.dwDevType == DI8DEVTYPE_KEYBOARD)
			inputDevice.type = InputType::Keyboard;
		else if (caps.dwDevType == DI8DEVTYPE_MOUSE)
			inputDevice.type = InputType::Mouse;
		else
		{
			GameOverlay_LogError(g_inputLog, "Unknown device type 0x%x.", caps.dwDevType);
			return nullptr;
		}
		
		return &inputDevice;
	}

	static DWORD CalculateNow()
	{
		Leacon_Profiler;
		// No time was calculated yet
		if (g_timeStampOffset == ~0)
			return 0;

		MilliSec delta = std::chrono::duration_cast<MilliSec>(Clock::now() - g_time);
		return g_timeStampOffset + delta.count();
	}

	static HRESULT WINAPI HookedGetDeviceData(IDirectInputDevice8* device, DWORD objectData, LPDIDEVICEOBJECTDATA deviceObjectData, LPDWORD inOut, DWORD flags)
	{
		Leacon_Profiler;
		GameOverlay_LogInfoOnce(g_inputLog, "Received GetDeviceData call.");

		HRESULT result = g_originalGetDeviceData(device, objectData, deviceObjectData, inOut, flags);
		bool isOk = result == DI_OK || result == DI_BUFFEROVERFLOW;
		if (isOk == false)
		{
			GameOverlay_LogError(g_inputLog, "Original GetDeviceData failed: %u", result);
			return result;
		}

		DIMOUSESTATE mouseState = {};
		InputDevice* inputDevice = GetInputDevice(device);
		if (inputDevice == nullptr) // We haven't gotten this device registered yet.
		{
			inputDevice = InitDeviceData(device);
			if (inputDevice == nullptr)
			{
				GameOverlay_LogWarning(g_inputLog, "Unable to initialise input device for 0x%x!", device);
				return result;
			}
			return result;
		}

		if (inOut == nullptr)
		{
			GameOverlay_LogDebug(g_inputLog, "HookedGetDeviceData.inOut is null");
			return result;
		}

		for (DWORD i = 0; i < *inOut; i++)
		{
			DIDEVICEOBJECTDATA& data = deviceObjectData[i];

			// Initial copy
			if (g_timeStampOffset == ~0)
			{
				g_nextSequence = data.dwSequence;
				g_appData = data.uAppData;
			}

			// Might loop around, so get them each initial event
			if (i == 0)
			{
				g_timeStampOffset = data.dwTimeStamp;
				g_time = Clock::now();
			}

			// Make sure we get these correct and sequential
			data.dwSequence = g_nextSequence;
			g_nextSequence++;

			bool isDown = (data.dwData & 0x80) != 0;
			switch (inputDevice->type)
			{
			case InputType::Keyboard:
			{
				UINT virtualKey = MapVirtualKeyEx(data.dwOfs, MAPVK_VSC_TO_VK, 0);
				if (isDown)
					Input::OnKeyDown(virtualKey);
				else
					Input::OnKeyUp(virtualKey);
				break;
			}

			case InputType::Mouse:
			{
				if (g_mouseDevice == nullptr || g_mouseDevice != device)
					g_mouseDevice = device;
				const char* type = "unknown";
				switch (data.dwOfs)
				{
				case DIMOFS_BUTTON0: type = "Button 0"; mouseState.rgbButtons[0] = isDown; break;
				case DIMOFS_BUTTON1: type = "Button 1"; mouseState.rgbButtons[1] = isDown; break;
				case DIMOFS_BUTTON2: type = "Button 2"; mouseState.rgbButtons[2] = isDown; break;

				case DIMOFS_X: type = "Move X"; mouseState.lX = data.dwData; break;
				case DIMOFS_Y: type = "Move Y"; mouseState.lY = data.dwData; break;
				case DIMOFS_Z: type = "Move Z"; mouseState.lZ = data.dwData; break;
				}
				
				if (data.dwOfs == DIMOFS_BUTTON0 || data.dwOfs == DIMOFS_BUTTON1 || data.dwOfs == DIMOFS_BUTTON2)
				{
					u32 buttonIndex = 0;
					if (data.dwOfs == DIMOFS_BUTTON0)
						buttonIndex = 0;
					else if (data.dwOfs == DIMOFS_BUTTON1)
						buttonIndex = 1;
					else if (data.dwOfs == DIMOFS_BUTTON2)
						buttonIndex = 2;
					
					if (isDown)
						Input::OnMouseDown(buttonIndex);
					else
						Input::OnMouseUp(buttonIndex);
				}

				// GameOverlay_LogDebug(g_inputLog, "GetDeviceData: Real input (%s on %x): %02x (%u // %u)", type, device, data.dwData, data.dwTimeStamp, data.dwSequence);
				break;
			}

			}
		}

		// Send over our converted data to our event handler
		if (inputDevice->type == InputType::Mouse)
			CreateMouseEvents(*inputDevice, mouseState);

		/*if (IsRendererInitialised() && IsImguiEnabled())
		{
			ImGuiIO& io = ImGui::GetIO();
			if ((io.WantCaptureKeyboard && inputDevice->type == InputType::Keyboard) ||
				(io.WantCaptureMouse && inputDevice->type == InputType::Mouse))
			{
				*inOut = 0;
				return result;
			}
		}*/

		// now fake our own inputs back to the game
		if (g_mouseDevice && device == g_mouseDevice)
		{
			for (const auto& input : g_mouseInputs)
			{
				DIDEVICEOBJECTDATA& data = deviceObjectData[*inOut];

				// Fake data
				data.dwOfs = DIMOFS_BUTTON0 + input.button;
				data.uAppData = g_appData;
				data.dwSequence = g_nextSequence;
				data.dwTimeStamp = CalculateNow();
				if (input.down)
					data.dwData |= 0x80;

				const char* type = "unknown";
				switch (data.dwOfs)
				{
				case DIMOFS_BUTTON0: type = "Button 0"; break;
				case DIMOFS_BUTTON1: type = "Button 1"; break;
				case DIMOFS_BUTTON2: type = "Button 2"; break;

				case DIMOFS_X: type = "Move X"; break;
				case DIMOFS_Y: type = "Move Y"; break;
				case DIMOFS_Z: type = "Move Z"; break;
				}

				// GameOverlay_LogDebug(g_inputLog, "GetDeviceData: Fake input (%s on %x): %02x (%u // %u)", type, device, data.dwData, data.dwTimeStamp, data.dwSequence);

				*inOut += 1;
				g_nextSequence++;
			}
			g_mouseInputs.clear();
		}

		return result;
	}

	bool GetMousePos(int& x, int& y)
	{
		Leacon_Profiler;
		POINT p;
		if (GetCursorPos(&p) == false)
			return false;

		static HWND window = (HWND)GetWindow();
		if (window == nullptr || IsWindow(window) == false)
		{
			if (window)
			{
				HWND newWindow = (HWND)GetWindow();
				GameOverlay_LogInfo(g_inputLog, "GetMousePos: Replaced old window %x with %x", window, newWindow);
			}
			window = (HWND)GetWindow();
		}

		if (ScreenToClient(window, &p) == false)
			return false;

		x = p.x;
		y = p.y;
		return true;
	}

	static void OnRender()
	{
		if (!IsImguiEnabled())
			return;
		
		Leacon_Profiler;
		ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Always);
		if (ImGui::Begin("Input"))
		{
			for (auto deviceData : g_inputDeviceMap)
			{
				auto& device = deviceData.second;

				switch (device.type)
				{
				case InputType::Mouse:
					ImGui::Text("Mouse (%d, %d)", device.x, device.y);
					for (int i = 0; i < 3; i++)
						ImGui::Text("- Button %d: %s", i, device.buttons[i] ? "down" : "up");
					break;
				}
			}

			ImGui::End();
		}
	}

	int InitInput()
	{
		Leacon_Profiler;
		GameOverlay_LogInfo(g_inputLog, "InitInput\n");

		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
		IDirectInput8* directInput = NULL;

		HRESULT dinputInitialised = DirectInput8Create(instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&directInput, NULL);
		GameOverlay_Assert(g_inputLog, dinputInitialised == DI_OK);
		if (dinputInitialised != DI_OK)
			return -1;

		LPDIRECTINPUTDEVICE8 dummyKeyboard;
		HRESULT keyboardCreated = directInput->CreateDevice(GUID_SysKeyboard, &dummyKeyboard, NULL);
		GameOverlay_Assert(g_inputLog, keyboardCreated == DI_OK);
		if (keyboardCreated != DI_OK)
		{
			directInput->Release();
			return -2;
		}

		void** dinputDeviceHook = *(void***)dummyKeyboard;

		g_getDeviceDataHook = dinputDeviceHook[10];
		MH_STATUS isHookCreated = MH_CreateHook(dinputDeviceHook[10], HookedGetDeviceData, (void**)&g_originalGetDeviceData);
		GameOverlay_AssertF(g_inputLog, isHookCreated == MH_OK, "MH_CreateHook failed for HookGetDeviceData");

		MH_STATUS isHooked = MH_EnableHook(dinputDeviceHook[10]);
		GameOverlay_AssertF(g_inputLog, isHooked == MH_OK, "MH_EnableHook failed for MH_EnableHook(HookGetDeviceData");

		isHookCreated = MH_CreateHook(dinputDeviceHook[9], HookedGetDeviceState, (void**)&g_originalGetDeviceState);
		GameOverlay_AssertF(g_inputLog, isHookCreated == MH_OK, "MH_CreateHook failed for HookedGetDeviceState");

		g_getDeviceStateHook = dinputDeviceHook[9];
		isHooked = MH_EnableHook(dinputDeviceHook[9]);
		GameOverlay_AssertF(g_inputLog, isHooked == MH_OK, "MH_EnableHook failed for HookedGetDeviceState");

		directInput->Release();
		return 0;
	}

	void DestroyInput()
	{
		Leacon_Profiler;
		if (g_getDeviceDataHook != nullptr)
		{
			MH_STATUS hookRemoved = MH_DisableHook(g_getDeviceDataHook);
			GameOverlay_AssertF(g_inputLog, hookRemoved == MH_OK, "MH_DisableHook failed for g_getDeviceDataHook");
			
			hookRemoved = MH_RemoveHook(g_getDeviceDataHook);
			GameOverlay_AssertF(g_inputLog, hookRemoved == MH_OK, "MH_RemoveHook failed for g_getDeviceDataHook");
		}

		if (g_getDeviceStateHook != nullptr)
		{
			MH_STATUS hookRemoved = MH_DisableHook(g_getDeviceStateHook);
			GameOverlay_AssertF(g_inputLog, hookRemoved == MH_OK, "MH_DisableHook failed for g_getDeviceStateHook");
			
			hookRemoved = MH_RemoveHook(g_getDeviceStateHook);
			GameOverlay_AssertF(g_inputLog, hookRemoved == MH_OK, "MH_RemoveHook failed for g_getDeviceStateHook");
		}

		g_originalGetDeviceData = NULL;
		g_originalGetDeviceState = NULL;
		g_getDeviceDataHook = NULL;
		g_getDeviceStateHook = NULL;
		g_mouseInputs.clear();

		g_timeStampOffset = 0;
		g_nextSequence = 0;
		g_appData = ~0;
		g_mouseDevice = nullptr;
	}

#define MakeLParam(x, y) (y << 16) | (x & 0xFFFF)

#if 1
	void FeignMouseInput(int button, bool down, int x, int y)
	{
		Leacon_Profiler;
		// Make sure our last input is not the same as our current
		if (g_mouseInputs.empty() == false)
		{
			auto& last = g_mouseInputs[g_mouseInputs.size() - 1];
			if (last.button == button && last.down == down)
				return;
		}

		g_mouseInputs.push_back({ button, down, x, y });
	}
#else
	void SendWindowMessage(UINT message, WPARAM w, LPARAM l);
	void FeignMouseInput(int button, bool down, int x, int y)
	{
		Leacon_Profiler;
		UINT message;
		switch (button)
		{
		default:
		case 0: message = WM_LBUTTONDOWN; break;
		case 1: message = WM_RBUTTONDOWN; break;
		case 2: message = WM_MBUTTONDOWN; break;
		}

		if (!down)
			message++;

		SendWindowMessage(message, 1, MakeLParam(x, y));
	}
#endif
}
