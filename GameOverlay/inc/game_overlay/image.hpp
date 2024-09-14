#pragma once

#include <string_view>
#include <memory>
#include <cstdint>
#include <glm/vec2.hpp>

struct sg_image;
struct ID3D11ShaderResourceView;
namespace GameOverlay
{
	struct ImageData;
	class Image
	{
	public:
		Image() {}
		Image(std::string_view imageFileName);
		Image(uint32_t* imageData, int imageWidth, int imageHeight);
		Image(int imageWidth, int imageHeight, int bpp = 4);
		~Image();

		ID3D11ShaderResourceView* GetAsDX11() const;
		sg_image GetForSokol() const;

		bool HasData() const;
		bool Reload();
		void Render(const glm::ivec2& position);
		static void OnFrameStart();

		uint8_t* GetBuffer() const;

	private:
		std::shared_ptr<ImageData> m_data;

		bool TryLoadResource(uint8_t* imageData, int imageWidth, int imageHeight);
	};
}
