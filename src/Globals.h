#ifndef GLOBALS_H
#define GLOBALS_H

#include <math.h>
#include <thread>
#include <mutex>
#include "Window.h"
#include "Object.h"
#include "SceneSettings.h"
#include "Transform.h"
#include "Scene.h"

extern Window window;
extern void* depth;
extern RenderState renderState;
extern bool running;
extern SceneSettings sceneSettings;
extern Vector canvas;
extern std::vector<std::thread> ppmThreads;
extern std::mutex* pixelLocks;
//less = more FOV , more = lesser FOV
const float d = 0.52f;
const float farDist = 1000.f;
extern Vector D;
extern float vpWidth;
extern float vpHeight;
//only rendering when frame change to make sure my cpu doesn't explode
//Especially for Ray tracer
extern bool change;
//true = raytracer , false = rasterizer
extern bool rayTraceMode;

//144 fps
extern float frameLimit;
extern double FOV;
extern Transform camera;
extern Scene scene;
//temp
extern Triangle tempTri;
extern Plane planes[6];

#endif