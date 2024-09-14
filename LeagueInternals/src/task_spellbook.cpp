#include <league_internals/task_spellbook.hpp>
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

	static constexpr std::initializer_list<u8>	spellInfoPattern = { 0x55, 0x8B, 0xA9, 0xCC, 0xCC, 0xCC, 0xCC, 0x85, 0xED, 0x74, 0x5E }; // 55 8B A9 ?? ?? ?? ?? 85 ED 74 5E 
	static constexpr u8							spellInfoIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	spellInfoNamePattern = { 0x55, 0x8B, 0xA9, 0xCC, 0xCC, 0xCC, 0xCC, 0x85, 0xED, 0x74, 0x5E }; // 56 8D 73 ?? 8B C6 72 ??
	static constexpr u8							spellInfoNameIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	dummyPattern = { };
	static constexpr u8							dummyIgnore = 0xAB;

	void DummyArg(u64& a) {}
	bool FixOffset(const char* name, const std::initializer_list<u8>& pattern, u8 ignore, HMODULE base, Address baseAddress, const MODULEINFO& mod, u64& usedLocation, u64& saveFileLocation, int instructionIndex, int operandIndex, std::function<void(u64&)> fixOperation = DummyArg)
	{
		if (usedLocation != saveFileLocation)
		{
			if (usedLocation == 0)
				usedLocation = saveFileLocation;

			else if (saveFileLocation == 0)
				saveFileLocation = usedLocation;
		}

		if (saveFileLocation == 0)
		{
			saveFileLocation = (Address)GetByOperand(saveFileLocation, pattern, ignore, instructionIndex, operandIndex, mod, baseAddress);
			if (usedLocation != saveFileLocation && saveFileLocation != 0)
			{
				if (fixOperation != nullptr)
					fixOperation(saveFileLocation);
				usedLocation = saveFileLocation;
			}
		}

		GameOverlay_LogDebug(g_offsetLog, "%s: 0x%x", name, usedLocation);
		return saveFileLocation != 0;
	}

	SpellBookTask::SpellBookTask()
	{
		m_approaches.push_back(std::make_unique<FunctionApproach>([](OffsetData& data, HMODULE base, Address baseAddress, const MODULEINFO& mod)
		{
			Leacon_Profiler;

			// Spell Info
			bool succeeded =  FixOffset("Spell SpellInfo Offset",		spellInfoPattern,	spellInfoIgnore,	base, baseAddress, mod, Spell::spellInfoOffset,			data.spellInfoOffset,				 1,  1,		[](u64& a) { a += 0x4; });
				 succeeded &= FixOffset("Spell Slot Offset",			dummyPattern,		dummyIgnore,		base, baseAddress, mod, Spell::slotOffset,				data.spellSlotOffset,				-1, -1);
				 succeeded &= FixOffset("Spell Level Offset",			dummyPattern,		dummyIgnore,		base, baseAddress, mod, Spell::levelOffset,				data.spellLevelOffset,				-1, -1);
				 succeeded &= FixOffset("Spell Cooldown Offset",		dummyPattern,		dummyIgnore,		base, baseAddress, mod, Spell::cooldownOffset,			data.spellCooldownOffset,			-1, -1);

				 succeeded &= FixOffset("Spell Info Name Offset",		spellInfoPattern,	spellInfoIgnore,	base, baseAddress, mod, SpellInfo::nameOffset,			data.spellInfoNameOffset,			 2,  1);
				 succeeded &= FixOffset("Spell Info Castable Offset",	dummyPattern,		dummyIgnore,		base, baseAddress, mod, SpellInfo::notCastableOffset,	data.spellInfoNotCastableOffset,	-1, -1);

				 succeeded &= FixOffset("Spell Book Name Offset",		dummyPattern,		dummyIgnore,		base, baseAddress, mod, SpellBook::activeCastOffset,	data.spellBookActiveCastOffset,		-1, -1);
				 succeeded &= FixOffset("Spell Book Castable Offset",	dummyPattern,		dummyIgnore,		base, baseAddress, mod, SpellBook::spellsOffset,		data.spellBookSpellsOffset,			-1, -1);

			if (succeeded)
				return ApproachState::Succeeded;

			static int attemptCount = 0;
			if (attemptCount >= 5)
				return ApproachState::Failed;

			attemptCount++;
			return ApproachState::InProgress;
		}));
	}
}
