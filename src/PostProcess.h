#ifndef POSTPROCESS_H
#define POSTPROCESS_H
#include "FSWindow.h"
namespace PostProcess {
void FXAAthr(int threadNum, int threadCount,FS::RenderState& renderState, float edgeThreshold);
void FXAA(FS::RenderState& renderState,bool multiThread = true);
void renderAO(FS::RenderState& renderState);
void boxBlur(FS::RenderState& renderState);
} // namespace PostProcess
#endif