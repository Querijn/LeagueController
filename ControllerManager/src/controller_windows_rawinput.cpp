#if _WIN32

#include <league_controller/controller_windows_rawinput.hpp>
#include <league_controller/controller_windows_xinput.hpp>
#include <league_controller/controller.hpp>
#include <league_controller/controller_manager.hpp>
#include <league_controller/profiler.hpp>
#include <spek/util/duration.hpp>

#include "controller_usage_type.hpp"

#include <Windows.h>
#include <xinput.h>
#include <hidsdi.h>
#include <cassert>
#include <memory>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <locale>
#include <vector>
#include <codecvt>
#include <thread>
#include <chrono>

#pragma comment(lib, "hid.lib")

namespace LeagueController
{
	using Duration = Spek::Duration;
	using namespace Spek;
	LRESULT CALLBACK InputWndProc(HWND window, UINT message, WPARAM w, LPARAM l);

	struct RawInputController
	{
		Controller controller;

		DWORD packetId;
		Duration packetTimestamp;
	};

	struct DeviceAxis
	{
		UsageType type;
		f32 min;
		f32 max;
		f32 value;
	};

	struct DeviceState
	{
		static constexpr u64 maxButtons = 128;
		static constexpr u64 maxAxes = 32;

		BOOL buttonStates[maxButtons];
		DeviceAxis axis[maxAxes];
		LONG dpad;
	};

	struct RawDevice
	{
		static constexpr u64 workingDataSize = 2 << 13;

		Controller controller;
		DeviceState sourceState;
;
		RID_DEVICE_INFO deviceInfo;
		std::string name;
		u8 workingData[workingDataSize];
		INT buttonCount;
		INT axisCount;

		DirectionalInputState prevLeftAnalog;
		DirectionalInputState prevLeftDPad;
		DirectionalInputState prevRightAnalog;

		Duration lastInputTime = 0_ms;
		Duration lastReportTime = 0_ms;
	};

	using WindowWndProcMap = std::map<HWND, WNDPROC>;
	static std::map<HANDLE, std::shared_ptr<RawDevice>> g_rawDeviceMap;
	static std::unique_ptr<WindowWndProcMap> g_originalWndProcMap;
	static UINT g_processedDeviceCount = 0;
	static std::atomic_uint g_deviceCount = 0;
	static std::thread g_rawInputUpdateThread;
	static std::atomic_bool g_rawInputUpdateThreadRunning;

	WindowWndProcMap& GetOriginalWndProcMap()
	{
		Leacon_Profiler;
		if (g_originalWndProcMap == nullptr)
			g_originalWndProcMap = std::make_unique<WindowWndProcMap>();
		return *g_originalWndProcMap;
	}

	void OnRawInput(const RAWINPUT* rawInput)
	{
		Leacon_Profiler;
		HANDLE handle = rawInput->header.hDevice;
		auto deviceIndex = g_rawDeviceMap.find(handle);
		if (deviceIndex == g_rawDeviceMap.end() || deviceIndex->second == nullptr)
			return;

		RawDevice& rawDevice = *deviceIndex->second;
		DeviceState& state = rawDevice.sourceState;

		u8* dataPointer = rawDevice.workingData;
		u8* dataPointerEnd = rawDevice.workingData + RawDevice::workingDataSize;

		UINT bufferCount;
		if (GetRawInputDeviceInfo(handle, RIDI_PREPARSEDDATA, NULL, &bufferCount) != 0)
			return; // Device disconnected
		PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)dataPointer;
		assert((int)GetRawInputDeviceInfo(handle, RIDI_PREPARSEDDATA, preparsedData, &bufferCount) >= 0);
		dataPointer += bufferCount;
		assert(dataPointer < dataPointerEnd);

		// button caps
		HIDP_CAPS caps;
		assert(HidP_GetCaps(preparsedData, &caps) == HIDP_STATUS_SUCCESS);
		PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)dataPointer;
		dataPointer += sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps;
		assert(dataPointer < dataPointerEnd);

		USHORT capCount = caps.NumberInputButtonCaps;
		assert(HidP_GetButtonCaps(HidP_Input, buttonCaps, &capCount, preparsedData) == HIDP_STATUS_SUCCESS);
		rawDevice.buttonCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;

		// Value caps
		PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)dataPointer;
		dataPointer += sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps;
		assert(dataPointer < dataPointerEnd);

		capCount = caps.NumberInputValueCaps;
		assert(HidP_GetValueCaps(HidP_Input, valueCaps, &capCount, preparsedData) == HIDP_STATUS_SUCCESS);

		USAGE usageDesc[DeviceState::maxButtons];
		ULONG usageDescCount = rawDevice.buttonCount;
		auto hidResult = HidP_GetUsages(HidP_Input, buttonCaps->UsagePage, 0, usageDesc, &usageDescCount, preparsedData, (PCHAR)rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid);
		assert(hidResult == HIDP_STATUS_SUCCESS);

		DeviceState beforeState = state;
		ZeroMemory(state.buttonStates, sizeof(state.buttonStates[0]) * DeviceState::maxButtons);
		for (ULONG i = 0; i < usageDescCount; i++)
			state.buttonStates[usageDesc[i] - buttonCaps->Range.UsageMin] = true;

		int currentAxisIndex = 0;
		ULONG value;
		for (USHORT i = 0; i < caps.NumberInputValueCaps; i++)
		{
			auto& valueCap = valueCaps[i];
			auto hasUsageValue = HidP_GetUsageValue(HidP_Input, valueCap.UsagePage, 0, valueCap.Range.UsageMin, &value, preparsedData, (PCHAR)rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid);
			assert(hasUsageValue == HIDP_STATUS_SUCCESS);
			auto& axis = state.axis[currentAxisIndex];

			f32 min = valueCap.LogicalMin;
			f32 max = valueCap.LogicalMax;
			assert((max - min) != 0.0f);

			f32 oneOverRange = 1.0f / (max - min);

			UsageType minType = (UsageType)valueCap.Range.UsageMin;
			UsageType maxType = (UsageType)valueCap.Range.UsageMax;
			assert(minType == maxType); // Not necessarily problematic, just need to implement it when we come across it.
			if (minType == UsageType::Hat) // DPad
			{
				state.dpad = value;
			}
			else if (IsValidUsageType(minType))
			{
				axis.min = min * oneOverRange;
				axis.max = max * oneOverRange;
				axis.value = ((f32)value - min) * oneOverRange;
				axis.type = minType;

				currentAxisIndex++;
			}
		}

		rawDevice.axisCount = currentAxisIndex;

		// Check for changes. If found, report new time.
		if (memcmp(&beforeState, &state, sizeof(DeviceState)) != 0)
			rawDevice.lastInputTime = GetTimeSinceStart();
	}

	LRESULT CALLBACK InputWndProc(HWND window, UINT msg, WPARAM w, LPARAM l)
	{
		Leacon_Profiler;
		switch (msg)
		{
		case WM_INPUT:
		{
			u8 buffer[256];
			HRAWINPUT rawInputHandle = (HRAWINPUT)l;
			UINT size = sizeof(buffer);
			assert(GetRawInputData(rawInputHandle, RID_INPUT, &buffer, &size, sizeof(RAWINPUTHEADER)) != ~0);

			const RAWINPUT* rawInputData = (const RAWINPUT*)&buffer;
			const bool isHid = rawInputData->header.dwType == RIM_TYPEHID;
			if (isHid)
				OnRawInput(rawInputData);
		}
		}

		WNDPROC parentProc = GetOriginalWndProcMap()[window];
		if (parentProc != nullptr)
			return CallWindowProc(parentProc, window, msg, w, l);
		return 0;
	}

	void RegisterDevices()
	{
		Leacon_Profiler;
		RAWINPUTDEVICE devices[2];
		devices[0].usUsagePage = 1;
		devices[0].usUsage = 4;
		devices[0].dwFlags = 0;
		devices[0].hwndTarget = 0;

		devices[1].usUsagePage = 1;
		devices[1].usUsage = 5;
		devices[1].dwFlags = 0;
		devices[1].hwndTarget = 0;

		bool hasSucceeded = RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE));
		assert(hasSucceeded);
	}

	void InitRawInput(void* windowPtr)
	{
		Leacon_Profiler;
		static bool g_devicesRegistered = false;
		if (g_devicesRegistered == false)
		{
			RegisterDevices();
			g_devicesRegistered = true;
		}

		static bool g_wndProcHooked = false;
		if (g_wndProcHooked == false && windowPtr != INVALID_HANDLE_VALUE)
		{
			HWND window = (HWND)windowPtr;
			WNDPROC currentWndProc = 0;
			{
				Leacon_ProfilerSection("GetOriginalWndProcMap");
				currentWndProc = (WNDPROC)GetWindowLongPtr(window, GWLP_WNDPROC);
			}
			
			WNDPROC& existingOriginal = GetOriginalWndProcMap()[window];
			if (existingOriginal == nullptr)
			{
				SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)InputWndProc);
				existingOriginal = currentWndProc;
			}
		}
	}

	void DestroyRawInput(void* windowPtr)
	{
		if (g_rawInputUpdateThreadRunning)
		{
			g_rawInputUpdateThreadRunning = false;
			g_rawInputUpdateThread.join();
		}

		for (auto& pair : GetOriginalWndProcMap())
		{
			HWND window = pair.first;
			WNDPROC originalWndProc = pair.second;
			SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
		}
		g_originalWndProcMap->clear();
		g_originalWndProcMap = nullptr;
		
		g_rawDeviceMap.clear();
		g_processedDeviceCount = 0;
		g_deviceCount = 0;
	}

	void ForgetRawInputDevice(HANDLE handle)
	{
		Leacon_Profiler;
		g_rawDeviceMap.erase(handle);
	}

	bool IsRawController(HANDLE handle)
	{
		Leacon_Profiler;
		return g_rawDeviceMap.find(handle) != g_rawDeviceMap.end();
	}

	std::string GetRawControllerName(HANDLE a_Handle)
	{
		Leacon_Profiler;
		if (a_Handle == INVALID_HANDLE_VALUE)
			return "Invalid";

		return g_rawDeviceMap[a_Handle]->name;
	}

	void RemapPlaystation(HANDLE handle, ControllerState& inState, RawDevice& rawDevice)
	{
		Leacon_Profiler;
		for (size_t i = 0; i < rawDevice.axisCount; i++)
		{
			auto& axis = rawDevice.sourceState.axis[i];
			switch (axis.type)
			{
			case UsageType::X:
				inState.leftAnalog.X = (axis.value - 0.5f) * 2.0f; // From 0.0f~1.0f to -1.0f~1.0f
				break;
			case UsageType::Y:
				inState.leftAnalog.Y = (axis.value - 0.5f) * -2.0f; // From 0.0f~1.0f to -1.0f~1.0f (also, invert Y)
				break;

			case UsageType::Z:
				inState.rightAnalog.X = (axis.value - 0.5f) * 2.0f; // From 0.0f~1.0f to -1.0f~1.0f
				break;

			case UsageType::Rz:
				inState.rightAnalog.Y = (axis.value - 0.5f) * -2.0f; // From 0.0f~1.0f to -1.0f~1.0f (also, invert Y)
				break;

			case UsageType::Rx:
				inState.button[(size_t)ControllerInput::ButtonL2] = axis.value;
				break;

			case UsageType::Ry:
				inState.button[(size_t)ControllerInput::ButtonR2] = axis.value;
				break;

			default:
				assert(false);
				break;
			}
		}

		auto& button = rawDevice.sourceState.buttonStates;
		inState.button[(size_t)ControllerInput::ButtonSquare] = button[0] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonCross] = button[1] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonCircle] = button[2] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonTriangle] = button[3] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonL1] = button[4] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonR1] = button[5] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonSelect] = button[8] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonStart] = button[9] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonL3] = button[10] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonR3] = button[11] ? 1.0f : 0.0f;
		inState.button[(size_t)ControllerInput::ButtonHome] = button[12] ? 1.0f : 0.0f;

		auto& dpad = rawDevice.sourceState.dpad;
		switch (dpad)
		{
		case 0: // Up
			inState.leftDPad.X = 0.0f;
			inState.leftDPad.Y = 1.0f;
			break;

		case 1: // UpRight
			inState.leftDPad.X = 1.0f;
			inState.leftDPad.Y = 1.0f;
			break;

		case 2: // Right
			inState.leftDPad.X = 1.0f;
			inState.leftDPad.Y = 0.0f;
			break;

		case 3: // RightDown
			inState.leftDPad.X = 1.0f;
			inState.leftDPad.Y = -1.0f;
			break;

		case 4: // Down
			inState.leftDPad.X = 0.0f;
			inState.leftDPad.Y = -1.0f;
			break;

		case 5: // DownLeft
			inState.leftDPad.X = -1.0f;
			inState.leftDPad.Y = -1.0f;
			break;

		case 6: // Left
			inState.leftDPad.X = -1.0f;
			inState.leftDPad.Y = 0.0f;
			break;

		case 7: // LeftUp
			inState.leftDPad.X = -1.0f;
			inState.leftDPad.Y = 1.0f;
			break;

		default: // None (8 on real devices, random on knockoffs)
			inState.leftDPad.X = 0.0f;
			inState.leftDPad.Y = 0.0f;
			break;
		};
	}

	void UpdateAxisTimes(RawDevice& inDevice, ControllerState& inState)
	{
		Leacon_Profiler;
		DirectionalInputState* states[] = { &inState.leftAnalog, &inState.leftDPad, &inState.rightAnalog };
		DirectionalInputState* prevStates[] = { &inDevice.prevLeftAnalog, &inDevice.prevLeftDPad, &inDevice.prevRightAnalog };
		for (int i = 0; i < 3; i++)
			*prevStates[i] = *states[i];
	}

	void ControllerHandlerRawInput::UpdateRawController(HANDLE handle, ControllerState& inState)
	{
		Leacon_Profiler;
		auto& data = g_rawDeviceMap[handle];
		if (data == nullptr)
			return;

		auto& hid = data->deviceInfo.hid;

		if (hid.dwVendorId == 0x54C)
		{
			if (hid.dwProductId == 0xCE6 ||	// PS5 Controller
				hid.dwProductId == 0x5C4)	// PS4 Controller
			{
				RemapPlaystation(handle, inState, *data);
				UpdateAxisTimes(*data, inState);
				return;
			}
			else
			{
				assert(false); // Another playstation controller?
			}
		}

		if (data->buttonCount >= 13) // TODO: Maybe more verification? Check for axes?
		{
			RemapPlaystation(handle, inState, *data); // yolo lmao
			UpdateAxisTimes(*data, inState);
		}
	}

	static void UpdateRawDeviceCount()
	{
		Leacon_Profiler;
		UINT deviceCount = 0;

		std::vector<RAWINPUTDEVICELIST> deviceList;
		bool canReceiveDeviceList = Leacon_ProfilerEvalRet(GetRawInputDeviceList(NULL, &deviceCount, sizeof(RAWINPUTDEVICELIST))) == 0;
		assert(canReceiveDeviceList);
		if (canReceiveDeviceList == false)
			return;

		g_deviceCount = deviceCount;
	}

	static void UpdateRawDeviceCountThread()
	{
		using namespace std::chrono_literals;
		Leacon_Profiler;

		while (g_rawInputUpdateThreadRunning)
		{
			UpdateRawDeviceCount();
			std::this_thread::sleep_for(2000ms);
		}
	}

	void ControllerHandlerRawInput::UpdateRawControllerConnected()
	{
		Leacon_Profiler;
		static bool g_fetchedInitialCount = false;
		if (g_fetchedInitialCount == false)
		{
			UpdateRawDeviceCount();
			g_rawInputUpdateThread = std::thread(UpdateRawDeviceCountThread);
			g_rawInputUpdateThread.detach();
			g_rawInputUpdateThreadRunning = true;
			g_fetchedInitialCount = true;
		}

		if (g_deviceCount == g_processedDeviceCount)
			return;

		UINT deviceCount = g_deviceCount;
		std::vector<RAWINPUTDEVICELIST> deviceList(deviceCount);
		if (GetRawInputDeviceList(deviceList.data(), &deviceCount, sizeof(RAWINPUTDEVICELIST)) == -1)
			return;

		std::map<HANDLE, bool> foundDevices;
		std::vector<HANDLE> newDevices;
		char deviceHandleName[1024];
		wchar_t deviceName[127];
		int failedDevices = 0;
		for (int i = 0; i < deviceCount; i++)
		{
			// Get Device info
			RID_DEVICE_INFO ridInfo;
			ridInfo.cbSize = sizeof(RID_DEVICE_INFO);
			UINT dataSize = sizeof(RID_DEVICE_INFO);
			bool deviceInfoSucceeded = GetRawInputDeviceInfo(deviceList[i].hDevice, RIDI_DEVICEINFO, &ridInfo, &dataSize) >= 0;
			assert(deviceInfoSucceeded);
			if (deviceInfoSucceeded == false)
				continue;

			if (ridInfo.hid.usUsage != 4 && ridInfo.hid.usUsage != 5) // Needs to be a joystick or a gamepad
				continue;

			if (m_manager.GetXInputHandler().HasXInputSupport(ridInfo.hid.dwVendorId, ridInfo.hid.dwProductId))
				continue;

			// Initialise local data
			foundDevices[deviceList[i].hDevice] = true;
			auto& deviceStateManager = g_rawDeviceMap[deviceList[i].hDevice];
			if (deviceStateManager == nullptr)
			{
				// Get device handle name (internal name to get HID product)
				dataSize = 1023;
				ZeroMemory(deviceHandleName, sizeof(char) * 1024);
				bool gotDeviceName = GetRawInputDeviceInfo(deviceList[i].hDevice, RIDI_DEVICENAME, &deviceHandleName, &dataSize) >= 1;
				assert(gotDeviceName);
				if (gotDeviceName == false)
					continue;

				// Get product name
				ZeroMemory(deviceName, sizeof(wchar_t) * 127);
				HANDLE hidHandle = CreateFile(deviceHandleName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
				BOOLEAN result = !!hidHandle;
				if (hidHandle)
				{
					result = HidD_GetProductString(hidHandle, deviceName, sizeof(wchar_t) * 126);
					// assert(result);
					CloseHandle(hidHandle);
				}

				if (result == false)
				{
					failedDevices++;
					continue;
				}

				static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> stringConverter;
				std::string name = stringConverter.to_bytes(deviceName);

				deviceStateManager = std::make_shared<RawDevice>();
				deviceStateManager->controller.SetName(name.c_str());
				deviceStateManager->deviceInfo = ridInfo;
				newDevices.push_back(deviceList[i].hDevice);
			}
		}
		g_processedDeviceCount = deviceCount - failedDevices;

		// Check what devices are no longer in the device list
		for (auto& existingDevicePair : g_rawDeviceMap)
		{
			if (foundDevices[existingDevicePair.first] || existingDevicePair.second == nullptr) // If we found this device connected, it's fine
				continue;

			std::shared_ptr<RawDevice>& deviceInfo = g_rawDeviceMap[existingDevicePair.first];
			m_manager.ControllerHasDisconnected(deviceInfo->controller);
			deviceInfo = nullptr; // Delete device info
		}

		// Notify of new devices
		for (auto& newDevice : newDevices)
		{
			std::shared_ptr<RawDevice>& deviceInfo = g_rawDeviceMap[newDevice];
			m_manager.ControllerHasConnected(deviceInfo->controller);
		}
	}

	HANDLE GetRawController(LONG vendorId, LONG productId)
	{
		Leacon_Profiler;
		for (auto& device : g_rawDeviceMap)
			if (device.second->deviceInfo.hid.dwVendorId == vendorId && device.second->deviceInfo.hid.dwProductId == productId)
				return device.first;

		return INVALID_HANDLE_VALUE;
	}

	ControllerHandlerRawInput::ControllerHandlerRawInput(ControllerManager& manager) :
		m_manager(manager)
	{
	}

	void ControllerHandlerRawInput::Update(void* window)
	{
		Leacon_Profiler;
		InitRawInput(window);
		UpdateRawControllerConnected();

		for (auto deviceInfoPair : g_rawDeviceMap)
		{
			if (deviceInfoPair.second == nullptr)
				continue;

			UpdateRawController(deviceInfoPair.first, deviceInfoPair.second->controller.SwapAndGetCurrentState());

			if (deviceInfoPair.second->lastInputTime > deviceInfoPair.second->lastReportTime)
			{
				deviceInfoPair.second->lastReportTime = GetTimeSinceStart();
				m_manager.ControllerHasInput(deviceInfoPair.second->controller);
			}
		}
	}
}

#endif
