#ifndef RENDERER_H
#define RENDERER_H
#include "Object.h"
#include "Transform.h"
#include <cassert>
#include <cstdint>
#include <string>
#define WHITE { 255, 255, 255 }
namespace Renderer {
void clearScreen(uint32_t color);
void putPixel(const int x, const int y, const Colour &color);
void putPixelD(const int x, const int y, const Colour &color);
void renderDepthBuffer();
Colour getPixel(const int x, const int y);
void drawSquare(float x, float y, int size, Colour color);
void drawNoise();
void printPPM(const std::string &filename);
void exportToPPM(const std::string &filename, uint32_t *buffer, int width, int height);
void drawLine(Vector a, Vector b, const Colour &color);
Vector canvasToViewport(float x, float y);
Vector viewportToCanvas(float x, float y);
Vector projectVertex(const Vector &v);
template<typename T>
void interpolate(T x0, float y0, T x1, float y1, std::vector<T> &arr);
void drawVerticesTriangle(const Vector p[3], const Vector n[3], const Material &material, bool wireframe);
void drawBox(const Box &box, const Transform &tf = {}, bool inTriangle = true);
float intersectRaySphere(const Vector &O, const Vector &D, const Sphere &sphere);
float intersectRayTriangle(const Vector &O, const Vector &D, Triangle &triangle);
bool RayIntersectsBox(const Vector &O, const Vector &D, const Box &box);
HitData closestIntersection(const Vector &O, const Vector &D, float tMin, float tMax);
Vector reflectRay(const Vector &R, const Vector &N);
float computeLight(const Vector &P, const Vector &N, const Vector V, float s, bool rtShadows = true);
float planeIntersection(const Plane &plane, const Vector &point);
float edgePlaneIntersection(const Plane &plane, const Vector &A, const Vector &B);
void clipTriangle(const std::vector<Triangle> &in, std::vector<Triangle> &out);
void modelSpaceToDrawable(const Vector p[3], const Vector n[3], const Transform &transform, std::vector<Triangle> &outData);
void modelSpaceToDrawableThr(const std::vector<Triangle> &triangleData, const Transform &transform, std::vector<Triangle> &outData, uint32_t start, uint32_t end);
void getDrawableTriangles(const std::vector<Triangle> &triangleData, const Transform &transform, std::vector<Triangle> &outData, bool multithread);
void drawVerticesThr(const std::vector<Triangle> &triangleData, const Material &material, bool wireframe, uint32_t start, uint32_t end);
void drawVertices(const std::vector<Triangle> &triangleData, const Material &material, bool wireframe, bool multithread);
void renderMesh(const Mesh &mesh, const Transform &transform, bool multithread = true);
Colour traceRay(const Vector &O, const Vector &D, float tMin, float tMax, int recursionLimit);
void rayTraceThr(const int threadNum, const int threadCount);
void rayTrace();
void renderScene();
}; // namespace Renderer
#endif