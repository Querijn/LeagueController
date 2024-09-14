#include <game_overlay/overlay.hpp>
#include <game_overlay/window.hpp>
#include <game_overlay/imgui_memory_editor.hpp>
#include <game_overlay/input.hpp>

#include <league_controller/controller_manager.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller.hpp>

#include <league_lib/navgrid/navgrid.hpp>
#include <league_lib/wad/wad_filesystem.hpp>
#include <memory_helper/address.hpp>

#include <league_internals/game_object.hpp>
#include <league_internals/offsets.hpp>

#include "script.hpp"
#include "setup.hpp"
#include <glm/glm.hpp>

#include <Windows.h>
#include <TlHelp32.h>
#include "imgui.h"
#include <d3d11.h>
#include <functional>
#include <sstream>
#include <set>
#include "controller.hpp"
#include "script_controller.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/norm.hpp>

#if !LEACON_SUBMISSION
#define IF_DEBUG(a) a
#else
#define IF_DEBUG(a)  
#endif

using namespace MemoryHelper;

namespace LeagueController
{
	using Duration = Spek::Duration;
	using namespace Spek;

	class BaseInputEvent;
	
	static float g_deadZone = 0.2f;
	static f32 g_pixelsMovedPerSecond = 5000.0f;
	static f32 g_isDownOffset = 0.2f;

	glm::vec2 g_currentMousePos;
	glm::vec2 g_targetMousePos;
	static bool g_moveMouse = false;
	static bool g_shouldResetControllerState = true;
	GameOverlay::LogCategory g_controllerLog("Controller");

	struct ClickMouseButtonEvent : public BaseInputEvent
	{
		ClickMouseButtonEvent(Duration delay, MouseButton inButton);

		BaseInputEvent::ExecutionState Execute() override;
		std::string ToString() const override;

		static bool HasEvent()
		{
			return Event[MouseButton::Left] != nullptr ||
				Event[MouseButton::Right] != nullptr;
		}

		MouseButton button;
		bool isDown = false;

		static std::map<MouseButton, std::shared_ptr<ClickMouseButtonEvent>> Event;
	};
	std::map<MouseButton, std::shared_ptr<ClickMouseButtonEvent>> ClickMouseButtonEvent::Event;

	struct PressKeyEvent : public BaseInputEvent
	{
		PressKeyEvent(Duration delay, Keyboard inKey, const glm::ivec2* mousePos = nullptr);

		BaseInputEvent::ExecutionState Execute() override;
		std::string ToString() const override;

		void AddRelease() { shouldRelease = true; }

		INPUT m_input = { 0 };
		WORD key;
		glm::ivec2 mousePos;
		bool isDown = false;
		bool shouldRelease = false;

		static std::map<Keyboard, std::shared_ptr<PressKeyEvent>> Event;
	};
	std::map<Keyboard, std::shared_ptr<PressKeyEvent>> PressKeyEvent::Event;

	class LeagueControllerListener : LeagueController::ControllerListener
	{
	public:
		LeagueController::ControllerState state;
		const LeagueController::Controller* lastController = nullptr;

		LeagueControllerListener(LeagueController::ControllerManager& manager) :
			ControllerListener(manager)
		{}

		void OnControllerConnected(const LeagueController::Controller & controller) override
		{
			GameOverlay_LogDebug(g_controllerLog, "A controller just connected: %s", controller.GetName());
		}

		void OnControllerDisconnected(const LeagueController::Controller& controller) override
		{
			GameOverlay_LogDebug(g_controllerLog, "A controller disconnected: %s", controller.GetName());
		}

		virtual void OnInput(const LeagueController::Controller& controller) 
		{
			Leacon_Profiler;

			if (lastController != &controller && HasInput(controller.GetCurrentState()))
			{
				GameOverlay_LogDebug(g_controllerLog, "Changed current controller to %s (%x)", controller.GetName(), &controller);
				lastController = &controller;
			}
			
			if (lastController == &controller)
			{
				if (g_shouldResetControllerState)
				{
					state = ControllerState();
					g_shouldResetControllerState = false;
				}

				MergeWithState(controller.GetCurrentState());
			}
		}

	private:
		static bool HasInput(const DirectionalInputState& inputState)
		{
			Leacon_Profiler;
			return abs(inputState.X) > g_deadZone && abs(inputState.Y) > g_deadZone;
		}

		static bool HasInput(const ControllerState& newState)
		{
			Leacon_Profiler;
			if (HasInput(newState.leftAnalog) ||
				HasInput(newState.leftDPad) ||
				HasInput(newState.rightAnalog))
				return true;

			for (int i = 0; i < (int)ControllerInput::InputCount; i++)
				if (newState.button[i] > 0.5f)
					return true;

			return false;
		}

		void MergeWithState(const LeagueController::ControllerState& newState)
		{
			Leacon_Profiler;
			for (int i = 0; i < (int)ControllerInput::InputCount; i++)
				if (state.button[i] < newState.button[i])
					state.button[i] = newState.button[i];

			MergeWithState(state.leftAnalog, newState.leftAnalog);
			MergeWithState(state.leftDPad, newState.leftDPad);
			MergeWithState(state.rightAnalog, newState.rightAnalog);

			if (state.timestamp < newState.timestamp)
				state.timestamp = newState.timestamp;
		}

		static void MergeWithState(LeagueController::DirectionalInputState& state, const LeagueController::DirectionalInputState& newState)
		{
			Leacon_Profiler;
			state.X += newState.X;
			state.Y += newState.Y;
			
			if (state.X > 1.0f)
				state.X = 1.0f;
			if (state.X < -1.0f)
				state.X = -1.0f;

			if (state.Y > 1.0f)
				state.Y = 1.0f;
			if (state.Y < -1.0f)
				state.Y = -1.0f;
		}
	};

	struct ControllerData
	{
		ControllerData() : listener(controllers) {}

		LeagueController::ControllerManager controllers;
		LeagueControllerListener listener;
		
	} g_controllerData;

	glm::vec2 WorldToScreen(const glm::vec3& worldCoords)
	{
		Leacon_Profiler;
		glm::mat4* viewMatrix = GetViewMatrix();
		glm::mat4* projMatrix = GetProjMatrix();
		if (viewMatrix == nullptr || projMatrix == nullptr)
			return glm::vec2();

		// Transform the world coordinates into clip coordinates
		glm::vec4 clipCoords = *projMatrix * *viewMatrix * glm::vec4(worldCoords, 1.0f);
		if (clipCoords.w < 1.0f)
			clipCoords.w = 1.0f;

		// Homogenize the clip coordinates into normalized device coordinates
		glm::vec2 ndcCoords = glm::vec2(clipCoords.x, clipCoords.y) / clipCoords.w;

		// Transform the normalized device coordinates into window coordinates
		glm::vec2 screenCoords;
		screenCoords.x = (ndcCoords.x + 1.0f) * 0.5f * GameOverlay::GetWidth();
		screenCoords.y = (1.0f - (ndcCoords.y + 1.0f) * 0.5f) * GameOverlay::GetHeight();
		return screenCoords;
	}

	glm::vec2 GetMouseTarget()
	{
		Leacon_Profiler;
		return g_targetMousePos;
	}

	void SetMouseTarget(const glm::vec2& mousePos, bool shouldMove)
	{
		Leacon_Profiler;
		if (ClickMouseButtonEvent::HasEvent())
		{
  			GameOverlay_LogWarning(g_controllerLog, "Wanted to set mouse position from (%.0f, %.0f) to (%.0f, %.0f), but couldn't, because there was a mouse event active.",
				g_targetMousePos.x, g_targetMousePos.y,
				mousePos.x, mousePos.y
			);
			return;
		}

		g_targetMousePos = mousePos;
		g_moveMouse = shouldMove;

		// Cache our current position if it isn't set explicitly
		if (shouldMove == false)
		{
			POINT p;
			HWND window = (HWND)GameOverlay::GetWindow();
			if (window == nullptr)
				return;
			
			if (::GetCursorPos(&p) == false)
				return;

			if (::ScreenToClient(window, &p))
			{
				g_currentMousePos.x = p.x;
				g_currentMousePos.y = p.y;

				/*GameOverlay_LogDebug(g_controllerLog, "Reset mouse position to %.0f, %.0f",
					g_currentMousePos.x, g_currentMousePos.y
				);*/
			}
		}
	}

	void SetInstantMouseTarget(const glm::vec2& mousePos)
	{
		Leacon_Profiler;
		if (ClickMouseButtonEvent::HasEvent())
		{
			GameOverlay_LogWarning(g_controllerLog, "Wanted to set mouse position (instantly) from (%.0f, %.0f) to (%.0f, %.0f), but couldn't, because there was a mouse event active.", 
				g_targetMousePos.x, g_targetMousePos.y,
				mousePos.x, mousePos.y
			);
			return;
		}

		g_targetMousePos = mousePos;
		g_currentMousePos = mousePos;
		g_moveMouse = true;
	}

	void FeignMouseClick(MouseButton button)
	{
		Leacon_Profiler;
		ClickMouseButtonEvent::Event[button] = std::make_shared<ClickMouseButtonEvent>(0_ms, button);
		GameOverlay_LogDebug(g_controllerLog, "Added mouse event: button %d", button);
	}

	void FeignKeyPress(Keyboard key)
	{
		Leacon_Profiler;
		PressKeyEvent::Event[key] = std::make_shared<PressKeyEvent>(0_ms, key);
		GameOverlay_LogDebug(g_controllerLog, "Added Keyboard Press event: key %d", key);
	}

	void FeignKeyPressOnMouse(Keyboard key, const glm::vec2& mousePos)
	{
		Leacon_Profiler;
		glm::ivec2 dummy = mousePos;
		PressKeyEvent::Event[key] = std::make_shared<PressKeyEvent>(0_ms, key, &dummy);
		GameOverlay_LogDebug(g_controllerLog, "Added Keyboard Press (With Mouse) event: key %d, mouse %d, %d", key, dummy.x, dummy.y);
	}

	void FeignKeyRelease(Keyboard key)
	{
		Leacon_Profiler;
		if (PressKeyEvent::Event[key] == nullptr)
		{
			GameOverlay_LogWarning(g_controllerLog, "Could not add Keyboard Release event for key %d, event did not exist", key);
			return;
		}

		PressKeyEvent::Event[key]->AddRelease();
		GameOverlay_LogDebug(g_controllerLog, "Added Keyboard Release event: key %d", key);
	}

	void CenterCamera(bool inCenterCamera)
	{
		Leacon_Profiler;
		static bool isCameraCentered = false;
		if (isCameraCentered == inCenterCamera)
			return;

		FeignKeyPress(Keyboard::KeyY);
		FeignKeyRelease(Keyboard::KeyY);
		isCameraCentered = inCenterCamera;
	}

	bool g_isMeasuring = false;
	u64 g_totalPixelsMeasured = 0;
	Duration g_measurementStartTime;

	void EmulateMouseMovement(Duration deltaTime)
	{
		Leacon_Profiler;
		static Duration lastTime = GetTimeSinceStart();
		if (IsLizardModeEnabled())
		{
			lastTime = GetTimeSinceStart();
			g_isMeasuring = false;
			return;
		}

		if (g_moveMouse == false || GameOverlay::IsWindowFocussed() == false)
		{
			lastTime = GetTimeSinceStart();
			g_isMeasuring = false;
			return;
		}

		int screenX, screenY;
		if (GameOverlay::GetClientGlobalPosition(screenX, screenY) == false)
		{
			lastTime = GetTimeSinceStart();
			g_isMeasuring = false;
			GameOverlay_LogDebug(g_controllerLog, "Unable to determine the window position, cannot move mouse!");
			return;
		}

		if (g_isMeasuring == false)
		{
			g_isMeasuring = true;
			g_totalPixelsMeasured = 0;
			g_measurementStartTime = GetTimeSinceStart();
		}

		glm::vec2 delta = g_targetMousePos - g_currentMousePos;
		if (ClickMouseButtonEvent::HasEvent() == false && // No click event is currently active
			glm::length2(delta) > 5.0f * 5.0f && // Over 5 pixels off, move towards target
			glm::length2(delta) < 400.0f * 400.0f) // Over 400 pixels off, force pos instead
			// TODO: Make < 400.0f a little more scientific (window size?)
		{
			f32 currentMouseSpeed = g_pixelsMovedPerSecond * (f32)deltaTime.ToSecF64();
			glm::vec2 dir = glm::normalize(delta);
			glm::vec2 displacement = dir * currentMouseSpeed;

			IF_DEBUG(bool didOvershoot = false);
			if (glm::length2(displacement) > glm::length2(delta)) // If we'd overshoot, take the difference
			{
				displacement = delta;
				IF_DEBUG(didOvershoot = true);
			}

			g_currentMousePos += displacement;
			
			if (g_currentMousePos.x < 0.0f)
				g_currentMousePos.x = 0.0f;
			else if (g_currentMousePos.x > GameOverlay::GetWidth())
				g_currentMousePos.x = GameOverlay::GetWidth() - 1;

			if (g_currentMousePos.y < 0.0f)
				g_currentMousePos.y = 0.0f;
			else if (g_currentMousePos.y > GameOverlay::GetHeight())
				g_currentMousePos.y = GameOverlay::GetHeight() - 1;

			SetCursorPos(g_currentMousePos.x + screenX, g_currentMousePos.y + screenY);

			f32 pixelsMoved = (f32)(u64)glm::length(displacement);
			g_totalPixelsMeasured += pixelsMoved;
			f64 totalTimeMeasuring = (GetTimeSinceStart() - g_measurementStartTime).ToSecF64();
			f64 avgPixelsPerSec = g_totalPixelsMeasured / totalTimeMeasuring;

			/*GameOverlay_LogDebug(g_controllerLog, "Displacement %.3f, %.3f. DT = %2.6f sec, mouse speed = %.2f, dist = %.2f, pixels moved = %.2f, pixels per sec = %.0f %s",
				displacement.x, displacement.y,
				deltaTime.ToSecF64(), currentMouseSpeed, glm::length(delta), pixelsMoved, avgPixelsPerSec,
				didOvershoot ? "(overshot)" : ""
			);*/
		}
		else
		{
			g_currentMousePos = g_targetMousePos;
			SetCursorPos(g_currentMousePos.x + screenX, g_currentMousePos.y + screenY);
			g_moveMouse = false;
			/*GameOverlay_LogDebug(g_controllerLog, "Forced mouse on %.0f, %.0f. DT = %2.6f sec, Dist = %.2f", 
				g_currentMousePos.x, g_currentMousePos.y, 
				deltaTime.ToSecF64(), glm::length(delta)
			);*/
		}
	}

	template<typename Key, typename T>
	void ExecuteEvent(std::map<Key, std::shared_ptr<T>>& map)
	{
		Leacon_Profiler;
		for (std::pair<const Key, std::shared_ptr<T>>& pair : map)
		{
			std::shared_ptr<T>& e = pair.second;
			if (e == nullptr)
				continue;

			BaseInputEvent::ExecutionState state = e->BaseExecute();
			if (state == BaseInputEvent::ExecutionState::DoneExecuting)
				e = nullptr;
		}
	}

	static void HandleInputEvents()
	{
		Leacon_Profiler;
	}

	void UpdateControllers(Duration deltaTime, void* window)
	{
		Leacon_Profiler;
		g_shouldResetControllerState = true;
		g_controllerData.controllers.Update(window);
		EmulateMouseMovement(deltaTime);

		ExecuteEvent(ClickMouseButtonEvent::Event);
		ExecuteEvent(PressKeyEvent::Event);
	}
	
	inline bool BuildAnalogDir(glm::vec3& dir, const DirectionalInputState& analogInput, bool make3D, bool normalize)
	{
		Leacon_Profiler;
		glm::vec3 controllerDir(
			analogInput.X,
			make3D ? 0 : analogInput.Y,
			make3D ? analogInput.Y : 0
		);
		f32 deadZone = GetSettings().serialisedSettings.controller.deadZone;

		f32 length = glm::length(controllerDir);
		if (length <= deadZone) // If we're below the deadzone, exit out
			return false;

		f32 oneOverLength = 1.0f / length;
		if (normalize)
		{
			dir = controllerDir * oneOverLength;
			GameOverlay_Assert(g_controllerLog, glm::length(dir - glm::normalize(controllerDir)) < 0.02f);
			return true;
		}

		f32 newLength = (glm::clamp(length, 0.0f, 1.0f) - g_deadZone) / (1.0f - g_deadZone);
		dir = (controllerDir * oneOverLength) * newLength;
		GameOverlay_Assert(g_controllerLog, glm::length(dir) <= 1.001f);
		return true;
	}

	bool GetLeftAnalogDir(glm::vec3& dir, bool make3D, bool normalize)
	{
		Leacon_Profiler;
		return BuildAnalogDir(dir, g_controllerData.listener.state.leftAnalog, make3D, normalize);
	}

	bool GetRightAnalogDir(glm::vec3& dir, bool make3D, bool normalize)
	{
		Leacon_Profiler;
		return BuildAnalogDir(dir, g_controllerData.listener.state.rightAnalog, make3D, normalize);
	}

	const ControllerState& GetState() { return g_controllerData.listener.state; }
	
	BaseInputEvent::BaseInputEvent(Type inType, Duration delay) :
		type(inType),
		time(GetTimeSinceStart() + delay)
	{}

	BaseInputEvent::ExecutionState BaseInputEvent::BaseExecute()
	{
		Leacon_Profiler;
		bool hasExecuted = state == ExecutionState::DoneExecuting;
		if (hasExecuted || GetTimeSinceStart() < time)
		{
			GameOverlay_LogDebug(g_controllerLog, "Event has executed: %s, time left: %f", hasExecuted ? "true" : "false", time - GetTimeSinceStart());
			return ExecutionState::Errorous;
		}

		state = Execute();
		return state;
	}

	void BaseInputEvent::SetDelay(Duration delay) { time = GetTimeSinceStart() + delay; }

	ClickMouseButtonEvent::ClickMouseButtonEvent(Duration delay, MouseButton inButton) :
		BaseInputEvent(BaseInputEvent::Type::MouseClick, delay),
		button(inButton)
	{
	}

	BaseInputEvent::ExecutionState ClickMouseButtonEvent::Execute()
	{
		Leacon_Profiler;
		INPUT input = { 0 };
		if (isDown == false) // First phase, press mouse button
		{
			GameOverlay_LogDebug(g_controllerLog, "Pressing mouse button: %d", button);

			::ZeroMemory(&input, sizeof(INPUT));
			input.type = INPUT_MOUSE;
			input.mi.dwFlags = button == MouseButton::Left ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
			::SendInput(1, &input, sizeof(INPUT));

			// Delay second phase by 16 ms
			isDown = true;
			SetDelay(16_ms);
			return ExecutionState::IsBusy;
		}
		else // Second phase, release mouse button
		{
			GameOverlay_LogDebug(g_controllerLog, "Releasing mouse button: %d", button);

			::ZeroMemory(&input, sizeof(INPUT));
			input.type = INPUT_MOUSE;
			input.mi.dwFlags = button == MouseButton::Left ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
			::SendInput(1, &input, sizeof(INPUT));

			return ExecutionState::DoneExecuting;
		}
	}

	std::string ClickMouseButtonEvent::ToString() const
	{
		Leacon_Profiler;
		std::string result;
		if (button == MouseButton::Left)
			result = "Left mouse button, ";
		else if (button == MouseButton::Right)
			result = "Right mouse button, ";

		if (isDown)
			result += "currently pressed";
		else
			result += "not pressed";
		return result;
	}

	PressKeyEvent::PressKeyEvent(Duration delay, Keyboard inKey, const glm::ivec2* inMousePos) :
		BaseInputEvent(BaseInputEvent::Type::KeyPress, delay),
		key(inKey),
		mousePos(inMousePos ? *inMousePos : glm::ivec2(-1,-1))
	{
	}

	BaseInputEvent::ExecutionState PressKeyEvent::Execute()
	{
		Leacon_Profiler;
		if (GameOverlay::IsWindowFocussed() == false)
			return ExecutionState::IsBusy;

		if (!isDown)
		{
			// Check if our event has a mouse pos
			if (mousePos.x >= 0)
			{
				int screenX, screenY;
				GameOverlay_Assert(g_controllerLog, GameOverlay::GetClientGlobalPosition(screenX, screenY));
				SetCursorPos(mousePos.x + screenX, mousePos.y + screenY);
			}

			// GameOverlay_LogDebug(g_controllerLog, "Pressing key: %s", ToString().c_str());
			::ZeroMemory(&m_input, sizeof(INPUT));
			::SendInput(1, &m_input, sizeof(INPUT));
			m_input.type = INPUT_KEYBOARD;
			m_input.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);
			m_input.ki.time = 0;
			m_input.ki.dwExtraInfo = 0;
			m_input.ki.wVk = key;
			m_input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
			SendInput(1, &m_input, sizeof(INPUT));

			isDown = true;
			return ExecutionState::IsBusy;
		}
		else if (shouldRelease)
		{
			// GameOverlay_LogDebug(g_controllerLog, "Releasing key: %s", ToString().c_str());
			m_input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &m_input, sizeof(INPUT));

			return ExecutionState::DoneExecuting;
		}

		return ExecutionState::IsBusy;
	}

	std::string PressKeyEvent::ToString() const
	{
		Leacon_Profiler;
		char text[64];
		GetKeyNameText(MapVirtualKey(key, MAPVK_VK_TO_VSC) << 16, text, 64);
		return std::string("Pressing ") + text;
	}

	const ControllerState& GetControllerState()
	{
		return g_controllerData.listener.state;
	}
	
	bool IsButtonDown(ControllerInput button, const ControllerState* state)
	{
		Leacon_Profiler;
		if (state == nullptr)
			state = &GetControllerState();

		return state->GetButtonState(button) >= g_isDownOffset;
	}
	bool IsButtonUp(ControllerInput button, const ControllerState* state)
	{
		Leacon_Profiler;
		if (state == nullptr)
			state = &GetControllerState();

		return state->GetButtonState(button) < g_isDownOffset;
	}

	bool IsButtonDown(DirectionalInputType button, InputDirection direction, const ControllerState* state)
	{
		Leacon_Profiler;
		if (state == nullptr)
			state = &GetControllerState();
		
		return state->GetButtonState(button, direction) >= g_isDownOffset;
	}

	bool IsButtonUp(DirectionalInputType button, InputDirection direction, const ControllerState* state)
	{
		Leacon_Profiler;
		if (state == nullptr)
			state = &GetControllerState();

		return state->GetButtonState(button, direction) < g_isDownOffset;
	}

	ControllerManager& GetControllerManager()
	{
		return g_controllerData.controllers;
	}
}