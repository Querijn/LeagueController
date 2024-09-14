#pragma once

#include <league_controller/types.hpp>
#include <spek/util/duration.hpp>
#include <league_controller/controller_input.hpp>

namespace LeagueController
{
	using Duration = Spek::Duration;
	struct DirectionalInputState
	{
		f32 X;
		f32 Y;
	};

	enum class DirectionalInputType
	{
		LeftAnalog,
		RightAnalog,
		DPad
	};

	enum class InputDirection
	{
		Up,
		Right,
		Down,
		Left,
	};

	struct ControllerState
	{
		f32 button[(u32)ControllerInput::InputCount];

		DirectionalInputState leftAnalog;
		DirectionalInputState leftDPad;
		DirectionalInputState rightAnalog;

		Duration timestamp;

		f32 GetButtonState(ControllerInput input) const;
		f32 GetButtonState(DirectionalInputType inputType, InputDirection direction) const;
	};

	ControllerInput GetControllerInputFromDirectionalInput(DirectionalInputType inputType, InputDirection direction);

	class ControllerManager;
	class Controller
	{
	public:
		~Controller();

		ControllerState& SwapAndGetCurrentState();

		const ControllerState& GetCurrentState() const;
		const ControllerState& GetPreviousState() const;

		bool HasCurrentState() const;
		bool HasPreviousState() const;

		const char* GetName() const;
		void SetName(const char* name, size_t length = ~0);

	private:
		char* m_name = nullptr;

		ControllerState m_states[2];
		ControllerState* m_current = nullptr;
		ControllerState* m_previous = nullptr;
	};
}
