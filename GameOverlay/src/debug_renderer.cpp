#include <game_overlay/overlay.hpp>
#include <game_overlay/debug_renderer.hpp>

#include <sokol_gfx.h>
#include <sokol_app.h>
#include <vector>

#include "shaders/debug_renderer.hpp"

namespace GameOverlay
{
	Surface DebugDraw(64, 64);

	static struct
	{
		sg_pipeline Pipeline;
		sg_bindings Bindings;
		sg_pass_action PassAction;

		int LastPixelCount = 0;

		bool Initialised = false;
	} g_state;

	void ValidateDebugDraw()
	{
		Leacon_Profiler;
		int pixelCount = GameOverlay::GetWidth() * GameOverlay::GetHeight();
		if (g_state.Initialised && pixelCount == g_state.LastPixelCount)
			return;

		sg_destroy_image(g_state.Bindings.fs_images[SLOT_tex]);
		sg_destroy_pipeline(g_state.Pipeline);

		float vertices[] = { -1.0f, -3.0f, 3.0f, 1.0f, -1.0f, 1.0f };
		sg_buffer_desc bufferDesc = {};
		bufferDesc.data = SG_RANGE(vertices);
		bufferDesc.label = "fsq vertices";
		g_state.Bindings.vertex_buffers[0] = sg_make_buffer(bufferDesc);

		sg_image_desc imageDesc = {};
		imageDesc.width = GameOverlay::GetWidth();
		imageDesc.height = GameOverlay::GetHeight();
		imageDesc.pixel_format = SG_PIXELFORMAT_RGBA8;
		imageDesc.min_filter = SG_FILTER_NEAREST;
		imageDesc.mag_filter = SG_FILTER_NEAREST;
		imageDesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
		imageDesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
		imageDesc.usage = SG_USAGE_STREAM;
		imageDesc.label = "DebugRenderImage";
		g_state.Bindings.fs_images[SLOT_tex] = sg_make_image(imageDesc);

		// shader and pipeline object for rendering a fullscreen quad
		sg_pipeline_desc pipelineDesc = {};
		pipelineDesc.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
		pipelineDesc.shader = sg_make_shader(DebugRenderer_shader_desc(sg_query_backend()));

		sg_blend_state& blendState = pipelineDesc.colors[0].blend;
		blendState.enabled = true;
		blendState.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		blendState.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		blendState.op_rgb = SG_BLENDOP_ADD;
		blendState.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA;
		blendState.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		blendState.op_alpha = SG_BLENDOP_ADD;
		g_state.Pipeline = sg_make_pipeline(pipelineDesc);

		// don't need to clear since the whole framebuffer is overwritten
		g_state.PassAction = {};
		g_state.PassAction.colors[0].action = SG_ACTION_DONTCARE;

		g_state.LastPixelCount = pixelCount;
		DebugDraw.Resize(GameOverlay::GetWidth(), GameOverlay::GetHeight());
		g_state.Initialised = true;
	}

	void RenderDebugDraw()
	{
		Leacon_Profiler;
		ValidateDebugDraw();
		
		static bool hasDrawnClear = true;
		bool isClear = DebugDraw.IsClear();
		bool isDirty = DebugDraw.IsDirty();
		if ((isClear && !hasDrawnClear) || isDirty)
		{
			Leacon_ProfilerSection("Update Image");
			static sg_image_data pixelData = {};
			pixelData.subimage[0][0].ptr = DebugDraw.GetBuffer();
			pixelData.subimage[0][0].size = DebugDraw.GetPixelCount() * 4;
			sg_update_image(g_state.Bindings.fs_images[SLOT_tex], pixelData);

			hasDrawnClear = isClear;
		}

		int width = GameOverlay::GetWidth();
		int height = GameOverlay::GetHeight();
		sg_begin_default_pass(&g_state.PassAction, width, height);
		sg_apply_pipeline(g_state.Pipeline);
		sg_apply_bindings(&g_state.Bindings);
		sg_draw(0, 3, 1);

		sg_end_pass();
		DebugDraw.Clear();
	}
}