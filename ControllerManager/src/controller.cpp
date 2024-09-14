#include <league_controller/controller.hpp>

#include <cassert>

namespace LeagueController
{
	Controller::~Controller()
	{
		if (m_name)
		{
			delete[] m_name;
			m_name = nullptr;
		}
	}

	ControllerState& Controller::SwapAndGetCurrentState()
	{
		if (m_current == nullptr)
		{
			m_current = &m_states[0];
		}
		else
		{
			m_previous = m_current;
			m_current = m_current == &m_states[1] ? &m_states[0] : &m_states[1];
		}

		return *m_current;
	}

	const ControllerState& Controller::GetCurrentState() const
	{
		assert(m_current);
		return *m_current;
	}

	const ControllerState& Controller::GetPreviousState() const
	{
		assert(m_previous);
		return *m_previous;
	}

	bool Controller::HasCurrentState() const
	{
		return !!m_current;
	}
	
	bool Controller::HasPreviousState() const
	{
		return !!m_previous;
	}

	const char* Controller::GetName() const
	{
		return m_name;
	}
	
	void Controller::SetName(const char* name, size_t length)
	{
		if (m_name)
			delete[] m_name;

		if (length == ~0)
			length = strlen(name) + 1;

		m_name = new char[length];
		memcpy(m_name, name, length);
	}

	f32 ControllerState::GetButtonState(ControllerInput input) const
	{
		switch (input)
		{
		case ControllerInput::ButtonDPadUp:				return GetButtonState(DirectionalInputType::DPad, InputDirection::Up);
		case ControllerInput::ButtonDPadDown:			return GetButtonState(DirectionalInputType::DPad, InputDirection::Down);
		case ControllerInput::ButtonDPadRight:			return GetButtonState(DirectionalInputType::DPad, InputDirection::Right);
		case ControllerInput::ButtonDPadLeft:			return GetButtonState(DirectionalInputType::DPad, InputDirection::Left);

		case ControllerInput::ButtonLeftAnalogUp:		return GetButtonState(DirectionalInputType::LeftAnalog, InputDirection::Up);
		case ControllerInput::ButtonLeftAnalogDown:		return GetButtonState(DirectionalInputType::LeftAnalog, InputDirection::Down);
		case ControllerInput::ButtonLeftAnalogRight:	return GetButtonState(DirectionalInputType::LeftAnalog, InputDirection::Right);
		case ControllerInput::ButtonLeftAnalogLeft:		return GetButtonState(DirectionalInputType::LeftAnalog, InputDirection::Left);

		case ControllerInput::ButtonRightAnalogUp:		return GetButtonState(DirectionalInputType::RightAnalog, InputDirection::Up);
		case ControllerInput::ButtonRightAnalogDown:	return GetButtonState(DirectionalInputType::RightAnalog, InputDirection::Down);
		case ControllerInput::ButtonRightAnalogRight:	return GetButtonState(DirectionalInputType::RightAnalog, InputDirection::Right);
		case ControllerInput::ButtonRightAnalogLeft:	return GetButtonState(DirectionalInputType::RightAnalog, InputDirection::Left);
		}
			
		return button[(int)input];
	}

	f32 GetDirInputState(const DirectionalInputState& state, InputDirection direction)
	{
		switch (direction)
		{
		case InputDirection::Up:	return state.Y > 0 ?  state.Y : 0;
		case InputDirection::Down:	return state.Y < 0 ? -state.Y : 0;

		case InputDirection::Right:	return state.X > 0 ?  state.X : 0;
		case InputDirection::Left:	return state.X < 0 ? -state.X : 0;
		}
		
		return 0.0f;
	}

	f32 ControllerState::GetButtonState(DirectionalInputType inputType, InputDirection direction) const
	{
		switch (inputType)
		{
		case DirectionalInputType::DPad: return GetDirInputState(leftDPad, direction);
		case DirectionalInputType::LeftAnalog: return GetDirInputState(leftAnalog, direction);
		case DirectionalInputType::RightAnalog: return GetDirInputState(rightAnalog, direction);
		}
	}

	ControllerInput GetControllerInputFromDirectionalInput(DirectionalInputType dirType, InputDirection inputDirection)
	{
		if (dirType == DirectionalInputType::DPad		 && inputDirection == InputDirection::Up)		return ControllerInput::ButtonDPadUp;
		if (dirType == DirectionalInputType::DPad		 && inputDirection == InputDirection::Down)		return ControllerInput::ButtonDPadDown;
		if (dirType == DirectionalInputType::DPad		 && inputDirection == InputDirection::Right)	return ControllerInput::ButtonDPadRight;
		if (dirType == DirectionalInputType::DPad		 && inputDirection == InputDirection::Left)		return ControllerInput::ButtonDPadLeft;

		if (dirType == DirectionalInputType::LeftAnalog	 && inputDirection == InputDirection::Up)		return ControllerInput::ButtonLeftAnalogUp;
		if (dirType == DirectionalInputType::LeftAnalog	 && inputDirection == InputDirection::Down)		return ControllerInput::ButtonLeftAnalogDown;
		if (dirType == DirectionalInputType::LeftAnalog	 && inputDirection == InputDirection::Right)	return ControllerInput::ButtonLeftAnalogRight;
		if (dirType == DirectionalInputType::LeftAnalog	 && inputDirection == InputDirection::Left)		return ControllerInput::ButtonLeftAnalogLeft;

		if (dirType == DirectionalInputType::RightAnalog && inputDirection == InputDirection::Up)		return ControllerInput::ButtonRightAnalogUp;
		if (dirType == DirectionalInputType::RightAnalog && inputDirection == InputDirection::Down)		return ControllerInput::ButtonRightAnalogDown;
		if (dirType == DirectionalInputType::RightAnalog && inputDirection == InputDirection::Right)	return ControllerInput::ButtonRightAnalogRight;
		if (dirType == DirectionalInputType::RightAnalog && inputDirection == InputDirection::Left)		return ControllerInput::ButtonRightAnalogLeft;

		assert(false); // This should not be possible?
		return ControllerInput::Max;
	}
}
