#pragma once

#include <league_controller/config.hpp>

#include <spek/util/types.hpp>
#include <spek/util/duration.hpp>

#include <game_overlay/debug_renderer.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace LeagueController
{
	void RenderImGui();
	void IssueWasOrdered();

	void DrawLine3D(const glm::vec3& start, const glm::vec3& end, GameOverlay::Pixel color, f32 thickness = 1.0f, Spek::Duration duration = 0_ms);
}
