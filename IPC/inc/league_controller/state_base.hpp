#pragma once

#include <league_controller/types.hpp>

#include <vector>

namespace LeagueController
{
	struct BaseConnectionState
	{
		virtual bool Serialise(std::vector<u8>& ioBuffer) = 0;
		virtual bool Deserialise(const std::vector<u8>& inBuffer) = 0;

		u8 canary = 0;
	};
}