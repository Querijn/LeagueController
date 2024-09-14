#include <league_controller/controller_listener.hpp>
#include <league_controller/controller_manager.hpp>
#include <league_controller/profiler.hpp>

namespace LeagueController
{
	ControllerListener::ControllerListener(ControllerManager& manager) :
		m_manager(&manager)
	{
		Leacon_Profiler;
		m_manager->AddListener(this);
	}

	ControllerListener::~ControllerListener()
	{
		Leacon_Profiler;
		m_manager->RemoveListener(this);
		DisconnectManager();
	}

	void ControllerListener::ConnectController(const Controller& controller)
	{
		Leacon_Profiler;
		OnControllerConnected(controller);
	}

	void ControllerListener::DisconnectController(const Controller& controller)
	{
		Leacon_Profiler;
		OnControllerDisconnected(controller);
	}

	void ControllerListener::DisconnectManager()
	{
		Leacon_Profiler;
		OnManagerDisconnected();
		m_manager = nullptr;
	}
}
