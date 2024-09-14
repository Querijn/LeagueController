#include <Windows.h>
#include <TlHelp32.h>
#include <shellapi.h>

#include <filesystem>

#if LEACON_SUBMISSION
#define SUB_FOLDER LEAGUECONTROLLER_OUTPUT_FOLDER"/Submission/"

static const char* g_submissionInjector = SUB_FOLDER "LeagueControllerLauncher.exe";
static const char* g_submissionDll = SUB_FOLDER "LeagueController.dll";
#endif

static const char* g_sourceInjector =	LEAGUECONTROLLER_OUTPUT_FOLDER "/OverlayInjector/"	CMAKE_INTDIR "/LeagueControllerLauncher.exe";
static const char* g_sourceLibrary = LEAGUECONTROLLER_OUTPUT_FOLDER "/LeagueController/"	CMAKE_INTDIR "/LeagueController.dll";

void KillLaunchers()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, "LeagueControllerLauncher.exe") == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

// This runs overlay injector as admin and lets it run
int main()
{
	// Kill all LeagueControllerLaunchers
	KillLaunchers();
	
	// If we're on submission, copy all the stuff over to the submission folder
#if LEACON_SUBMISSION
	// Create SUB_FOLDER if it does not exist
	std::filesystem::create_directories(SUB_FOLDER);

	std::filesystem::copy_file(g_sourceInjector, g_submissionInjector,	std::filesystem::copy_options::overwrite_existing);
	std::filesystem::copy_file(g_sourceLibrary,  g_submissionDll,		std::filesystem::copy_options::overwrite_existing);

	static const char* g_launchable = g_submissionInjector;
	ShellExecuteA(NULL, "open", SUB_FOLDER, NULL, NULL, SW_SHOWDEFAULT);
#else
	static const char* g_launchable = g_sourceInjector;
#endif

	// Open launcher
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	CreateProcessA(g_launchable, nullptr, nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, NULL, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}