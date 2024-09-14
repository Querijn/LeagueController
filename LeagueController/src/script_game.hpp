#pragma once

#include <game_overlay/input.hpp>
#include <string_view>

namespace sol { class state; }

namespace LeagueController
{
	void InitScriptGame(sol::state& state);
	void AddGameScriptSystem(sol::state& state);
	void UpdateScriptGame(sol::state& state);
	void DestroyScriptGame();

	GameOverlay::KeyboardKey GetSpellKeyByName(std::string_view spellIdentifier);
}