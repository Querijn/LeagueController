#include <league_controller/shared_memory.hpp>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <string>
#include <cassert>

namespace LeagueController
{
	namespace __internal
	{
		static SECURITY_ATTRIBUTES* GetSec()
		{
			static SECURITY_DESCRIPTOR sd;
			static SECURITY_ATTRIBUTES sa;

			memset(&sd, 0, sizeof(sd));
			InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
			memset(&sa, 0, sizeof(sa));
			sa.nLength = sizeof(sa);
			sa.lpSecurityDescriptor = &sd;

			return &sa;
		}

		static bool MakeBufferWin32(std::string_view name, size_t size, HANDLE& outMapFile, void*& outBuffer)
		{

			std::string fullName = std::string("Global\\") + name.data();
			outMapFile = nullptr;
			outBuffer = nullptr;

			outMapFile = CreateFileMapping
			(
				INVALID_HANDLE_VALUE,	// use paging file
				GetSec(),				// explicit access
				PAGE_READWRITE,			// read/write access
				0,						// maximum object size (high-order DWORD)
				size,					// maximum object size (low-order DWORD)
				fullName.c_str()		// name of mapping object
			);

			if (outMapFile == NULL)
			{
				auto error = GetLastError();
				assert(error != 0);
				return false;
			}

			outBuffer = MapViewOfFile
			(
				outMapFile,			// handle to map object
				FILE_MAP_ALL_ACCESS,	// read/write permission
				0,
				0,
				size
			);

			if (outBuffer == NULL)
			{
				auto error = GetLastError();
				assert(error != 0);
				CloseHandle(outMapFile);
				return false;
			}

			return true;
		}

		static bool OpenBufferWin32(std::string_view name, size_t size, HANDLE& outMapFile, void*& outBuffer)
		{
			std::string fullName = std::string("Global\\") + name.data();
			outMapFile = nullptr;
			outBuffer = nullptr;

			outMapFile = OpenFileMapping
			(
				FILE_MAP_ALL_ACCESS,	// read/write access
				FALSE,					// do not inherit the name
				fullName.c_str()		// name of mapping object
			);

			if (outMapFile == NULL)
			{
				assert(false);
				return false;
			}

			outBuffer = MapViewOfFile
			(
				outMapFile,	// handle to map object
				FILE_MAP_READ,	// read/write permission
				0,
				0,
				size
			);

			if (outBuffer == NULL)
			{
				assert(false);
				CloseHandle(outMapFile);
				return false;
			}

			return true;
		}

		void CloseBufferWin32(HANDLE& mapFile, void*& buffer)
		{
			UnmapViewOfFile(buffer);
			buffer = nullptr;

			CloseHandle(mapFile);
			mapFile = nullptr;
		}
	}

	struct SharedMemoryData
	{
		SharedMemoryData(bool isOwner) :
			owner(isOwner)
		{}

		~SharedMemoryData()
		{
			__internal::CloseBufferWin32(handle, buffer);
		}

		HANDLE handle = nullptr;
		void* buffer = nullptr;
		bool owner;
	};
	
	BaseSharedMemory::BaseSharedMemory(std::string_view name, size_t size, bool owner) :
		m_data(std::make_shared<SharedMemoryData>(owner))
	{
		if (owner)
			__internal::MakeBufferWin32(name.data(), size, m_data->handle, m_data->buffer);
		else
			__internal::OpenBufferWin32(name.data(), size, m_data->handle, m_data->buffer);
	}

	bool BaseSharedMemory::IsValid() const
	{
		return m_data ? m_data->buffer && m_data->handle : false;
	}
	
	void* BaseSharedMemory::GetBuffer() const
	{
		return m_data ? m_data->buffer : nullptr;
	}
}