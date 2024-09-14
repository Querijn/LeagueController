#pragma once

#include <game_overlay/surface.hpp>

#include <spek/util/types.hpp>

#include <glm/vec2.hpp>

namespace GameOverlay
{
	inline constexpr u32 GetDebugColor(float inRed, float inGreen, float inBlue, float inAlpha = 1.0f)
	{
		u32 color = 0;

		color |=  (u8)(inRed * 255.0f) % 256;
		color |= ((u8)(inGreen * 255.0f) % 256) << 8;
		color |= ((u8)(inBlue * 255.0f) % 256) << 16;
		color |= ((u8)(inAlpha * 255.0f) % 256) << 24;

		return color;
	}

	extern Surface DebugDraw;

	void ValidateDebugDraw();
	void RenderDebugDraw();
}