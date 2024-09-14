#include "league_controller/controller_manager.hpp"
#include "league_controller/controller.hpp"
#include "league_controller/controller_listener.hpp"
#include "league_controller/profiler.hpp"

#if WIN32
#include <league_controller/controller_windows_rawinput.hpp>
#include <league_controller/controller_windows_xinput.hpp>
#endif

#include <vector>

namespace LeagueController
{
	struct ControllerManagerData
	{
		ControllerManagerData(ControllerManager& manager) :
			m_rawInput(manager),
			m_xinput(manager)
		{

		}

		std::vector<Controller*> m_controllers;
		std::vector<ControllerListener*> m_listeners;

#if WIN32
		ControllerHandlerRawInput m_rawInput;
		ControllerHandlerXInput m_xinput;
#endif
	};

	ControllerManager::ControllerManager() :
		m_data(std::make_unique<ControllerManagerData>(*this))
	{
	}

	ControllerManager::~ControllerManager()
	{
		Leacon_Profiler;
		for (auto listener : m_data->m_listeners)
			listener->DisconnectManager();
	}

	void ControllerManager::Update(void* window)
	{
		Leacon_Profiler;
#if WIN32
		m_data->m_xinput.Update();
		m_data->m_rawInput.Update(window);
#endif
	}

	void ControllerManager::ControllerHasConnected(Controller& controller)
	{
		Leacon_Profiler;
		for (auto listener : m_data->m_listeners)
			listener->ConnectController(controller);
	}

	void ControllerManager::ControllerHasDisconnected(Controller& controller)
	{
		Leacon_Profiler;
		for (auto listener : m_data->m_listeners)
			listener->DisconnectController(controller);

		auto index = std::find(m_data->m_controllers.begin(), m_data->m_controllers.end(), &controller);
		if (index != m_data->m_controllers.end())
			m_data->m_controllers.erase(index);
	}

	void ControllerManager::ControllerHasInput(Controller& controller)
	{
		Leacon_Profiler;
		for (auto listener : m_data->m_listeners)
			listener->OnInput(controller);
	}

	void ControllerManager::AddListener(ControllerListener* listener)
	{
		Leacon_Profiler;
		m_data->m_listeners.push_back(listener);
	}

	void ControllerManager::RemoveListener(ControllerListener* listener)
	{
		Leacon_Profiler;
		auto index = std::find(m_data->m_listeners.begin(), m_data->m_listeners.end(), listener);
		if (index != m_data->m_listeners.end())
			m_data->m_listeners.erase(index);
	}
	
	ControllerHandlerXInput& ControllerManager::GetXInputHandler()
	{
		Leacon_Profiler;
		return m_data->m_xinput;
	}

	void DestroyXInput();
	void DestroyRawInput(void* window);
	void DestroyControllerManager(void* window)
	{
		DestroyXInput();
		DestroyRawInput(window);
	}
}
