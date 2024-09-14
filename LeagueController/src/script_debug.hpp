#pragma once

#include <string>
#include <map>

#include <spek/util/duration.hpp>

namespace sol { class state; }

namespace LeagueController
{
	struct DebugReport
	{
		Spek::Duration time;
		std::string message;
	};
	
	using LuaDebugReportMap = std::map<std::string, DebugReport>;
	
	void AddDebugScriptSystem(sol::state& state);
	void InjectDebugData(const std::string& fileName, std::string& script, bool silent = false);
	void DestroyScriptDebug();
	const LuaDebugReportMap& GetLuaDebugReportMap();
}