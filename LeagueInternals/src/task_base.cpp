#include <league_internals/task_base.hpp>
#include <league_controller/profiler.hpp>

namespace LeagueController
{
	ApproachState BaseApproach::InternalResolve(OffsetData& addressData, HMODULE base, Address baseAddress, const MODULEINFO& mod)
	{
		Leacon_Profiler;
		if (m_state != ApproachState::InProgress && m_state != ApproachState::NotStarted)
			return m_state;

		m_state = Resolve(addressData, base, baseAddress, mod);
#ifdef LEACON_SHOULD_TEST
		if (m_state == ApproachState::Succeeded)
			m_state = Test() ? ApproachState::Succeeded : ApproachState::Failed;
#endif
		return m_state;
	}

	bool BaseApproach::Test()
	{
		return false;
	}
	
	ApproachState BaseTask::Resolve(OffsetData& addressData, HMODULE base, Address baseAddress, const MODULEINFO& mod)
	{
		Leacon_Profiler;
		if (m_approaches.empty())
			return ApproachState::Failed; // Can't succeed without approaches

		for (const auto& approach : m_approaches)
		{
			ApproachState result = approach->InternalResolve(addressData, base, baseAddress, mod);
			if (result == ApproachState::Succeeded || 
				result == ApproachState::InProgress) // In progress or succeeded? Don't continue.
				return result;
		}

		return ApproachState::Failed;
	}
	
	FunctionApproach::FunctionApproach(Callback function) :
		m_function(function)
	{
	}

	ApproachState FunctionApproach::Resolve(OffsetData& addressData, HMODULE base, Address baseAddress, const MODULEINFO& mod)
	{
		Leacon_Profiler;
		return m_function(addressData, base, baseAddress, mod);
	}
}
