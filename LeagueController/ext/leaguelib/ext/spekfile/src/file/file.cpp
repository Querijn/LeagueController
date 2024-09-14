#include <spek/file/file.hpp>
#include <spek/file/base_filesystem.hpp>
#include <spek/file/default_filesystem.hpp>
#include <spek/util/assert.hpp>

#include <map>
#include <algorithm>
#include <mutex>

namespace Spek
{
	Loadable::LoadState Loadable::GetLoadState() const
	{
		return m_Status;
	}

	const char* Loadable::GetLoadStateString(File::LoadState a_State)
	{
		switch (a_State)
		{
		case File::LoadState::FailedToLoad:
			return "failed to load";
		case File::LoadState::NotFound:
			return "not found";
		case File::LoadState::Loaded:
			return "loaded";
		default:
		case File::LoadState::NotLoaded:
			return "not loaded";
		}
	}

	struct UnmountedFileSystem
	{
		std::unique_ptr<BaseFileSystem> fs;
		size_t index;
	};

	std::unordered_map<std::string, UnmountedFileSystem> m_Unmounted;
	std::map<size_t, std::unique_ptr<BaseFileSystem>> m_Mounted;
	size_t m_IndexIterator;
	std::mutex m_FileSystemMutex;

	void File::MountDefault(const char* a_Root, u32 a_MountFlags)
	{
		MountFileSystemInternal(a_Root, std::make_unique<DefaultFileSystem>(a_Root), a_MountFlags);
	}

	bool File::IsFullyMounted()
	{
		return m_Unmounted.empty() && m_Mounted.empty() == false;
	}

	bool File::IsReadyToExit()
	{
		for (auto& t_FileSystem : m_Mounted)
			if (t_FileSystem.second->IsReadyToExit() == false)
				return false;

		return true;
	}

	size_t File::GetUnmountedFileSystems()
	{
		return m_Unmounted.size();
	}

	size_t File::GetMountedFileSystems()
	{
		return m_Mounted.size();
	}

	bool File::IsValid(File::Handle a_FileHandle)
	{
		if (a_FileHandle == nullptr)
			return false;

		std::lock_guard t_Lock(m_FileSystemMutex);
		for (auto i = m_Mounted.rbegin(); i != m_Mounted.rend(); ++i)
		{
			const auto& t_Filesystem = i->second;
			bool t_Result = t_Filesystem->Has(a_FileHandle);
			if (t_Result)
			{
				printf("We found %s in '%s'\n", a_FileHandle->GetName().c_str(), t_Filesystem->m_Root.c_str());
				return t_Result;
			}
		}

		return false;
	}

	void File::Update()
	{
		for (auto& t_Mounted : m_Mounted)
			t_Mounted.second->Update();

		// This loop is a bit special because unmounted might move to the mounted list unexpectedly
		for (auto it = m_Unmounted.begin(); it != m_Unmounted.end();)
		{
			size_t t_Size = m_Unmounted.size();
			it->second.fs->Update();
			if (t_Size != m_Unmounted.size())
				it = m_Unmounted.begin();
			else
				it++;
		}
	}

	void File::OnMount(OnMountFunction a_OnMount)
	{
		if (a_OnMount)
			a_OnMount();
		/*Application::AddInterruptLoopFunction([a_OnMount](Duration)
		{
			if (File::IsFullyMounted() == false)
				return Application::Actions::DoNotRender;

			if (a_OnMount)
				a_OnMount();
			return Application::Actions::IsDone;
		});*/
	}

	File::File(BaseFileSystem& a_FileSystem, const char* a_FileName) : 
		m_FileSystem(&a_FileSystem), m_Name(a_FileName) 
	{ }

	File::~File() { printf("Destroying file: %s\n", m_Name.c_str()); }

	File::Handle File::Load(const char* a_Name, OnLoadFunctionLegacy a_OnLoadCallback, u32 a_LoadFlags)
	{
		return File::Load(a_Name, [a_OnLoadCallback](File::Handle a_File)
		{
			if (a_OnLoadCallback)
				a_OnLoadCallback(a_File, a_File ? a_File->GetLoadState() : File::LoadState::NotFound);
		}, a_LoadFlags);
	}

	File::Handle File::Load(const char* a_Name, OnLoadFunction a_OnLoadCallback, u32 a_LoadFlags)
	{
		File::Handle t_Pointer = nullptr;
		{
			std::lock_guard t_Lock(m_FileSystemMutex);
			for (auto i = m_Mounted.rbegin(); i != m_Mounted.rend(); ++i)
			{
				const auto& t_FileSystem = i->second;
				t_Pointer = t_FileSystem->Get(a_Name, a_LoadFlags);
				if (t_Pointer)
					break;
			}
		}

		auto t_OnDone = [a_OnLoadCallback](File::Handle a_Pointer, bool a_TriedCreating, bool a_WasJustCreated)
		{
			if (a_Pointer == nullptr)
			{
				if (a_OnLoadCallback)
					a_OnLoadCallback(nullptr);
				return nullptr;
			}

			if (a_OnLoadCallback)
			{
				a_Pointer->AddLoadFunction(a_OnLoadCallback);

				if (a_TriedCreating)
				{
					if (a_WasJustCreated)
					{
						a_Pointer->m_Status = File::LoadState::Loaded;
					}
					else
					{
						a_Pointer->m_Status = File::LoadState::FailedToLoad;
					}
				}

				// If we've already Loaded the File, the callback doesn't launch until the file is reloaded, so we just call it.
				if (a_Pointer->m_Status != File::LoadState::NotLoaded)
					a_OnLoadCallback(a_Pointer);
			}
		};

		// If we've told the system that we should create it if it does not exist,
		// try create it with the first filesystem that can save.
		if (t_Pointer == nullptr && (a_LoadFlags & LoadFlags::TryCreate) != 0)
		{
			std::lock_guard t_Lock(m_FileSystemMutex);
			for (auto i = m_Mounted.rbegin(); i != m_Mounted.rend(); ++i)
			{
				const auto& t_FileSystem = i->second;
				if (t_FileSystem->CanStore() == false || t_FileSystem->IsFileStorable(a_Name) == false)
					continue;

				// This might be an absolute path. Make it relative.
				std::string t_Name = t_FileSystem->GetRelativePath(a_Name);

				// This is kind of a hack to make sure we have the name stored in a pointer that we just discard laterS
				BaseFileSystem::MakeFile(*t_FileSystem, t_Name.c_str(), t_Pointer);

				// TODO: this system is imperfect, it tries the first one that claims it works,
				// but if it fails to save, it won't try any other, and just returns a nullptr file.
				t_FileSystem->Store(t_Pointer, [&t_Pointer, t_OnDone, &t_FileSystem](File::Handle a_File, File::SaveState a_SaveState)
				{
					t_OnDone(a_File, true, a_File && a_SaveState == File::SaveState::Saved);
				});
				return t_Pointer;
			}
		}

		t_OnDone(t_Pointer, false, false);
		return t_Pointer;
	}

	File::CallbackHandle File::AddLoadFunction(OnLoadFunction a_OnLoadCallback)
	{
		return m_OnLoadCallbacks.Add(a_OnLoadCallback);
	}

	const std::vector<u8>& File::GetData() const
	{
		return m_Data;
	}

	std::string File::GetDataAsString() const
	{
		std::vector<u8> t_Data = GetData();
		t_Data.push_back(0);
		return (const char*)t_Data.data();
	}

	void File::SetData(const std::vector<u8>& a_Data)
	{
		m_Data = a_Data;
	}

	void File::SetData(const u8* a_Data, size_t a_Size)
	{
		m_Data = std::vector(a_Data, a_Data + a_Size);
	}

	size_t File::Read(u8* a_Dest, size_t a_Bytes, size_t& a_Offset) const
	{
		if (a_Offset + a_Bytes > m_Data.size())
		{
			printf("OUT OF BUFFER RANGE FOR %s: We're returning a reduced size => %zu\n", m_Name.c_str(), a_Offset > m_Data.size() ? 0 : m_Data.size() - a_Offset);
			if (a_Offset > m_Data.size())
				return 0;
			else
				a_Bytes = m_Data.size() - a_Offset;
		}

		if (a_Bytes == 0)
			return 0;

		memcpy(a_Dest, m_Data.data() + a_Offset, a_Bytes);

		a_Offset += a_Bytes;
		return a_Bytes;
	}

	void File::MountFileSystemInternal(const char* a_Root, std::unique_ptr<BaseFileSystem>&& a_FileSystem, u32 a_MountFlags)
	{
		std::string a_RootString = a_Root;
		a_FileSystem->IndexFiles([a_RootString](BaseFileSystem::IndexState a_State)
		{
			std::lock_guard t_Lock(m_FileSystemMutex);

			auto t_UnmountedIndex = m_Unmounted.find(a_RootString);
			if (t_UnmountedIndex == m_Unmounted.end())
				return;

			UnmountedFileSystem& t_Unmounted = t_UnmountedIndex->second;
			if (a_State == BaseFileSystem::IndexState::IsIndexed)
				m_Mounted[t_Unmounted.index] = std::move(t_Unmounted.fs);

			printf("File system %zu (\"%s\") is mounted\n", t_Unmounted.index, a_RootString.c_str());
			m_Unmounted.erase(t_UnmountedIndex);
		});

		a_FileSystem->SetFlags(a_MountFlags);

		std::lock_guard t_Lock(m_FileSystemMutex);
		size_t t_Index = m_IndexIterator++;
		if (a_FileSystem->m_IndexState == BaseFileSystem::IndexState::IsIndexed)
		{
			m_Mounted[t_Index] = std::move(a_FileSystem);
			printf("File system %zu (\"%s\") was immediately mounted\n", t_Index, a_Root);
		}
		else if (a_FileSystem->m_IndexState == BaseFileSystem::IndexState::NotIndexed)
		{
			m_Unmounted[a_Root] = { std::move(a_FileSystem), t_Index };
			printf("File system %zu (\"%s\") was moved to mount queue\n", t_Index, a_Root);
		}
		else
		{
			printf("File system %zu (\"%s\") immediately failed to index.\n", t_Index, a_Root);
		}
	}

	void File::PrepareSize(size_t a_Size)
	{
		m_Data.resize(a_Size);
	}

	u8* File::GetDataPointer()
	{
		return m_Data.data();
	}

	void File::NotifyReloaded(File::Handle a_Handle)
	{
		m_OnLoadCallbacks.Call(a_Handle);
	}

	std::string File::GetName() const
	{
		return m_Name;
	}

	std::string File::GetFullName() const
	{
		std::string t_Root = m_FileSystem->GetRoot();
		if (t_Root.empty() == false && t_Root[t_Root.size() - 1] != '/')
			t_Root += '/';

		return t_Root + GetName();
	}
	
	void File::Save(OnSaveFunction a_OnSaveFunction)
	{
		File::Handle t_Handle = m_FileSystem->Get(m_Name.c_str(), 0);
		if (m_FileSystem->CanStore() == false)
		{
			if (a_OnSaveFunction)
				a_OnSaveFunction(t_Handle, File::SaveState::NotSupported);
			return;
		}

		m_FileSystem->Store(t_Handle, a_OnSaveFunction);
	}
}
