#include "script_debug.hpp"
#include "lua_wrapper.hpp"
#include "controller.hpp"
#include "debug_render.hpp"

// Game Overlay
#include <game_overlay/log.hpp>
#include <game_overlay/debug_renderer.hpp>

// League Hacks
#include <league_internals/offsets.hpp>
#include <league_internals/game_object.hpp>

// Controller
#include <league_controller/config.hpp>
#include <league_controller/controller.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller_manager.hpp>

// STD
#include <string>

namespace LeagueController
{
	extern GameOverlay::LogCategory g_scriptLog;
	LuaDebugReportMap g_debugReports;

	const LuaDebugReportMap& GetLuaDebugReportMap() { return g_debugReports; }

	void ReportString(const std::string& variable, const std::string& value)
	{
		Leacon_Profiler;
		IF_NOT_SUBMISSION((g_debugReports[variable] = { Spek::GetTimeSinceStart(),value }));
	}

	void ReportFloat(const std::string& variable, float value)
	{
		Leacon_Profiler;
		ReportString(variable, std::to_string(value));
	}

	void ReportInt(const std::string& variable, int value)
	{
		Leacon_Profiler;
		ReportString(variable, std::to_string(value));
	}

	void ReportBool(const std::string& variable, bool value)
	{
		Leacon_Profiler;
		ReportString(variable, value ? "true" : "false");
	}

	void ReportVec3(const std::string& variable, const glm::vec3* value)
	{
		Leacon_Profiler;
		if (value)
			ReportString(variable, std::to_string(value->x) + ", " + std::to_string(value->y) + ", " + std::to_string(value->z));
		else
			ReportString(variable, "null");
	}

	void ReportVec2(const std::string& variable, const glm::vec2* value)
	{
		Leacon_Profiler;
		if (value)
			ReportString(variable, std::to_string(value->x) + ", " + std::to_string(value->y));
		else
			ReportString(variable, "null");
	}

	void ReportGameObject(const std::string& variable, const GameObject* value)
	{
		Leacon_Profiler;
	#if !LEACON_SUBMISSION
		if (value == nullptr)
		{
			ReportString(variable, "null");
			return;
		}

		char fullName[256];
		snprintf(fullName, 256, "%s (ptr: 0x%x)", value->GetName(), value);
		ReportString(variable, fullName);
	#else
		ReportString(variable, value ? value->GetName() : "null");
	#endif
	}

	void LocalDrawLine3D(const glm::vec3& start, const glm::vec3& end, GameOverlay::Pixel color, f32 thickness, f32 durationSeconds)
	{
		DrawLine3D(start, end, color, thickness, Spek::Duration::FromMilliseconds((i64)(durationSeconds * 1000.0f)));
	}

	void AddDebugScriptSystem(sol::state& state)
	{
		state.set_function("Log", [](const char* text) { GameOverlay_LogInfo(g_scriptLog, text); });
		state.set_function("LogWarning", [](const char* text) { GameOverlay_LogWarning(g_scriptLog, text); });
		state.set_function("LogError", [](const char* text) { GameOverlay_LogError(g_scriptLog, text); });
		state.set_function("LogDebug", [](const char* text) { GameOverlay_LogDebug(g_scriptLog, text); });
		state.set_function("Break", [](const char* text) { GameOverlay_AssertF(g_scriptLog, false, text); });

		state.set_function("ReportString", ReportString);
		state.set_function("ReportString", ReportString);
		state.set_function("ReportFloat", ReportFloat);
		state.set_function("ReportInt", ReportInt);
		state.set_function("ReportBool", ReportBool);
		state.set_function("ReportVec3", ReportVec3);
		state.set_function("ReportVec2", ReportVec2);
		state.set_function("ReportGameObject", ReportGameObject);

		state.set_function("DrawLine3D", LocalDrawLine3D);
	}

	void InjectDebugData(const std::string& fileName, std::string& script, bool silent)
	{
		Leacon_Profiler;
#if _DEBUG
		std::string origScript = script;
		script = "function codeWrapper()";

		if (silent == false)
			script += "Log(\"Debugging '" + fileName + "'.\");";

		script += "; " + origScript + "; end\n\
			local success, result = xpcall(codeWrapper,\n\
				function(err)\n\
					errorString = tostring(debug.traceback(err));\n\
					Break(\"The following error occurred:\\n\" .. errorString);\n\
				end)\n";

		if (silent == false)
		{
			script +=
			"if success then\n\
				Log(\"'" + fileName + "' has started successfully.\")\n\
			else\n\
				Break(\"'" + fileName + "' has FAILED to run successfully.\")\n\
			end\n";
		}

		script += "return result";
#endif
	}
	
	void DestroyScriptDebug()
	{

	}
}
