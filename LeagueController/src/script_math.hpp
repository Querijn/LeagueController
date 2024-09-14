#pragma once

namespace sol { class state; }

namespace LeagueController
{
	void AddMathScriptSystem(sol::state& state);
	void DestroyScriptMath();
}