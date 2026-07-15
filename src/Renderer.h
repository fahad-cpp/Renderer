#ifndef RENDERER_H
#define RENDERER_H
#include "Object.h"
#include "Transform.h"
#include <FSWindow.h>
#include <cassert>
#include <cstdint>
#include <string>

#define WHITE { 255, 255, 255 }
namespace Renderer {
void clearScreen(uint32_t color, FS::RenderState &renderState);
void putPixel(const int x, const int y, const Colour &color, FS::RenderState &renderState);
void putPixelD(const int x, const int y, const Colour &color, FS::RenderState &renderState);
void renderDepthBuffer(FS::RenderState &renderState);
Colour getPixel(const int x, const int y, FS::RenderState &renderState);
void drawSquare(float x, float y, int size, Colour color, FS::RenderState &renderState);
void drawNoise(FS::RenderState &renderState);
void printPPM(const std::string &filename, FS::RenderState &renderState);
void exportToPPM(const std::string &filename, uint32_t *buffer, int width, int height);
void drawLine(Vector a, Vector b, const Colour &color, FS::RenderState &renderState);
Vector canvasToViewport(float x, float y);
Vector viewportToCanvas(float x, float y);
Vector projectVertex(const Vector &v);
template <typename T>
void interpolate(T x0, float y0, T x1, float y1, std::vector<T> &arr);
void drawVerticesTriangle(const Vector p[3], const Vector n[3], const Material &material, bool wireframe, FS::RenderState &renderState);
void drawTriangleDepth(const Vector p[3], FS::RenderState &renderState);
void drawBox(const Box &box, const Transform &tf, bool inTriangle, FS::RenderState &renderState);
float intersectRaySphere(const Vector &O, const Vector &D, const Sphere &sphere);
float intersectRayTriangle(const Vector &O, const Vector &D, Triangle &triangle);
bool RayIntersectsBox(const Vector &O, const Vector &D, const Box &box);
HitData closestIntersection(const Vector &O, const Vector &D, float tMin, float tMax);
Vector reflectRay(const Vector &R, const Vector &N);
float computeLight(const Vector &P, const Vector &N, const Vector V, float s, bool rtShadows = true);
float planeIntersection(const Plane &plane, const Vector &point);
float edgePlaneIntersection(const Plane &plane, const Vector &A, const Vector &B);
void clipPlane(const Plane &plane, const std::vector<Triangle> &in, std::vector<Triangle> &out);
void clipTriangle(const std::vector<Triangle> &in, std::vector<Triangle> &out);
void modelSpaceToDrawable(const Vector p[3], const Vector n[3], const Transform &transform, std::vector<Triangle> &outData);
void modelSpaceToDrawableThr(const std::vector<Triangle> &triangleData, const Transform &transform, std::vector<Triangle> &outData, uint32_t start, uint32_t end);
void getDrawableTriangles(const std::vector<Triangle> &triangleData, const Transform &transform, std::vector<Triangle> &outData, bool multithread);
void drawVerticesThr(const std::vector<Triangle> &triangleData, const Material &material, bool wireframe, FS::RenderState &renderState, uint32_t start, uint32_t end);
void drawVerticesDepthThr(const std::vector<Triangle> &triangleData, FS::RenderState &renderState, uint32_t start, uint32_t end);
void drawVertices(const std::vector<Triangle> &triangleData, const Material &material, bool wireframe, bool multithread, FS::RenderState &renderState);
void drawVerticesDepth(const std::vector<Triangle> &triangleData, FS::RenderState &renderState, bool multithread);
void renderMesh(const Mesh &mesh, const Transform &transform, bool multithread, FS::RenderState &renderState);
Colour traceRay(const Vector &O, const Vector &D, float tMin, float tMax, int recursionLimit);
void rayTraceThr(const int threadNum, const int threadCount, FS::RenderState &renderState);
void rayTrace(FS::RenderState &renderState);
void renderScene(FS::RenderState &renderState);
}; // namespace Renderer
#endif