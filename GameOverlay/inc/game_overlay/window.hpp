#pragma once

namespace GameOverlay
{
	bool InitWindow(void* windowHandle);
	void DestroyWindow();
	void* GetWindow();

	bool GetClientCenter(int& x, int& y);
	bool GetClientGlobalPosition(int& x, int& y);
	bool GetWindowPosition(int& x, int& y);
	bool GetWindowCenter(int& x, int& y);
	bool IsWindowFocussed();
	bool IsImguiEnabled();
}