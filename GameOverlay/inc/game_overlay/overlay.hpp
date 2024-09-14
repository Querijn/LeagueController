#pragma once

#include <game_overlay/log.hpp>

#include <functional>
#include <cstdio>
#include <string_view>
#include <d3d11.h>

namespace GameOverlay
{
	void* GetDevice();
	void* GetDeviceContext();

	int GetWidth();
	int GetHeight();

	void AddLoopFunction(std::function<void()>);
	void RemoveLoopFunctions();

	void Start(std::string_view logFileName, bool isApp);
	void Destroy(bool immediately = false);

	bool IsReady();

	void SetRenderTarget(ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView);
}