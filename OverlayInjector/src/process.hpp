#pragma once

#include <string>
#include <string_view>
#include <Windows.h>

#undef GetCommandLine

namespace LeagueController
{
	enum ProcessQuery
	{
		No,
		NotAvailable,
		Yes
	};

	using u32 = unsigned int;
	struct Process
	{
		std::string processName;
		void* handle = INVALID_HANDLE_VALUE;
		u32 pid = 0;

		Process(const char* name);
		~Process();

		void Update();
		void Close();

		bool IsAlive() const;
		ProcessQuery IsModuleLoaded(std::string_view moduleName) const;

		u32 GetCommandLine(std::wstring& commandLine);

	private:
		static void* InitHandle(const char* name);
		static u32 GetPID(const char* name);
		bool CheckAlive();

	};
}