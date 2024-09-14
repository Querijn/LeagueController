#pragma once

#include <memory_helper/address.hpp>

#include <initializer_list>
#include <Windows.h>
#include <psapi.h>

namespace LeagueController
{
	struct OffsetData;
	using Address = MemoryHelper::Address;
	
	void LoadOffsetsInternal(OffsetData& data);
	bool IsInGameInternal(OffsetData& data);
	
	void* GetByOperand(Address& relativeAddress, const std::initializer_list<u8>& pattern, u8 ignoreCase, int initialInstructionIndex, int initialOperandIndex, const MODULEINFO& mod, Address inBegin, Address inEnd = 0, Address* outFoundLocation = nullptr);
	bool LoadSingletonInternal(const char* name, void*& dest, Address& address, const std::initializer_list<u8>& pattern, u8 ignoreCase, int instructionIndex, int operandIndex, HMODULE base, Address baseAddress, const MODULEINFO& mod, Address* outFoundLocation = nullptr);
	
	template<typename T>
	bool LoadSingleton(const char* name, T*& dest, Address& address, const std::initializer_list<u8>& pattern, u8 ignoreCase, int instructionIndex, int operandIndex, HMODULE base, Address baseAddress, const MODULEINFO& mod, Address* outFoundLocation = nullptr)
	{
		return LoadSingletonInternal(name, (void*&)dest, address, pattern, ignoreCase, instructionIndex, operandIndex, base, baseAddress, mod, outFoundLocation);
	}

	template<typename T>
	static std::string PrintBytePattern(const T& pattern, u8 ignoreCase)
	{
		char buffer[256];
		int offset = 0;
		for (auto b : pattern)
		{
			if (b != ignoreCase)
			{
				if (b < 10)
					offset += snprintf(buffer + offset, 128 - offset, "%0x ", (int)b);
				else
					offset += snprintf(buffer + offset, 128 - offset, "%x ", (int)b);
			}
			else
			{
				offset += snprintf(buffer + offset, 128 - offset, "?? ");
			}
		}

		return buffer;
	}
}
