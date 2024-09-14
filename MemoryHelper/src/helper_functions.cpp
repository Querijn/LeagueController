#include <memory_helper/helper_functions.hpp>

#if _WIN32
#include <Windows.h>
#endif

namespace MemoryHelper
{
	void* FindArrayOfBytes(void* startAddress, void* endAddress, const std::vector<u8>& pattern, u8 ignoreCase)
	{
		u8* begin = (u8*)startAddress;
		u8* end = (u8*)endAddress;

		int patternIterator = 0;
		for (int i = 0; begin + i < end; i++)
		{
			if (begin[i] == pattern[patternIterator] || pattern[patternIterator] == ignoreCase)
			{
				patternIterator++;

				if (patternIterator == pattern.size())
					return begin + i - patternIterator + 1;
			}
			else
			{
				i -= patternIterator;
				patternIterator = 0;
			}
		}

		return 0;
	}

#if _WIN32
	void* FindArrayOfBytesRPM(void* startAddress, void* endAddress, const std::vector<u8>& pattern, u8 ignoreCase)
	{
		u8* begin = (u8*)startAddress;
		u8* end = (u8*)endAddress;

		auto curProc = GetCurrentProcess();
		static constexpr size_t readSize = 0x1000;
		std::vector<u8> copy(readSize);
		SIZE_T bytesRead = 0;
		size_t step = readSize - pattern.size() + 1;

		for (; begin < end; begin += step)
		{
			size_t bytesToRead = std::min((size_t)(end - begin), readSize);
			if (ReadProcessMemory(curProc, begin, copy.data(), bytesToRead, &bytesRead) == false)
				continue;

			size_t bytesToScan = std::min(bytesToRead, (size_t)bytesRead);
			void* result = FindArrayOfBytes(copy.data(), copy.data() + bytesToScan, pattern, ignoreCase);
			if (result != nullptr)
			{
				u64 res64 = (u64)result - (u64)copy.data() + (u64)begin;
				return (void*)res64;
			}
		}

		return nullptr;
	}
#endif
}