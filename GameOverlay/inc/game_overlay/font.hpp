#pragma once

#include <spek/util/types.hpp>
#include <game_overlay/image.hpp>

#include <glm/vec2.hpp>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace GameOverlay
{
	struct FontData;
	class Font
	{
	public:
		struct Quad
		{
			struct Vertex
			{
				glm::vec2 pos;
				glm::vec2 uv;
			};

			Vertex topLeft, bottomRight;
		};

		Font();

		using OnLoadFunction = std::function<void(Font&)>;
		void Load(const char* inFile, OnLoadFunction inOnLoad = nullptr);

		Image* GetTexture(size_t inFontSize);

		Quad GetCharQuad(const char* inChar, size_t inSize, float& inX, float& inY) const;

		glm::vec2 WrapWords(std::string& ioText, size_t inFontSize, f32 inMaxWidth) const;
		glm::vec2 GetTextBounds(std::string_view inText, size_t inFontSize) const;
		float GetTextHeight(std::string_view inText, size_t inFontSize) const;
		float GetTextWidth(std::string_view inText, size_t inFontSize) const;
		float GetLineAscender(size_t inFontSize) const;

		~Font();

		bool HasData() const { return m_data != nullptr; }
		
		friend struct FontData;
	protected:
		std::shared_ptr<FontData> m_data;
	};
}