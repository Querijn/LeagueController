#if WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <league_controller/profiler.hpp>
#include <league_controller/controller_windows_xinput.hpp>
#include <league_controller/controller.hpp>
#include <league_controller/controller_manager.hpp>
#include <spek/util/duration.hpp>

#include <Windows.h>
#include <objbase.h>
#include <xinput.h>
#include <cassert>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

#pragma comment(lib, "Xinput.lib")

namespace LeagueController
{
	static std::atomic_int g_realControllerCount = 0;

	using Duration = Spek::Duration;
	using namespace Spek;
	struct XINPUT_CAPABILITIES_EX
	{
		XINPUT_CAPABILITIES Capabilities;
		WORD vendorId;
		WORD productId;
		WORD revisionId;
		DWORD a4;
	};
	typedef DWORD(_stdcall* XInputGetCapabilitiesExFunc)(DWORD a1, DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES_EX* pCapabilities);
	static XInputGetCapabilitiesExFunc XInputGetCapabilitiesEx;
	static std::thread g_xInputUpdateThread;
	static std::atomic_bool g_xInputUpdateThreadRunning;
	HMODULE g_xInputModuleHandle = nullptr;

	struct XInputController
	{
		Controller controller;

		DWORD packetId;
		Duration packetTimestamp;
	};

	struct XInputData
	{
		XInputData(ControllerManager& manager);

		void Update();

		ControllerManager& m_manager;
		std::vector<XInputController> m_controllers;
	};

	ControllerHandlerXInput::ControllerHandlerXInput(ControllerManager& a_manager) :
		m_data(std::make_unique<XInputData>(a_manager))
	{
	}

	ControllerHandlerXInput::~ControllerHandlerXInput()
	{
	}

	void ControllerHandlerXInput::Update()
	{
		Leacon_Profiler;
		m_data->Update();
	}

	bool ControllerHandlerXInput::HasXInputSupport(long vendorId, long productId)
	{
		Leacon_Profiler;
		XINPUT_CAPABILITIES_EX deviceData;
		size_t controllerCount = m_data->m_controllers.size();
		for (int i = 0; i < controllerCount; i++)
		{
			assert(XInputGetCapabilitiesEx(1, i, 0, &deviceData) == ERROR_SUCCESS);
			if (vendorId == deviceData.vendorId && productId == deviceData.productId)
				return true;
		}

		return false;
	}

	static inline float FloatClamp(float a, float b, float v)
	{
		Leacon_Profiler;
		if (v < a)
			return a;
		if (v > b)
			return b;

		return v;
	}
#define WBUTTON_IS_DOWN(buttons, i) ((buttons&(1<<(i)))!=0)
#define RANGE_TO_FLOAT(range) FloatClamp(-1.0f, 1.0f, (f32)(range)/(f32)std::numeric_limits<decltype(range)>::max())

	constexpr size_t GetButtonIndex(int index)
	{
		switch (index)
		{
		case 12: return (size_t)ControllerInput::ButtonA;
		case 13: return (size_t)ControllerInput::ButtonB;
		case 14: return (size_t)ControllerInput::ButtonX;
		case 15: return (size_t)ControllerInput::ButtonY;

		case 11: // TODO
		case 10:
			return ~0;

		case 9: return (size_t)ControllerInput::ButtonR1;
		case 8: return (size_t)ControllerInput::ButtonL1;

		case 7: return (size_t)ControllerInput::ButtonR3;
		case 6: return (size_t)ControllerInput::ButtonL3;

		case 5: return (size_t)ControllerInput::ButtonSelect;
		case 4: return (size_t)ControllerInput::ButtonStart;
		}

		assert(false);
		return ~0;
	}

	constexpr int GetDPadUp() { return 0; }
	constexpr int GetDPadDown() { return 1; }
	constexpr int GetDPadLeft() { return 2; }
	constexpr int GetDPadRight() { return 3; }

	XInputData::XInputData(ControllerManager& manager) :
		m_manager(manager)
	{
		Leacon_Profiler;
		m_controllers.reserve(XUSER_MAX_COUNT);

		CoInitialize(nullptr); // Needed to get rid of some debug spam from XInputGetState

		if (g_xInputModuleHandle == nullptr)
		{
			g_xInputModuleHandle = LoadLibrary(TEXT("XInput1_4.dll"));
			assert(g_xInputModuleHandle != nullptr);
				
			XInputGetCapabilitiesEx = (XInputGetCapabilitiesExFunc)GetProcAddress(g_xInputModuleHandle, (char*)108);
			assert(XInputGetCapabilitiesEx);
		}

	}

	void UpdateController(XInputController& xinputController, const XINPUT_STATE& sourceState, bool force = false)
	{
		Leacon_Profiler;
		if (xinputController.controller.GetName() == nullptr)
		{
			// TODO:
			xinputController.controller.SetName("XInput Controller");
		}

		xinputController.packetId = sourceState.dwPacketNumber;
		xinputController.packetTimestamp = GetTimeSinceStart();

		ControllerState& targetState = xinputController.controller.SwapAndGetCurrentState();
		targetState.timestamp = xinputController.packetTimestamp;

		for (int i = 4; i < 16; i++) // Skip 0~3 = DPAD
		{
			size_t buttonIndex = GetButtonIndex(i);
			if (buttonIndex != ~0) // Skip undefined
				targetState.button[buttonIndex] = WBUTTON_IS_DOWN(sourceState.Gamepad.wButtons, i) ? 1.0f : 0.0f;
		}

		targetState.button[(size_t)ControllerInput::ButtonL2] = RANGE_TO_FLOAT(sourceState.Gamepad.bLeftTrigger);
		targetState.button[(size_t)ControllerInput::ButtonR2] = RANGE_TO_FLOAT(sourceState.Gamepad.bRightTrigger);

		// Left Thumbstick
		targetState.leftAnalog.X = RANGE_TO_FLOAT(sourceState.Gamepad.sThumbLX);
		targetState.leftAnalog.Y = RANGE_TO_FLOAT(sourceState.Gamepad.sThumbLY);

		// Right Thumbstick
		targetState.rightAnalog.X = RANGE_TO_FLOAT(sourceState.Gamepad.sThumbRX);
		targetState.rightAnalog.Y = RANGE_TO_FLOAT(sourceState.Gamepad.sThumbRY);

		// DPAD
		if (WBUTTON_IS_DOWN(sourceState.Gamepad.wButtons, GetDPadLeft()))
			targetState.leftDPad.X = -1.0f;
		else if (WBUTTON_IS_DOWN(sourceState.Gamepad.wButtons, GetDPadRight()))
			targetState.leftDPad.X = 1.0f;
		else
			targetState.leftDPad.X = 0.0f;

		if (WBUTTON_IS_DOWN(sourceState.Gamepad.wButtons, GetDPadDown()))
			targetState.leftDPad.Y = -1.0f;
		else if (WBUTTON_IS_DOWN(sourceState.Gamepad.wButtons, GetDPadUp()))
			targetState.leftDPad.Y = 1.0f;
		else
			targetState.leftDPad.Y = 0.0f;
	}

	static void UpdateXInputControllerCount()
	{
		Leacon_Profiler;
		XINPUT_STATE state = {};
		DWORD i;
		for (i = g_realControllerCount; i < XUSER_MAX_COUNT; i++)
		{
			DWORD getStateResult = 0;
			{
				Leacon_ProfilerSection("XInputGetState");
				getStateResult = XInputGetState(i, &state);
			}

			// Not a controller?
			if (getStateResult != ERROR_SUCCESS)
				break;
		}

		g_realControllerCount = i;
	}

	void UpdateControllerCountThread()
	{
		using namespace std::chrono_literals;
		Leacon_Profiler;

		while (g_xInputUpdateThreadRunning)
		{
			UpdateXInputControllerCount();
			std::this_thread::sleep_for(2000ms);
		}
	}

	void XInputData::Update()
	{
		Leacon_Profiler;
		static bool g_fetchedInitialCount = false;
		if (g_fetchedInitialCount == false)
		{
			UpdateXInputControllerCount();
			g_xInputUpdateThread = std::thread(UpdateControllerCountThread);
			g_xInputUpdateThread.detach();
			g_xInputUpdateThreadRunning = true;
			g_fetchedInitialCount = true;
		}

		size_t controllerCount = m_controllers.size();
		for (DWORD i = 0; i < g_realControllerCount; i++)
		{
			Leacon_ProfilerSection("Controller ");
			XINPUT_STATE state = {};

			DWORD getStateResult = 0;
			{
				Leacon_ProfilerSection("XInputGetState");
				getStateResult = XInputGetState(i, &state);
			}

			bool wasConnected = i < controllerCount;
			bool isConnected = getStateResult == ERROR_SUCCESS;

			bool newlyConnected = !wasConnected && isConnected;
			bool newlyDisconnected = wasConnected && !isConnected;

			if (newlyConnected)
				m_controllers.emplace_back();
			else if (wasConnected == false && isConnected == false) // No controller here
				break;

			DWORD oldPacketId = m_controllers[i].packetId;
			UpdateController(m_controllers[i], state, newlyConnected);
			DWORD newPacketId = m_controllers[i].packetId;
			if (newPacketId != oldPacketId)
				m_manager.ControllerHasInput(m_controllers[i].controller);

			if (newlyConnected)
			{
				m_manager.ControllerHasConnected(m_controllers[i].controller);
			}
			else if (newlyDisconnected)
			{
				m_manager.ControllerHasDisconnected(m_controllers[i].controller);

				controllerCount--;
				m_controllers.resize(controllerCount);
			}
		}
	}

	void DestroyXInput()
	{
		if (g_xInputUpdateThreadRunning)
		{
			g_xInputUpdateThreadRunning = false;
			g_xInputUpdateThread.join();
		}

		if (g_xInputModuleHandle)
		{
			FreeLibrary(g_xInputModuleHandle);
			g_xInputModuleHandle = nullptr;
			XInputGetCapabilitiesEx = nullptr;
		}
	}
}

#endif
