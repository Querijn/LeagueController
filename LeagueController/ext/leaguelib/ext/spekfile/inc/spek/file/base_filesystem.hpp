#pragma once

#include <spek/file/file.hpp>
#include <spek/util/types.hpp>

#include <string>
#include <functional>

namespace Spek
{
	class BaseFileSystem
	{
	public:
		BaseFileSystem(const char* a_Root);
		virtual ~BaseFileSystem();

		virtual void Update() {}

		bool IsIndexed() const;

		std::string GetRoot() const;
		void SetFlags(u32 a_Flags);

		friend class File;
	protected:
		enum class IndexState
		{
			NotIndexed,
			FailedToIndex,
			IsIndexed
		};
		using OnIndexFunction = std::function<void(IndexState a_State)>;

		virtual bool Exists(const char* a_Location) = 0;
		virtual void IndexFiles(OnIndexFunction a_OnIndex) = 0;
		virtual File::Handle Get(const char* a_Location, u32 a_LoadFlags) = 0;
		virtual bool Has(File::Handle a_File) const = 0;
		virtual bool IsFileStorable(std::string_view a_Path) const;
		virtual std::string GetRelativePath(std::string_view a_Path) const = 0;
		virtual bool IsReadyToExit() const = 0;

		virtual bool CanStore() const;
		virtual void Store(File::Handle& a_File, File::OnSaveFunction a_Function);

		IndexState m_IndexState = IndexState::NotIndexed;
		std::string m_Root;
		u32 m_Flags = 0;

		static void MakeFile(BaseFileSystem& a_FileSystem, const char* a_Location, File::Handle& a_FilePointer);
		static u8* GetFilePointer(File::Handle& a_File, size_t a_Size);
		static void ResolveFile(File::Handle& a_File, File::LoadState a_State);
	};
}
