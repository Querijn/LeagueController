#pragma once

namespace GameOverlay
{
	enum class RendererType
	{
		Unknown,
		DX11
	};

	void InitRenderer();
	void DestroyRenderer();
	RendererType GetRendererType();
	bool IsRendererInitialised();
}