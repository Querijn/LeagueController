// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2020

#pragma once

#include <spek/file/file.hpp>
#include <spek/util/types.hpp>
#include <vector>

namespace GameOverlay
{

#define REDMASK	(0xff0000)
#define GREENMASK (0x00ff00)
#define BLUEMASK (0x0000ff)

typedef u32 Pixel; // unsigned int is assumed to be 32-bit, which seems a safe assumption.

inline Pixel AddBlend( Pixel inColor1, Pixel inColor2 )
{
	const unsigned int r = (inColor1 & REDMASK) + (inColor2 & REDMASK);
	const unsigned int g = (inColor1 & GREENMASK) + (inColor2 & GREENMASK);
	const unsigned int b = (inColor1 & BLUEMASK) + (inColor2 & BLUEMASK);
	const unsigned r1 = (r & REDMASK) | (REDMASK * (r >> 24));
	const unsigned g1 = (g & GREENMASK) | (GREENMASK * (g >> 16));
	const unsigned b1 = (b & BLUEMASK) | (BLUEMASK * (b >> 8));
	return (r1 + g1 + b1);
}

// subtractive blending
inline Pixel SubBlend( Pixel inColor1, Pixel inColor2 )
{
	int red = (inColor1 & REDMASK) - (inColor2 & REDMASK);
	int green = (inColor1 & GREENMASK) - (inColor2 & GREENMASK);
	int blue = (inColor1 & BLUEMASK) - (inColor2 & BLUEMASK);
	if (red < 0) red = 0;
	if (green < 0) green = 0;
	if (blue < 0) blue = 0;
	return (Pixel)(red + green + blue);
}

// color scaling
inline Pixel ScaleColor( Pixel c, int s )
{
	const unsigned int rb = (((c & (REDMASK | BLUEMASK)) * s) >> 5) & (REDMASK | BLUEMASK);
	const unsigned int g = (((c & GREENMASK) * s) >> 5) & GREENMASK;
	return rb + g;
}

class Surface
{
	enum { OWNER = 1 };
public:
	// constructor / destructor
	Surface( int inWidth, int inHeight, Pixel* inBuffer, int inPitch );
	Surface( int inWidth, int inHeight );
	Surface( const char* inFile );
	~Surface();
	// member data access
	void Resize(int inWidth, int inHeight);
	Pixel* GetBuffer() { return m_buffer; m_dirty = true; }
	const Pixel* GetBuffer() const { return m_buffer; }
	void SetBuffer( Pixel* inBuffer );
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetPitch() { return m_pitch; }
	void SetPitch( int inPitch ) { m_pitch = inPitch; }
	size_t GetPixelCount() const { return m_bufferSource.size(); }
	// Special operations
	void InitCharset();
	void SetChar( int c, const char* c1, const char* c2, const char* c3, const char* c4, const char* c5 );
	void Centre( const char* inString, int y1, Pixel color );
	void Print( const char* inString, int x1, int y1, Pixel color );
	void Clear( Pixel inColor = 0);
	void Line( float x1, float y1, float x2, float y2, Pixel color );
	void Plot( int x, int y, Pixel c );
	void LoadImage( const char* inFile );
	void PlotTo(Surface* inDst, int inX, int inY, int inWidth, int inHeight) const;
	void CopyTo( Surface* inDst, int inX, int inY ) const;
	void BlendCopyTo( Surface* inDst, int inX, int inY );
	void ScaleColor( unsigned int inScale );
	void Box( int x1, int y1, int x2, int y2, Pixel color );
	void Bar( int x1, int y1, int x2, int y2, Pixel color );
	void Resize( Surface* inOrig );

	void SetDirty(bool inDirty = true);
	bool IsDirty() const;
	bool IsClear() const;
private:
	// Attributes
	std::vector<Pixel> m_bufferSource;
	Pixel* m_buffer;
	Spek::File::Handle m_sourceFile = nullptr;
	int m_width, m_height;
	int m_pitch;
	int m_flags;
	bool m_dirty = false;
	bool m_clear = true;
};

class Sprite
{
public:
	// Sprite flags
	enum
	{
		FLARE = (1 << 0),
		OPFLARE = (1 << 1),
		FLASH = (1 << 4),
		DISABLED = (1 << 6),
		GMUL = (1 << 7),
		BLACKFLARE = (1 << 8),
		BRIGHTEST = (1 << 9),
		RFLARE = (1 << 12),
		GFLARE = (1 << 13),
		NOCLIP = (1 << 14)
	};

	// Structors
	Sprite( Surface* inSurface, unsigned int inNumFrames );
	~Sprite();
	// Methods
	void Draw( Surface* inTarget, int inX, int inY );
	void DrawScaled( int inX, int inY, int inWidth, int inHeight, Surface* inTarget );
	void SetFlags( unsigned int inFlags ) { m_flags = inFlags; }
	void SetFrame( unsigned int inIndex ) { m_currentFrame = inIndex; }
	unsigned int GetFlags() const { return m_flags; }
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	Pixel* GetBuffer() { return m_surface->GetBuffer(); }
	unsigned int Frames() { return m_numFrames; }
	Surface* GetSurface() { return m_surface; }
	void InitializeStartData();
private:
	// Attributes
	int m_width, m_height, m_pitch;
	unsigned int m_numFrames;
	unsigned int m_currentFrame;
	unsigned int m_flags;
	unsigned int** m_start;
	Surface* m_surface;
};

class SurfaceFont
{
public:
	SurfaceFont( const char *inFile, const char *inChars );
	~SurfaceFont();
	void Print( Surface* inTarget, const char *inText, int inX, int inY, bool clip = false );
	void Centre( Surface* inTarget, const char *inText, int inY );
	int Width( const char *inText );
	int Height() { return m_surface->GetHeight(); }
	void YClip( int y1, int y2 ) { m_cy1 = y1; m_cy2 = y2; }
private:
	Surface* m_surface;
	int* m_offset, *m_width, *m_trans, m_height, m_cy1, m_cy2;
};

}; // namespace Tmpl8

// EOF