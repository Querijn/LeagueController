#pragma once

#include <league_controller/config.hpp>
#include <league_controller/secure_string.hpp>
#include <league_controller/profiler.hpp>

#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <ctime>

#define GameOverlay_Profiler					Leacon_Profiler
#define GameOverlay_ProfilerFrame(x)			Leacon_ProfilerFrame
#define GameOverlay_ProfilerSection(x)			Leacon_ProfilerSection(x)
#define GameOverlay_ProfilerTag(y, x)			Leacon_ProfilerTag(y, x)
#define GameOverlay_ProfilerLog(text, size)		Leacon_ProfilerLog(text, size)
#define GameOverlay_ProfilerValue(text, value)	Leacon_ProfilerValue(text, value)

namespace GameOverlay
{
	enum class LogType
	{
		Debug,
		Info,
		Warning,
		Error
	};

	const char* GetLogTypeName(LogType type);
	class LogCategory;
	struct LogMessage
	{
		LogMessage(const LogCategory& inCategory, LogType t, std::string_view msg);

		const LogCategory* category;
		std::time_t time;
		LogType type;
		std::string message;
	};

	struct LogCategoryManager;
	struct LogCategoryData;
	class LogCategory
	{
	public:
		LogCategory(std::string_view name);
		~LogCategory();

		const char* GetName() const;

		LogCategoryData& GetData() const;
		static LogCategoryManager& GetLogManager();
	private:
		std::shared_ptr<LogCategoryManager> m_manager;
		std::string m_name;
		std::unique_ptr<LogCategoryData> m_data;
	};

	void AppendLog(LogCategory& category, LogType type, const char* format, ...);
	void OnAssertFailed(LogCategory& category, const char* file, int line, const char* functionName, const char* format, ...);

	void InitLog(std::string_view filename);
	void DestroyLog();
	void RenderLogImgui();
	void ForceFlush();
	void WaitForDebugger();
	void ForceRenderLog();
}

#ifndef GameOverlay_CONCAT
#define GameOverlay_CONCAT(a, b) CONCAT_INNER(a, b)
#define GameOverlay_CONCAT_INNER(a, b) a ## b
#define GameOverlay_UNIQUE_NAME(base) CONCAT(base, __COUNTER__)
#endif

#ifndef GameOverlay_XStringify
#define GameOverlay_XStringify(a) GameOverlay_Stringify(a)
#define GameOverlay_Stringify(a) #a
#define GameOverlay_StringifySecure(a) SecureString(#a)
#endif

#define GameOverlay_DoOnce										static bool g_onlyOnce = false; if (g_onlyOnce) break; g_onlyOnce = true

// Debug logs only appear in debug mode
#if _DEBUG
#define GameOverlay_LogDebug(Category, Format, ...)				do { GameOverlay::AppendLog(Category, GameOverlay::LogType::Debug, Format, __VA_ARGS__); } while (0)
#define GameOverlay_WaitForDebugger()							do { GameOverlay::WaitForDebugger(); __debugbreak(); } while (false)
#define GameOverlay_WaitForDebuggerOnce()						do { GameOverlay_DoOnce; GameOverlay::WaitForDebugger(); __debugbreak(); } while (false)
#else
#define GameOverlay_LogDebug(Category, Format, ...)				do { } while (0)
#define GameOverlay_WaitForDebugger()							do { } while (0)
#define GameOverlay_WaitForDebuggerOnce()						do { static_assert(false, "Cannot wait for debugger in submission mode"); } while (0)
#endif

#define GameOverlay_LogInfo(Category, Format, ...)				do { GameOverlay::AppendLog(Category, GameOverlay::LogType::Info, Format, __VA_ARGS__); } while (0)
#define GameOverlay_LogWarning(Category, Format, ...)			do { GameOverlay::AppendLog(Category, GameOverlay::LogType::Warning, Format, __VA_ARGS__); } while (0)
#define GameOverlay_LogError(Category, Format, ...)				do { GameOverlay::AppendLog(Category, GameOverlay::LogType::Error, Format, __VA_ARGS__); } while (0)

#define GameOverlay_AssertF(Category, Cond, Format, ...)		do { if (Cond) break; GameOverlay::OnAssertFailed(Category, EXPOSE_FILE, __LINE__, __FUNCTION__, Format, __VA_ARGS__); } while (0)
#define GameOverlay_Assert(Category, Cond)						do { if (Cond) break; GameOverlay::OnAssertFailed(Category, EXPOSE_FILE, __LINE__, __FUNCTION__, GameOverlay_XStringify(Cond)); } while (0)

#define GameOverlay_LogDebugOnce(Category, Format, ...)			do { GameOverlay_DoOnce; GameOverlay_LogDebug(Category, Format, __VA_ARGS__); } while (0)
#define GameOverlay_LogInfoOnce(Category, Format, ...)			do { GameOverlay_DoOnce; GameOverlay_LogInfo(Category, Format, __VA_ARGS__); } while (0)
#define GameOverlay_LogWarningOnce(Category, Format, ...)		do { GameOverlay_DoOnce; GameOverlay_LogWarning(Category, Format, __VA_ARGS__); } while (0)
#define GameOverlay_LogErrorOnce(Category, Format, ...)			do { GameOverlay_DoOnce; GameOverlay_LogError(Category, Format, __VA_ARGS__); } while (0)