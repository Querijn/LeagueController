#pragma once

namespace sol { class state; }

namespace LeagueController
{
	void AddOutputScriptSystem(sol::state& state);
	void DestroyScriptOutput();
}