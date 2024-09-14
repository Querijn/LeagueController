#pragma once

#include <skerm/source_file.hpp>

#include <memory>
#include <vector>
#include <cstdint>
#include <string>

namespace Skerm
{
	namespace Internal { struct CompilerData; }

	class Compiler
	{
	public:
		Compiler();
		~Compiler();

		struct CompileOutput
		{
			std::vector<std::string> Errors;
			std::vector<std::string> Warnings;

		#if ASDF_DEBUG_DATA_PARSING
			std::string DebugData;
		#endif

			std::vector<uint8_t> Data;
		};

		void AddFolder(const char* inSource);

		bool CompileFile(const SourceFile& inUIFile, CompileOutput& inOutput);
		bool CompileFile(const SourceFile& inUIFile, const char* inOutputFile);
		bool CompileFile(const char* inUIFile, CompileOutput& inOutput);
		bool CompileFile(const char* inUIFile, const char* inOutputFile);

	private:
		std::unique_ptr<Internal::CompilerData> m_data;
	};
}
