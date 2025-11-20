#pragma once
#include "Globals.h"
#include "Logging.h"
#include "Object.h"
#include "Transform.h"
#include "Hash.h"
#include "Utility.h"
#include "Typedefs.h"
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <string>
#include <string_view>
#include <sstream>
#include <thread>
#define WHITE {255,255,255}
namespace Renderer {
	void clearScreen(u32 color);
	void putPixel(int x, int y, Colour color);
	void putPixelD(int x, int y, Colour color);
	Colour getPixel(int x, int y);
	void renderDepthBuffer();
	void drawSquare(float x, float y, int size, Colour color);
	void drawNoise();
	void printPPM(const std::string& filename);
	void exportToPPM(const std::string& filename, u32* buffer, int width, int height);
	void drawLine(Vector a, Vector b, Colour color);
	Mesh loadOBJ(std::string filename, Colour color = { 0,0,0 }, float reflectiveness = 0, float specular = -1);
	Vector canvasToViewport(float x, float y);
	std::pair<float, float> viewportToCanvas(float x, float y);
	Vector projectVertex(Vector v);
	void interpolate(float x0, float y0, float x1, float y1, std::vector<float>& arr);
	void drawTriangle(const Triangle& t, bool wireframe = true);
	std::vector<Triangle> clipTriangle(const Triangle&);
	void drawTrianglesMultiThread(const std::vector<Triangle>&, bool, unsigned int);
	void drawBox(Box box, Transform tf = {}, bool inTriangle = true);
	float intersectRaySphere(Vector& O, Vector& D, Sphere& sphere);
	float intersectRayTriangle(Vector O, Vector D, Triangle triangle);
	bool RayIntersectsBox(Vector& O, Vector& D, Box& box);
	HitData closestIntersection(Vector O, Vector D, float tMin, float tMax);
	Vector reflectRay(const Vector& R, Vector& N);
	float computeLight(Vector& P, Vector& N, const Vector V, float s, bool rtShadows = true);
	float planeIntersection(Plane& plane, Vector& point);
	float edgePlaneIntersection(Plane& plane, const Vector& A, const Vector& B);
	void getDrawableTriangles(const std::vector<Triangle>& inTris,const Transform& transform, std::vector<Triangle>& outTris, bool multithread = true);
	void modelSpaceToDrawableThr(const std::vector<Triangle>* inTris,const Transform& transform, std::vector<Triangle>* outTris, u32 start , u32 end);
	void FXAAthr(int threadNum, int threadCount, float edgeThreshold = 0.f);
	void FXAA(bool multiThread = true);
	void drawTrianglesThr(const std::vector<Triangle>& tris, size_t start, size_t end, bool drawWireframe);
	void renderMesh(const Mesh& mesh,const Transform& transform, bool multithread = true);
	Colour traceRay(Vector O, Vector D, float tMin, float tMax, int recursionLimit);
	void rayTraceThr(int threadNum, int threadCount);
	void rayTrace();
	void renderScene();
	void renderAO();
};