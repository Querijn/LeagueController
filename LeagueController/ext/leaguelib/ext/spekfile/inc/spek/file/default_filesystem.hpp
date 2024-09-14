#pragma once

#include <spek/file/base_filesystem.hpp>
#include <spek/file/file.hpp>

#include <vector>
#include <unordered_map>
#include <map>
#include <memory>

namespace Spek
{
	struct DirectoryData;
	class DefaultFileSystem : public BaseFileSystem
	{
	public:
		DefaultFileSystem(const char* a_Root);
		~DefaultFileSystem();

		void Update() override;

		friend class File;
		friend struct DirectoryData;
	protected:
		bool Exists(const char* a_Location) override;
		File::Handle Get(const char* a_Location, u32 a_LoadFlags) override;
		File::Handle GetInternal(const char* a_Location, bool a_Invalidate);
		bool Has(File::Handle a_Pointer) const override;
		void Store(File::Handle& a_File,	File::OnSaveFunction a_OnSave) override;
		void IndexFiles(OnIndexFunction a_OnIndex) override;
		bool IsFileStorable(std::string_view a_Path) const override;
		std::string GetRelativePath(std::string_view a_Path) const override;
		bool IsReadyToExit() const override;

		bool CanStore() const override;

		std::vector<std::string> m_Entries;
		std::unordered_map<std::string, std::shared_ptr<File>> m_Files;
		std::map<std::string, DirectoryData*> m_WatchedDirectories;
	};
}
