#pragma once
#include <FSWindow.h>

//#define isDown(b) input.buttons[b].isDown
#define pressed(b) (input.buttons[b].isDown && input.buttons[b].changed)
#define released(b) (!input.buttons[b].isDown && input.buttons[b].changed)
FS::Vector2 getMouseDiff(FS::Window& window);