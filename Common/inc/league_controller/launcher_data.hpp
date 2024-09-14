#pragma once

#include <league_controller/types.hpp>
#include <league_controller/state_base.hpp>

namespace LeagueController
{
	struct LauncherData : public BaseConnectionState
	{
		LauncherData()
		{
			memset(currentWorkingDirectory, 0, cwdLength);
		}

		virtual bool Serialise(std::vector<u8>& ioBuffer) override
		{
			const u8* buffer = (const u8*)this;
			ioBuffer = std::vector<u8>(buffer, buffer + sizeof(LauncherData));
			return true;
		}

		virtual bool Deserialise(const std::vector<u8>& inBuffer) override
		{
			if (inBuffer.empty() || inBuffer.size() < sizeof(LauncherData))
				return false;
			
			memcpy(this, inBuffer.data(), sizeof(LauncherData));
			return true;
		}

		static constexpr char name[] = "LeagueControllerLauncherData";
		static constexpr size_t cwdLength = 1024;
		char currentWorkingDirectory[cwdLength];
		bool shouldFreeSelf = false;
	};
}