#pragma once
#include <Windows.h>
#include "Vector.h"
#include "Timer.h"
#include "Typedefs.h"
#include "Platform_common.h"

#define isDown(b) input.buttons[b].isDown
#define pressed(b) (input.buttons[b].isDown && input.buttons[b].changed)
#define released(b) (!input.buttons[b].isDown && input.buttons[b].changed)
struct RenderState {
	int height;
	int width;
	void* memory;
	float* ambientOcclusion;
	BITMAPINFO bitmapinfo;
};
struct Window {
	HWND handle;
	HDC dc;
	Input input;
};
void turnConsoleOff();
Vector getMouseDiff();
LRESULT CALLBACK window_callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void swapBuffers();
void initWindow();
void deleteWindow();