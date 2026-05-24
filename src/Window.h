#pragma once
#include "Platform_common.h"
#include "Vector.h"
#include <Windows.h>

#define isDown(b) input.buttons[b].isDown
#define pressed(b) (input.buttons[b].isDown && input.buttons[b].changed)
#define released(b) (!input.buttons[b].isDown && input.buttons[b].changed)
struct RenderState {
    BITMAPINFO bitmapinfo;
    uint32_t height;
    float *ambientOcclusion;
    void *memory;
    uint32_t width;
};
struct Window {
    Input input;
    HWND handle;
    HDC dc;
};
void turnConsoleOff();
Vector getMouseDiff();
LRESULT CALLBACK window_callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void swapBuffers();
void initWindow();
void deleteWindow();
