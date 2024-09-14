#pragma once

namespace sol { class state; }

#include <league_controller/controller_input.hpp>

#include <game_overlay/input.hpp>

#include <vector>

namespace LeagueController
{
	void InitScriptControllerMapper(sol::state& state);
	void AddControllerMapperScriptSystem(sol::state& state);
	void UpdateScriptControllerMapper();
	void DestroyScriptControllerMapper();

	std::vector<ControllerInput> GetUnboundKeys();
}