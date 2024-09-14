#include <league_controller/connection.hpp>
#include <league_controller/profiler.hpp>

#include "libipc/ipc.h"
#include <cassert>

namespace LeagueController
{
	std::string GetCorrectName(std::string_view name, bool isOwner, bool isSender)
	{
		std::string base(name);
		if ((isOwner && isSender) || (!isOwner && !isSender))
			return base + "S2C";
		else if ((isOwner && !isSender) || (!isOwner && isSender))
			return base + "C2S";
		assert(false);
		return base + "UNKNOWN";
	}
	
	struct ConnectionData
	{
		ConnectionData(std::string_view name, bool isOwner) :
			sender { GetCorrectName(name, isOwner, true).c_str(), ipc::sender },
			receiver { GetCorrectName(name, isOwner, false).c_str(), ipc::receiver }
		{
		}

		ipc::channel sender;
		ipc::channel receiver;
	};

	BaseConnection::BaseConnection(std::string_view name, bool owner) :
		m_connectionData(std::make_shared<ConnectionData>(name, owner)),
		m_thread([this]() { MessageHandler(); }),
		m_isOwner(owner)
	{
	}

	BaseConnection::~BaseConnection()
	{
		m_running = false;
		m_thread.detach();
	}
	
	void BaseConnection::Send(const std::vector<u8>& buffer)
	{
		Leacon_Profiler;
		m_connectionData->sender.send(buffer.data(), buffer.size());
	}

	void BaseConnection::MessageHandler()
	{
		using namespace std::chrono_literals;
		Leacon_Profiler;
		while (m_running)
		{
			ipc::buff_t buffer = m_connectionData->receiver.try_recv();
			if (buffer.empty())
			{
				std::this_thread::sleep_for(20ms);
				continue;
			}

			const u8* ptr = buffer.get<const u8*>();
			std::vector<u8> data(ptr, ptr + buffer.size());
			OnReceive(data);
			std::this_thread::sleep_for(20ms);
		}
	}
}
