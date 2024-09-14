#pragma once

#include <asdf/type_info.hpp>

#include <cstdint>
#include <string_view>
#include <map>
#include <unordered_map>

namespace ASDF
{
	class SerialisationParser;
	class SchemaParser
	{
	public:
		SchemaParser(TypeInfo& a_Info);

		enum class AddResultType
		{
			NothingAdded = 0,
			CanNotParse = 1,
			ExtendUndefined = 2,
			Added = 3,
		};

		struct AddResult
		{
			AddResult(AddResultType a_Type) : Type(a_Type) {}
			AddResult(AddResultType a_Type, std::string_view a_Info) : Type(a_Type), Info(a_Info) {}

			AddResultType Type;
			std::string Info;
		};

		void AddFolder(std::string_view a_Folder);
		AddResult AddFile(std::string_view a_File);

		void OutputNative(std::string_view a_OutputFolder, std::string_view a_MoustacheFile, std::string_view a_FileExtension, SerialisationParser& a_SerialisationParser);
		void OutputIndex(std::string_view a_OutputFile, std::string_view a_MoustacheFile);

	private:
		std::map<std::string, std::string> m_FileToObjectMap;
		void OutputSingle(std::string_view a_ObjectName, std::string_view a_OutputFolder, std::string_view a_MoustacheFile, std::string_view a_FileExtension, SerialisationParser& a_SerialisationParser);

		void AddFolderFile(std::string_view a_File, std::unordered_map<std::string, std::vector<std::string>>& t_FilesToParseLater);

		ErrorHandler& m_ErrorHandler;
		TypeInfo& m_Info;
	};
}