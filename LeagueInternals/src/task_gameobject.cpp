#include <league_internals/task_gameobject.hpp>
#include <league_internals/offset_loader.hpp>
#include <league_internals/game_object.hpp>

#include <game_overlay/log.hpp>
#include <memory_helper/instruction.hpp>

#include <capstone/capstone.h>
#include <memory_helper/helper_functions.hpp>

#if LEAGUEHACKS_DEBUG_LOG
#include <game_overlay/overlay.hpp>
#include <game_overlay/log.hpp>
#endif

namespace LeagueController
{
#if LEAGUEHACKS_DEBUG_LOG
	extern GameOverlay::LogCategory g_offsetLog;
#else
	extern int g_offsetLog;
#define GameOverlay_LogDebug
#endif
	using namespace MemoryHelper;

	static constexpr std::initializer_list<u8>	championNamePattern = { 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 05, 0xCC, 0xCC, 0xCC, 0xCC, 0x5B }; // A1 ?? ?? ?? ?? 05 ?? ?? ?? ?? 5B
	static constexpr u8							championNameIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	championLevelPattern = { 0x8D, 0x9E, 0xCC, 0xCC, 0xCC, 0xCC, 0x83, 0xC4, 0x14, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0x6A, 0x00, 0x68, 0xCC, 0xCC, 0xCC, 0xCC, 0x53, 0x56 }; // 8D 9E ?? ?? ?? ?? 83 C4 14 B9 ?? ?? ?? ?? 6A 00 68 ?? ?? ?? ?? 53 56 
	static constexpr u8							championLevelIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	healthPattern = { 0x8D, 0xB0, 0xCC, 0xCC, 0xCC, 0xCC, 0x89, 0x44, 0x24, 0x14 }; // 8D B0 ?? ?? ?? ?? 89 44 24 14
	static constexpr u8							healthIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	spellBookPattern = { 0x81, 0xC1, 0xCC, 0xCC, 0x00, 0x00, 0x50, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x85, 0xC0, 0x74, 0x0E }; // 81 C1 ?? ?? 00 00 50 E8 ?? ?? ?? ?? 85 C0 74 0E
	static constexpr u8							spellBookIgnore = 0xCC;

	GameObjectTask::GameObjectTask()
	{
		m_approaches.push_back(std::make_unique<FunctionApproach>([](OffsetData& data, HMODULE base, Address baseAddress, const MODULEINFO& mod)
		{
			Leacon_Profiler;
			if (GameObject::championNameOffset != data.championNameOffset)
			{
				if (GameObject::championNameOffset == 0)
					GameObject::championNameOffset = data.championNameOffset;

				else if (data.championNameOffset == 0)
					data.championNameOffset = GameObject::championNameOffset;
			}

			if (GameObject::championLevelOffset != data.championLevelOffset)
			{
				if (GameObject::championLevelOffset == 0)
					GameObject::championLevelOffset = data.championLevelOffset;

				else if (data.championLevelOffset == 0)
					data.championLevelOffset = GameObject::championLevelOffset;
			}

			if (GameObject::healthOffset != data.healthOffset)
			{
				if (GameObject::healthOffset == 0)
					GameObject::healthOffset = data.healthOffset;

				else if (data.healthOffset == 0)
					data.healthOffset = GameObject::healthOffset;
			}

			if (GameObject::spellBookOffset != data.spellBookOffset)
			{
				if (GameObject::spellBookOffset == 0)
					GameObject::spellBookOffset = data.spellBookOffset;

				else if (data.spellBookOffset == 0)
					data.spellBookOffset = GameObject::spellBookOffset;
			}

			if (data.championNameOffset == 0)
			{
				data.championNameOffset = (Address)GetByOperand(data.championNameOffset, championNamePattern, championNameIgnore, 1, 1, mod, baseAddress);
				if (GameObject::championNameOffset != data.championNameOffset && data.championNameOffset != 0)
					GameObject::championNameOffset = data.championNameOffset;
			}

			if (data.championLevelOffset == 0)
			{
				data.championLevelOffset = (Address)GetByOperand(data.championLevelOffset, championLevelPattern, championLevelIgnore, 0, 1, mod, baseAddress);
				if (GameObject::championLevelOffset != data.championLevelOffset && data.championLevelOffset != 0)
					GameObject::championLevelOffset = data.championLevelOffset;
			}

			if (data.healthOffset == 0)
			{
				data.healthOffset = (Address)GetByOperand(data.healthOffset, healthPattern, healthIgnore, 0, 1, mod, baseAddress);
				if (GameObject::healthOffset != data.healthOffset && data.healthOffset != 0)
					GameObject::healthOffset = data.healthOffset;
			}

			if (data.spellBookOffset == 0)
			{
				u8* current = (u8*)base;
				u8* end = current + mod.SizeOfImage;
				while (true)
				{
					u8* code = (u8*)FindArrayOfBytesRPM(current, end, spellBookPattern, spellBookIgnore);
					if (code == nullptr)
					{
						GameOverlay_LogDebug(g_offsetLog, "FindArrayOfBytesRPM(spellBookOffset) failed: Unable to find array of bytes: %s\n", PrintBytePattern(spellBookPattern, spellBookIgnore).c_str());
						break;
					}

					if (code[0] != spellBookPattern.begin()[0])
					{
						current = code + 1;
						continue;
					}

					GameObject::spellBookOffset = *(u32*)(code + 2); // 81 C1 F8 24 00 00 -> F8 24 00 00 (not technically correct, but close enough)
					data.spellBookOffset = GameObject::spellBookOffset;
					break;
				}
			}

			GameOverlay_LogDebug(g_offsetLog, "Champion Name offset: 0x%x", GameObject::championNameOffset);
			GameOverlay_LogDebug(g_offsetLog, "Champion Level offset: 0x%x", GameObject::championLevelOffset);
			GameOverlay_LogDebug(g_offsetLog, "Health offset: 0x%x", GameObject::championLevelOffset);
			GameOverlay_LogDebug(g_offsetLog, "SpellBook offset: 0x%x", GameObject::spellBookOffset);
			if (data.championNameOffset != 0 && 
				data.championLevelOffset != 0 &&
				data.championLevelOffset != 0 &&
				data.spellBookOffset != 0 &&
				data.healthOffset != 0)
				return ApproachState::Succeeded;

			static int attemptCount = 0;
			if (attemptCount >= 5)
				return ApproachState::Failed;

			attemptCount++;
			return ApproachState::InProgress;
		}));
	}
}
