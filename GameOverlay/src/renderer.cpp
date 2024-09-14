#include "renderer.hpp"
#include "renderer_dx11.hpp"

#include <league_controller/config.hpp>

#include <game_overlay/image.hpp>
#include <game_overlay/image_instance.hpp>
#include <game_overlay/log.hpp>

#if !LEACON_SUBMISSION
#include <game_overlay/debug_renderer.hpp>
#endif

namespace GameOverlay
{
	void InitRenderer()
	{
		Leacon_Profiler;
		InitRendererDX11();
	}

	void DestroyRenderer()
	{
		Leacon_Profiler;
		DestroyRendererDX11();
	}

	RendererType GetRendererType()
	{
		Leacon_Profiler;
		if (IsDX11Initialised())
			return RendererType::DX11;
		return RendererType::Unknown;
	}

	bool IsRendererInitialised()
	{
		Leacon_Profiler;
		return IsDX11Initialised();
	}

	bool IsImguiEnabled()
	{
		Leacon_Profiler;
		return IsDX11ImguiEnabled();
	}

	void CommonRender()
	{
		Leacon_Profiler;
		Image::OnFrameStart();
		ImageInstance::RenderAll();

		IF_NOT_SUBMISSION(RenderDebugDraw());
	}
}