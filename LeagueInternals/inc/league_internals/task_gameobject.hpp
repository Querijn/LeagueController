#pragma once

#include <league_internals/task_base.hpp>

namespace LeagueController
{
	class GameObjectTask : public BaseTask
	{
	public:
		GameObjectTask();
		~GameObjectTask() = default;
	};
}