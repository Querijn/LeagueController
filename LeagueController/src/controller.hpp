#pragma once

#include <game_overlay/simple_input.hpp>

#include <spek/util/types.hpp>
#include <spek/util/duration.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <string>

namespace LeagueController
{
	using Duration = Spek::Duration;
	struct ControllerState;
	class ControllerManager;

	class BaseInputEvent
	{
	public:
		enum class Type
		{
			MouseClick,
			KeyPress
		};

		enum class ExecutionState
		{
			Errorous,
			NotStarted,
			IsBusy,
			DoneExecuting
		};

		BaseInputEvent(Type inType, Duration delay);

		ExecutionState BaseExecute();

		void SetDelay(Duration delay);

		virtual std::string ToString() const = 0;

		Type GetType() const { return type; }

		// Should be executed by BaseExecute
	protected:
		virtual ExecutionState Execute() = 0;

	private:
		Type type;
		Duration time;
		ExecutionState state = ExecutionState::NotStarted;
	};

	using Keyboard = GameOverlay::KeyboardKey;
	using MouseButton = GameOverlay::MouseButton;
	enum class ControllerInput;
	enum class DirectionalInputType;
	enum class InputDirection;

	void UpdateControllers(Duration deltaTime, void* window);
	ControllerManager& GetControllerManager();

	const ControllerState& GetControllerState();
	bool IsButtonDown(ControllerInput button, const ControllerState* state = nullptr);
	bool IsButtonUp(ControllerInput button, const ControllerState* state = nullptr);
	bool IsButtonDown(DirectionalInputType directional, InputDirection direction, const ControllerState* state = nullptr);
	bool IsButtonUp(DirectionalInputType directional, InputDirection direction, const ControllerState* state = nullptr);
	
	bool GetLeftAnalogDir(glm::vec3& dir, bool make3D = false, bool normalize = false);
	bool GetRightAnalogDir(glm::vec3& dir, bool make3D = false, bool normalize = false);
	glm::vec2 WorldToScreen(const glm::vec3& pos);
	glm::vec2 GetMouseTarget();
	void SetMouseTarget(const glm::vec2& mousePos, bool shouldMove);
	void SetInstantMouseTarget(const glm::vec2& mousePos);
	void FeignMouseClick(MouseButton button);
	void FeignKeyPressOnMouse(Keyboard key, const glm::vec2& mousePos);
	void FeignKeyPress(Keyboard key);
	void FeignKeyRelease(Keyboard key);
	void CenterCamera(bool inCenterCamera);
}
