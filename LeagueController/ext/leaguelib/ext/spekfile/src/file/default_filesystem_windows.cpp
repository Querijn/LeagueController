#if SPEK_WINDOWS

#include <spek/util/duration.hpp>
#include <spek/file/default_filesystem.hpp>

#include <filesystem>
#include <fstream>

#include <windows.h>
#include <tchar.h>
#include <set>
#include <cassert>

namespace fs = std::filesystem;

namespace Spek
{
	static const Spek::Duration g_TimeBetweenUpdates = 200_ms;

	struct FileData
	{
		File::Handle Pointer;
		fs::file_time_type LastWrite;
	};

	struct FileDataCompare
	{
		bool operator()(const FileData& lhs, const FileData& rhs) const
		{
			return lhs.Pointer.get() < rhs.Pointer.get();
		}
	};

	struct DirectoryData
	{
		DirectoryData(LPTSTR a_Dir) :
			m_Dir(a_Dir),
			Handle(FindFirstChangeNotification(a_Dir, false, FILE_NOTIFY_CHANGE_LAST_WRITE))
		{
		}

		struct LoadRequest
		{
			File::Handle Handle;
			fs::path Path;
			bool Exists;
		};

		struct LoadRequestCompare
		{
			bool operator()(const LoadRequest& lhs, const LoadRequest& rhs) const
			{
				assert(lhs.Path != rhs.Path);
				return lhs.Path < rhs.Path;
			}
		};
		using LoadRequestSet = std::set<LoadRequest, LoadRequestCompare>;

		struct SaveRequest
		{
			File::Handle Handle;
			File::OnSaveFunction Callback;
		};

		struct SaveRequestCompare
		{
			bool operator()(const SaveRequest& lhs, const SaveRequest& rhs) const
			{
				return lhs.Handle.get() < rhs.Handle.get();
			}
		};
		using SaveRequestSet = std::set<SaveRequest, SaveRequestCompare>;

		LPSTR m_Dir;
		HANDLE Handle;
		std::set<FileData, FileDataCompare> Files;
		LoadRequestSet LoadRequests;
		SaveRequestSet SaveRequests;
		Duration LastSaveTime;
		Duration LastUpdateTime;

		void ProcessLoadRequests(DefaultFileSystem& a_FS, Duration a_Begin);
		void ProcessSaveRequests();
	};

	DefaultFileSystem::~DefaultFileSystem()
	{
		for (auto& t_Pair : m_WatchedDirectories)
			delete t_Pair.second;
	}

	void DirectoryData::ProcessLoadRequests(DefaultFileSystem& a_FS, Duration a_Begin)
	{
		for (auto it = LoadRequests.begin(); it != LoadRequests.end();)
		{
			auto& t_FileToLoad = *it;
			auto t_End = GetTimeSinceStart();
			if (t_End - a_Begin > 2_ms)
				break;

			File::Handle t_FilePointer = t_FileToLoad.Handle;
			if (t_FileToLoad.Exists == false)
			{
				a_FS.ResolveFile(t_FilePointer, File::LoadState::NotFound);
				it = LoadRequests.erase(it);
				continue;
			}

			fs::path t_Path = t_FileToLoad.Path;
			auto t_ParentDirectory = fs::absolute(t_Path.parent_path());

			std::ifstream t_Stream;
			t_Stream.exceptions(t_Stream.exceptions() | std::ifstream::failbit | std::ifstream::badbit);

			try
			{
				t_Stream.open(t_Path.generic_string().c_str(), std::ios::binary | std::ios::ate);

				size_t t_Size = (size_t)t_Stream.tellg();

				bool t_IsValid = t_Size != ~0;
				if (t_IsValid && t_Size != 0)
				{
					u8* t_Data = a_FS.GetFilePointer(t_FilePointer, t_Size);
					t_Stream.seekg(0, std::ios::beg);
					t_Stream.read((char*)t_Data, t_Size);
				}

				// Add this file to the directory watcher
				Files.insert(FileData{ t_FilePointer, fs::last_write_time(t_Path) });

				// Message systems
				a_FS.ResolveFile(t_FilePointer, t_IsValid ? File::LoadState::Loaded : File::LoadState::FailedToLoad);
				it = LoadRequests.erase(it);
			}
			catch (std::system_error& e)
			{
				printf("Error loading %s: %s\n", t_Path.generic_string().c_str(), e.what());
			}
			catch (std::ios_base::failure& e)
			{
				printf("Error loading %s: %s\n", t_Path.generic_string().c_str(), e.what());
			}
		}
	}

	void DirectoryData::ProcessSaveRequests()
	{
		Duration t_TimeDiff = GetTimeSinceStart() - LastSaveTime;
		if (t_TimeDiff < 5_ms || SaveRequests.empty())
			return;
		
		// Copy and clear
		SaveRequestSet t_SaveRequests = SaveRequests;
		SaveRequests.clear();

		for (auto& t_SaveRequest : t_SaveRequests)
		{
			auto& t_FileToSave = t_SaveRequest.Handle;

			auto& t_Data = t_FileToSave->GetData();
			const std::string&& t_File = t_FileToSave->GetFullName();

			bool t_Success = false;
			if (t_Data.empty() == false)
			{
				std::ofstream t_Stream(t_File, std::ios::out | std::ios::binary);
				t_Stream.write((const char*)t_Data.data(), t_Data.size());
				t_Success = t_Stream.good();
			}
			else // idk why but it doesn't like the above for empty files. It has to be this exact format
			{
				std::ofstream t_Stream(t_File);
				t_Success = t_Stream.good();
			}

			if (t_SaveRequest.Callback)
				t_SaveRequest.Callback(t_FileToSave, t_Success && fs::exists(t_File) ? File::SaveState::Saved : File::SaveState::FailedToSave);
		}

		LastSaveTime = GetTimeSinceStart();
	}

	void DefaultFileSystem::Update()
	{
		auto t_Begin = GetTimeSinceStart();

		for (auto& t_DirInfo : m_WatchedDirectories)
		{

			if (m_IndexState == IndexState::IsIndexed)
			{
				t_DirInfo.second->ProcessLoadRequests(*this, t_Begin);
				t_DirInfo.second->ProcessSaveRequests();
			}

			auto t_End = GetTimeSinceStart();
			if (t_End - t_Begin > 2_ms)
				break;

			if (t_Begin < t_DirInfo.second->LastUpdateTime + g_TimeBetweenUpdates)
				continue;
			t_DirInfo.second->LastUpdateTime = GetTimeSinceStart();

			// Check for file updates
			DirectoryData& t_Dir = *t_DirInfo.second;
			if (WaitForSingleObject(t_Dir.Handle, 0) == WAIT_TIMEOUT)
				continue;

			bool t_Found = false;
			for (auto& t_FileData : t_Dir.Files)
			{
				fs::path t_FilePath = t_FileData.Pointer->GetName();
				try
				{
					auto t_NewWriteTime = fs::last_write_time(fs::path(m_Root) / t_FilePath);
					t_Found = true;
					if (t_FileData.LastWrite >= t_NewWriteTime)
						continue;
				}
				catch (std::filesystem::filesystem_error e) // Most likely "cannot find file", when the update just catches it while it's still saving
				{
					printf("Could not refresh %s: %s\n", t_FilePath.generic_string().c_str(), e.what());
					continue;
				}

				printf("Refreshing %s\n", t_FilePath.generic_string().c_str());
				GetInternal(t_FilePath.generic_string().c_str(), true); // Refresh file
			}

			if (t_Found == false) // Try again next update
				break;

			FindNextChangeNotification(t_Dir.Handle);

			t_End = GetTimeSinceStart();
			if (t_End - t_Begin > 2_ms)
				break;
		}
	}

	void DefaultFileSystem::IndexFiles(OnIndexFunction a_OnIndex)
	{
		auto t_Debug = fs::absolute(m_Root);
		m_IndexState = m_Root == "" || fs::exists(m_Root) ? IndexState::IsIndexed : IndexState::FailedToIndex;
		if (a_OnIndex)
			a_OnIndex(m_IndexState);
	}

	bool DefaultFileSystem::Exists(const char* a_Location)
	{
		if (IsFileStorable(a_Location) == false)
			return false;

		fs::path t_Path = m_Root;
		t_Path /= a_Location;
		return fs::exists(t_Path) && fs::is_directory(t_Path) == false;
	}

	File::Handle DefaultFileSystem::Get(const char* a_Location, u32 a_LoadFlags)
	{
		if (m_IndexState == IndexState::IsIndexed)
			return GetInternal(a_Location, false);

		printf("Requested '%s' but could not find this file!\n", a_Location);
		return nullptr;
	}

	File::Handle DefaultFileSystem::GetInternal(const char* a_Location, bool a_Invalidate)
	{
		bool t_Exists = Exists(a_Location);
		if (t_Exists == false && a_Invalidate == false)
			return nullptr;

		// printf("%s exists\n", t_Path.generic_string().c_str());
		File::Handle& t_FilePointer = m_Files[a_Location];
		
		bool t_Invalidate = t_FilePointer == nullptr || a_Invalidate;
		if (t_FilePointer == nullptr)
			MakeFile(*this, a_Location, t_FilePointer);

		// Add new directory watcher if it doesn't exist
		fs::path t_Path = m_Root;
		t_Path /= a_Location;
		auto t_ParentDirectory = fs::absolute(t_Path.parent_path());
		if (m_WatchedDirectories[t_ParentDirectory.generic_string()] == nullptr)
		{
		#if defined(UNICODE)
			std::wstring t_DirString = t_ParentDirectory.generic_wstring();
		#else
			std::string t_DirString = t_ParentDirectory.generic_string();
		#endif
			std::replace(t_DirString.begin(), t_DirString.end(), '/', '\\');
			m_WatchedDirectories[t_ParentDirectory.generic_string()] = new DirectoryData(const_cast<LPTSTR>(t_DirString.c_str()));
			printf("Watching directory %s\n", t_ParentDirectory.generic_string().c_str());
		}

		// Add file to load requests
		if (t_Invalidate)
			m_WatchedDirectories[t_ParentDirectory.generic_string()]->LoadRequests.insert({ t_FilePointer, t_Path, a_Invalidate || t_Exists });

		return t_FilePointer;
	}

	int GetCommonLength(const char* a, const char* b)
	{
		int i;
		for (i = 0; a[i] != 0 && b[i] != 0; i++)
		{
			if (a[i] != b[i])
				return i;
		}

		return i;
	}

	bool DefaultFileSystem::IsFileStorable(std::string_view a_Path) const
	{
		auto t_Path = fs::path(a_Path);
		if (t_Path.is_absolute() == false)
		{
			if (t_Path.has_parent_path() == false) // test.txt
				return true;
			
			return fs::exists(m_Root / t_Path.parent_path()); // parent_path/test.txt, parent path needs to exist
		}

		auto t_Root = fs::absolute(m_Root).generic_string();
		int t_CommonLength = GetCommonLength(t_Root.c_str(), t_Path.generic_string().c_str());
		return t_CommonLength >= t_Root.length();
	}

	std::string Spek::DefaultFileSystem::GetRelativePath(std::string_view a_Path) const
	{
		return fs::relative(a_Path, fs::path(m_Root)).generic_string();
		/*auto t_Root = fs::absolute(m_Root).generic_string();
		auto t_Path = fs::path(a_Path).generic_string();
		int t_CommonLength = GetCommonLength(t_Root.c_str(), t_Path.c_str());
		if (t_Path[t_CommonLength] == '/')
			t_CommonLength++;

		return t_Path.substr(t_CommonLength);*/
	}

	bool Spek::DefaultFileSystem::IsReadyToExit() const
	{
		for (const auto& t_Dir : m_WatchedDirectories)
			if (t_Dir.second->SaveRequests.empty() == false)
				return false;

		return true;
	}

	bool DefaultFileSystem::CanStore() const
	{
		return (m_Flags & File::MountFlags::CantStore) == 0;
	}

	void DefaultFileSystem::Store(File::Handle& a_File, File::OnSaveFunction a_OnSave)
	{
		fs::path t_Path = a_File->GetName();

		a_File = m_Files[t_Path.generic_string().c_str()];
		if (a_File == nullptr)
			MakeFile(*this, t_Path.generic_string().c_str(), a_File);

		// Add new directory watcher if it doesn't exist
		fs::path t_AbsolutePath = fs::absolute(m_Root) / GetRelativePath(t_Path.generic_string());
		fs::path t_ParentDirectory = t_AbsolutePath.parent_path();
		if (m_WatchedDirectories[t_ParentDirectory.generic_string()] == nullptr)
		{
		#if defined(UNICODE)
			std::wstring t_DirString = t_ParentDirectory.generic_wstring();
		#else
			std::string t_DirString = t_ParentDirectory.generic_string();
		#endif
			std::replace(t_DirString.begin(), t_DirString.end(), '/', '\\');
			m_WatchedDirectories[t_ParentDirectory.generic_string()] = new DirectoryData(const_cast<LPTSTR>(t_DirString.c_str()));
			printf("Watching directory %s\n", t_ParentDirectory.generic_string().c_str());
		}

		m_WatchedDirectories[t_ParentDirectory.generic_string()]->SaveRequests.insert({ a_File, a_OnSave });
	}
}
#endif
