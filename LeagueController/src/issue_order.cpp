#include "issue_order.hpp"
#include "controller.hpp"
#include "debug_render.hpp"
#include "script_game.hpp"

#include <spek/util/duration.hpp>

#include <league_internals/offsets.hpp>
#include <league_internals/game_object.hpp>

#if LEAGUEHACKS_DEBUG_LOG
#include <game_overlay/overlay.hpp>
#include <game_overlay/log.hpp>
#endif

namespace LeagueController
{
	using Duration = Spek::Duration;
	using namespace Spek;

	// This file is heavily redacted for security reasons.
	// The original code used to call functions in the game to issue orders to the player's character.
	// This is very useful to scripters, and contained non-public information.
	// The code has been removed to prevent abuse.

	bool StopMoving()
	{
		Leacon_Profiler;
		// REDACTED for security reasons
		return false;
	}

	bool TryAttack(GameObject* object)
	{
		Leacon_Profiler;
		// REDACTED for security reasons
		return false;
	}

	bool MovePlayerTo(const glm::vec3& pos)
	{
		Leacon_Profiler;
		// REDACTED for security reasons
		return false;
	}
	
	void InitOrders()
	{
		// REDACTED for security reasons
	}

	void UpdateOrders()
	{
		Leacon_Profiler;
		// REDACTED for security reasons
	}
	
	void DestroyOrders()
	{
		Leacon_Profiler;
		// REDACTED for security reasons
	}
}