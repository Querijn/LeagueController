#include <league_controller/config.hpp>

#if !LEACON_FINAL

#include <sokol.hpp>
#include <imgui.h>

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

#include <game_overlay/log.hpp>
#include <game_overlay/image.hpp>
#include <game_overlay/image_instance.hpp>
#include <game_overlay/overlay.hpp>
#include <game_overlay/skerm.hpp>
#include <game_overlay/radial_menu.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <windows.h>
#include <set>

namespace LeagueController
{
	using Duration = Spek::Duration;
	using namespace Spek;
	using UISrcFile = GameOverlay::SourceHandler::Source::Ptr;

	// From setup.cpp
	void __cdecl Thread(void* handle);
	void Update();

	static int m_dockID = 0;
	static GameOverlay::LogCategory g_mainLog("Main");
	static std::unique_ptr<GameOverlay::ImageInstance> image;
	static sg_pass_action clearPassAction;
	static sg_pass_action imguiPassAction;
	static int m_dockId;

	// static GameOverlay::SourceHandler m_sourceHandler;
	static std::set<UISrcFile> m_sources;

	static GameOverlay::RadialMenu g_radialMenu;

	void init()
	{
		Leacon_Profiler;

		// This forces my default working area on my 32/9 monitor
		RECT workArea;
		SystemParametersInfoA(SPI_GETWORKAREA, sizeof(RECT), &workArea, 0);
		int width = workArea.right - workArea.left;
		int height = workArea.bottom - workArea.top;
		int div3 = (5120 / 3);
		if (width == 5120)
		{
			int x = div3 * 2;
			int y = 0;
			SetWindowPos((HWND)sapp_win32_get_hwnd(), 0, x, y, div3, height, 0);
		}

		Thread(nullptr);
		/*m_sourceHandler.CompileFile("data/ui/controller.ui", [](UISrcFile source, Spek::File::Handle file, Spek::File::LoadState state)
		{
			assert(state == Spek::File::LoadState::Loaded);
			m_sources.insert(source);
		});*/
		
		// setup sokol-gfx, sokol-time and sokol-imgui
		sg_desc desc = { };
		desc.context = sapp_sgcontext();
		sg_setup(&desc);

		// use sokol-imgui with all default-options (we're not doing
		// multi-sampled rendering or using non-default pixel formats)
		simgui_desc_t simgui_desc = { };
		simgui_setup(&simgui_desc);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// initial clear color
		clearPassAction.colors[0].action = SG_ACTION_CLEAR;
		clearPassAction.colors[0].value = { 0.3f, 0.7f, 0.5f, 1.0f };

		imguiPassAction.colors[0].action = SG_ACTION_DONTCARE;

		GameOverlay::ForceRenderLog();
	}

	void frame()
	{
		Leacon_Profiler;
		Leacon_ProfilerFrame;
		const int width = sapp_width();
		const int height = sapp_height();
		Leacon_ProfilerEval(sg_begin_default_pass(&clearPassAction, width, height));
		Leacon_ProfilerEval(sg_end_pass());

		auto dt = Spek::Duration::FromSecondsF(sapp_frame_duration());
		Leacon_ProfilerEval(simgui_new_frame({ width, height, sapp_frame_duration(), sapp_dpi_scale() }));

		ImGuiViewport* viewport = Leacon_ProfilerEvalRet(ImGui::GetMainViewport());
		m_dockID = Leacon_ProfilerEvalRet(ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode));

		Update();
		sg_begin_default_pass(imguiPassAction, width, height);

		/*for (const auto& source : m_sources)
			for (const auto& container : source->Output.Elements)
				GameOverlay::RenderUI(container.get(), width, height);*/
		
		glm::mat4 ortho = glm::ortho(0.0f, (f32)width, 0.f, (f32)height, -1.0f, 1.0f);
		// g_radialMenu.Draw(dt, ortho, width, height);
		
		simgui_render();
		sg_end_pass();
	}

	void cleanup()
	{
		Leacon_Profiler;
		g_radialMenu.Reset();
		image = nullptr;
		GameOverlay::Destroy(true);
		simgui_shutdown();
		sg_shutdown();
	}

	void input(const sapp_event* event)
	{
		Leacon_Profiler;
		simgui_handle_event(event);
	}
}


int main()
{
	Leacon_Profiler;
	sapp_desc desc = { };

	desc.init_cb = LeagueController::init;
	desc.frame_cb = LeagueController::frame;
	desc.cleanup_cb = LeagueController::cleanup;
	desc.event_cb = LeagueController::input;

	desc.width = 1024;
	desc.height = 768;
	desc.window_title = "Spek";
	desc.ios_keyboard_resizes_canvas = false;
	desc.icon.sokol_default = true;
	sapp_run(&desc);
}
#endif
