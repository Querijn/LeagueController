#include <league_internals/task_rendering_flag.hpp>
#include <league_internals/offset_loader.hpp>

#include <game_overlay/log.hpp>
#include <memory_helper/instruction.hpp>

#include <capstone/capstone.h>

namespace LeagueController
{
	RenderingFlagTask::RenderingFlagTask()
	{
		m_approaches.push_back(std::make_unique<FunctionApproach>([](OffsetData& data, HMODULE base, Address baseAddress, const MODULEINFO& mod)
		{
			Leacon_Profiler;
			// REDACTED for sec reasons
			return ApproachState::Failed;
		}));
	}
}
