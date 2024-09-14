#pragma once

#include <spek/file/base_filesystem.hpp>

#include <filesystem>

namespace LeagueController
{
	class EmbeddedFileSystem : public Spek::BaseFileSystem
	{
	public:
		using File = Spek::File;
		struct FileData
		{
			FileData() = default;
			FileData(const u8* inData, size_t inSize) :
				data(inData),
				size(inSize)
			{
				
			}

			const u8* data = nullptr;
			size_t size = 0;
		};
		EmbeddedFileSystem(const char* root);

		static void Add(std::string_view path, const u8* data, size_t size);
		static void RemoveAll();

		const u8* GetFileData(std::string_view path, size_t& size);
		
		friend class File;
	protected:
		bool Exists(const char* location) override;
		void IndexFiles(OnIndexFunction onIndex) override;
		File::Handle Get(const char* location, u32 loadFlags) override;
		bool Has(File::Handle file) const override;
		bool IsFileStorable(std::string_view path) const override;
		std::string GetRelativePath(std::string_view path) const override;
		bool IsReadyToExit() const override;

	private:
		static std::unordered_map<std::filesystem::path, FileData> m_fileData;
		std::unordered_map<std::string, File::Handle> m_files;
	};
}