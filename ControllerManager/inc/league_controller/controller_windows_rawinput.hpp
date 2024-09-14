#pragma once
#if _WIN32

namespace LeagueController
{
	class ControllerManager;
	struct ControllerState;
	class ControllerHandlerRawInput
	{
	public:
		ControllerHandlerRawInput(ControllerManager& manager);

		void Update(void* window);

	private:
		ControllerManager& m_manager;

		void UpdateRawController(void* handle, ControllerState& state);
		void UpdateRawControllerConnected();
	};
}

#endif