#if SPEK_EMSCRIPTEN

#include <spek/file/default_filesystem.hpp>
#include <spek/util/json.hpp>

#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;

#include <emscripten.h>

namespace Spek
{
	std::vector<File::Handle> m_IndexFiles;

	DefaultFileSystem::~DefaultFileSystem()
	{
	}

	void DefaultFileSystem::IndexFiles(OnIndexFunction a_OnIndex)
	{
		File::Handle t_File = GetInternal("index.json", false);
		m_IndexFiles.push_back(t_File);
		
		t_File->AddLoadFunction([this, a_OnIndex](File::Handle a_File)
		{
			m_IndexState = IndexState::FailedToIndex;
			if (a_File->GetLoadState() == File::LoadState::Loaded)
			{
				const auto& t_Data = a_File->GetData();
				JSON t_JSON((char*)t_Data.data(), t_Data.size());

				for (auto t_Entry : t_JSON)
					m_Entries.push_back(t_Entry.second->to_string());
				m_IndexState = IndexState::IsIndexed;
			}

			if (a_OnIndex)
				a_OnIndex(m_IndexState);
		});
	}

	struct LoadData
	{
		LoadData(File::Handle a_FilePointer) : FilePointer(a_FilePointer) {}
		File::Handle FilePointer;
	};

	void (*ResolveFunction)(File::Handle& a_File, File::LoadState a_State) = nullptr;
	u8* (*GetPointerFunction)(File::Handle& a_File, size_t a_Size) = nullptr;

	void OnLoad(void* a_UserArg, void* a_Data, int a_Size)
	{
		LoadData* t_Load = (LoadData*)a_UserArg;

		if (a_Size != 0)
		{
			u8* t_Target = GetPointerFunction(t_Load->FilePointer, a_Size);
			memcpy(t_Target, a_Data, a_Size);
		}
		ResolveFunction(t_Load->FilePointer, a_Size != 0 ? File::LoadState::Loaded : File::LoadState::FailedToLoad);
		delete t_Load;
	}

	void OnLoadFailed(void* a_UserArg)
	{
		OnLoad(a_UserArg, nullptr, 0);
	}

	File::Handle DefaultFileSystem::Get(const char* a_Location)
	{
		if (m_IndexState != IndexState::IsIndexed)
			return nullptr;

		if (Exists(a_Location))
			return GetInternal(a_Location, false);

		fs::path t_Path = "/" + m_Root;
		t_Path /= a_Location;
		auto t_PathString = t_Path.generic_string();
		printf("Requested '%s' but could not find this File!\n", t_PathString.c_str());
		return nullptr;
	}

	File::Handle DefaultFileSystem::GetInternal(const char* a_Location, bool a_Invalidate)
	{
		fs::path t_Path = "/" + m_Root;
		t_Path /= a_Location;
		auto t_PathString = t_Path.generic_string();

		File::Handle& t_FilePointer = m_Files[a_Location];
		if (t_FilePointer == nullptr)
		{
			MakeFile(*this, a_Location, t_FilePointer);

			if (ResolveFunction == nullptr)
			{
				ResolveFunction = BaseFileSystem::ResolveFile;
				GetPointerFunction = BaseFileSystem::GetFilePointer;
			}

			LoadData* t_Data = new LoadData(t_FilePointer);
			printf("Requesting '%s'\n", t_PathString.c_str());
			emscripten_async_wget_data(t_PathString.c_str(), (void*)t_Data, OnLoad, OnLoadFailed);
		}

		return t_FilePointer;
	}

	bool DefaultFileSystem::CanStore() const
	{
		return false;
	}

	void DefaultFileSystem::Update()
	{
	}

	void DefaultFileSystem::Store(File::Handle a_File, File::OnSaveFunction a_OnSave)
	{
		if (a_OnSave)
			a_OnSave(a_File, File::SaveState::NotSupported);
	}

	std::string DefaultFileSystem::GetAbsolutePath(std::string_view a_Path)
	{
		printf("ERROR: You forgot to make GetAbsolutePath!\n");
		assert(0);
		return std::string();
	}

	bool DefaultFileSystem::Exists(const char* a_Location)
	{
		fs::path t_Path = "/" + m_Root;
		t_Path /= a_Location;
		auto t_PathString = t_Path.generic_string();
		auto t_Index = std::find(m_Entries.begin(), m_Entries.end(), t_PathString);
		return t_Index != m_Entries.end();
	}
}
#endif
