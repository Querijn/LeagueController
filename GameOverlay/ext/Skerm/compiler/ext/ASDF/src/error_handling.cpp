#include <vector>
#include <string>
#include <asdf/error_handling.hpp>

namespace ASDF
{
	void ErrorHandler::OutputErrors()
	{
		if (m_ErrorMessages.size() == 0)
			return;

		printf("The following errors occurred during the run:\n");

		for (size_t i = 0; i < m_ErrorMessages.size(); i++)
		{
			printf("Error %zu: %s\n", i, m_ErrorMessages[i].c_str());
		}
	}

	void ErrorHandler::OutputWarnings()
	{
		if (m_WarningMessages.size() == 0)
			return;

		printf("The following warnings occurred during the run:\n");

		for (size_t i = 0; i < m_WarningMessages.size(); i++)
		{
			printf("Warning %zu: %s\n", i, m_WarningMessages[i].c_str());
		}
	}

	void ErrorHandler::Clear()
	{
		m_WarningMessages.clear();
		m_ErrorMessages.clear();
	}

	const std::vector<std::string>& ErrorHandler::GetErrors() { return m_ErrorMessages; }
	const std::vector<std::string>& ErrorHandler::GetWarnings() { return m_WarningMessages; }
}
