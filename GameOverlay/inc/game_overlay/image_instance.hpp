#pragma once

#include <string_view>
#include <memory>
#include <cstdint>
#include <set>
#include <glm/vec2.hpp>

struct ID3D11ShaderResourceView;
namespace GameOverlay
{
	class Image;
	class ImageInstance
	{
	public:
		ImageInstance(std::string_view imageFileName, const glm::ivec2& pos = glm::ivec2(0));
		ImageInstance(uint32_t* imageData, int imageWidth, int imageHeight, const glm::ivec2& pos = glm::ivec2(0));
		~ImageInstance();

		static void RenderAll();

		void SetPos(int x, int y);
		void SetPos(const glm::ivec2& pos);
		glm::ivec2 GetPos() const;

		void SetShouldRender(bool shouldRender);
		bool ShouldRender() const;

	private:
		static std::set<ImageInstance*> g_instances;
		std::shared_ptr<Image> m_image;
		glm::ivec2 m_pos;
		bool m_shouldRender = true;
	};
}
