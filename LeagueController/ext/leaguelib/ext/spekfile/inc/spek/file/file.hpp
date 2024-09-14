#pragma once

#include <spek/util/types.hpp>
#include <spek/util/callback_container.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

namespace Spek
{
	// TODO: Move this to its own file
	class Loadable
	{
	public:
		using Handle = std::shared_ptr<Loadable>;
		enum class LoadState
		{
			NotLoaded,
			NotFound,
			FailedToLoad,
			Loaded
		};

		static const char* GetLoadStateString(LoadState a_State);

		LoadState GetLoadState() const;

	protected:
		LoadState m_Status = LoadState::NotLoaded;
	};

	class BaseFileSystem;
	class File : public Loadable
	{
	public:
		using LoadState = Loadable::LoadState;

		enum class SaveState
		{
			NotSupported,
			NoChanges,
			InProgress,
			FailedToSave,
			Saved
		};

		enum LoadFlags
		{
			NoLoadFlags,
			TryCreate = 0b1,
		};

		enum MountFlags
		{
			NoMountFlags,
			CantStore = 0b1,
		};

		using Handle = std::shared_ptr<File>;
		using WeakHandle = std::weak_ptr<File>;
		using OnLoadFunctionLegacy = std::function<void(File::Handle f, File::LoadState a_LoadState)>;
		using OnLoadFunction = std::function<void(File::Handle)>;
		using OnSaveFunction = std::function<void(File::Handle f, File::SaveState a_SaveState)>;
		using OnMountFunction = std::function<void()>;
		using CallbackContainer = CallbackContainer<File::Handle>;
		using CallbackHandle = CallbackContainer::Handle;

		~File();

		template<typename FileSystemType>
		static FileSystemType* Mount(const char* a_Root, u32 a_MountFlags = 0)
		{
			auto ptr = std::make_unique<FileSystemType>(a_Root);
			auto* fs = ptr.get();
			MountFileSystemInternal(a_Root, std::move(ptr), a_MountFlags);
			return fs;
		}

		static void MountDefault(const char* a_Root = "", u32 a_MountFlags = 0);
		static bool IsFullyMounted();
		static bool IsReadyToExit();
		static size_t GetUnmountedFileSystems();
		static size_t GetMountedFileSystems();
		static bool IsValid(File::Handle a_File);
		static void Update();
		static void OnMount(OnMountFunction a_OnMount);

		static Handle Load(const char* a_Name, OnLoadFunctionLegacy a_OnLoadCallback, u32 a_LoadFlags = LoadFlags::NoLoadFlags);
		static Handle Load(const char* a_Name, OnLoadFunction a_OnLoadCallback, u32 a_LoadFlags = LoadFlags::NoLoadFlags);

		CallbackHandle AddLoadFunction(OnLoadFunction a_OnLoadCallback);
		const std::vector<u8>& GetData() const;
		std::string GetDataAsString() const;
		void SetData(const std::vector<u8>& a_Data);
		void SetData(const u8* a_Data, size_t a_Size);
		std::string GetName() const;
		std::string GetFullName() const;

		void Save(OnSaveFunction a_OnSaveFunction = nullptr);

		size_t Read(u8* a_Dest, size_t a_ByteCount, size_t& a_Offset) const;

		template<typename Type>
		bool Get(Type& a_Element, size_t& a_Offset)
		{
			return Read((u8*)&a_Element, sizeof(Type), a_Offset) == sizeof(Type);
		}

		template<typename Type>
		bool Get(std::vector<Type>& a_Element, size_t& a_Offset)
		{
			size_t t_Count = a_Element.size();
			return Read((u8*)a_Element.data(), t_Count * sizeof(Type), a_Offset) == t_Count * sizeof(Type);
		}

		template<typename Type>
		bool Get(std::vector<Type>& a_Element, size_t a_Count, size_t& a_Offset)
		{
			a_Element.resize(a_Count);
			return Read((u8*)a_Element.data(), a_Count * sizeof(Type), a_Offset) == a_Count * sizeof(Type);
		}

		friend class BaseFileSystem;
	private:
		File(BaseFileSystem& a_FileSytem, const char* a_FileName);
		static void MountFileSystemInternal(const char* a_Root, std::unique_ptr<BaseFileSystem>&& a_FileSystem, u32 a_MountFlags);
		void PrepareSize(size_t a_Size);
		u8* GetDataPointer();
		void NotifyReloaded(File::Handle a_Handle);

		BaseFileSystem* m_FileSystem = nullptr;
		std::string m_Name;
		std::vector<u8> m_Data;

		CallbackContainer m_OnLoadCallbacks;
	};
}