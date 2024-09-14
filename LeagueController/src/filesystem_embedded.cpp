#include "filesystem_embedded.hpp"

#include <league_controller/profiler.hpp>
#include <filesystem>

namespace Spek
{
	int GetCommonLength(const char* a, const char* b);
}

namespace LeagueController
{
	using File = Spek::File;
	namespace fs = std::filesystem;

	std::unordered_map<fs::path, EmbeddedFileSystem::FileData> EmbeddedFileSystem::m_fileData;

	EmbeddedFileSystem::EmbeddedFileSystem(const char* root) :
		BaseFileSystem(root)
	{
	}

	void EmbeddedFileSystem::Add(std::string_view path, const u8* data, size_t size)
	{
		fs::path asPath(path);
		m_fileData[asPath] = FileData(data, size);
	}

	void EmbeddedFileSystem::RemoveAll()
	{
		m_fileData.clear();
	}

	const u8* EmbeddedFileSystem::GetFileData(std::string_view path, size_t& size)
	{
		Leacon_Profiler;
		if (Exists(path.data()) == false)
		{
			size = 0;
			return nullptr;
		}

		fs::path fullPath = GetRelativePath(path);
		const auto& origData = m_fileData[fullPath];
		size = origData.size;
		return origData.data;
	}

	bool EmbeddedFileSystem::Exists(const char* location)
	{
		Leacon_Profiler;
		if (IsFileStorable(location) == false)
			return false;

		fs::path fullPath = GetRelativePath(location);
		return m_fileData.find(fullPath) != m_fileData.end();
	}

	void EmbeddedFileSystem::IndexFiles(OnIndexFunction onIndex)
	{
		Leacon_Profiler;
		m_IndexState = IndexState::IsIndexed;
		if (onIndex)
			onIndex(m_IndexState);
	}

	File::Handle EmbeddedFileSystem::Get(const char* location, u32 loadFlags)
	{
		Leacon_Profiler;

		std::string lowerCasePath = location;
		std::transform(lowerCasePath.begin(), lowerCasePath.end(), lowerCasePath.begin(), ::tolower);
		
		if (Exists(lowerCasePath.c_str()) == false)
			return nullptr;
		
		File::Handle& filePointer = m_files[lowerCasePath];
		if (filePointer == nullptr)
		{
			MakeFile(*this, lowerCasePath.c_str(), filePointer);

			fs::path fullPath = GetRelativePath(lowerCasePath.c_str());
			const auto& origData = m_fileData[fullPath];
			filePointer->SetData(std::vector<u8>(origData.data, origData.data + origData.size));
			ResolveFile(filePointer, File::LoadState::Loaded);
		}
		
		return filePointer;
	}

	bool EmbeddedFileSystem::Has(File::Handle ptr) const
	{
		Leacon_Profiler;
		if (ptr == nullptr)
			return false;

		for (auto& t_Handle : m_files)
			if (t_Handle.second == ptr)
				return true;
		return false;
	}

	bool EmbeddedFileSystem::IsFileStorable(std::string_view inPath) const
	{
		Leacon_Profiler;
		fs::path path(inPath);
		if (path.is_absolute() == false)
		{
			if (path.has_parent_path() == false) // test.txt
				return true;

			return true;
		}

		auto root = fs::absolute(m_Root).generic_string();
		size_t commonLength = Spek::GetCommonLength(root.c_str(), path.generic_string().c_str());
		return commonLength >= root.length();
	}

	std::string EmbeddedFileSystem::GetRelativePath(std::string_view inPath) const
	{
		Leacon_Profiler;
		auto root = fs::absolute(m_Root).generic_string();
		auto path = fs::path(inPath).generic_string();
		int commonLength = Spek::GetCommonLength(root.c_str(), path.c_str());
		if (path[commonLength] == '/')
			commonLength++;

		return path.substr(commonLength);
	}

	bool EmbeddedFileSystem::IsReadyToExit() const
	{
		return true;
	}
}