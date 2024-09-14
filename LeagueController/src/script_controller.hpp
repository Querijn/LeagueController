#pragma once

#include <spek/util/duration.hpp>

namespace sol { class state; }

namespace LeagueController
{
	using Duration = Spek::Duration;

	void AddControllerScriptSystem(sol::state& state);
	void UpdateScriptController(Duration dt);

	void SetLizardMode(bool enabled);
	bool IsLizardModeEnabled();
	void DestroyScriptController();
}