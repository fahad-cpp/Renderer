#ifndef GLOBALS_H
#define GLOBALS_H
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include "Object.h"
#include "Scene.h"
#include "SceneSettings.h"
#include "Transform.h"
#include <math.h>
#include <mutex>
#include <thread>

extern SceneSettings sceneSettings;
extern Vector canvas;
extern std::vector<std::thread> ppmThreads;
extern std::mutex *pixelLocks;
const float d = 0.52f;
const float farDist = 1000.f;
extern float vpWidth;
extern float vpHeight;
extern bool change;
extern float frameLimit;
extern double FOV;
extern Transform camera;
extern Scene scene;
extern Plane planes[6];

#endif
