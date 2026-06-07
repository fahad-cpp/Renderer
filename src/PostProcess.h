#ifndef POSTPROCESS_H
#define POSTPROCESS_H
namespace PostProcess {
void FXAAthr(int threadNum, int threadCount, float edgeThreshold = 0.f);
void FXAA(bool multiThread = true);
void renderAO();
void boxBlur();
} // namespace PostProcess
#endif