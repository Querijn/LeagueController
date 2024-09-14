#include <spek/file/default_filesystem.hpp>

namespace Spek
{
	DefaultFileSystem::DefaultFileSystem(const char* a_Root) :
		BaseFileSystem(a_Root)
	{
	}

	bool DefaultFileSystem::Has(File::Handle a_Pointer) const
	{
		if (a_Pointer == nullptr)
			return false;

		for (auto& t_Handle : m_Files)
			if (t_Handle.second == a_Pointer)
				return true;
		return false;
	}
}
