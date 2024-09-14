#pragma once

#include <memory>

namespace LeagueController
{
	class ControllerHandlerXInput;
	class Controller;
	struct ControllerManagerData;
	class ControllerManager
	{
	public:
		ControllerManager();
		~ControllerManager();

		void Update(void* window);

		friend class ControllerHandlerRawInput;
		friend class ControllerListener;
		friend struct XInputData;
	private:
		std::unique_ptr<ControllerManagerData> m_data;

		void ControllerHasConnected(Controller& controller);
		void ControllerHasDisconnected(Controller& controller);
		void ControllerHasInput(Controller& controller);

		void AddListener(ControllerListener* listener);
		void RemoveListener(ControllerListener* listener);

		ControllerHandlerXInput& GetXInputHandler();
	};

	void DestroyControllerManager(void* window);
}
