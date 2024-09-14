#pragma once

namespace LeagueController
{
	class ControllerManager;
	class Controller;
	class ControllerListener
	{
	public:
		~ControllerListener();

		virtual void OnControllerConnected(const Controller& controller) {}
		virtual void OnControllerDisconnected(const Controller& controller) {}
		virtual void OnManagerDisconnected() {}

		virtual void OnInput(const Controller& controller) {}

		friend class ControllerManager;
	protected:
		ControllerManager* m_manager = nullptr;

		ControllerListener(ControllerManager& manager);
		void ConnectController(const Controller& controller);
		void DisconnectController(const Controller& controller);
		void DisconnectManager();
	};
}
