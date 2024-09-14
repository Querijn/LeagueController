// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2020

#include <game_overlay/surface.hpp>
#include <league_controller/profiler.hpp>

#include <cstring>
#include <cassert>
#include <stb_image.h>

using namespace Spek;
namespace GameOverlay
{

static char s_Font[51][5][6];
static bool fontInitialized = false;
static int s_Transl[256];

Surface::Surface(int inWidth, int inHeight, Pixel* inBuffer, int inPitch) :
	m_buffer(NULL),
	m_width( inWidth ),
	m_height( inHeight ),
	m_pitch( inPitch )
{
	Leacon_Profiler;
	m_flags = 0;
	Resize(inWidth, inHeight);
	SetBuffer(inBuffer);
}

Surface::Surface(int inWidth, int inHeight) :
	m_buffer(NULL),
	m_width( inWidth ),
	m_height( inHeight ),
	m_pitch( inWidth )
{
	Leacon_Profiler;
	m_flags = OWNER;
	Resize(inWidth, inHeight);
}

Surface::Surface( const char *inFile ) :
	m_buffer( NULL ),
	m_width( 0 ), m_height( 0 )
{
	Leacon_Profiler;
	FILE* f = fopen(inFile, "rb");
	LoadImage(inFile);
	fclose(f);
}

void Surface::LoadImage( const char *inFile )
{
	Leacon_Profiler;
	m_sourceFile = File::Load(inFile, [this](File::Handle inFile)
	{
		Leacon_Profiler;
		if (inFile == nullptr || inFile->GetLoadState() != File::LoadState::Loaded)
			return;
		
		int width, height, bpp = 0;
		auto& fileData = inFile->GetData();
		Pixel* data = (Pixel*)stbi_load_from_memory(fileData.data(), fileData.size(), &width, &height, &bpp, 4);

		Resize(width, height);
		SetBuffer(data);

		free(data);
		m_sourceFile = nullptr;
	});
}

Surface::~Surface()
{
	Leacon_Profiler;
}

void Surface::Resize(int inWidth, int inHeight)
{
	Leacon_Profiler;
	m_width = inWidth;
	m_pitch = inWidth;
	m_height = inHeight;

	m_bufferSource.resize(inWidth * inHeight);
	m_buffer = m_bufferSource.data();
}

void Surface::SetBuffer(Pixel* inBuffer)
{
	Leacon_Profiler;
	memcpy(m_buffer, inBuffer, sizeof(Pixel) * m_width * m_height);
	m_dirty = true;
}

void Surface::Clear( Pixel inColor )
{
	Leacon_Profiler;
	int s = m_width * m_height;
	// for (int i = 0; i < s; i++) m_buffer[i] = inColor;
	std::fill_n(m_buffer, m_width * m_height, inColor);

	m_dirty = false;
	m_clear = true;
}

void Surface::Centre( const char *inString, int y1, Pixel color )
{
	Leacon_Profiler;
	int x = (m_width - (int)strlen( inString ) * 6) / 2;
	Print( inString, x, y1, color );
}

void Surface::Print( const char *inString, int x1, int y1, Pixel color )
{
	Leacon_Profiler;
	if (!fontInitialized)
	{
		InitCharset();
		fontInitialized = true;
	}
	Pixel* t = m_buffer + x1 + y1 * m_pitch;
	for (int i = 0; i < (int)(strlen( inString )); i++, t += 6)
	{
		int pos = 0;
		if ((inString[i] >= 'A') && (inString[i] <= 'Z')) pos = s_Transl[(unsigned short)(inString[i] - ('A' - 'a'))];
		else pos = s_Transl[(unsigned short)inString[i]];
		Pixel* a = t;
		const char *c = (const char *)s_Font[pos];
		for (int v = 0; v < 5; v++, c++, a += m_pitch)
			for (int h = 0; h < 5; h++) if (*c++ == 'o') *(a + h) = color, *(a + h + m_pitch) = 0;
	}

	m_dirty = true;
}

void Surface::Resize( Surface* inOrig )
{
	Leacon_Profiler;
	Pixel* src = inOrig->GetBuffer(), *dst = m_buffer;
	int u, v, owidth = inOrig->GetWidth(), oheight = inOrig->GetHeight();
	int dx = (owidth << 10) / m_width, dy = (oheight << 10) / m_height;
	for (v = 0; v < m_height; v++)
	{
		for (u = 0; u < m_width; u++)
		{
			int su = u * dx, sv = v * dy;
			Pixel* s = src + (su >> 10) + (sv >> 10) * owidth;
			int ufrac = su & 1023, vfrac = sv & 1023;
			int w4 = (ufrac * vfrac) >> 12;
			int w3 = ((1023 - ufrac) * vfrac) >> 12;
			int w2 = (ufrac * (1023 - vfrac)) >> 12;
			int w1 = ((1023 - ufrac) * (1023 - vfrac)) >> 12;
			int x2 = ((su + dx) > ( (owidth - 1) << 10 )) ? 0 : 1;
			int y2 = ((sv + dy) > ((oheight - 1) << 10)) ? 0 : 1;
			Pixel p1 = *s, p2 = *(s + x2), p3 = *(s + owidth * y2), p4 = *(s + owidth * y2 + x2);
			unsigned int r = (((p1 & REDMASK) * w1 + (p2 & REDMASK) * w2 + (p3 & REDMASK) * w3 + (p4 & REDMASK) * w4) >> 8) & REDMASK;
			unsigned int g = (((p1 & GREENMASK) * w1 + (p2 & GREENMASK) * w2 + (p3 & GREENMASK) * w3 + (p4 & GREENMASK) * w4) >> 8) & GREENMASK;
			unsigned int b = (((p1 & BLUEMASK) * w1 + (p2 & BLUEMASK) * w2 + (p3 & BLUEMASK) * w3 + (p4 & BLUEMASK) * w4) >> 8) & BLUEMASK;
			*(dst + u + v * m_pitch) = (Pixel)(r + g + b);
		}
	}

	m_dirty = true;
}

void Surface::SetDirty(bool inDirty)
{
	Leacon_Profiler;
	m_dirty = inDirty;
	if (m_dirty)
		m_clear = false;
}

bool Surface::IsDirty() const
{
	Leacon_Profiler;
	return m_dirty;
}

bool Surface::IsClear() const
{
	return m_clear;
}

#define OUTCODE(x,y) (((x)<xmin)?1:(((x)>xmax)?2:0))+(((y)<ymin)?4:(((y)>ymax)?8:0))

void Surface::Line( float x1, float y1, float x2, float y2, Pixel c )
{
	Leacon_Profiler;
	// clip (Cohen-Sutherland, https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
	const float xmin = 0, ymin = 0, xmax = (float)m_width - 1, ymax = (float)m_height - 1;
	int c0 = OUTCODE( x1, y1 ), c1 = OUTCODE( x2, y2 );
	bool accept = false;
	while (1)
	{
		if (!(c0 | c1)) { accept = true; break; }
		else if (c0 & c1) break; else
		{
			float x, y;
			const int co = c0 ? c0 : c1;
			if (co & 8) x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1), y = ymax;
			else if (co & 4) x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1), y = ymin;
			else if (co & 2) y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1), x = xmax;
			else if (co & 1) y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1), x = xmin;
			if (co == c0) x1 = x, y1 = y, c0 = OUTCODE( x1, y1 );
			else x2 = x, y2 = y, c1 = OUTCODE( x2, y2 );
		}
	}
	if (!accept) return;
	float b = x2 - x1;
	float h = y2 - y1;
	float l = fabsf( b );
	if (fabsf( h ) > l) l = fabsf( h );
	int il = (int)l;
	float dx = b / (float)l;
	float dy = h / (float)l;
	for (int i = 0; i <= il; i++)
	{
		*(m_buffer + (int)x1 + (int)y1 * m_pitch) = c;
		x1 += dx, y1 += dy;
	}
	m_dirty = true;
}

void Surface::Plot( int x, int y, Pixel c )
{
	Leacon_Profiler;
	if ((x >= 0) && (y >= 0) && (x < m_width) && (y < m_height))
		m_buffer[x + y * m_pitch] = c;
	m_dirty = true;
}

void Surface::Box( int x1, int y1, int x2, int y2, Pixel c )
{
	Leacon_Profiler;
	Line( (float)x1, (float)y1, (float)x2, (float)y1, c );
	Line( (float)x2, (float)y1, (float)x2, (float)y2, c );
	Line( (float)x1, (float)y2, (float)x2, (float)y2, c );
	Line( (float)x1, (float)y1, (float)x1, (float)y2, c );
	m_dirty = true;
}

void Surface::Bar( int x1, int y1, int x2, int y2, Pixel c )
{
	Leacon_Profiler;
	Pixel* a = x1 + y1 * m_pitch + m_buffer;
	for (int y = y1; y <= y2; y++)
	{
		for (int x = 0; x <= (x2 - x1); x++) a[x] = c;
		a += m_pitch;
	}
	m_dirty = true;
}

void Surface::PlotTo(Surface* surf, int draw_x, int draw_y, int w, int h) const
{
	Leacon_Profiler;
	Pixel* dst = surf->GetBuffer();
	const Pixel* src = m_buffer;
	if (!src || !dst)
		return;

	int target_pitch = surf->GetPitch();
	int remaining_width = surf->GetWidth() - abs(draw_x);
	int remaining_height = surf->GetHeight() - abs(draw_y);
	int draw_width = w < remaining_width ? w : remaining_width;
	int draw_height = h < remaining_height ? h : remaining_height;

	int start_x = draw_x >= 0 ? 0 : abs(draw_x);
	int start_y = draw_y >= 0 ? 0 : abs(draw_y);

	f32 source_width = m_width;
	f32 source_height = m_height;

	for (int y = start_y; y < draw_height; y++)
	{
		f32 py = (f32)y / (f32)h;
		assert(py >= 0.0f && py <= 1.0f);

		f32 source_y = py * source_height;
		int source_pixel_y = (int)floorf(source_y + 0.5f) * m_pitch;

		int target_pixel_y = (draw_y + y) * target_pitch;

		for (int x = start_y; x < draw_width; x++)
		{
			f32 px = (f32)x / (f32)w;
			assert(px >= 0.0f && px <= 1.0f);

			f32 source_x = px * source_width;
			int source_pixel_x = (int)floorf(source_x + 0.5f);
			
			dst[(draw_x + x) + target_pixel_y] = m_buffer[source_pixel_x + source_pixel_y];
		}
	}
}

void Surface::CopyTo( Surface* inDst, int inX, int inY ) const
{
	Leacon_Profiler;
	Pixel* dst = inDst->GetBuffer();
	const Pixel* src = m_buffer;
	if ((src) && (dst))
	{
		int srcwidth = m_width;
		int srcheight = m_height;
		int srcpitch = m_pitch;
		int dstwidth = inDst->GetWidth();
		int dstheight = inDst->GetHeight();
		int dstpitch = inDst->GetPitch();
		if ((srcwidth + inX) > dstwidth) srcwidth = dstwidth - inX;
		if ((srcheight + inY) > dstheight) srcheight = dstheight - inY;
		if (inX < 0) src -= inX, srcwidth += inX, inX = 0;
		if (inY < 0) src -= inY * srcpitch, srcheight += inY, inY = 0;
		if ((srcwidth > 0) && (srcheight > 0))
		{
			dst += inX + dstpitch * inY;
			for (int y = 0; y < srcheight; y++)
			{
				memcpy( dst, src, srcwidth * 4 );
				dst += dstpitch;
				src += srcpitch;
			}
		}
	}
}

void Surface::BlendCopyTo( Surface* inDst, int inX, int inY )
{
	Leacon_Profiler;
	Pixel* dst = inDst->GetBuffer();
	const Pixel* src = m_buffer;
	if ((src) && (dst))
	{
		int srcwidth = m_width;
		int srcheight = m_height;
		int srcpitch = m_pitch;
		int dstwidth = inDst->GetWidth();
		int dstheight = inDst->GetHeight();
		int dstpitch = inDst->GetPitch();
		if ((srcwidth + inX) > dstwidth) srcwidth = dstwidth - inX;
		if ((srcheight + inY) > dstheight) srcheight = dstheight - inY;
		if (inX < 0) src -= inX, srcwidth += inX, inX = 0;
		if (inY < 0) src -= inY * srcpitch, srcheight += inY, inY = 0;
		if ((srcwidth > 0) && (srcheight > 0))
		{
			dst += inX + dstpitch * inY;
			for (int y = 0; y < srcheight; y++)
			{
				for (int x = 0; x < srcwidth; x++) dst[x] = AddBlend( dst[x], src[x] );
				dst += dstpitch;
				src += srcpitch;
			}
		}
	}
}

void Surface::SetChar( int c, const char *c1, const char *c2, const char *c3, const char *c4, const char *c5 )
{
	Leacon_Profiler;
	strcpy( s_Font[c][0], c1 );
	strcpy( s_Font[c][1], c2 );
	strcpy( s_Font[c][2], c3 );
	strcpy( s_Font[c][3], c4 );
	strcpy( s_Font[c][4], c5 );
}

void Surface::InitCharset()
{
	Leacon_Profiler;
	SetChar( 0, ":ooo:", "o:::o", "ooooo", "o:::o", "o:::o" );
	SetChar( 1, "oooo:", "o:::o", "oooo:", "o:::o", "oooo:" );
	SetChar( 2, ":oooo", "o::::", "o::::", "o::::", ":oooo" );
	SetChar( 3, "oooo:", "o:::o", "o:::o", "o:::o", "oooo:" );
	SetChar( 4, "ooooo", "o::::", "oooo:", "o::::", "ooooo" );
	SetChar( 5, "ooooo", "o::::", "ooo::", "o::::", "o::::" );
	SetChar( 6, ":oooo", "o::::", "o:ooo", "o:::o", ":ooo:" );
	SetChar( 7, "o:::o", "o:::o", "ooooo", "o:::o", "o:::o" );
	SetChar( 8, "::o::", "::o::", "::o::", "::o::", "::o::" );
	SetChar( 9, ":::o:", ":::o:", ":::o:", ":::o:", "ooo::" );
	SetChar( 10, "o::o:", "o:o::", "oo:::", "o:o::", "o::o:" );
	SetChar( 11, "o::::", "o::::", "o::::", "o::::", "ooooo" );
	SetChar( 12, "oo:o:", "o:o:o", "o:o:o", "o:::o", "o:::o" );
	SetChar( 13, "o:::o", "oo::o", "o:o:o", "o::oo", "o:::o" );
	SetChar( 14, ":ooo:", "o:::o", "o:::o", "o:::o", ":ooo:" );
	SetChar( 15, "oooo:", "o:::o", "oooo:", "o::::", "o::::" );
	SetChar( 16, ":ooo:", "o:::o", "o:::o", "o::oo", ":oooo" );
	SetChar( 17, "oooo:", "o:::o", "oooo:", "o:o::", "o::o:" );
	SetChar( 18, ":oooo", "o::::", ":ooo:", "::::o", "oooo:" );
	SetChar( 19, "ooooo", "::o::", "::o::", "::o::", "::o::" );
	SetChar( 20, "o:::o", "o:::o", "o:::o", "o:::o", ":oooo" );
	SetChar( 21, "o:::o", "o:::o", ":o:o:", ":o:o:", "::o::" );
	SetChar( 22, "o:::o", "o:::o", "o:o:o", "o:o:o", ":o:o:" );
	SetChar( 23, "o:::o", ":o:o:", "::o::", ":o:o:", "o:::o" );
	SetChar( 24, "o:::o", "o:::o", ":oooo", "::::o", ":ooo:" );
	SetChar( 25, "ooooo", ":::o:", "::o::", ":o:::", "ooooo" );
	SetChar( 26, ":ooo:", "o::oo", "o:o:o", "oo::o", ":ooo:" );
	SetChar( 27, "::o::", ":oo::", "::o::", "::o::", ":ooo:" );
	SetChar( 28, ":ooo:", "o:::o", "::oo:", ":o:::", "ooooo" );
	SetChar( 29, "oooo:", "::::o", "::oo:", "::::o", "oooo:" );
	SetChar( 30, "o::::", "o::o:", "ooooo", ":::o:", ":::o:" );
	SetChar( 31, "ooooo", "o::::", "oooo:", "::::o", "oooo:" );
	SetChar( 32, ":oooo", "o::::", "oooo:", "o:::o", ":ooo:" );
	SetChar( 33, "ooooo", "::::o", ":::o:", "::o::", "::o::" );
	SetChar( 34, ":ooo:", "o:::o", ":ooo:", "o:::o", ":ooo:" );
	SetChar( 35, ":ooo:", "o:::o", ":oooo", "::::o", ":ooo:" );
	SetChar( 36, "::o::", "::o::", "::o::", ":::::", "::o::" );
	SetChar( 37, ":ooo:", "::::o", ":::o:", ":::::", "::o::" );
	SetChar( 38, ":::::", ":::::", "::o::", ":::::", "::o::" );
	SetChar( 39, ":::::", ":::::", ":ooo:", ":::::", ":ooo:" );
	SetChar( 40, ":::::", ":::::", ":::::", ":::o:", "::o::" );
	SetChar( 41, ":::::", ":::::", ":::::", ":::::", "::o::" );
	SetChar( 42, ":::::", ":::::", ":ooo:", ":::::", ":::::" );
	SetChar( 43, ":::o:", "::o::", "::o::", "::o::", ":::o:" );
	SetChar( 44, "::o::", ":::o:", ":::o:", ":::o:", "::o::" );
	SetChar( 45, ":::::", ":::::", ":::::", ":::::", ":::::" );
	SetChar( 46, "ooooo", "ooooo", "ooooo", "ooooo", "ooooo" );
	SetChar( 47, "::o::", "::o::", ":::::", ":::::", ":::::" ); // Tnx Ferry
	SetChar( 48, "o:o:o", ":ooo:", "ooooo", ":ooo:", "o:o:o" );
	SetChar( 49, "::::o", ":::o:", "::o::", ":o:::", "o::::" );
	char c[] = "abcdefghijklmnopqrstuvwxyz0123456789!?:=,.-() #'*/";
	int i;
	for (i = 0; i < 256; i++) s_Transl[i] = 45;
	for (i = 0; i < 50; i++) s_Transl[(unsigned char)c[i]] = i;
}

void Surface::ScaleColor( unsigned int inScale )
{
	Leacon_Profiler;
	int s = m_pitch * m_height;
	for (int i = 0; i < s; i++)
	{
		Pixel c = m_buffer[i];
		unsigned int rb = (((c & (REDMASK | BLUEMASK)) * inScale) >> 5) & (REDMASK | BLUEMASK);
		unsigned int g = (((c & GREENMASK) * inScale) >> 5) & GREENMASK;
		m_buffer[i] = rb + g;
	}
	m_dirty = true;
}

Sprite::Sprite( Surface* inSurface, unsigned int inNumFrames ) :
	m_width( inSurface->GetWidth() / inNumFrames ),
	m_height( inSurface->GetHeight() ),
	m_pitch( inSurface->GetWidth() ),
	m_numFrames( inNumFrames ),
	m_currentFrame( 0 ),
	m_flags( 0 ),
	m_start( new unsigned int*[inNumFrames] ),
	m_surface( inSurface )
{
	Leacon_Profiler;
	InitializeStartData();
}

Sprite::~Sprite()
{
	Leacon_Profiler;
	delete m_surface;
	for (unsigned int i = 0; i < m_numFrames; i++) delete m_start[i];
	delete[] m_start;
}

void Sprite::Draw( Surface* inTarget, int inX, int inY )
{
	Leacon_Profiler;
	if ((inX < -m_width) || (inX > ( inTarget->GetWidth() + m_width ))) return;
	if ((inY < -m_height) || (inY > ( inTarget->GetHeight() + m_height ))) return;
	int x1 = inX, x2 = inX + m_width;
	int y1 = inY, y2 = inY + m_height;
	Pixel* src = GetBuffer() + m_currentFrame * m_width;
	if (x1 < 0)
	{
		src += -x1;
		x1 = 0;
	}
	if (x2 > inTarget->GetWidth()) x2 = inTarget->GetWidth();
	if (y1 < 0)
	{
		src += -y1 * m_pitch;
		y1 = 0;
	}
	if (y2 > inTarget->GetHeight()) y2 = inTarget->GetHeight();
	Pixel* dest = inTarget->GetBuffer();
	int xs;
	const int dpitch = inTarget->GetPitch();
	if ((x2 > x1) && (y2 > y1))
	{
		unsigned int addr = y1 * dpitch + x1;
		const int width = x2 - x1;
		const int height = y2 - y1;
		for (int y = 0; y < height; y++)
		{
			const int line = y + (y1 - inY);
			const int lsx = m_start[m_currentFrame][line] + inX;
			if (m_flags & FLARE)
			{
				xs = (lsx > x1) ? lsx - x1 : 0;
				for (int x = xs; x < width; x++)
				{
					const Pixel c1 = *(src + x);
					if (c1 & 0xffffff)
					{
						const Pixel c2 = *(dest + addr + x);
						*(dest + addr + x) = AddBlend( c1, c2 );
					}
				}
			}
			else
			{
				xs = (lsx > x1) ? lsx - x1 : 0;
				for (int x = xs; x < width; x++)
				{
					const Pixel c1 = *(src + x);
					if (c1 & 0xffffff) *(dest + addr + x) = c1;
				}
			}
			addr += dpitch;
			src += m_pitch;
		}
	}
}

void Sprite::DrawScaled( int inX, int inY, int inWidth, int inHeight, Surface* inTarget )
{
	Leacon_Profiler;
	if ((inWidth == 0) || (inHeight == 0)) return;
	for (int x = 0; x < inWidth; x++) for (int y = 0; y < inHeight; y++)
	{
		int u = (int)((float)x * ((float)m_width / (float)inWidth));
		int v = (int)((float)y * ((float)m_height / (float)inHeight));
		Pixel color = GetBuffer()[u + v * m_pitch];
		if (color & 0xffffff) inTarget->GetBuffer()[inX + x + ((inY + y) * inTarget->GetPitch())] = color;
	}
}

void Sprite::InitializeStartData()
{
	for (unsigned int f = 0; f < m_numFrames; ++f)
	{
		m_start[f] = new unsigned int[m_height];
		for (int y = 0; y < m_height; ++y)
		{
			m_start[f][y] = m_width;
			Pixel* addr = GetBuffer() + f * m_width + y * m_pitch;
			for (int x = 0; x < m_width; ++x)
			{
				if (addr[x])
				{
					m_start[f][y] = x;
					break;
				}
			}
		}
	}
}

SurfaceFont::SurfaceFont( const char *inFile, const char *inChars )
{
	Leacon_Profiler;
	m_surface = new Surface( inFile );
	Pixel* b = m_surface->GetBuffer();
	int w = m_surface->GetWidth();
	int h = m_surface->GetHeight();
	unsigned int charnr = 0, start = 0;
	m_trans = new int[256];
	memset( m_trans, 0, 1024 );
	unsigned int i;
	for (i = 0; i < strlen( inChars ); i++) m_trans[(unsigned char)inChars[i]] = i;
	m_offset = new int[strlen( inChars )];
	m_width = new int[strlen( inChars )];
	m_height = h;
	m_cy1 = 0, m_cy2 = 1024;
	int x, y;
	bool lastempty = true;
	for (x = 0; x < w; x++)
	{
		bool empty = true;
		for (y = 0; y < h; y++) if (*(b + x + y * w) & 0xffffff)
		{
			if (lastempty) start = x;
			empty = false;
		}
		if ((empty) && (!lastempty))
		{
			m_width[charnr] = x - start;
			m_offset[charnr] = start;
			if (++charnr == strlen( inChars )) break;
		}
		lastempty = empty;
	}
}

SurfaceFont::~SurfaceFont()
{
	Leacon_Profiler;
	delete m_surface;
	delete m_trans;
	delete m_width;
	delete m_offset;
}

int SurfaceFont::Width( const char *inText )
{
	Leacon_Profiler;
	int w = 0;
	unsigned int i;
	for (i = 0; i < strlen( inText ); i++)
	{
		unsigned char c = (unsigned char)inText[i];
		if (c == 32) w += 4; else w += m_width[m_trans[c]] + 2;
	}
	return w;
}

void SurfaceFont::Centre( Surface* inTarget, const char *inText, int inY )
{
	Leacon_Profiler;
	int x = (inTarget->GetPitch() - Width( inText )) / 2;
	Print( inTarget, inText, x, inY );
}

void SurfaceFont::Print( Surface* inTarget, const char *inText, int inX, int inY, bool clip )
{
	Leacon_Profiler;
	Pixel* b = inTarget->GetBuffer() + inX + inY * inTarget->GetPitch();
	Pixel* s = m_surface->GetBuffer();
	unsigned int i, cx;
	int x, y;
	if (((inY + m_height) < m_cy1) || (inY > m_cy2)) return;
	for (cx = 0, i = 0; i < strlen( inText ); i++)
	{
		if (inText[i] == ' ') cx += 4; else
		{
			int c = m_trans[(unsigned char)inText[i]];
			Pixel* t = s + m_offset[c], *d = b + cx;
			if (clip)
			{
				for (y = 0; y < m_height; y++)
				{
					if (((inY + y) >= m_cy1) && ((inY + y) <= m_cy2))
					{
						for (x = 0; x < m_width[c]; x++)
							if ((t[x]) && ((x + (int)cx + inX) < inTarget->GetPitch()))
								d[x] = AddBlend( t[x], d[x] );
					}
					t += m_surface->GetPitch(), d += inTarget->GetPitch();
				}
			}
			else
			{
				for (y = 0; y < m_height; y++)
				{
					if (((inY + y) >= m_cy1) && ((inY + y) <= m_cy2))
						for (x = 0; x < m_width[c]; x++) if (t[x]) d[x] = AddBlend( t[x], d[x] );
					t += m_surface->GetPitch(), d += inTarget->GetPitch();
				}
			}
			cx += m_width[c] + 2;
			if ((int)(cx + inX) >= inTarget->GetPitch()) break;
		}
	}
}

}; // namespace Tmpl8

// EOF