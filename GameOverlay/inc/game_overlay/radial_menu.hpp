#pragma once

#include <spek/util/duration.hpp>

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <sokol.hpp>

namespace GameOverlay
{
	struct CirclePieceMesh
	{
		CirclePieceMesh();
		CirclePieceMesh(float inInnerSize, float inOuterSize, int inCircleSliceCount, int inCircleSegments = 64);
		
		struct DrawData
		{
			~DrawData();

			sg_pipeline pip;
			sg_bindings bind;
		};

		void Draw(const glm::mat4& inMvp, Spek::Duration inHoverTime);
		
		glm::vec2 min, max;
		std::vector<glm::vec2> pos;
		std::vector<int> indices;
		std::shared_ptr<DrawData> drawData;
	};

	class RadialMenu
	{
	public:
		struct MenuItem
		{
			std::string icon;
			std::string text;
			std::function<void(const MenuItem& item)> onClick;
		};

		struct HoverItem
		{
			Spek::Duration time;
			int index = -1;
		};

		RadialMenu(const glm::vec2& inPos = glm::vec2(0, 0), float inSize = 200);
		~RadialMenu();

		void AddMenuItem(const MenuItem& item);

		void Reset();
		void Draw(Spek::Duration inDt, const glm::mat4& viewProjection, float screenWidth, float screenHeight);

		void Set(const glm::vec2& inPos, float inSize);
		void SetPos(const glm::vec2& inPos);
		const glm::vec2& GetPos() const;
		
		void SetSize(float inSize);
		const float GetSize() const;

		static void Init();
		static bool WantsInput();

	private:
		std::vector<MenuItem> m_items;
		std::shared_ptr<CirclePieceMesh> m_mesh = nullptr;
		float m_size;
		glm::vec2 m_pos;

		HoverItem m_hoverItem;
		HoverItem m_lastHoverItem;
	};
}