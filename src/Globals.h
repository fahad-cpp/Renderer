#ifndef GLOBALS_H
#define GLOBALS_H
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include "Object.h"
#include "Scene.h"
#include "SceneSettings.h"
#include "Transform.h"
#include "Window.h"
#include <math.h>
#include <mutex>
#include <thread>

extern Window window;
extern void *depthBuffer;
extern RenderState renderState;
extern bool running;
extern SceneSettings sceneSettings;
extern Vector canvas;
extern std::vector<std::thread> ppmThreads;
extern std::mutex *pixelLocks;
// less = more FOV , more = lesser FOV
const float d = 0.52f;
const float farDist = 1000.f;
extern float vpWidth;
extern float vpHeight;
// only rendering when frame change to make sure my cpu doesn't explode
// Especially for Ray tracer
extern bool change;

// 144 fps
extern float frameLimit;
extern double FOV;
extern Transform camera;
extern Scene scene;
// temp
extern Triangle tempTri;
extern Plane planes[6];

#endif
