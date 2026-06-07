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
std::pair<float, float> viewportToCanvas(float x, float y);
Vector projectVertex(const Vector &v);
void interpolate(float x0, float y0, float x1, float y1, std::vector<float> &arr);
void drawVerticesTriangle(const Vector p[3], const Material &material, bool wireframe);
void drawBox(const Box &box, const Transform &tf = {}, bool inTriangle = false);
float intersectRaySphere(Vector &O, Vector &D, const Sphere &sphere);
float intersectRayTriangle(const Vector &O, const Vector &D, Triangle &triangle);
bool RayIntersectsBox(const Vector &O, const Vector &D, const Box &box);
HitData closestIntersection(const Vector &O, const Vector &D, float tMin, float tMax);
Vector reflectRay(const Vector &R, const Vector &N);
float computeLight(const Vector &P, const Vector &N, const Vector V, float s, bool rtShadows = true);
float planeIntersection(Plane &plane, Vector &point);
float edgePlaneIntersection(Plane &plane, const Vector &A, const Vector &B);
std::vector<Triangle> clipTriangle(const Triangle &);
void modelSpaceToDrawable(const Vector p[3], const Transform &transform, std::vector<Vector> &outTris);
void modelSpaceToDrawableThr(const std::vector<Vector> &inTris, const Transform &transform, std::vector<Vector> &outTris, uint32_t start, uint32_t end);
void getDrawableTriangles(const std::vector<Vector> &inTris, const Transform &transform, std::vector<Vector> &outTris, bool multithread = true);
void drawVerticesThr(const std::vector<Vector> &vertices, const Material &material, bool wireframe, uint32_t start, uint32_t end);
void drawVertices(const std::vector<Vector> &vertices, const Material &material, bool wireframe, bool multithread);
void renderMesh(const Mesh &mesh, const Transform &transform, bool multithread = true);
Colour traceRay(const Vector &O, const Vector &D, float tMin, float tMax, int recursionLimit);
void rayTraceThr(const int threadNum, const int threadCount);
void rayTrace();
void renderScene();
}; // namespace Renderer
#endif