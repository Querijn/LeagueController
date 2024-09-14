#include <game_overlay/log.hpp>
#include <game_overlay/window.hpp>
#include <league_controller/config.hpp>

#include "renderer.hpp"

#include <mutex>
#include <cstdio>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>

#if !LEACON_SUBMISSION
#define PPK_ASSERT_ENABLED 1
#endif

#include "imgui.h"
#include "ppk_assert.h"

#if _WIN32
#include <Windows.h>
#endif

namespace GameOverlay
{
	static constexpr const char* timeFormat = "%T";
	static constexpr const char* logFormat = "%s [%5s] %16s: %s\n";
	std::shared_ptr<LogCategoryManager> g_manager;
	std::map<LogType, bool> g_displayedLogTypes;
	static constexpr int g_maxLogSize = -1;
	
	std::vector<std::string> g_messageBacklog;
	std::mutex g_backlogMutex;
	IF_NOT_SUBMISSION(static bool g_forceRenderLog = false);

	struct
	{
		std::mutex fileMutex;
		FILE* fp = nullptr;
		std::string filename;
		std::time_t lastLog = std::time(nullptr);
	} g_logData;

	using OpenLogMap = std::map<LogCategory*, bool>;
	OpenLogMap& GetDisplayedLogs()
	{
		static std::shared_ptr<OpenLogMap> displayedLogs;
		static std::mutex selectedLogsMutex;
		std::lock_guard lock(selectedLogsMutex);
		if (displayedLogs == nullptr)
			displayedLogs = std::make_shared<OpenLogMap>();
		return *displayedLogs;
	}

	struct LogCategoryData
	{
		std::mutex mutex;
		std::vector<LogMessage> log;
	};

	struct LogCategoryManager
	{
		using Array = std::vector<LogCategory*>;

		const LogCategory* operator[](const char* name) const
		{
			int low = 0;
			int high = m_log.size() - 1;

			for (int i = 0; i < 100; i++) // 100 attempts
			{
				int curr = (high - low) / 2 + low;

				int result = strcmp(m_log[curr]->GetName(), name);
				if (result == 0)
					return m_log[curr];
				else if (result > 0)
					low = curr;
				else
					high = curr;
			}

			return nullptr;
		}

		void Insert(LogCategory& category)
		{
			std::lock_guard t(m_mutex);
			m_log.push_back(&category);

			// Insertion sort
			for (auto i = begin(); i != end(); ++i)
				std::rotate(std::upper_bound(begin(), i, *i), i, std::next(i));
		}

		void Remove(LogCategory& category)
		{
			std::lock_guard t(m_mutex);
			auto index = std::find(m_log.begin(), m_log.end(), &category);
			if (index != m_log.end())
				m_log.erase(index);
		}

		Array::iterator begin() { return m_log.begin(); }
		Array::iterator end() { return m_log.end(); }
		Array::const_iterator begin() const { return m_log.begin(); }
		Array::const_iterator end() const { return m_log.end(); }
		bool empty() const { return m_log.empty(); }
		size_t size() const { return m_log.size(); }

		std::mutex& GetMutex() { return m_mutex; }

	private:
		std::mutex m_mutex;
		Array m_log;
	};
	
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

	std::stringstream GetCurrentTime()
	{
		time_t time = std::time(nullptr);
		std::stringstream ss;
		ss << std::put_time(std::localtime(&time), "%F %T"); // ISO 8601 without timezone information.
		return ss;
	}

	void ResetFile()
	{
		std::lock_guard t(g_logData.fileMutex);
		if (g_logData.fp != nullptr)
		{
			fclose(g_logData.fp);
			g_logData.fp = nullptr;
		}
	}

	FILE* GetFile()
	{
		if (g_logData.fp == nullptr)
		{
			std::lock_guard t(g_logData.fileMutex);
			if (g_logData.fp == nullptr && g_logData.filename.empty() == false)
			{
				char fileName[128];
				snprintf(fileName, 128, "%s.log", g_logData.filename.c_str());
				g_logData.fp = fopen(fileName, "a");

				fprintf(g_logData.fp, "\n\n\n-- Started session at %s --\n\n\n", GetCurrentTime().str().c_str());

				std::lock_guard t(g_backlogMutex);
				for (auto& message : g_messageBacklog)
					fprintf(g_logData.fp, "%s", message.c_str());
				g_messageBacklog.clear();
			}
		}

		return g_logData.fp;
	}

	const char* GameOverlay::GetLogTypeName(LogType type)
	{
		switch (type)
		{
		case GameOverlay::LogType::Debug:	return "Debug";
		case GameOverlay::LogType::Info:	return "Info";
		case GameOverlay::LogType::Warning:	return "Warning";
		case GameOverlay::LogType::Error:	return "Error";
		}

		return "Unknown";
	}

	void InitLog(std::string_view filename)
	{
		g_logData.filename = filename;
		ResetFile();
		GetFile();

		g_displayedLogTypes[LogType::Debug] = false;
		g_displayedLogTypes[LogType::Info] = true;
		g_displayedLogTypes[LogType::Warning] = true;
		g_displayedLogTypes[LogType::Error] = true;
	}

	void DestroyLog()
	{
		ResetFile();
		g_manager = nullptr;
	}

	const char* GetLogTypePrefixString(LogType type)
	{
		switch (type)
		{
		case LogType::Debug:	return "DEBUG";
		case LogType::Info:		return "INFO ";
		case LogType::Warning:	return "WARN ";
		case LogType::Error:	return "ERROR";
		}

		return "UNKN ";
	}

	LogCategory::LogCategory(std::string_view name) :
		m_name(name),
		m_data(std::make_unique<LogCategoryData>())
	{
		m_data->log.reserve(200);

		// This part is pretty dumb but necessary. All categories are probably initialised in static memory,
		// this means we have to use shared_ptrs that are locally defined in the categories.
		// That's because the deinitialisation between libraries and static memory is effectively random,
		// and the manager needs to be removed last.
		if (LogCategory::m_manager == nullptr)
		{
			static std::mutex mutex;
			static std::lock_guard t(mutex);
			if (g_manager == nullptr)
				g_manager = std::make_shared<LogCategoryManager>();
			LogCategory::m_manager = g_manager;
		}

		GetDisplayedLogs()[this] = true;
		LogCategory::m_manager->Insert(*this);
	}
	
	LogCategory::~LogCategory()
	{
		LogCategory::m_manager->Remove(*this);
	}
	
	const char* LogCategory::GetName() const
	{
		return m_name.c_str();
	}

	LogCategoryData& LogCategory::GetData() const
	{
		return *m_data;
	}

	LogCategoryManager& LogCategory::GetLogManager()
	{
		return *g_manager;
	}

	std::string PrintF(const char* format, va_list args, size_t& length)
	{
		va_list copy;
		va_copy(copy, args);
		int len = std::vsnprintf(nullptr, 0, format, copy);
		va_end(copy);

		assert(len >= 0);

		std::string s(std::size_t(len) + 1, '\0');
		std::vsnprintf(&s[0], s.size(), format, args);
		s.resize(len);
		length = len;
		return s;
	}

	static bool PrintToFile(const char* logFormat, const char* datetime, LogType type, const LogCategory& category, const char* msg)
	{
		FILE* fp = GetFile();
		if (fp)
		{
			fprintf(fp, logFormat, datetime, GetLogTypePrefixString(type), category.GetName(), msg);
			return true;
		}

		std::lock_guard t(g_backlogMutex);
		char messageBuffer[2048];
		snprintf(messageBuffer, 2048, logFormat, datetime, GetLogTypePrefixString(type), category.GetName(), msg);
		g_messageBacklog.push_back(messageBuffer);
		return false;
	}

	void AppendLog(LogCategory& category, LogType type, const char* format, ...)
	{
		std::string msg;
		size_t len;

		va_list args;
		va_start(args, format);
		msg = PrintF(format, args, len);
		va_end(args);

		// remove trailing endline
		if (msg[len - 1] == '\n')
		{
			msg[len - 1] = 0;
			len--;
		}

#if 1
		std::lock_guard lock(category.GetData().mutex);
		const auto& logEntry = category.GetData().log.emplace_back(category, type, msg);
		std::string datetime(100, 0);
		datetime.resize(std::strftime(&datetime[0], datetime.size(), timeFormat, std::localtime(&logEntry.time)));
#else
		std::string datetime = GetCurrentTime().str();
#endif

		printf(logFormat, datetime.c_str(), GetLogTypePrefixString(type), category.GetName(), msg.c_str());
		PrintToFile(logFormat, datetime.c_str(), type, category, msg.c_str());
		GameOverlay_ProfilerLog(msg.c_str(), msg.length());

#if _WIN32
		OutputDebugStringA((msg + "\n").c_str());
#endif
	}

	void OnAssertFailed(LogCategory& category, const char* file, int line, const char* functionName, const char* format, ...)
	{
		char msg[1024];
		int len = 0;
		{
			va_list args;
			va_start(args, format);
			len = vsnprintf(msg, 1024, format, args);
			va_end(args);
		}

		char fullAssert[1024];

		const char* offset1 = strrchr(file, '\\');
		const char* offset2 = strrchr(file, '/');
		const char* baseFileName = offset1 > offset2 ? offset1 : offset2;
		if (baseFileName == nullptr)
			baseFileName = file;
		else
			baseFileName = baseFileName + 1;

		len = snprintf(fullAssert, 1024, "Assertion failed at %s:%d (%s): %s", baseFileName, line, functionName, msg);

		// remove trailing endline
		if (fullAssert[len - 1] == '\n')
		{
			fullAssert[len - 1] = 0;
			len--;
		}
		
#if 1
		std::lock_guard lock(category.GetData().mutex);
		const auto& logEntry = category.GetData().log.emplace_back(category, LogType::Error, fullAssert);
		std::string datetime(100, 0);
		datetime.resize(std::strftime(&datetime[0], datetime.size(), timeFormat, std::localtime(&logEntry.time)));
#else
		std::string datetime = GetCurrentTime().str();
#endif

		printf(logFormat, datetime.c_str(), GetLogTypePrefixString(LogType::Error), category.GetName(), fullAssert);
		if (PrintToFile(logFormat, datetime.c_str(), LogType::Error, category, fullAssert))
			ForceFlush();

		GameOverlay_ProfilerLog(fullAssert, len);
#if _WIN32
		OutputDebugStringA((std::string(fullAssert) + "\n").c_str());
#endif
#if LEACON_SUBMISSION && _WIN32
		MessageBoxA(nullptr, fullAssert, "An error has occurred", 0);
#else
		PPK_ASSERT(false, fullAssert);
#endif
	}

	static bool IsShowingAnyLogs()
	{
		for (const auto& log : GetDisplayedLogs())
			if (log.second)
				return true;

		return false;
	}
	static bool IsShowingAllLogs()
	{
		for (const auto& log : GetDisplayedLogs())
			if (log.second == false)
				return false;
		return GetDisplayedLogs().empty() == false;
	}

	void ForceRenderLog()
	{
		IF_NOT_SUBMISSION(g_forceRenderLog = true);
	}

	bool CanRenderLog()
	{
		return IF_NOT_SUBMISSION(g_forceRenderLog ||) IsImguiEnabled();
	}

	void RenderLogImgui()
	{
		if (!CanRenderLog())
			return;

		Leacon_Profiler;
		if (g_logData.fp)
			fflush(g_logData.fp);

		if (!ImGui::Begin("GameOverlay Log"))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		bool first = true;

		auto& logManager = LogCategory::GetLogManager();
		if (ImGui::CollapsingHeader("Logs"))
		{
			Leacon_ProfilerSection("LogsHeader");
			ImGui::Indent();

			bool isShowingAllLogs = IsShowingAllLogs();
			if (ImGui::Checkbox("All", &isShowingAllLogs))
				for (auto& log : GetDisplayedLogs())
					log.second = isShowingAllLogs;

			for (auto categoryData : logManager)
			{
				bool& isShowing = GetDisplayedLogs()[categoryData];
				ImGui::SameLine();
				ImGui::Checkbox(categoryData->GetName(), &isShowing);
			}

			ImGui::Checkbox("Debug", &g_displayedLogTypes[LogType::Debug]);
			ImGui::SameLine();
			ImGui::Checkbox("Info", &g_displayedLogTypes[LogType::Info]);
			ImGui::SameLine();
			ImGui::Checkbox("Warning", &g_displayedLogTypes[LogType::Warning]);
			ImGui::SameLine();
			ImGui::Checkbox("Error", &g_displayedLogTypes[LogType::Error]);

			if (ImGui::Button("Clear log"))
				g_logData.lastLog = std::time(nullptr);

			ImGui::Unindent();
		}

		ImGui::NewLine();

		std::lock_guard t(LogCategory::GetLogManager().GetMutex());
		{
			Leacon_ProfilerSection("MessageView");
			std::vector<LogMessage*> sortedLog;
			size_t totalSize = 0;
			for (const auto& log : GetDisplayedLogs())
			{
				if (log.second == false)
					continue;

				const char* categoryName = log.first->GetName();
				auto& data = log.first->GetData();
				std::lock_guard t(data.mutex);

				size_t size = data.log.size();
				LogMessage* messages = data.log.data();
				for (size_t i = 0; i < size; i++)
				{
					LogMessage& msg = messages[size - 1 - i];
					if (msg.time < g_logData.lastLog)
						continue;

					if (g_displayedLogTypes[msg.type] == false)
						continue;

					sortedLog.push_back(&msg);
					totalSize++;
				}
			}

			std::sort(sortedLog.begin(), sortedLog.end(), [](const LogMessage* lhs, const LogMessage* rhs)
			{
				if (lhs == nullptr || rhs == nullptr)
					return lhs < rhs;
				return std::difftime(lhs->time, rhs->time) > 0.0;
			});
			if (totalSize > 0 && totalSize > g_maxLogSize)
				sortedLog.resize(g_maxLogSize);

			int logSize = (int)sortedLog.size();
			ImGuiListClipper clipper(logSize);
			while (clipper.Step())
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					auto& msg = *sortedLog[i];

					std::string datetime(100, 0);
					datetime.resize(std::strftime(&datetime[0], datetime.size(), timeFormat, std::localtime(&msg.time)));

					switch (msg.type)
					{
					case LogType::Info:
					case LogType::Debug:
						ImGui::Text(logFormat, datetime.c_str(), GetLogTypePrefixString(msg.type), msg.category->GetName(), msg.message.c_str());
						break;

					case LogType::Warning:
						ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), logFormat, datetime.c_str(), GetLogTypePrefixString(msg.type), msg.category->GetName(), msg.message.c_str());
						break;

					case LogType::Error:
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), logFormat, datetime.c_str(), GetLogTypePrefixString(msg.type), msg.category->GetName(), msg.message.c_str());
						break;
					}
				}
		}

		ImGui::End();
	}

	LogMessage::LogMessage(const LogCategory& inCategory, LogType t, std::string_view msg) :
		category(&inCategory),
		time(std::time(nullptr)),
		type(t),
		message(msg)
	{
	}

	void ForceFlush()
	{
		Leacon_Profiler;
		if (g_logData.fp)
			fflush(g_logData.fp);
	}

	void WaitForDebugger()
	{
		Leacon_Profiler;
		while (!IsDebuggerPresent())
			Sleep(1);
	}
}