#pragma once

#include <spek/util/types.hpp>
#include <vector>

namespace MemoryHelper
{
	void* FindArrayOfBytes(void* startAddress, void* endAddress, const std::vector<u8>& pattern, u8 ignoreCase);

#if _WIN32
	void* FindArrayOfBytesRPM(void* startAddress, void* endAddress, const std::vector<u8>& pattern, u8 ignoreCase);
#endif
}