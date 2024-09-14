#include "lua_wrapper.hpp"
#include "script.hpp"
#include "script_debug.hpp"
#include "script_require.hpp"
#include "script_game.hpp"
#include "script_math.hpp"
#include "script_ui.hpp"
#include "script_output.hpp"
#include "script_controller.hpp"
#include "script_controller_mapper.hpp"
#include "filesystem_embedded.hpp"
#include "controller.hpp"
#include "setup.hpp"

#include <spek/file/file.hpp>
#include <game_overlay/log.hpp>
#include <league_controller/config.hpp>

#include <vector>
#include <fstream>

namespace LeagueController
{
#define LEACON_LUA_FRAME "Frame"
	using File = Spek::File;
	bool FileToString(const char* relativePath, std::string& output);

	static sol::state g_lua;
	GameOverlay::LogCategory g_scriptLog("Script");

	static File::Handle g_scriptFile;
	static std::vector<File::Handle> g_files;
	static bool g_hasReloadedThisFrame = false;
	static bool g_shouldReload = false;
	static EmbeddedFileSystem* g_scriptFileSystem = nullptr;
	static std::string g_mainScript;

	bool InitScripts(bool isApp)
	{
		Leacon_Profiler;
		static bool g_initialisedScripts = false;
		if (g_initialisedScripts)
			return true;
		
		if (GetLauncherLocation() == nullptr)
			return false;

		// Mount all script folders
		static bool g_scriptFoldersMounted = false;
		if (g_scriptFoldersMounted == false)
		{
			Spek::File::MountDefault("scripts");
		#if LEACON_SUBMISSION
			g_scriptFileSystem = Spek::File::Mount<EmbeddedFileSystem>(GetLauncherLocation());
		#else
			Spek::File::MountDefault(LEAGUE_DEBUG_FOLDER);
		#endif
			g_scriptFoldersMounted = true; 
		}

	#if !LEACON_SUBMISSION
		g_scriptFile = Spek::File::Load("scripts/main.lua", [](auto file)
		{
			static bool g_hasDoneInitialLoad = false;
			if (g_hasDoneInitialLoad == false)
			{
				g_hasDoneInitialLoad = true;
				return;
			}

			GameOverlay_LogDebug(g_scriptLog, "Detected change in '%s', flagging script system for reload.", file->GetName().c_str());
			g_mainScript = file->GetDataAsString();
			FlagScriptsForReload();
		});
	#endif

		if (FileToString("scripts/main.lua", g_mainScript) == false)
		{
			GameOverlay_AssertF(g_scriptLog, false, "Unable to load main.lua");
			return 1;
		}

#if LEACON_LUAJIT
		GameOverlay_LogDebug(g_scriptLog, "Successfully loaded main.lua, running using Lua %s, LuaJIT", LUA_RELEASE);
#else
		GameOverlay_LogDebug(g_scriptLog, "Successfully loaded main.lua, running using Lua %s, interpreted", LUA_RELEASE);
#endif
		g_initialisedScripts = true;
		FlagScriptsForReload();
		return true;
	}

	void SetupDefaultScriptState(sol::state& state, bool isMain)
	{
		state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math);

#if !LEACON_SUBMISSION
		state.open_libraries(sol::lib::debug);
#endif

		state.set_function("IsApp", []() { return IsApp(); });
		AddRequireScriptSystem(state);
		AddDebugScriptSystem(state);
		AddMathScriptSystem(state);
		AddOutputScriptSystem(state);
		AddGameScriptSystem(state);
		AddControllerScriptSystem(state);
		AddUIScriptSystem(state);
		AddControllerMapperScriptSystem(state);

		if (isMain)
		{
			InitScriptUI(state);
			InitScriptControllerMapper(state);
		}
	}

	void ReloadScriptState(sol::state& state)
	{
		Leacon_Profiler;
		if (g_hasReloadedThisFrame)
			return;
		g_hasReloadedThisFrame = true;

		state = sol::state();
		SetupDefaultScriptState(state, true);

	#if !LEACON_SUBMISSION
		std::string codeCopy = g_mainScript;
		InjectDebugData("main.lua", codeCopy);
	#endif

		sol::optional<sol::error> error = g_lua.safe_script(g_mainScript.c_str(), sol::script_throw_on_error);
		GameOverlay_AssertF(g_scriptLog, error.has_value() == false, "Error in main.lua: %s", error.has_value() ? error->what() : "");

		auto initFunction = g_lua["Init"];
		if (initFunction.valid())
		{
			GameOverlay_LogDebug(g_scriptLog, "Running Lua Init()");
#if !LEACON_SUBMISSION
			std::string initFunction = "Init();";
			InjectDebugData("Init", initFunction, true);
			g_lua.safe_script(initFunction);
#else
			initFunction();
#endif
		}
	}

	void UpdateScripts(Duration dt)
	{
		Leacon_Profiler;
		g_hasReloadedThisFrame = false;
		if (g_shouldReload)
		{
			g_shouldReload = false;
			ReloadScriptState(g_lua);
		}

		UpdateScriptGame(g_lua);
		UpdateScriptController(dt);
		UpdateScriptControllerMapper();

		auto frame = Leacon_ProfilerEvalRet(g_lua[LEACON_LUA_FRAME]);
		if (frame.valid())
		{
#if !LEACON_SUBMISSION
			std::string updateFunction = LEACON_LUA_FRAME "();";
			InjectDebugData(LEACON_LUA_FRAME, updateFunction, true);
			Leacon_ProfilerEval(g_lua.safe_script(updateFunction));
#else
			Leacon_ProfilerEval(frame());
#endif
		}
		else
		{
			// GameOverlay_AssertF(g_scriptLog, false, "No Frame function found in main script");
			g_shouldReload = true;
		}
	}

	void DestroyScripts()
	{
		Leacon_Profiler;
		DestroyScriptRequire();
		DestroyScriptDebug();
		DestroyScriptMath();
		DestroyScriptUI();
		DestroyScriptOutput();
		DestroyScriptGame();
		DestroyScriptController();
		DestroyScriptControllerMapper();

		g_scriptFile = nullptr;
		g_files.clear();
		g_hasReloadedThisFrame = false;
		g_scriptFileSystem = nullptr;
	}
	
	EmbeddedFileSystem* GetScriptFileSystem()
	{
		return g_scriptFileSystem;
	}

	void FlagScriptsForReload()
	{
		g_shouldReload = true;
	}
}
