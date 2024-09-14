#pragma once

#include <spek/util/duration.hpp>

namespace sol { class state; }

namespace LeagueController
{
	using Duration = Spek::Duration;
	class EmbeddedFileSystem;

	void ReloadScriptState(sol::state& state);
	bool InitScripts(bool isApp);
	void UpdateScripts(Duration dt);
	void DestroyScripts();

	EmbeddedFileSystem* GetScriptFileSystem();
	void FlagScriptsForReload();
	void SetupDefaultScriptState(sol::state& state, bool isMain);
}