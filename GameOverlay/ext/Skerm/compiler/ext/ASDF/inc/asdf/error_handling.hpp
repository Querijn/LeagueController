#pragma once

#include <vector>
#include <cstdio>
#include <exception>
#include <string>

#define ASDF_ERROR_BUFFER_SIZE 1024

namespace ASDF
{
	class ErrorHandler
	{
	public:
		template <typename... Args>
		void AssertWarning(bool a_Condition, char const* const a_Message, Args const& ...a_Args) noexcept
		{
			if (a_Condition)
				return;

			char t_ErrorBuffer[ASDF_ERROR_BUFFER_SIZE];
			// TODO: sprintf an error message
			snprintf(t_ErrorBuffer, ASDF_ERROR_BUFFER_SIZE, a_Message, a_Args...);

			m_WarningMessages.push_back(t_ErrorBuffer);
			// printf("Warning: %s\n", t_ErrorBuffer);
		}

		template <typename... Args>
		void AssertError(bool a_Condition, char const* const a_Message, Args const& ...a_Args) noexcept
		{
			if (a_Condition)
				return;

			char t_ErrorBuffer[ASDF_ERROR_BUFFER_SIZE];
			// TODO: sprintf an error message
			snprintf(t_ErrorBuffer, ASDF_ERROR_BUFFER_SIZE, a_Message, a_Args...);

			m_ErrorMessages.push_back(t_ErrorBuffer);
			// printf("Error: %s\n", t_ErrorBuffer);
		}

		void AssertError(bool a_Condition, char const* const a_Message) noexcept
		{
			if (a_Condition)
				return;

			char t_ErrorBuffer[ASDF_ERROR_BUFFER_SIZE];
			// TODO: sprintf an error message
			snprintf(t_ErrorBuffer, ASDF_ERROR_BUFFER_SIZE, "%s", a_Message);

			m_ErrorMessages.push_back(t_ErrorBuffer);
			// printf("Error: %s\n", t_ErrorBuffer);
		}

		template <typename... Args>
		void AssertFatal(bool a_Condition, char const* const a_Message, Args const& ...a_Args)
		{
			if (a_Condition)
				return;

			char t_ErrorBuffer[ASDF_ERROR_BUFFER_SIZE];
			// TODO: sprintf an error message
			snprintf(t_ErrorBuffer, ASDF_ERROR_BUFFER_SIZE, a_Message, a_Args...);

			m_ErrorMessages.push_back(t_ErrorBuffer);
			// printf("Error: %s\n", t_ErrorBuffer);
			// throw new std::exception(t_ErrorBuffer);
			exit(-1);
		}

		void AssertFatal(bool a_Condition, char const* const a_Message)
		{
			if (a_Condition)
				return;

			char t_ErrorBuffer[ASDF_ERROR_BUFFER_SIZE];
			// TODO: sprintf an error message
			snprintf(t_ErrorBuffer, ASDF_ERROR_BUFFER_SIZE, "%s", a_Message);

			m_ErrorMessages.push_back(t_ErrorBuffer);
			printf("Error: %s\n", t_ErrorBuffer);
			// throw new std::exception(t_ErrorBuffer);
			exit(-1);
		}

		void OutputErrors();
		void OutputWarnings();

		void Clear();
		const std::vector<std::string>& GetErrors();
		const std::vector<std::string>& GetWarnings();

	private:
		std::vector<std::string> m_ErrorMessages;
		std::vector<std::string> m_WarningMessages;
	};
	
}