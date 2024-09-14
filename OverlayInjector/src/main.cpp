#include <Windows.h>
#include <TlHelp32.h>
#include <cassert>
#include <string>
#include <iostream>
#include <filesystem>
#include <shellapi.h>
#include <locale>
#include <codecvt>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <league_controller/httplib.h>
#include <league_controller/config.hpp>
#include <league_controller/connection.hpp>
#include <league_controller/launcher_data.hpp>
#include <league_controller/profiler.hpp>
#include <league_controller/settings.hpp>
#include "process.hpp"

// Settings
#define DLL_NAME				"LeagueController.dll"
#define SURROGATE_DLL_NAME		"LeagueControllerRuntime.dll"
#define CONFIGURATION_NAME		IF_DEBUG_ELSE("debug", "release")
#define SUBMISSION_NAME			IF_SUBMISSION_ELSE("submission", "developer")
#define PROCESS_NAME			"League of Legends.exe"

IF_NOT_SUBMISSION(static const char* g_debugLocation = LEAGUECONTROLLER_OUTPUT_FOLDER "/LeagueController/" CMAKE_INTDIR "/" DLL_NAME);

// Defines
namespace fs = std::filesystem;
using LeagueConnection = LeagueController::Connection<LeagueController::LauncherData>;

IF_NOT_SUBMISSION(void CheckForDLLReload(fs::file_time_type& lastEditTime, std::string_view cwd, LeagueConnection& sharedData, DWORD& lastPid, std::string& sourceLibraryName, std::string& libraryName));
bool Inject(std::string_view inLibraryName, HANDLE processHandle IF_NOT_SUBMISSION(, fs::file_time_type& lastEditTime, std::string_view cwd, LeagueConnection& sharedData, DWORD& lastPid, std::string& sourceLibraryName, std::string& libraryName))
{
	IF_NOT_SUBMISSION(CheckForDLLReload(lastEditTime, cwd, sharedData, lastPid, sourceLibraryName, libraryName));
	auto fullPath = fs::absolute(inLibraryName).generic_string();

	// Create a bit of memory for a string containing our library name.
	void* allocAddress = VirtualAllocEx(processHandle, nullptr, fullPath.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (allocAddress == nullptr)
	{
		auto error = GetLastError();
		printf("Unable to allocate memory in %s (Error = %d).\n", PROCESS_NAME, error);
		return false;
	}
	
	// Get LoadLibrary in that process
	HMODULE moduleHandle = GetModuleHandle("Kernel32");
	assert(moduleHandle);

	void* remoteLoadLibrary = GetProcAddress(moduleHandle, "LoadLibraryA");
	assert(remoteLoadLibrary);

	// Write string memory to allocAddress
	SIZE_T bytesWritten = 0;
	bool result = WriteProcessMemory(processHandle, allocAddress, fullPath.c_str(), fullPath.size(), &bytesWritten);
	assert(result && bytesWritten == fullPath.size());

	HANDLE thread = CreateRemoteThread(processHandle, nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(remoteLoadLibrary), allocAddress, 0, nullptr);
	assert(thread != INVALID_HANDLE_VALUE && thread != 0 && bytesWritten == fullPath.size() && bytesWritten != 0);

	WaitForSingleObject(thread, INFINITE);
	VirtualFreeEx(processHandle, allocAddress, 0, MEM_RELEASE);
	CloseHandle(thread);
	CloseHandle(processHandle);
	printf("Injected %s into %s.\n", fullPath.c_str(),  PROCESS_NAME);
	return true;
}

#if !LEACON_SUBMISSION
bool CopyLibrary(std::string_view cwd, std::string& sourceLibraryName, std::string& libraryName)
{
	Leacon_Profiler;
	auto path = fs::path(cwd) / SURROGATE_DLL_NAME;

	if (sourceLibraryName != libraryName && libraryName != path)
		sourceLibraryName = libraryName;

	try
	{
		if (fs::exists(path))
		{
			libraryName = path.generic_string().c_str();
			fs::remove(path);
			printf("CopyLibrary: Deleted '%s'\n", SURROGATE_DLL_NAME);
		}

		fs::copy_file(sourceLibraryName, path);
		printf("Using surrogate location, copied '%s' to '%s'\n", 
			fs::path(sourceLibraryName).filename().generic_string().c_str(), 
			path.generic_string().c_str());
		return true;
	}
	catch (std::exception e)
	{
		libraryName = sourceLibraryName;
		printf("Unable to copy '%s' to '%s'.\n", libraryName.c_str(), path.generic_string().c_str());
		printf("Is there a lingering process running in the background with our library?\n");
	}

	return false;
}
#endif

bool CreateGlobalMutex(HANDLE& globalMutex)
{
	Leacon_Profiler;
	globalMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, "LeagueControllerLauncher");
	if (globalMutex == nullptr)
	{
		globalMutex = CreateMutex(0, 0, "LeagueControllerLauncher");
		return true;
	}
	
	printf("LeagueControllerLauncher already is running, stopping this instance.\n");
	return false;
}

#if !LEACON_SUBMISSION
void CheckForDLLReload(fs::file_time_type& lastEditTime, std::string_view cwd, LeagueConnection& sharedData, DWORD& lastPid, std::string& sourceLibraryName, std::string& libraryName)
{
	Leacon_Profiler;
	auto newEditTime = fs::last_write_time(sourceLibraryName);
	if (newEditTime != lastEditTime)
	{
		printf("'%s' was edited.. Reloading!\n", sourceLibraryName.data());
		Sleep(250);
		if (CopyLibrary(cwd, sourceLibraryName, libraryName))
		{
			lastPid = 0;
			lastEditTime = newEditTime;
		}

		sharedData.GetData().shouldFreeSelf = true;
	}
}
#endif

bool CheckLibrary(std::string_view cwd, std::string& libraryName)
{
	Leacon_Profiler;
	if (fs::exists(libraryName))
		return true;

#if !LEACON_SUBMISSION
	if (fs::exists(g_debugLocation))
	{
		printf("Using debug library location '%s'.\n", g_debugLocation);
		libraryName = g_debugLocation;
		return true;
	}

	printf("'%s' does not exist, please build this library first.\n", libraryName.data());
#else
	printf("'%s' does not exist, please make sure to launch from the correct location.\n", libraryName.data());
#endif

	printf("Location = %s.\n", cwd.data());
	return false;
}

std::string GetDDragonRequest(httplib::Client& client, std::string_view path)
{
	Leacon_Profiler;
	httplib::Result res = client.Get(path.data());
	if (!res)
	{
		httplib::Error err = res.error();
		std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
		return "null";
	}

	if (res->status == 200)
		return res->body;

	std::cout << "HTTP error trying to fetch " << path.data() << ": " << res->status << std::endl;
	return "null";
}

void GetDataDragonData()
{
	Leacon_Profiler;
	httplib::Client client("http://ddragon.leagueoflegends.com");
	fs::path path = fs::current_path() / "data/items.json";
	nlohmann::json json;
	if (fs::exists(path))
	{
		Leacon_ProfilerSection("Load items.json");
		printf("Fetching existing items.json from disk\n");
		std::ifstream file(path.generic_string().c_str());
		file >> json;
		file.close();
	}

	// Ensure the parent folder exists
	else if (fs::exists(fs::current_path() / "data") == false)
	{
		Leacon_ProfilerSection("Create /data/");
		fs::create_directories(fs::current_path() / "data");
	}

	std::string versionArrayContents = GetDDragonRequest(client, "/api/versions.json");
	nlohmann::json versionArray = nlohmann::json::parse(versionArrayContents);
	if (versionArray.is_null())
	{
		printf("Unable to fetch version array from DataDragon.");
		return;
	}

	std::string version = versionArray[0];
	if (json["version"] == version)
	{
		printf("Local items.json file is up to date (%s)\n", version.c_str());
		return;
	}

	printf("Fetching new items.json file from DataDragon (%s)\n", version.c_str());
	std::string newFileContents = GetDDragonRequest(client, "/cdn/" + version + "/data/en_US/item.json");
	nlohmann::json newFile = nlohmann::json::parse(newFileContents);
	if (newFile.is_null())
	{
		printf("Unable to fetch items.json from DataDragon.");
		return;
	}

	{
		Leacon_ProfilerSection("Write to file");
		// Write back to file
		std::ofstream file(path.generic_string().c_str());
		file << newFile.dump(IF_SUBMISSION_ELSE(-1, 4));
		file.close();
	}
}

int CommonMain(int argc, char* argv[])
{
	using namespace std::chrono_literals;
	Leacon_Profiler;

	std::string libraryName = DLL_NAME;
	IF_NOT_SUBMISSION(std::string sourceLibraryName);
	try
	{
		HANDLE globalMutex;
		bool succeeded = CreateGlobalMutex(globalMutex);
		if (!succeeded) return -1;

#if LEACON_SUBMISSION
		printf("**************************************\n");
		printf("*                                    *\n");
		printf("*    WARNING: USE AT YOUR OWN RISK   *\n");
		printf("*                                    *\n");
		printf("**************************************\n\n");
		
		printf("This application should only be used for testing purposes. It is not a final product and is not intended to be used for competitive or ranked games.\n");
		printf("It is entirely possible that you need to intervene and take over with a mouse and keyboard.\n\n");
		printf("This application was made semi-patch-independent, but it might break when a new patch is launched. Before starting any matchmade game, check in Practice Tool if the game crashes when LeagueController is active. Open the settings.json file, and check if there's a `\"patchVerified\": true` line near the bottom!\n\n");
		printf("Report any bugs on the Discord!\n\n");
#endif

		printf("Running LeagueControllerLauncher, built at " __DATE__ ", " __TIME__ " (" SUBMISSION_NAME ", " CONFIGURATION_NAME ").\n");
		std::string cwd = fs::current_path().generic_string() + "/";
		succeeded = CheckLibrary(cwd, libraryName);
		if (!succeeded) return -2;

		IF_NOT_SUBMISSION(CopyLibrary(cwd, sourceLibraryName, libraryName));

		bool firstFrame = true;
		LeagueController::Process process(PROCESS_NAME);
		DWORD lastPid = 0;
		IF_NOT_SUBMISSION(auto lastEditTime = fs::last_write_time(sourceLibraryName));

		LeagueConnection sharedData(LeagueController::LauncherData::name, true);
		GetDataDragonData();

		strncpy(sharedData.GetData().currentWorkingDirectory, cwd.c_str(), LeagueController::LauncherData::cwdLength);
		printf("- Working directory: %s\n", sharedData.GetData().currentWorkingDirectory);
		IF_NOT_SUBMISSION(printf("- Source library location: %s\n", sourceLibraryName.c_str()));
		printf("- Library: %s\n\n", libraryName.c_str());
		printf("Waiting for %s...\n", PROCESS_NAME);

		do
		{
			while (true)
			{
				Leacon_ProfilerFrame;
				process.Update();

				IF_NOT_SUBMISSION(CheckForDLLReload(lastEditTime, cwd, sharedData, lastPid, sourceLibraryName, libraryName));

				if (process.IsAlive())
				{
					if (process.pid == lastPid && process.IsModuleLoaded(libraryName) != LeagueController::ProcessQuery::No)
					{
						firstFrame = false;
						continue;
					}

					printf("Found %s alive (%u), did not contain our module (%s)\n", PROCESS_NAME, process.pid, libraryName.c_str());
					assert(process.handle != 0 && process.handle != INVALID_HANDLE_VALUE);

					if (!firstFrame)
						std::this_thread::sleep_for(4s);

					sharedData.GetData().shouldFreeSelf = false;
					if (Inject(libraryName, process.handle IF_NOT_SUBMISSION(, lastEditTime, cwd, sharedData, lastPid, sourceLibraryName, libraryName)))
						break;
					firstFrame = false;
				}
				else if (lastPid != 0)
				{
					lastPid = 0;
					printf("%s (%u) was closed. Waiting for new instance.\n", PROCESS_NAME, process.pid);
				}

				firstFrame = false;
				Leacon_ProfilerSection("Sleep");
				std::this_thread::sleep_for(20ms);
			}

			lastPid = process.pid;
			printf("Waiting a bit before starting over..\n");
			std::this_thread::sleep_for(IF_NOT_SUBMISSION_ELSE(200ms, 25s)); // 25 sec between runs
		} while (true);

		ReleaseMutex(globalMutex);
	}
	catch (std::exception e)
	{
		printf("An unexpected error occurred. Process had to be terminated: %s\n", e.what());
	}
	catch (...)
	{
		printf("An unexpected error occurred. Process had to be terminated\n");
	}

	return 0;
}

#if _WIN32
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	int argc = 0;
	LPWSTR* wideStringArguments = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		args.push_back(converter.to_bytes(wideStringArguments[i]));
	}
	LocalFree(wideStringArguments);

	std::vector<char*> argv;
	for (auto& arg : args)
		argv.push_back(arg.data());

	return CommonMain(args.size(), argv.data());
}
#endif

int main(int argc, char* argv[])
{
	return CommonMain(argc, argv);
}