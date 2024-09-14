#pragma once

#include <league_internals/task_base.hpp>

namespace LeagueController
{
	class HudManagerTask : public BaseTask
	{
	public:
		HudManagerTask();
		~HudManagerTask() = default;
	};
}