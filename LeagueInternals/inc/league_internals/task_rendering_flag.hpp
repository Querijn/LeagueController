#pragma once

#include <league_internals/task_base.hpp>

namespace LeagueController
{
	class RenderingFlagTask : public BaseTask
	{
	public:
		RenderingFlagTask();
		~RenderingFlagTask() = default;
	};
}