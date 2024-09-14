#include "league_lib/wad/wad_filesystem.hpp"

#include <spek/util/assert.hpp>

#include <filesystem>
#include <string>
#include <algorithm>
#include <fstream>

#include <xxhash64.h>

namespace LeagueLib
{
	using namespace Spek;
	namespace fs = std::filesystem;

	WADFileSystem::WADFileSystem(const char* a_Root) :
		BaseFileSystem(a_Root)
	{
		// This ensures that the WAD can find the archive file
		Spek::File::MountDefault(a_Root, File::MountFlags::CantStore);
	}

	WADFileSystem::~WADFileSystem()
	{
	}

	void WriteAllBytesToFile(const char* fileName, const std::vector<u8>& data)
	{
		const char* actualFileName = strrchr(fileName, '/') + 1;
		std::ofstream fout;
		fout.open(actualFileName, std::ios::binary | std::ios::out);
		fout.write((char*)data.data(), data.size());
		fout.close();
	}

	void WADFileSystem::Update()
	{
		if (m_IndexState != BaseFileSystem::IndexState::IsIndexed)
			return;

		while (m_LoadRequests.empty() == false)
		{
			// Copy the array, because callbacks might introduce new files to load!
			auto t_LoadRequests = m_LoadRequests;
			m_LoadRequests.clear();

			for (auto& t_FileToLoad : t_LoadRequests)
			{
				std::vector<u8> t_Container;
				if (t_FileToLoad.Archive->ExtractFile(t_FileToLoad.Name.c_str(), t_Container) == false)
				{
					SPEK_ASSERT(false, "Was unable to load this file!");
					ResolveFile(t_FileToLoad.File, File::LoadState::FailedToLoad);
					continue;
				}

				u8* t_Data = GetFilePointer(t_FileToLoad.File, t_Container.size());
				memcpy(t_Data, t_Container.data(), t_Container.size());
				// WriteAllBytesToFile(t_FileToLoad.Name.c_str(), t_Container); // Uncomment for debug files

				ResolveFile(t_FileToLoad.File, File::LoadState::Loaded);
			}
		}

		for (auto& t_FileData : m_Files)
			if (t_FileData.second.use_count() < 2 && t_FileData.second->GetLoadState() != File::LoadState::NotLoaded) // Usecount = 1 if only the file system is holding onto it
				t_FileData.second = nullptr;

		for (auto t_FileData = m_Files.begin(); t_FileData != m_Files.end(); )
		{
			if (t_FileData->second == nullptr)
				t_FileData = m_Files.erase(t_FileData);
			else
				t_FileData++;
		}
	}

	bool WADFileSystem::Has(File::Handle a_Pointer) const
	{
		if (a_Pointer == nullptr)
			return false;

		for (auto& t_Handle : m_Files)
			if (t_Handle.second == a_Pointer)
				return true;
		return false;
	}

	void WADFileSystem::IndexFiles(OnIndexFunction a_OnIndex)
	{
		try
		{
			for (auto& t_WadEntry : fs::recursive_directory_iterator(m_Root))
			{
				auto t_Extension = t_WadEntry.path().extension();
				if (t_WadEntry.is_regular_file() == false || t_Extension != ".client")
					continue;

				m_Archives.emplace_back(std::make_unique<WAD>(t_WadEntry.path().generic_string().c_str()));
			}
		}
		catch (fs::filesystem_error e)
		{
		}

		IndexState t_IndexState = BaseFileSystem::IndexState::IsIndexed;
		for (auto& t_Archive : m_Archives)
		{
			if (t_Archive->IsParsed() == false)
				t_Archive->Parse();

			if (t_Archive->GetLoadState() == File::LoadState::NotLoaded)
			{
				t_IndexState = BaseFileSystem::IndexState::NotIndexed;
				break;
			}
			else if (t_Archive->GetLoadState() == File::LoadState::FailedToLoad)
			{
				t_IndexState = BaseFileSystem::IndexState::FailedToIndex;
				break;
			}
		}

		if (t_IndexState != BaseFileSystem::IndexState::NotIndexed)
		{
			m_IndexState = t_IndexState;
			if (a_OnIndex)
				a_OnIndex(m_IndexState);
		}
	}

	bool WADFileSystem::Exists(const char* a_Location)
	{
		uint64_t t_Hash = XXHash64::hash(a_Location, strlen(a_Location), 0);
		for (auto& t_Archive : m_Archives)
			if (t_Archive->HasFile(t_Hash))
				return true;

		return false;
	}

	std::string WADFileSystem::GetRelativePath(std::string_view a_AbsolutePath) const
	{
		return a_AbsolutePath.data();
	}

	File::Handle WADFileSystem::Get(const char* a_Location, u32 a_LoadFlags)
	{
		if (m_IndexState == IndexState::IsIndexed)
			return GetInternal(a_Location, false);

		printf("Requested '%s' but WADFileSystem is not indexed!\n", a_Location);
		return nullptr;
	}

	File::Handle WADFileSystem::GetInternal(const char* a_Location, bool a_Invalidate)
	{
		// Transform to lowercase
		std::string t_Location = a_Location;
		std::transform(t_Location.begin(), t_Location.end(), t_Location.begin(), tolower);

		WAD* t_ContainingArchive = nullptr;
		uint64_t t_Hash = XXHash64::hash(t_Location.c_str(), t_Location.length(), 0);
		for (auto& t_Archive : m_Archives)
		{
			if (t_Archive->HasFile(t_Hash))
			{
				t_ContainingArchive = t_Archive.get();
				break;
			}
		}

		if (t_ContainingArchive == nullptr) // We don't have this file in our WAD archives
			return nullptr;

		File::Handle& t_FilePointer = m_Files[t_Hash];

		bool t_Invalidate = t_FilePointer == nullptr || a_Invalidate;
		if (t_FilePointer == nullptr)
		{
			MakeFile(*this, t_Location.c_str(), t_FilePointer);

			m_LoadRequests.push_back({ t_ContainingArchive, t_FilePointer, t_Location });
			printf("Requested a load for '%s'.\n", a_Location);
		}
		return t_FilePointer;
	}

	bool WADFileSystem::IsReadyToExit() const
	{
		return true;
	}
}