#pragma once

#include <glm/vec2.hpp>

namespace LeagueController
{
	void SetCrosshairPos(const glm::vec2& pos);
	void ShowCrosshair(bool shouldShow);
	void UpdateCursor();
}
