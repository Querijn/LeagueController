#include "lua_wrapper.hpp"
#include "script.hpp"
#include "script_debug.hpp"
#include "script_require.hpp"
#include "setup.hpp"
#include "filesystem_embedded.hpp"

#include <spek/file/file.hpp>
#include <game_overlay/log.hpp>
#include <league_controller/config.hpp>

#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <set>

namespace LeagueController
{
	extern sol::state g_lua;
	extern GameOverlay::LogCategory g_scriptLog;
	namespace fs = std::filesystem;
	
#if !LEACON_SUBMISSION

	struct FileCacheItem
	{
		bool hasHadInitialLoad;
		Spek::File::Handle file;
	};
	static std::unordered_map<std::string, FileCacheItem> g_fileCache;
	static std::unordered_map<Spek::File*, FileCacheItem*> g_cacheByFile;
#endif

	bool FileToString(const char* relativePath, std::string& output)
	{
		Leacon_Profiler;
		// This ensures that we auto-reload files that have been modified
	#if !LEACON_SUBMISSION
		auto fileIndex = g_fileCache.find(relativePath);
		if (fileIndex == g_fileCache.end())
		{
			auto file = Spek::File::Load(relativePath, [](auto file)
			{
				FileCacheItem* cacheItem = g_cacheByFile[file.get()];
				if (file == nullptr || cacheItem == nullptr)
					return;
				
				if (cacheItem->hasHadInitialLoad == false)
					cacheItem->hasHadInitialLoad = true;
				else
					FlagScriptsForReload();
			});

			if (file != nullptr)
			{
				g_fileCache[relativePath] = { file->GetLoadState() == Spek::File::LoadState::Loaded, file };
				g_cacheByFile[file.get()] = &g_fileCache[relativePath];
			}
		}
	#endif
		
		fs::path fullScriptFileName = IF_DEBUG_ELSE(LEAGUE_DEBUG_FOLDER, GetLauncherLocation());
		fullScriptFileName /= relativePath;

		// Read local file
		std::ifstream t(fullScriptFileName.c_str());
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		if (t.bad() || size == 0 || size == ~0)
		{
		#if LEACON_SUBMISSION
			auto fs = GetScriptFileSystem();
			if (fs)
			{
				size_t size = 0;
				const u8* data = fs->GetFileData(relativePath, size);
				if (data && size > 0)
				{
					output = std::string((const char*)data, size);
					return true;
				}
			}
		#endif
			return false;
		}

		output = std::string(size, ' ');
		t.seekg(0);
		t.read(&output[0], size);
		return true;
	}

	int LoadFileRequire(lua_State* luaState)
	{
		Leacon_Profiler;
		std::string relativePath = sol::stack::get<std::string>(luaState, 1);

		std::string fullScriptFileName = "scripts/" + relativePath + ".lua";
		std::string script;
		if (FileToString(fullScriptFileName.c_str(), script))
		{
			GameOverlay_LogDebug(g_scriptLog, "Successfully loaded included file: '%s'", fullScriptFileName.c_str());
			luaL_loadbuffer(luaState, script.data(), script.size(), relativePath.c_str());
			return 1;
		}

		GameOverlay_AssertF(g_scriptLog, false, "Unable to included file '%s'", fullScriptFileName.c_str());
		sol::stack::push(luaState, "This is not the module you're looking for!");
		return 1;
	}

	bool LoadFileToState(sol::state& state, const char* fileName, bool shouldReload IF_NOT_SUBMISSION(, bool allowDebug))
	{
		Leacon_Profiler;
		std::string data;
		if (FileToString(fileName, data) == false)
			return false;

		return LoadFileDataToState(state, data, fileName, shouldReload IF_NOT_SUBMISSION(, allowDebug));
	}

	bool LoadFileDataToState(sol::state& state, std::string& fileData, const char* fileName, bool shouldReload IF_NOT_SUBMISSION(, bool allowDebug))
	{
		Leacon_Profiler;
		if (shouldReload)
			FlagScriptsForReload();

	#if !LEACON_SUBMISSION
		if (allowDebug)
			InjectDebugData(fileName, fileData);
	#endif

		// Parse and run
		GameOverlay_LogDebug(g_scriptLog, "(Re)Loading File: %s", fileName);
		sol::optional<sol::error> error = state.safe_script(fileData.c_str(), sol::script_pass_on_error);
		GameOverlay_AssertF(g_scriptLog, error.has_value() == false, "Error while parsing '%s': %s", fileName, error.value_or<sol::error>(std::string("")).what());
		return error.has_value() == false;
	}

	void AddRequireScriptSystem(sol::state& state)
	{
		Leacon_Profiler;
		state.clear_package_loaders();
		state.add_package_loader(LoadFileRequire);
	}

	void DestroyScriptRequire()
	{
		Leacon_Profiler;
#if !LEACON_SUBMISSION
		g_fileCache.clear();
#endif
	}
}
