#include "asdf/compiler.hpp"

#include <vector>
#include <map>
#include <string>
#include <set>
#include <cstring>

using namespace ASDF;

#if ASDF_APPLICATION
int main(int a_Argc, const char** a_Argv)
{
	Compiler t_Compiler;

	for (int i = 1; i < a_Argc; i++)
	{
		// --struct_folder "C:/path/to/struct/dir/"
		if (strcmp(a_Argv[i], "--struct_folder") == 0)
			t_Compiler.Schema.AddFolder(a_Argv[++i]);
	}

	for (int i = 1; i < a_Argc; i++)
	{
		// --struct_xml "C:/path/to/struct.xml"
		if (strcmp(a_Argv[i], "--struct_xml") == 0)
			t_Compiler.Schema.AddFile(a_Argv[++i]);
	}

	for (int i = 1; i < a_Argc; i++)
	{
		// --include_dir "asdf/"
		if (strcmp(a_Argv[i], "--include_dir") == 0)
			t_Compiler.Info.SetIncludeDirectory(a_Argv[++i]);
	}

	std::map<std::string, DataParser::ElementData> t_DataFiles;
	for (int i = 1; i < a_Argc; i++)
	{
		// --data_xml "C:/path/to/data.xml"
		if (strcmp(a_Argv[i], "--data_xml") == 0)
		{
			std::string t_SourceFile = a_Argv[++i];

			auto& t_ParseResult = t_Compiler.Data.ParseXML(t_SourceFile);
			t_DataFiles[t_SourceFile] = t_ParseResult;
		}
	}

	for (int i = 1; i < a_Argc; i++)
	{
		// --output_native "C:/path/to/output/location/" "C:/path/to/moustache_file.mst" ".ext"
		if (strcmp(a_Argv[i], "--output_native") == 0)
		{
			std::string_view t_OutputFolder = a_Argv[++i];
			std::string_view t_MoustacheFile = a_Argv[++i];
			std::string_view t_Extension = a_Argv[++i]; // TODO: Make optional
			t_Compiler.Schema.OutputNative(t_OutputFolder, t_MoustacheFile, t_Extension, t_Compiler.Serialisation);
		}
	}

	for (int i = 1; i < a_Argc; i++)
	{
		// --output_blob "C:/path/to/blob.asdf" "C:/path/to/data.xml"
		if (strcmp(a_Argv[i], "--output_blob") == 0)
		{
			std::string_view t_OutputFile = a_Argv[++i];
			std::string_view t_SourceFile = a_Argv[++i];
			auto t_SourceFileIndex = t_DataFiles.find(std::string(t_SourceFile));
			if (t_SourceFileIndex == t_DataFiles.end())
				t_DataFiles[std::string(t_SourceFile)] = t_Compiler.Data.ParseXML(t_SourceFile);

			t_Compiler.Data.OutputBlob(t_OutputFile, t_SourceFileIndex->second, t_Compiler.Serialisation);
		}
	}

	for (int i = 1; i < a_Argc; i++)
	{
		// --output_index "C:/path/to/header_output.hpp" "C:/path/to/moustache_file.mst"
		if (strcmp(a_Argv[i], "--output_index") == 0)
		{
			std::string_view t_OutputFolder = a_Argv[++i];
			std::string_view t_MoustacheFile = a_Argv[++i];
			t_Compiler.Schema.OutputIndex(t_OutputFolder, t_MoustacheFile);
		}
	}

	t_Compiler.Errors.OutputErrors();
	t_Compiler.Errors.OutputWarnings();
}
#endif