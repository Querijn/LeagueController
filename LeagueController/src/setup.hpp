#pragma once

#include <league_lib/navgrid/navgrid.hpp>
#include <league_controller/launcher_data.hpp>

namespace LeagueController
{
	class ChampionData;

	LeagueLib::NavGrid::Handle GetNavGrid();
	const ChampionData& GetChampionData();

	const LauncherData* GetLauncherData();
	const char* GetLauncherLocation();
	void UnloadSelf();
	bool IsApp();
}
