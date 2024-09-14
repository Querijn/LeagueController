#pragma once

#include <league_controller/types.hpp>
#include <league_controller/derived_type.hpp>

#include <vector>
#include <string_view>
#include <memory>
#include <thread>

#include <Windows.h>

namespace LeagueController
{
	struct ConnectionData;
	class BaseConnection
	{
	public:
		BaseConnection(std::string_view name, bool owner);
		virtual ~BaseConnection();

		void MessageHandler();
		void Send(const std::vector<u8>& buffer);

		virtual void OnReceive(const std::vector<u8>& inBuffer) = 0;
		
	private:
		std::shared_ptr<ConnectionData> m_connectionData;
		std::thread m_thread;
		bool m_running = true;

	protected:
		bool m_isOwner;
	};

	struct BaseConnectionState;
	template<DerivedType(StateObject, BaseConnectionState)>
	class Connection : public BaseConnection
	{
	public:
		Connection(std::string_view name, bool owner) :
			BaseConnection(name, owner)
		{
			if (!m_isOwner)
				Send({ 'R' });
		}

		virtual ~Connection() = default;

		void OnReceive(const std::vector<u8>& inBuffer)
		{
			u8 messageType = inBuffer.front();
			if (messageType == 'R')
			{
				m_state.canary++;
				Update();
			}
			else if (messageType == 'M')
			{
				memcpy(&m_lastState,	inBuffer.data() + 1, sizeof(StateObject));
				memcpy(&m_state,		inBuffer.data() + 1, sizeof(StateObject));
			}
		}

		void Update()
		{
			if (memcmp((void*)&m_lastState, (void*)&m_state, sizeof(StateObject)) == 0)
				return;

			std::vector<u8> buffer;
			m_state.Serialise(buffer);

			size_t size = buffer.size();
			std::vector<u8> data(size + 1);
			memcpy(data.data() + 1, buffer.data(), size);
			data[0] = 'M'; // Message
			
			Send(data);
			memcpy((void*)&m_lastState, (void*)&m_state, sizeof(StateObject));
		}

		void ForceFetch()
		{
			Send({ 'R' });
		}

		StateObject& GetData() { return m_state; }
		const StateObject& GetData() const { return m_state; }
		
	private:
		StateObject m_lastState;
		StateObject m_state;
	};
}