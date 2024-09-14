#include <league_internals/task_hud_manager.hpp>
#include <league_internals/offset_loader.hpp>
#include <league_controller/profiler.hpp>

#include <memory_helper/instruction.hpp>

#include <capstone/capstone.h>

namespace LeagueController
{
	static constexpr std::initializer_list<u8>	hudInstancePattern = { 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x48, 0xCC, 0x8B, 0x41, 0xCC }; // A1 ? ? ? ? 8B 48 ? 8B 41 ?
	static constexpr u8							hudInstanceIgnore = 0xCC;
	
	HudManagerTask::HudManagerTask()
	{
		m_approaches.push_back(std::make_unique<FunctionApproach>([](OffsetData& data, HMODULE base, Address baseAddress, const MODULEINFO& mod)
		{
			Leacon_Profiler;
			Address foundLocation = 0;
			bool succeeded = LoadSingleton("hudManager", data.hudManager, data.hudManagerAddress, hudInstancePattern, hudInstanceIgnore, 0, 1, base, baseAddress, mod, &foundLocation);
			if (succeeded == false || foundLocation == 0)
				return ApproachState::Failed;

			u8* loc = (u8*)(baseAddress + foundLocation);
			auto instructions = MemoryHelper::Instruction::FromPattern(loc, 0xB, baseAddress);
			// instructions will look like this:
			// mov     eax, HudManager
			// mov     ecx, [eax + 14h]
			// mov     eax, [ecx + 1Ch]

			// mov     ecx, [eax + 14h] (instruction 1, cursorTargetLogicAddress)
			{
				auto&& cursorTargetLogicInstruction = instructions[1];
				if (cursorTargetLogicInstruction.GetOperandCount() != 2)
					return ApproachState::Failed;

				auto ops = cursorTargetLogicInstruction.GetOperands();
				if (ops[1].type != X86_OP_MEM)
					return ApproachState::Failed;

				data.cursorTargetLogicAddress = ops[1].mem.disp;
				if (data.cursorTargetLogicAddress == 0 || data.cursorTargetLogicAddress > 0x100) 
					return ApproachState::Failed;
			}

			// mov     eax, [ecx + 1Ch] (instruction 2, cursorTargetPosRawAddress)
			{
				auto&& cursorTargetLogicInstruction = instructions[2];
				if (cursorTargetLogicInstruction.GetOperandCount() != 2)
					return ApproachState::Failed;

				auto ops = cursorTargetLogicInstruction.GetOperands();
				if (ops[1].type != X86_OP_MEM)
					return ApproachState::Failed;

				data.cursorTargetPosRawAddress = ops[1].mem.disp;
				if (data.cursorTargetPosRawAddress == 0 || data.cursorTargetPosRawAddress > 0x100)
					return ApproachState::Failed;
			}

			return ApproachState::Succeeded;
		}));

		// Fallback to old data
		m_approaches.push_back(std::make_unique<FunctionApproach>([](OffsetData& data, HMODULE base, Address baseAddress, const MODULEINFO& mod)
		{
			Leacon_Profiler;
			if (data.hudManagerAddress == 0)
				return ApproachState::Failed;

			data.cursorTargetLogicAddress  = 0x14;
			data.cursorTargetPosRawAddress = 0x1C;
			return ApproachState::Succeeded;
		}));
	}
}
