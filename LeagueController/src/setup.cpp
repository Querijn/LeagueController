#include <game_overlay/overlay.hpp>
#include <game_overlay/imgui_memory_editor.hpp>
#include <game_overlay/image_instance.hpp>
#include <game_overlay/window.hpp>

#include <league_controller/controller_manager.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller.hpp>
#include <league_controller/launcher_data.hpp>
#include <league_controller/connection.hpp>
#include <league_controller/config.hpp>

#include <league_lib/navgrid/navgrid.hpp>
#include <league_lib/wad/wad_filesystem.hpp>

#include <memory_helper/address.hpp>

#include <league_internals/game_object.hpp>
#include <league_internals/offsets.hpp>

#include "script.hpp"
#include "script_ui.hpp"
#include "setup.hpp"
#include "debug_render.hpp"
#include "controller.hpp"
#include "champion_data.hpp"
#include "cursor.hpp"
#include "filesystem_embedded.hpp"
#include "files_embedded.hpp"
#include "sokol.hpp"
#include "item.hpp"

#include <glm/glm.hpp>

#include <Windows.h>
#include <TlHelp32.h>
#include "imgui.h"
#include <d3d11.h>
#include <functional>
#include <sstream>
#include <set>
#include <algorithm>
#include <cctype>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <shellapi.h>

using namespace MemoryHelper;
namespace fs = std::filesystem;

namespace LeagueController
{
	using Duration = Spek::Duration;
	using namespace Spek;
	
	GameOverlay::LogCategory g_setupLog("Setup");
	std::unique_ptr<LeagueController::Connection<LeagueController::LauncherData>> g_sharedMemory;

	struct
	{
#if !LEACON_SUBMISSION
		HMODULE library;
		std::thread thread;
		bool isShuttingDown = false;
		bool shouldShutDown = false;
#endif

		LeagueLib::NavGrid::Handle navGrid;
		ChampionData champions;
		
		SerialisedSettings serialisedSettings;
		Settings settings;
		Settings settingsAtStartup;
		bool settingsLoaded = false;
		sg_pass_action uiPassAction;
		
		IF_NOT_FINAL(bool isApp = false);
	} g_setupData;

	bool GetExecutableVersion(std::string& version); // From offset_loader.cpp

	const LauncherData* GetLauncherData()
	{
		return g_sharedMemory ? &g_sharedMemory->GetData() : nullptr;
	}
	
	static void LoadChampionData()
	{
		GameOverlay_Profiler;
		static bool championsLoaded = false;
		if (championsLoaded)
			return;

		// Little debug setup to load specific champions
#if !LEACON_FINAL
		if (g_setupData.isApp)
		{
			g_setupData.champions.AddChampion("Tristana");
			championsLoaded = true;
			return;
		}
#endif

		Manager* heroManager = GetHeroManager();
		if (heroManager == nullptr)
			return;

		GameObject** begin = heroManager->items.begin;
		GameObject** end = &heroManager->items.begin[heroManager->items.count];
		int count = 0;
		for (auto i = begin; i != end; i++)
		{
			auto hero = *i;
			auto l = hero->GetChampionName() ? strlen(hero->GetChampionName()) : 0;
			std::string champName = l != 0 ? hero->GetChampionName() : "";
			if (IsValidChampionId(champName.c_str()) == false)
				continue;

			std::transform(champName.begin(), champName.end(), champName.begin(), [](unsigned char c) { return std::tolower(c); });
			GameOverlay_LogInfo(g_setupLog, "Game contains champion '%s'", champName.c_str());
			g_setupData.champions.AddChampion(champName);

			count++;
		}

		GameOverlay_LogInfo(g_setupLog, "Requested load for %d champions", count);
		championsLoaded = true;
	}

	void RenderUIPass(Spek::Duration inDt)
	{
		int width = GameOverlay::GetWidth();
		int height = GameOverlay::GetHeight();

#if !LEACON_FINAL
		if (g_setupData.isApp)
		{
			width = sapp_width();
			height = sapp_height();
		}
#endif

		g_setupData.uiPassAction.colors[0].action = SG_ACTION_DONTCARE;
		sg_begin_default_pass(g_setupData.uiPassAction, width, height);
		RenderScriptUI(inDt, width, height);
		sg_end_pass();
	}

	static void InternalUnload()
	{
		DestroyControllerManager(GameOverlay::GetWindow());

		GameOverlay::RemoveLoopFunctions();
		GameOverlay::Destroy();

#if !LEACON_SUBMISSION
		if (g_setupData.library)
		{
			if (!FreeLibrary(g_setupData.library))
			{
				GameOverlay_LogError(g_setupLog, "Couldn't unload library. Error Code: %02X\n. Attempting to unmap...", GetLastError());
				if (!UnmapViewOfFile(g_setupData.library))
					GameOverlay_LogError(g_setupLog, "Couldn't unmap the file! Error Code: %02X\n", GetLastError());
			}
			g_setupData.library = nullptr;
		}
		g_setupData.shouldShutDown = false;
#endif
	}

	void __cdecl Update()
	{
		Leacon_Profiler;
#if !LEACON_SUBMISSION
		// Check if we need to free our library.
		if (g_sharedMemory && (g_sharedMemory->GetData().shouldFreeSelf || g_setupData.shouldShutDown))
		{
			InternalUnload();
			return;
		}
#endif
		if (GetLauncherLocation() == nullptr)
		{
			g_sharedMemory->ForceFetch(); // Re-fetch
			if (GetLauncherLocation() == nullptr)
				return;
		}

		LoadSettings(GetLauncherLocation(), IF_NOT_FINAL_ELSE(g_setupData.isApp, false));
		LoadOffsets();
		WriteSettings(GetLauncherLocation());
		LoadItems(GetLauncherLocation());

		RenderImGui();

		static Duration lastTime = GetTimeSinceStart();
		Duration currentTime = GetTimeSinceStart();
		Duration deltaTime = currentTime - lastTime;
		lastTime = GetTimeSinceStart();

		RenderUIPass(deltaTime);
		GameOverlay::ImageInstance::RenderAll();
		GameOverlay::Input::Update();

		bool isApp = IF_NOT_FINAL_ELSE(g_setupData.isApp, false);
		if (AreOffsetsLoaded() || isApp)
		{
			GameOverlay_LogInfoOnce(g_setupLog, "Offsets loaded");
			LoadChampionData();
		}

		if (AreOffsetsLoaded() && !isApp && GetGameTime() > 0.1f && GameOverlay::IsWindowFocussed()) // Center camera on startup
			CenterCamera(true);
		Spek::File::Update();
		UpdateControllers(deltaTime, GameOverlay::GetWindow());
		UpdateScripts(deltaTime);
		UpdateCursor();
		sg_commit();
	}

	static void WaitForWindow()
	{
		GameOverlay_Profiler;
		GameOverlay_LogInfo(g_setupLog, "Waiting for Window..");
		static bool shouldLoop;
		shouldLoop = true;
		do
		{
			std::set<DWORD> threadIds;
			DWORD procId = GetCurrentProcessId();
			threadIds.insert(procId);

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (snapshot == INVALID_HANDLE_VALUE)
				continue;

			THREADENTRY32 threadEntry;
			threadEntry.dwSize = sizeof(threadEntry);
			if (Thread32First(snapshot, &threadEntry))
			{
				do {
					if (threadEntry.dwSize < FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(threadEntry.th32OwnerProcessID))
						continue;

					if (threadEntry.th32OwnerProcessID == procId)
						threadIds.insert(threadEntry.th32ThreadID);
					threadEntry.dwSize = sizeof(threadEntry);
				} while (Thread32Next(snapshot, &threadEntry));
			}
			CloseHandle(snapshot);

			EnumWindows([](HWND hWnd, LPARAM lparam)
			{
				std::set<DWORD>& threadIds = *(std::set<DWORD>*)lparam;
#if !LEACON_SUBMISSION
				static constexpr size_t bufferSize = 256;
#else
				static constexpr size_t bufferSize = 5;
#endif
				char buffer[bufferSize];
				if (GetWindowText(hWnd, buffer, bufferSize) == 0) // If zero, it's not visible, not initialised, or has no title.
					return TRUE;

				// List visible windows with a non-empty title
				if (IsWindowVisible(hWnd) == false)
					return TRUE;

				DWORD windowProcessId; GetWindowThreadProcessId(hWnd, &windowProcessId);
				auto index = std::find(threadIds.begin(), threadIds.end(), windowProcessId);
				if (index == threadIds.end())
					return TRUE;

				shouldLoop = false;
				return FALSE;
			}, (LPARAM)& threadIds);

			Sleep(1000);
		} while (shouldLoop);
	}

	std::string GetCmdOption(const std::vector<std::string>& commandLine, const std::string& option)
	{
		GameOverlay_Profiler;
		for (const std::string& arg : commandLine)
		{
			// Check if arg starts with option
			if (arg.find(option) != 0)
				continue;
			
			// If it has a value, return value, otherwise return arg
			if (arg[option.size()] == '=')
				return arg.substr(option.size() + 1);

			return arg;
		}
		return "";
	}

	std::string ws2s(const std::wstring& wide)
	{
		std::string str(wide.length(), 0);
		std::transform(wide.begin(), wide.end(), str.begin(), [](wchar_t c) { return (char)c;});
		return str;
	}

	void ProcessCommandLine(std::vector<std::string>& commandLine)
	{
		GameOverlay_Profiler;
		commandLine.clear();
		int nArgs;
		LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
		if (NULL == szArglist)
			return;

		for (int i = 0; i < nArgs; i++)
			commandLine.push_back(ws2s(szArglist[i]));
		LocalFree(szArglist);
	}

	void __cdecl Thread(void* handle)
	{
		GameOverlay_Profiler;
		GameOverlay_LogInfo(g_setupLog, "LeagueController v%d.%d thread started , built at %s %s", MajorVersion, MinorVersion, __DATE__, __TIME__);

	#if NDEBUG
		LoadEmbeddedData();
		Spek::File::Mount<EmbeddedFileSystem>("");
		Spek::File::Mount<EmbeddedFileSystem>("Data");
	#endif
	#if !LEACON_SUBMISSION
		IF_NOT_FINAL(g_setupData.isApp = !handle);
		Spek::File::MountDefault();
		Spek::File::MountDefault("Data");
	#endif

		std::vector<std::string> commandLine;
		ProcessCommandLine(commandLine);
		GameOverlay_LogDebug(g_setupLog, "Command line:");
		for (auto i = 0; i < commandLine.size(); i++)
			GameOverlay_LogDebug(g_setupLog, "- %d: %s", i + 1, commandLine[i].c_str());

		{
			std::string installFolder = "C:/Riot Games/League of Legends/Game/DATA/FINAL";
			if (handle)
			{
				std::string commandLineOption = GetCmdOption(commandLine, "-GameBaseDir");
				if (commandLineOption.empty() == false || fs::exists(commandLineOption))
				{
					fs::path baseDir = fs::path(commandLineOption) / "Game/DATA/FINAL";
					installFolder = baseDir.generic_string();
					GameOverlay_LogDebug(g_setupLog, "Mounting '%s' (found with -GameBaseDir='%s')", installFolder.c_str(), commandLineOption.c_str());
				}
			}
			Spek::File::Mount<LeagueLib::WADFileSystem>(installFolder.c_str());
		}

		std::string gameId;
		if (handle)
		{
			size_t lastSpace = commandLine[1].find_last_of(' ');
			if (lastSpace != std::string::npos)
			{
				gameId = commandLine[1].substr(lastSpace + 1);
				GameOverlay_LogDebug(g_setupLog, "Game ID: %s", gameId.c_str());
			}
		}

		Spek::File::Update();

		g_setupData.navGrid = LeagueLib::NavGrid::Load("levels/map11/aipath.aimesh_ngrid", [handle](LeagueLib::NavGrid::Handle grid)
		{
			GameOverlay_Profiler;
			assert(grid && grid->GetLoadState() == Spek::File::LoadState::Loaded);
			if (handle == nullptr) // Dont create texture when we're debugging locally
				return;

			GameOverlay_LogInfo(g_setupLog, "Finished loading navgrid: %s", grid->GetLoadState() == Spek::File::LoadState::Loaded ? "Loaded" : "Not Loaded");
		});
		GameOverlay_LogInfo(g_setupLog, "Loading navgrid");

		if (handle)
		{
			g_sharedMemory = std::make_unique<LeagueController::Connection<LeagueController::LauncherData>>(LeagueController::LauncherData::name, false);
			GameOverlay_LogInfo(g_setupLog, "Started connection");
			
			WaitForWindow();

			// Wait for launcher data
			while (GetLauncherLocation() == nullptr)
			{
				GameOverlay_ProfilerSection("WaitForLauncherLocation");
				g_sharedMemory->ForceFetch(); // Re-fetch
				Sleep(100);
			}

			GameOverlay_LogDebug(g_setupLog, "Launcher location = '%s'", GetLauncherLocation());

		#if !LEACON_SUBMISSION
			fs::path dataPathLocation = GetLauncherLocation(); dataPathLocation /= "data";
			GameOverlay_LogInfo(g_setupLog, "Mounting '%s'", dataPathLocation.generic_string().c_str());
			Spek::File::MountDefault(dataPathLocation.generic_string().c_str());
			
			dataPathLocation = GetLauncherLocation();
			GameOverlay_LogInfo(g_setupLog, "Mounting '%s'", dataPathLocation.generic_string().c_str());
			Spek::File::MountDefault(dataPathLocation.generic_string().c_str());
			
			dataPathLocation = GetLauncherLocation(); dataPathLocation /= "script";
			GameOverlay_LogInfo(g_setupLog, "Mounting '%s'", dataPathLocation.generic_string().c_str());
			Spek::File::MountDefault(dataPathLocation.generic_string().c_str());
		#endif

			// Wait to be in-game
			GameOverlay_LogInfo(g_setupLog, "Waiting to be in-game..");
			while (IsInGame() == false)
			{
				GameOverlay_ProfilerSection("WaitForGame");
				Sleep(100);
			}

			// Wait for scripts to be working
			GameOverlay_LogInfo(g_setupLog, "Wait for scripts..");
			while (InitScripts(IF_NOT_FINAL_ELSE(g_setupData.isApp, false)) == false)
			{
				GameOverlay_ProfilerSection("WaitForScripts");
				Sleep(100);
			}

			fs::path launcherFolder = fs::path(GetLauncherLocation()) / "logs";
			fs::create_directories(launcherFolder);

			fs::path launcherLocation = launcherFolder / ("LeagueController_" + gameId);
			GameOverlay_LogInfo(g_setupLog, "Starting loop, logfile = %s", launcherLocation.generic_string().c_str());
			GameOverlay::Start(launcherLocation.generic_string(), IF_NOT_FINAL_ELSE(g_setupData.isApp, false));
			GameOverlay::AddLoopFunction(Update);
		}
		else
		{
			// Wait for scripts to be working
			GameOverlay_LogInfo(g_setupLog, "Wait for scripts..");
			while (InitScripts(IF_NOT_FINAL_ELSE(g_setupData.isApp, false)) == false)
			{
				GameOverlay_ProfilerSection("WaitForScripts");
				Sleep(100);
			}
			
			GameOverlay_LogInfo(g_setupLog, "Waiting for navgrid");
			while (g_setupData.navGrid == nullptr || g_setupData.navGrid->GetLoadState() == Spek::File::LoadState::NotLoaded)
			{
				GameOverlay_ProfilerSection("WaitForNavGrid");
				Spek::File::Update();
				Sleep(10);
			}

			GameOverlay::Start("LeagueController", IF_NOT_FINAL_ELSE(g_setupData.isApp, false));
			// Update is handled by the underlying sokol app
		}
	}

	void Destroy()
	{
		while (Spek::File::IsReadyToExit() == false)
			Sleep(1);
	}

	LeagueLib::NavGrid::Handle GetNavGrid()
	{
		return g_setupData.navGrid;
	}

	const ChampionData& GetChampionData()
	{
		return g_setupData.champions;
	}
	
	const char* GetLauncherLocation()
	{
		GameOverlay_Profiler;
		static std::string localCWD;
		if (localCWD.empty())
		{
			if (IF_NOT_FINAL_ELSE(g_setupData.isApp, false))
			{
				char buffer[1024];
				GetCurrentDirectoryA(1024, buffer);
				localCWD = buffer;
			}
			else
			{
				localCWD = g_sharedMemory ? g_sharedMemory->GetData().currentWorkingDirectory : "";
			}
		}

		return !localCWD.empty() ? localCWD.c_str() : nullptr;
	}

	#if !LEACON_SUBMISSION
	void UnloadSelf() { g_setupData.shouldShutDown = true; }
	#else
	void UnloadSelf() {}
	#endif

	bool IsApp()
	{
		return IF_NOT_FINAL_ELSE(g_setupData.isApp, false);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
#if !LEACON_SUBMISSION
		LeagueController::g_setupData.library = hinstDLL;
#endif
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)LeagueController::Thread, hinstDLL, 0, nullptr);
		break;

	case DLL_PROCESS_DETACH:
		GameOverlay::Destroy();
		LeagueController::Destroy();
		break;
	}

	return TRUE;
}
