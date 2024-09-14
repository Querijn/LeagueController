#pragma once

#include <league_controller/types.hpp>
#include <game_overlay/input.hpp>

#include <glm/vec2.hpp>

namespace GameOverlay
{
	namespace Input
	{
		bool IsKeyDown(KeyboardKey inKey);
		bool IsMouseButtonDown(MouseButton inButton);
		bool IsKeyPressed(KeyboardKey inKey);
		bool IsMouseButtonPressed(MouseButton inButton);
		bool HasScrolled();
		f32 ScrollDelta();

		const glm::ivec2& GetMousePosition();

		void Update();
	}
}
