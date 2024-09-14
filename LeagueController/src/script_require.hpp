#pragma once

#include <league_controller/config.hpp>

#include <string>

namespace sol { class state; }

namespace LeagueController
{
	bool LoadFileToState(sol::state& state, const char* fileName, bool shouldReload IF_NOT_SUBMISSION(, bool allowDebug = true));
	bool LoadFileDataToState(sol::state& state, std::string& fileData, const char* fileName, bool shouldReload IF_NOT_SUBMISSION(, bool allowDebug = true));
	void AddRequireScriptSystem(sol::state& state);
	void DestroyScriptRequire();
}