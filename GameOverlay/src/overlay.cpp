#include <game_overlay/overlay.hpp>
#include <game_overlay/log.hpp>
#include <game_overlay/input.hpp>
#include <game_overlay/window.hpp>
#include <game_overlay/skerm.hpp>

#include <skerm/compiler.hpp>

#include "renderer.hpp"

#include <MinHook.h>

namespace GameOverlay
{
	LogCategory g_overlayLog("Overlay");
	bool g_shouldDestroyGameOverlay = false;

	void Start(std::string_view logFileName, bool isApp)
	{
		InitLog(logFileName);
		GameOverlay_LogInfo(g_overlayLog, "GameOverlayThread is running. Built at %s %s\n", __DATE__, __TIME__);

		MH_STATUS isInitialized = MH_Initialize();
		GameOverlay_Assert(g_overlayLog, isInitialized == MH_OK);

		InitInput();
		
		GameOverlay_LogInfo(g_overlayLog, "Initialising renderer..\n");
		if (isApp == false)
			InitRenderer();
	}

	void DestroyInternal()
	{
		DestroySkerm();
		DestroyRenderer();
		DestroyLog();
		DestroyInput();
		GameOverlay::DestroyWindow();

		MH_DisableHook(MH_ALL_HOOKS);
		MH_RemoveHook(MH_ALL_HOOKS);
		MH_STATUS isUninitialised = MH_Uninitialize();
		GameOverlay_Assert(g_overlayLog, isUninitialised == MH_OK);
	}

	void Destroy(bool immediately)
	{
		if (immediately)
			DestroyInternal();
		else
			g_shouldDestroyGameOverlay = true;
	}

	bool IsReady()
	{
		return IsRendererInitialised();
	}
}