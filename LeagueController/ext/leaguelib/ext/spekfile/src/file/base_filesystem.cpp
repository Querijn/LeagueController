#include <spek/file/base_filesystem.hpp>
#include <spek/file/file.hpp>

namespace Spek
{
	BaseFileSystem::BaseFileSystem(const char* a_Root) :
		m_Root(a_Root)
	{
	}

	BaseFileSystem::~BaseFileSystem()
	{
	}

	bool BaseFileSystem::IsIndexed() const { return m_IndexState == IndexState::IsIndexed; }

	std::string BaseFileSystem::GetRoot() const
	{
		return m_Root;
	}

	void BaseFileSystem::SetFlags(u32 a_Flags)
	{
		m_Flags |= a_Flags;
	}
	
	bool BaseFileSystem::CanStore() const
	{
		return false;
	}

	void BaseFileSystem::Store(File::Handle& a_File, File::OnSaveFunction a_Function)
	{
		if (a_Function)
			a_Function(a_File, File::SaveState::NotSupported);
	}

	void BaseFileSystem::MakeFile(BaseFileSystem& a_FileSystem, const char* a_Location, File::Handle& a_FilePointer)
	{
		struct Enabler : public File { Enabler(BaseFileSystem& a_FileSystem, const char* a_Location) : File(a_FileSystem, a_Location) {} };
		a_FilePointer = std::make_shared<Enabler>(a_FileSystem, a_Location);
	}
	
	u8* BaseFileSystem::GetFilePointer(File::Handle& a_File, size_t a_Size)
	{
		a_File->PrepareSize(a_Size);
		return a_File->GetDataPointer();
	}
	
	void BaseFileSystem::ResolveFile(File::Handle& a_File, File::LoadState a_State)
	{
		a_File->m_Status = a_State;
		a_File->NotifyReloaded(a_File);
	}

	bool BaseFileSystem::IsFileStorable(std::string_view a_Path) const
	{
		return false;
	}
}
