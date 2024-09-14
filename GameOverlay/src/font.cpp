#include <game_overlay/font.hpp>
#include <game_overlay/log.hpp>
#include <spek/util/types.hpp>
#include <spek/file/file.hpp>

#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

#include "schrift.h"

#if !LEACON_SUBMISSION
// #define SPEK_OUTPUT_FONT_TEST
#endif

#if defined(SPEK_OUTPUT_FONT_TEST)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_STATIC
#include "stb_image_write.h"
#endif

namespace GameOverlay
{
	struct GlyphInfo
	{
		int x, y;
		int w, h;
		char glyph;

		SFT_GMetrics metrics;
	};

	struct RenderedGlyphInfo : public GlyphInfo
	{
		std::shared_ptr<char[]> pixels;
		SFT_Image img;
		bool wasPacked = false;
	};

	bool PackIntoRows(std::vector<RenderedGlyphInfo>& rects, int imageSize)
	{
		Leacon_Profiler;
		// Sort by area, seemed to work best for this algorithm
		std::sort(rects.begin(), rects.end(), [](const auto& a, const auto& b)
		{
			// return a.w * a.h < b.w * b.h;
			return a.h > b.h;
		});

		int xPos = 0;
		int yPos = 0;
		int largestHThisRow = 0;

		// Loop over all the rectangles
		for (RenderedGlyphInfo& rect : rects)
		{
			// If this rectangle will go past the width of the image
			// Then loop around to next row, using the largest height from the previous row
			if ((xPos + rect.w) > imageSize)
			{
				yPos += largestHThisRow;
				xPos = 0;
				largestHThisRow = 0;
			}

			// If we go off the bottom edge of the image, then we've failed
			if ((yPos + rect.h) > imageSize)
				return false;

			// This is the position of the rectangle
			rect.x = xPos;
			rect.y = yPos;

			// Move along to the next spot in the row
			xPos += rect.w;

			// Just saving the largest height in the new row
			if (rect.h > largestHThisRow)
				largestHThisRow = rect.h;

			// Success!
			rect.wasPacked = true;
		}
		
		return true;
	}

	struct FontDataAtSize
	{
		std::vector<uint8_t> bitmap;
		size_t bitmapSize;
		std::vector<GlyphInfo> glyphInfo;
		SFT sft;
		bool contextAllocated = false;

		std::shared_ptr<Image> texture = nullptr;
	};

	struct FontData
	{
		FontData(Font& inFont) : parent(inFont) {}
		~FontData() { sft_freefont(font); }
		FontDataAtSize* GetFontDataAtSize(size_t inSize);

		Font& parent;
		SFT_Font* font = nullptr;
		Spek::File::Handle file = nullptr;
		
		std::unordered_map<size_t, std::shared_ptr<FontDataAtSize>> sizeTextures;
	};

	Font::Font()
	{
	}

	Font::Quad Font::GetCharQuad(const char* inCharacter, size_t inFontSize, float& inX, float& inY) const
	{
		Leacon_Profiler;
		auto* data = m_data->GetFontDataAtSize(inFontSize);
		assert(data && "Expected font data for size to be valid.");

		int index = inCharacter[0]; // TODO: Support more ranges and Unicode
		SFT_LMetrics lineMetrics;
		sft_lmetrics(&data->sft, &lineMetrics);

		if (index == '\n')
		{
			inX = 0;
			inY += lineMetrics.ascender + lineMetrics.lineGap;

			Font::Quad result;
			result.topLeft.pos.x =
				result.topLeft.pos.y =
				result.bottomRight.pos.x =
				result.bottomRight.pos.y = 0;

			result.topLeft.uv.x =
				result.topLeft.uv.y =
				result.bottomRight.uv.x =
				result.bottomRight.uv.y = 0;

			return result;
		}

		for (const GlyphInfo& glyph : data->glyphInfo)
		{
			if (index != glyph.glyph)
				continue;

			const SFT_GMetrics& metrics = glyph.metrics;

			Font::Quad result;
			result.topLeft.pos.x = inX + metrics.leftSideBearing;
			result.topLeft.pos.y = inY + metrics.yOffset;

			result.bottomRight.pos.x = result.topLeft.pos.x + glyph.w;
			result.bottomRight.pos.y = result.topLeft.pos.y + glyph.h;

			f64 sizeInvert = 1.0f / data->bitmapSize;

			result.topLeft.uv.x = glyph.x * sizeInvert;
			result.topLeft.uv.y = glyph.y * sizeInvert;

			result.bottomRight.uv.x = (glyph.x + glyph.w) * sizeInvert;
			result.bottomRight.uv.y = (glyph.y + glyph.h) * sizeInvert;

			inX += metrics.advanceWidth;

			return result;
		}

		assert(false && "Could not find character quad for this character!");

		Font::Quad result;
		result.topLeft.pos.x = 
			result.topLeft.pos.y = 
			result.bottomRight.pos.x = 
			result.bottomRight.pos.y = 0;

		result.topLeft.uv.x = 
			result.topLeft.uv.y = 
			result.bottomRight.uv.x = 
			result.bottomRight.uv.y = 0;

		return result;
	}

	glm::vec2 Font::WrapWords(std::string& ioText, size_t inFontSize, f32 inMaxWidth) const
	{
		Leacon_Profiler;
		glm::vec2 size(0, 0);
		glm::vec2 cursor(0, 0);

		FontDataAtSize* data = m_data->GetFontDataAtSize(inFontSize);
		SFT_LMetrics lineMetrics;
		sft_lmetrics(&data->sft, &lineMetrics);

		int textSize = (int)ioText.size();
		int lastBreakpoint = -1;
		for (int i = 0; i < textSize; i++)
		{
			char c = ioText[i];
			for (const GlyphInfo& glyph : data->glyphInfo)
			{
				if (c != glyph.glyph)
					continue;

				f32 nextPos = cursor.x + glyph.metrics.advanceWidth;
				if (nextPos > inMaxWidth && lastBreakpoint >= 0)
				{
					ioText[lastBreakpoint] = '\n';
					i = lastBreakpoint;
					lastBreakpoint = -1;
					c = '\n';
				}

				if (c == '\n')
				{
					cursor.x = 0;
					cursor.y += lineMetrics.ascender + lineMetrics.lineGap;
				}
				else
				{
					if (c == ' ')
					{
						lastBreakpoint = i;
						// printf("%s (%d, %f)\n", ioText.substr(0, i).c_str(), i, nextPos);
					}

					if (size.x < nextPos)
						size.x = nextPos;
					cursor.x = nextPos;
				}
			}
		}

		size.y = cursor.y + lineMetrics.ascender;
		return size;
	}

	glm::vec2 Font::GetTextBounds(std::string_view inText, size_t inFontSize) const
	{
		Leacon_Profiler;
		glm::vec2 result(0, 0);

		auto* data = m_data->GetFontDataAtSize(inFontSize);
		if (data == nullptr)
			return result;
		for (char c : inText)
		{
			for (const GlyphInfo& glyph : data->glyphInfo)
			{
				if (c != glyph.glyph)
					continue;

				if (c == '\n')
					__debugbreak(); // TODO

				if (result.y < glyph.h)
					result.y = glyph.h;

				result.x += glyph.metrics.advanceWidth;
			}
		}

		return result;
	}

	float Font::GetTextHeight(std::string_view inText, size_t inFontSize) const
	{
		Leacon_Profiler;
		return GetTextBounds(inText, inFontSize).y;
	}

	float Font::GetTextWidth(std::string_view inText, size_t inFontSize) const
	{
		Leacon_Profiler;
		return GetTextBounds(inText, inFontSize).x;
	}

	float Font::GetLineAscender(size_t inFontSize) const
	{
		Leacon_Profiler;
		auto* data = m_data->GetFontDataAtSize(inFontSize);
		SFT_LMetrics lineMetrics;
		sft_lmetrics(&data->sft, &lineMetrics);
		return lineMetrics.ascender;
	}

	Font::~Font()
	{
	}

	void Font::Load(const char* inFileName, OnLoadFunction inOnLoad)
	{
		Leacon_Profiler;
		m_data = std::make_shared<FontData>(*this);

		m_data->file = Spek::File::Load(inFileName, [this](Spek::File::Handle f)
		{
			m_data->font = f ? sft_loadfile(f->GetFullName().c_str()) : nullptr;
		});

		if (m_data->font == nullptr)
			Spek::File::Update();
		assert(m_data->font);
		if (inOnLoad)
			inOnLoad(*this);
	}
	
	Image* Font::GetTexture(size_t inFontSize)
	{
		Leacon_Profiler;
		assert(m_data && "Cannot get the texture of a font that has not been loaded!");

		auto data = m_data->GetFontDataAtSize(inFontSize);
		if (data != nullptr && data->texture == nullptr)
		{
			data->texture = std::make_shared<Image>(data->bitmapSize, data->bitmapSize, 1);
			auto buffer = data->texture->GetBuffer();
			auto source = data->bitmap.data();
			auto size = data->bitmapSize;
			memcpy(buffer, source, size * size);

			data->texture->Reload();

#if defined(SPEK_OUTPUT_FONT_TEST)
			stbi_write_png("font_bitmap_test.png", (int)size, (int)size, 1, buffer, 0);
#endif
		}
		
		return data != nullptr ? data->texture.get() : nullptr;
	}
	
	int utf8_strlen(std::string_view str)
	{
		Leacon_Profiler;
		int c, i, ix, q;
		for (q = 0, i = 0, ix = str.length(); i < ix; i++, q++)
		{
			c = (unsigned char)str[i];
			if (c >= 0 && c <= 127) i += 0;
			else if ((c & 0xE0) == 0xC0) i += 1;
			else if ((c & 0xF0) == 0xE0) i += 2;
			else if ((c & 0xF8) == 0xF0) i += 3;
			//else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
			//else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
			else return 0;//invalid utf8
		}
		return q;
	}

	FontDataAtSize* FontData::GetFontDataAtSize(size_t inFontSize)
	{
		Leacon_Profiler;
		if (inFontSize == 0)
			return nullptr;

		std::shared_ptr<FontDataAtSize>& sizeElement = sizeTextures[inFontSize];
		if (sizeElement != nullptr)
			return sizeElement.get();

		sizeElement = std::make_shared<FontDataAtSize>();
		sizeElement->sft.flags = SFT_DOWNWARD_Y;
		sizeElement->sft.xScale = inFontSize;
		sizeElement->sft.yScale = inFontSize;
		sizeElement->sft.font = font;
		auto sft = &sizeElement->sft;
		assert(font);

		int glyphMin = 32;
		int glyphMax = 128;
		int glyphCount = glyphMax - glyphMin;
		std::vector<RenderedGlyphInfo> glyphInfos;

		// Render all glyphs to separate bitmaps
		for (int codePoint = glyphMin; codePoint < glyphMax; codePoint++)
		{
			RenderedGlyphInfo& glyphInfo = glyphInfos.emplace_back();
			glyphInfo.glyph = (u32)codePoint;

			SFT_Glyph gid;
			if (sft_lookup(sft, (u32)codePoint, &gid) < 0)
				assert(false);

			if (sft_gmetrics(sft, gid, &glyphInfo.metrics) < 0)
				assert(false);

			SFT_Image& sftCharBitmap = glyphInfo.img;
			glyphInfo.w = sftCharBitmap.width = (glyphInfo.metrics.minWidth + 3) & ~3;
			glyphInfo.h = sftCharBitmap.height = glyphInfo.metrics.minHeight;
			glyphInfo.x = 0;
			glyphInfo.y = 0;

			if (sftCharBitmap.width == 0 && sftCharBitmap.height == 0)
				continue;
			
			glyphInfo.pixels = std::shared_ptr<char[]>(new char[sftCharBitmap.width * sftCharBitmap.height]);
			sftCharBitmap.pixels = glyphInfo.pixels.get();
			if (sft_render(sft, gid, sftCharBitmap) < 0)
				assert(false);
#if defined(SPEK_OUTPUT_FONT_TEST)
			std::string name = std::string("char_debug_") + std::to_string(codePoint) + ".png";
			stbi_write_png(name.c_str(), sftCharBitmap.width, sftCharBitmap.height, 1, sftCharBitmap.pixels, 0);
#endif
		}

		// Find the best packing for all glyphs
		int sizeApprox = 255;
		{
			std::vector<RenderedGlyphInfo> origGlyphInfos = glyphInfos;
			bool fits = false;
			while (!fits)
			{
				// Find next pow2
				sizeApprox |= sizeApprox >> 1;
				sizeApprox |= sizeApprox >> 2;
				sizeApprox |= sizeApprox >> 4;
				sizeApprox |= sizeApprox >> 8;
				sizeApprox |= sizeApprox >> 16;
				sizeApprox++;

				fits = PackIntoRows(glyphInfos, sizeApprox);
				if (fits)
					break;
				glyphInfos = origGlyphInfos;
			}
		}

		sizeElement->bitmap.resize(sizeApprox * sizeApprox);
		sizeElement->bitmapSize = sizeApprox;

		size_t targetWidth = sizeApprox;
		size_t targetHeight = sizeApprox;
		u8* targetBitmap = (u8*)sizeElement->bitmap.data();
		memset(targetBitmap, 0, sizeElement->bitmap.size());

		int locX = 0;
		int locY = 0;
		int targetX = 0;
		int targetY = 0;
		
		// Draw to single bitmap
		// TODO: Optimise
		for (int i = 0; i < glyphInfos.size(); i++)
		{
			const SFT_GMetrics& glyphMetrics = glyphInfos[i].metrics;
			const SFT_Image& sourceBitmap = glyphInfos[i].img;
			const int& targetX = glyphInfos[i].x;
			const int& targetY = glyphInfos[i].y;

			for (int yOffset = 0; yOffset < sourceBitmap.height; yOffset++)
			{
				for (int xOffset = 0; xOffset < sourceBitmap.width; xOffset++)
				{
					u8* sourcePixel = (u8*)sourceBitmap.pixels + (yOffset * sourceBitmap.width) + xOffset;
					u8* targetPixel = targetBitmap + ((targetY + yOffset) * sizeApprox) + (targetX + xOffset);

					*targetPixel = *sourcePixel;
				}
			}
		}

		for (const RenderedGlyphInfo& glyphInfo : glyphInfos)
			sizeElement->glyphInfo.emplace_back(glyphInfo);

#if defined(SPEK_OUTPUT_FONT_TEST)
		stbi_write_png("font_debug.png", targetWidth, targetHeight, 1, sizeElement->bitmap.data(), 0);
#endif
		return sizeElement.get();
	}
}