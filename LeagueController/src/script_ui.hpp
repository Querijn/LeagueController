#pragma once

#include <spek/util/duration.hpp>

namespace sol { class state; }

namespace LeagueController
{
	void InitScriptUI(sol::state& state);
	void AddUIScriptSystem(sol::state& state);
	void RenderScriptUI(Spek::Duration inDt, int width, int height);
	void DestroyScriptUI();
}