#include "common.hpp"
#include "renderer.hpp"
#include "renderer_dx11.hpp"
#include "renderer_vtable_dx11.hpp"

#include <league_controller/config.hpp>
#include <game_overlay/input.hpp>
#include <game_overlay/simple_input.hpp>
#include <spek/util/duration.hpp>

#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <cstdint>

#include <sokol.hpp>
#include <d3d11.h>
#include "MinHook.h"
#include "imgui.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")

#define RENDER_FUNCTION_COUNT 6

#define Hook(source, functionName, original, hook) do\
{\
	void** vtable = *(void***)source;\
	MH_STATUS isHookCreated = MH_CreateHook(vtable[DX11VTI_##functionName], hook, (void**)&original);\
	GameOverlay_AssertF(g_rendererLog, isHookCreated == MH_OK, "Hook for " GameOverlay_XStringify(functionName) " could not be created.");\
	MH_STATUS isHooked = MH_EnableHook(vtable[DX11VTI_##functionName]);\
	GameOverlay_AssertF(g_rendererLog, isHooked == MH_OK, GameOverlay_XStringify(functionName) " could not be hooked.");\
	g_hooks.emplace_back(vtable[DX11VTI_##functionName]);\
} while(0)

// Function typedefs
typedef HRESULT	(STDMETHODCALLTYPE* FuncPresent)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT	(STDMETHODCALLTYPE* FuncResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
typedef void	(STDMETHODCALLTYPE* FuncOMSetRenderTargets)(ID3D11DeviceContext*, UINT, ID3D11RenderTargetView* const*, ID3D11DepthStencilView*);

namespace GameOverlay
{
	LogCategory g_rendererLog("Renderer");
	extern bool g_shouldDestroyGameOverlay;
	using namespace Spek;

	void CommonRender();
	void DestroyInternal();

	// D3D11 defines
	static ID3D11Device* g_device = nullptr;
	static ID3D11DeviceContext* g_deviceContext = nullptr;
	static IDXGISwapChain* g_swapChain = nullptr;
	static ID3D11RenderTargetView* g_renderTargetView = nullptr;
	static ID3D11DepthStencilView* g_depthStencilView = nullptr;
	static std::vector<void*> g_hooks;

	// Render callbacks
	static std::function<void()> g_renderFunctions[RENDER_FUNCTION_COUNT];
	static int g_renderFunctionCount = 0;

	// Overrides
	static FuncPresent g_originalPresent;
	static FuncResizeBuffers g_originalResizeBuffers;

	static int g_bufferWidth = 0;
	static int g_bufferHeight = 0;

	static int g_dockId = 0;
	static bool g_shouldRender = true;
	static bool g_isImGuiEnabled = false;
	static bool g_isImGuiInitialised = false;

	void AddLoopFunction(std::function<void()> function)
	{
		Leacon_Profiler;
		g_renderFunctions[g_renderFunctionCount] = function;
		g_renderFunctionCount++;
	}

	void RemoveLoopFunctions()
	{
		Leacon_Profiler;
		for (int i = 0; i < RENDER_FUNCTION_COUNT; i++)
			g_renderFunctions[i] = nullptr;
		g_renderFunctionCount = 0;
	}

	bool InitRenderView()
	{
		Leacon_Profiler;
		// We need these to render something
		if (g_swapChain == nullptr || g_device == nullptr || g_deviceContext == nullptr)
			return false;

		// We don't need to call this function if we have these initialised
		if (g_bufferWidth != 0 && g_renderTargetView != nullptr)
			return true;

		ID3D11Texture2D* backbuffer = nullptr;
		HRESULT hr = g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
		if (FAILED(hr))
		{
			GameOverlay_LogError(g_rendererLog, "Unable to get backbuffer: %lu.\n", hr);
			return false;
		}

		if (g_bufferWidth == 0)
		{
			D3D11_TEXTURE2D_DESC desc = {};
			backbuffer->GetDesc(&desc);
			g_bufferWidth = desc.Width;
			g_bufferHeight = desc.Height;
			GameOverlay_LogDebug(g_rendererLog, "Backbuffer size: %d %d", g_bufferWidth, g_bufferHeight);
		}

		if (g_renderTargetView == nullptr)
		{
			hr = g_device->CreateRenderTargetView(backbuffer, nullptr, &g_renderTargetView);
			GameOverlay_Assert(g_rendererLog, SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, nullptr);
				GameOverlay_LogDebug(g_rendererLog, "New RenderTargetView: %p", g_renderTargetView);
			}
		}

		backbuffer->Release();

		return true;
	}

	bool InitImGui()
	{
		Leacon_Profiler;
		if (!g_isImGuiEnabled)
			return true;

		if (g_isImGuiInitialised)
			return true;

		if (!InitRenderView())
			return false;

		ImGui::CreateContext();
		bool imguiWin32Initialised = ImGui_ImplWin32_Init(GetWindow());
		GameOverlay_Assert(g_rendererLog, imguiWin32Initialised);
		bool imguiDx11Initialised = ImGui_ImplDX11_Init(g_device, g_deviceContext);
		GameOverlay_Assert(g_rendererLog, imguiDx11Initialised);

		g_isImGuiInitialised = true;
		return g_isImGuiInitialised;
	}

	HRESULT STDMETHODCALLTYPE ResizeBuffersOverride(IDXGISwapChain* swapChain, UINT bufferCount, UINT w, UINT h, DXGI_FORMAT format, UINT flags)
	{
		Leacon_Profiler;
		GameOverlay_LogInfo(g_rendererLog, "ResizeBuffers was called, new size = %u, %u\n", w, h);
		g_bufferWidth = w;
		g_bufferHeight = h;
		if (g_renderTargetView)
		{
			g_renderTargetView->Release();
			g_renderTargetView = nullptr;
		}

		return g_originalResizeBuffers(swapChain, bufferCount, w, h, format, flags);
	}

	class Window
	{
	public:
		Window(LPCSTR className = TEXT("GameOverlayTest"), LPCSTR windowName = TEXT("GameOverlay"))
		{
			Leacon_Profiler;
			WNDCLASSEX windowClass = {};

			m_instance = windowClass.hInstance = GetModuleHandle(nullptr);
			m_className = windowClass.lpszClassName = className;

			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = DefWindowProc;
			RegisterClassEx(&windowClass);
			m_window = CreateWindowA(windowClass.lpszClassName, windowName, WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, nullptr, nullptr, windowClass.hInstance, nullptr);
		}

		~Window()
		{
			Leacon_Profiler;
			Destroy();
		}

		void Destroy()
		{
			Leacon_Profiler;
			if (m_window == nullptr)
			{
				DestroyWindow(m_window);
				m_window = nullptr;
			}

			if (m_instance)
			{
				UnregisterClass(m_className, m_instance);
				m_instance = nullptr;
			}
		}

		HWND GetHandle() const { return m_window; }

	private:
		HINSTANCE m_instance = nullptr;
		HWND m_window = nullptr;
		LPCSTR m_className = nullptr;
	};

	HRESULT STDMETHODCALLTYPE PresentOverride(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
	void InitRendererDX11()
	{
		Leacon_Profiler;
		GameOverlay_LogInfo(g_rendererLog, "InitRendererDX11\n");

		Window window;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = 64;
		swapChainDesc.BufferDesc.Height = 64;
		swapChainDesc.BufferDesc.RefreshRate = { 60, 1 };
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = window.GetHandle();
		swapChainDesc.Windowed = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* deviceContext = nullptr;
		IDXGISwapChain* swapChain = nullptr;

		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &deviceContext);
		GameOverlay_Assert(g_rendererLog, SUCCEEDED(hr));

		Hook(swapChain, Present, g_originalPresent, PresentOverride);
		Hook(swapChain, ResizeBuffers, g_originalResizeBuffers, ResizeBuffersOverride);

		swapChain->Release();
		swapChain = nullptr;

		device->Release();
		device = nullptr;

		deviceContext->Release();
		deviceContext = nullptr;
	}

	HRESULT STDMETHODCALLTYPE PresentOverride(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
	{
		static int pressedCount = 0;
		if (Input::IsKeyPressed(KeyboardKey::KeyF1))
		{
			pressedCount++;
			if (pressedCount >= IF_SUBMISSION_ELSE(10, 1))
				g_isImGuiEnabled = !g_isImGuiEnabled;
		}

		Duration frameDurationStart = GetTimeSinceStart();
		{
			Leacon_Profiler;
			auto present = g_originalPresent;
			if (g_shouldDestroyGameOverlay)
				GameOverlay::DestroyInternal();
			if (g_shouldRender == false || g_shouldDestroyGameOverlay)
				return present(swapChain, syncInterval, flags);

			if (g_device == nullptr || g_swapChain != swapChain)
			{
				if (g_swapChain != nullptr)
					GameOverlay_LogDebug(g_rendererLog, "SwapChain changed from %p to %p\n", g_swapChain, swapChain);

				g_swapChain = swapChain;
				g_swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_device);
				g_device->GetImmediateContext(&g_deviceContext);

				// Forward initialise window
				DXGI_SWAP_CHAIN_DESC desc;
				swapChain->GetDesc(&desc);
				GameOverlay_LogDebug(g_rendererLog, "New Present Setup => (SwapChain: %p, Device: %p, Context: %p, Window: %p)\n", g_swapChain, g_device, g_deviceContext, desc.OutputWindow);
				InitWindow(desc.OutputWindow);

				sg_desc graphicsDesc = { 0 };
				graphicsDesc.context.d3d11.render_target_view_cb = []() { return (const void*)g_renderTargetView; };
				graphicsDesc.context.d3d11.depth_stencil_view_cb = []() { return (const void*)g_depthStencilView; };
				graphicsDesc.context.d3d11.device = g_device;
				graphicsDesc.context.d3d11.device_context = g_deviceContext;
				sg_setup(graphicsDesc);
			}

			if (InitRenderView() == false)
			{
				GameOverlay_LogInfo(g_rendererLog, "Waiting for InitRenderView\n");
				return g_originalPresent(swapChain, syncInterval, flags);
			}

			if (g_isImGuiEnabled)
			{
				if (InitImGui() == false)
				{
					GameOverlay_LogInfo(g_rendererLog, "Waiting for InitImGui\n");
					return g_originalPresent(swapChain, syncInterval, flags);
				}
			}

			CommonRender();

			if (IsDX11ImguiEnabled())
			{
				Leacon_ProfilerEval(ImGui_ImplDX11_NewFrame());
				Leacon_ProfilerEval(ImGui_ImplWin32_NewFrame());
				Leacon_ProfilerEval(ImGui::NewFrame());

				ImGuiViewport* viewport = Leacon_ProfilerEvalRet(ImGui::GetMainViewport());
				g_dockId = Leacon_ProfilerEvalRet(ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode));
			}

			try
			{
#define RUN_PROFILED_RENDER_FUNC(i) if (i < g_renderFunctionCount && g_renderFunctions[i] != nullptr)\
			{\
				Leacon_ProfilerSection("RenderFunction " ## #i);\
				g_renderFunctions[i]();\
			}
				RUN_PROFILED_RENDER_FUNC(0);
				RUN_PROFILED_RENDER_FUNC(1);
				RUN_PROFILED_RENDER_FUNC(2);
				RUN_PROFILED_RENDER_FUNC(3);
				RUN_PROFILED_RENDER_FUNC(4);
				RUN_PROFILED_RENDER_FUNC(5);
			}
			catch (std::exception e)
			{
				GameOverlay_LogError(g_rendererLog, "Exception in render function: %s\n", e.what());
			}

			if (IsDX11ImguiEnabled())
			{
				Leacon_ProfilerEval(ImGui::EndFrame());
				Leacon_ProfilerEval(ImGui::Render());
			}
			
			Leacon_ProfilerEval(g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencilView));
			
			if (IsDX11ImguiEnabled())
				Leacon_ProfilerEval(ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()));
		}

#if NDEBUG
		Duration frameDurationEnd = GetTimeSinceStart();
		Duration frameDuration = frameDurationEnd - frameDurationStart;
		static const Duration maxDuration = Duration::FromMicroseconds(500);
		if (frameDuration > maxDuration)
		{
			int frameDurationUs = (int)(frameDuration.ToSecF64() * 1000.0 * 1000.0);
			static int maxFrameDurationUs = (int)(maxDuration.ToSecF64() * 1000.0 * 1000.0);
			GameOverlay_LogWarning(g_rendererLog, "Took too long to render, took %dus, max is %dus", frameDurationUs, maxFrameDurationUs);
		}
#endif

		auto result = g_originalPresent(swapChain, syncInterval, flags);
		return result;
	}

	void DestroyRendererDX11()
	{
		Leacon_Profiler;
		for (void* hook : g_hooks)
		{
			auto status = MH_DisableHook(hook);
			GameOverlay_Assert(g_rendererLog, status == MH_STATUS::MH_OK);
			
			status = MH_RemoveHook(hook);
			GameOverlay_Assert(g_rendererLog, status == MH_STATUS::MH_OK);
		}
		g_hooks.clear();

		if (g_isImGuiInitialised)
		{
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();

			ImGui::DestroyContext();
		}
		
		sg_shutdown();

		//if (g_deviceContext)
		//	g_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		if (g_renderTargetView)
			g_renderTargetView->Release();
		
		g_device = nullptr;
		g_deviceContext = nullptr;
		g_swapChain = nullptr;
		g_renderTargetView = nullptr;

		for (int i = 0; i < RENDER_FUNCTION_COUNT; i++)
			g_renderFunctions[i] = nullptr;
		g_renderFunctionCount = 0;

		g_originalResizeBuffers = nullptr;

		g_bufferWidth = 0;
		g_bufferHeight = 0;

		g_dockId = 0;
		g_shouldRender = false;
	}

	void SetRenderTarget(ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView)
	{
		g_renderTargetView = renderTargetView;
		g_depthStencilView = depthStencilView;
	}

	bool IsDX11Initialised()
	{
		Leacon_Profiler;
		return g_isImGuiInitialised &&
			g_device != nullptr &&
			g_deviceContext != nullptr &&
			g_renderTargetView != nullptr &&
			g_swapChain != nullptr;
	}

	void* GetDevice()
	{
		Leacon_Profiler;
		return g_device;
	}

	void* GetDeviceContext()
	{
		Leacon_Profiler;
		return g_deviceContext;
	}

	int GetWidth()
	{
		Leacon_Profiler;
		return g_bufferWidth;
	}

	int GetHeight()
	{
		Leacon_Profiler;
		return g_bufferHeight;
	}

	bool IsDX11ImguiEnabled()
	{
		Leacon_Profiler;
		return g_isImGuiEnabled && g_isImGuiInitialised;
	}
}
