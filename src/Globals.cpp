#include "Globals.h"

Window window = {};
void *depthBuffer = nullptr;
RenderState renderState;
std::vector<std::thread> ppmThreads;
std::mutex *pixelLocks;
bool running = true;
SceneSettings sceneSettings = { true, false, 0, DebugState::DS_OFF, true, RenderMode::RM_COLOR, false, LightingMode::LIGHT_ONLY };
Vector canvas(720, 720);
Vector D = { 1, 1, 1 };
float vpWidth = 1.f;
float vpHeight = 1.f;
// only ray tracing when frame change to make sure my cpu doesn't explode
// Especially for Ray tracer
bool change = true;
// true = raytracer , false = rasterizer

// 144 fps
float frameLimit = 6.944444415f;
// float frameLimit = 1.f;
double FOV = 90;
// sponza ideal camera
Transform camera{ Vector{ 80.f, 60.f, -3.6f }, 1, { 0, -270, 0 } };
// Transform camera{ Vector{ 0, 2.f, 0 }, 1, { 0, 0, 0 } };
Scene scene = {};
// temp
Triangle tempTri;
Plane planes[6] = {
    // Near
    { { 0, 0, 1 }, -d },
    // far
    { { 0, 0, -1 }, farDist },
    // Right
    { { float(-1 / sqrt(2)), 0, float(1 / sqrt(2)) }, 0.f },
    // Left
    { { float(1 / sqrt(2)), 0, float(1 / sqrt(2)) }, 0.f },
    // Up
    { { 0, float(-1 / sqrt(2)), float(1 / sqrt(2)) }, 0.f },
    // Down
    { { 0, float(1 / sqrt(2)), float(1 / sqrt(2)) }, 0.f }
};
