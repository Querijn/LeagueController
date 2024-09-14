#pragma once
#if WIN32

#include <memory>

namespace LeagueController
{
	struct XInputData;
	class ControllerManager;
	class ControllerHandlerXInput
	{
	public:
		ControllerHandlerXInput(ControllerManager& manager);
		~ControllerHandlerXInput();

		void Update();

		friend class ControllerHandlerRawInput;
	private:
		std::unique_ptr<XInputData> m_data;

		bool HasXInputSupport(long vendorId, long productId);
	};
}

#endif