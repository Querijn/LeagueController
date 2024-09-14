#include "cursor.hpp"
#include "setup.hpp"

#include <game_overlay/log.hpp>
#include <game_overlay/image_instance.hpp>
#include <game_overlay/overlay.hpp>
#include <game_overlay/window.hpp>
#include <game_overlay/input.hpp>

#include <Windows.h>
#include <filesystem>

namespace LeagueController
{
	namespace fs = std::filesystem;

	extern GameOverlay::LogCategory g_setupLog;
	static std::shared_ptr<GameOverlay::ImageInstance> g_crosshair;
	static bool g_wasOrigCursorVisible = false;
	static bool g_isCursorVisible = false;

	void UpdateCursor()
	{
		Leacon_Profiler;
		if (g_crosshair == nullptr)
		{
			if (GetLauncherData() != nullptr && GameOverlay::IsReady())
				g_crosshair = std::make_shared<GameOverlay::ImageInstance>("Data/Crosshairs/crosshair012.png");
			else
				return;
		}

		// Hide the cursor when the game is in focus, show our crosshair.
		bool cursorVisible = GetFocus() != GameOverlay::GetWindow();
		bool crossHairVisible = g_isCursorVisible && cursorVisible == false;

		g_crosshair->SetShouldRender(crossHairVisible);
		if (g_wasOrigCursorVisible != cursorVisible)
		{
			GameOverlay_LogInfo(g_setupLog, "%s cursor", cursorVisible ? "Showing" : "Hiding");
			::ShowCursor(cursorVisible);
			g_wasOrigCursorVisible = cursorVisible;
		}
	}
	
	void ShowCrosshair(bool shouldShow)
	{
		Leacon_Profiler;
		g_isCursorVisible = shouldShow;
	}

	void SetCrosshairPos(const glm::vec2& inPos)
	{
		Leacon_Profiler;
		if (g_crosshair)
			g_crosshair->SetPos(glm::ivec2(inPos));
	}
}