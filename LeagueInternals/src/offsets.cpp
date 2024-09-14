#include <league_internals/offsets.hpp>
#include <league_internals/offset_loader.hpp>
#include <league_internals/game_object.hpp>

#if LEAGUEHACKS_DEBUG_LOG
#include <game_overlay/overlay.hpp>
#endif

#include <memory_helper/instruction.hpp>
#include <memory_helper/helper_functions.hpp>

#include <capstone/capstone.h>
#include <Windows.h>
#include <psapi.h>
#include <glm/mat4x4.hpp>

using namespace MemoryHelper;

namespace LeagueController
{
	OffsetData g_offsetData;

#if LEAGUEHACKS_DEBUG_LOG
	GameOverlay::LogCategory g_offsetLog("Offsets");
#endif

	bool AreOffsetsLoaded()
	{
		// Check if all addresses are loaded
		AddressData::AddressName result = g_offsetData.FindFirstAddress([](auto data) { return data == 0; });
		
		// If so, return if all offsets are loaded.
		return result == 0 ? g_offsetData.ForEachOffset([](const auto& data) { return data != nullptr; }) : false;
	}

	void LoadOffsets()
	{
		LoadOffsetsInternal(g_offsetData);
	}

	bool IsInGame()
	{
		return IsInGameInternal(g_offsetData);
	}

	std::string GetMissingAddresses()
	{
#if LEAGUEHACKS_DEBUG_LOG
		std::string result;
		bool first = true;
		g_offsetData.ForEachNamedAddress([&result, &first](AddressData::AddressName name, Address a)
		{
			if (a)
				return;

			if (first)
				first = false;
			else
				result += ", ";
			result += name;
		});
		return result;
#else
		return "Undeterminable";
#endif
	}

	bool HasGameTime() { return g_offsetData.gameTime != nullptr; }
	f32 GetGameTime() { return g_offsetData.gameTime ? *g_offsetData.gameTime : 0.0f; }
	glm::mat4* GetViewMatrix() { return g_offsetData.viewMatrix; }
	glm::mat4* GetProjMatrix() { return g_offsetData.projMatrix; }
	GameObject* GetLocalPlayer() { return g_offsetData.localPlayer; }
	Manager* GetHeroManager() { return g_offsetData.heroManager; }
	GuiMenu* GetGuiMenu() { return g_offsetData.guiMenu; }
	GameObject* GetSelectedGameObject() { return g_offsetData.underMouseObject; }
	Manager* GetAttackableManager() { return g_offsetData.attackableManager; }

	bool HasMouseWorldPosition()
	{
		return g_offsetData.hudManager != nullptr &&
			g_offsetData.cursorTargetLogicAddress != 0 &&
			g_offsetData.cursorTargetPosRawAddress != 0;
	}

	bool GetMouseWorldPosition(glm::vec3& outPosition)
	{
		if (!HasMouseWorldPosition())
			return false;

		const u8* hud = (const u8*)g_offsetData.hudManager;
		const u8* ptr = *(const u8**)(hud + g_offsetData.cursorTargetLogicAddress);
		outPosition = (const glm::vec3&)(ptr[g_offsetData.cursorTargetPosRawAddress]);
		return true;
	}
}

