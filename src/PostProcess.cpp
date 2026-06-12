#include "PostProcess.h"
#include "Globals.h"
#include "Renderer.h"
#include "Utility.h"
#include <FSWindow.h>
#include <random>

namespace PostProcess {
void FXAAthr(int threadNum, int threadCount, float edgeThreshold) {
    int yCount = (renderState.height - 2) / threadCount;
    int ymin = (threadNum * yCount) + 1;
    int ymax = ymin + yCount;
    for (int y = ymin; y < ymax; y++) {
        for (uint32_t x = 1; x < renderState.width - 1; x++) {
            Colour colorCenter = Renderer::getPixel(x, y);
            Colour colorTop = Renderer::getPixel(x, y - 1);
            Colour colorBottom = Renderer::getPixel(x, y + 1);
            Colour colorLeft = Renderer::getPixel(x - 1, y);
            Colour colorRight = Renderer::getPixel(x + 1, y);

            float topLuma = colorTop.luminance();
            float bottomLuma = colorBottom.luminance();
            float leftLuma = colorLeft.luminance();
            float rightLuma = colorRight.luminance();

            float edgeHorizontal = std::abs(leftLuma - rightLuma);
            float edgeVertical = std::abs(topLuma - bottomLuma);

            bool isHorizontal = (edgeHorizontal >= edgeVertical);

            Colour blendColour;
            if (isHorizontal) {
                blendColour.R = ((colorTop.R + colorBottom.R) * 0.5f);
                blendColour.G = ((colorTop.G + colorBottom.G) * 0.5f);
                blendColour.B = ((colorTop.B + colorBottom.B) * 0.5f);
            } else {
                blendColour.R = ((colorLeft.R + colorRight.R) * 0.5f);
                blendColour.G = ((colorLeft.G + colorRight.G) * 0.5f);
                blendColour.B = ((colorLeft.B + colorRight.B) * 0.5f);
            }

            bool isEdge = (getMax(edgeHorizontal, edgeVertical) > edgeThreshold);

            if (isEdge) {
                Renderer::putPixelD(x, y, blendColour);
            } else {
                Renderer::putPixelD(x, y, colorCenter);
            }
        }
    }
}
void FXAA(bool multiThread) {
    float edgeThreshold = 0.f;
    if (!multiThread) {
        // Single Threaded FXAA
        for (uint32_t y = 1; y < (renderState.height - 1); y++) {
            for (uint32_t x = 1; x < (renderState.width - 1); x++) {
                Colour colorCenter = Renderer::getPixel(x, y);
                Colour colorTop = Renderer::getPixel(x, y - 1);
                Colour colorBottom = Renderer::getPixel(x, y + 1);
                Colour colorLeft = Renderer::getPixel(x - 1, y);
                Colour colorRight = Renderer::getPixel(x + 1, y);

                float topLuma = colorTop.luminance();
                float bottomLuma = colorBottom.luminance();
                float leftLuma = colorLeft.luminance();
                float rightLuma = colorRight.luminance();

                float edgeHorizontal = std::abs(leftLuma - rightLuma);
                float edgeVertical = std::abs(topLuma - bottomLuma);

                bool isHorizontal = (edgeHorizontal >= edgeVertical);

                Colour blendColour;
                if (isHorizontal) {
                    blendColour.R = ((colorTop.R + colorBottom.R) * 0.5f);
                    blendColour.G = ((colorTop.G + colorBottom.G) * 0.5f);
                    blendColour.B = ((colorTop.B + colorBottom.B) * 0.5f);
                } else {
                    blendColour.R = ((colorLeft.R + colorRight.R) * 0.5f);
                    blendColour.G = ((colorLeft.G + colorRight.G) * 0.5f);
                    blendColour.B = ((colorLeft.B + colorRight.B) * 0.5f);
                }

                bool isEdge = (getMax(edgeHorizontal, edgeVertical) > edgeThreshold);

                if (isEdge) {
                    Renderer::putPixelD(x, y, blendColour);
                } else {
                    Renderer::putPixelD(x, y, colorCenter);
                }
            }
        }
    } else {
        // Multi-Threaded FXAA
        int threadCount = 12;
        std::vector<std::thread> tObjs(threadCount);
        for (int i = 0; i < threadCount; i++) {
            tObjs[i] = std::thread(FXAAthr, i, threadCount, 0.f);
        }
        for (int i = 0; i < threadCount; i++) {
            tObjs[i].join();
        }
    }
}
void renderAO() {
    std::random_device rd;
    std::mt19937 gen(rd());
    const int SAMPLE_COUNT = 64;
    const float SAMPLE_RADIUS = 2.f;
    std::uniform_int_distribution<> dist(-SAMPLE_RADIUS, SAMPLE_RADIUS);
    Vector samplesLoc[SAMPLE_COUNT];
    for (uint32_t y = 0; y < renderState.height; y++) {
        for (uint32_t x = 0; x < renderState.width; x++) {
            uint32_t pixelIndex = (y * renderState.width) + x;
            float pixelDepth = ((float *)depthBuffer)[pixelIndex];
            float z = 1.f / pixelDepth;
            Vector point = Renderer::canvasToViewport(x * z / d, y * z / d);
            
            for (uint32_t i = 0; i < SAMPLE_COUNT; i++) {
                samplesLoc[i] = { float(dist(gen)) + point.x, float(dist(gen)) + point.y, float(dist(gen)) + z};
            }
            int occlusionFactor = 0;
            for (uint32_t i = 0; i < SAMPLE_COUNT; i++) {
                //if (!isIn(uint32_t(samplesLoc[i].x), 0u, renderState.width) || !isIn(uint32_t(samplesLoc[i].y), 0u, renderState.height)) {
                //    continue;
                //}
                Vector offset = Renderer::projectVertex(samplesLoc[i]);
                if (!isIn(uint32_t(offset.x), 0u, renderState.width) || !isIn(uint32_t(offset.y), 0u, renderState.height)) {
                    clamp(offset.x, 0.f, (float)renderState.width);
                    clamp(offset.y, 0.f, (float)renderState.height);
                }
                uint32_t offsetIndex = uint32_t(offset.x) + (uint32_t(offset.y) * renderState.width);
                float sampleDepth = ((float *)depthBuffer)[offsetIndex];
                // float threshold = 0.001f;
                if ((sampleDepth > pixelDepth) && (pixelDepth != 0.f)) {
                    occlusionFactor++;
                }
            }
            Colour color = { 255, 255, 255 }; // Renderer::getPixel(x, y);
            Renderer::putPixelD(x, y, color * (1 - float(occlusionFactor) / SAMPLE_COUNT));
            renderState.ambientOcclusion[pixelIndex] = (float(occlusionFactor) / SAMPLE_COUNT);
        }
    }
    //boxBlur();
}
void boxBlur() {
    uint32_t *buffer = (uint32_t *)malloc(renderState.width * renderState.height * sizeof(uint32_t));
    if (!buffer) {
        return;
    }
    memcpy(buffer, renderState.memory, renderState.width * renderState.height * sizeof(uint32_t));
    for (uint32_t y = 1; y < renderState.height - 1; y++) {
        for (uint32_t x = 1; x < renderState.width - 1; x++) {
            Colour grid[9] = {
                Renderer::getPixel(x - 1, y - 1),
                Renderer::getPixel(x, y - 1),
                Renderer::getPixel(x + 1, y - 1),

                Renderer::getPixel(x - 1, y),
                Renderer::getPixel(x, y),
                Renderer::getPixel(x + 1, y),

                Renderer::getPixel(x - 1, y + 1),
                Renderer::getPixel(x, y + 1),
                Renderer::getPixel(x + 1, y + 1)
            };

            int totalR = 0;
            int totalG = 0;
            int totalB = 0;

            for (int i = 0; i < 9; i++) {
                totalR += grid[i].R;
                totalG += grid[i].G;
                totalB += grid[i].B;
            }

            uint8_t R = totalR / 9;
            uint8_t G = totalG / 9;
            uint8_t B = totalB / 9;

            uint32_t index = (y * renderState.width) + x;
            buffer[index] = rgbtoHex({ R, G, B });
        }
    }
    memcpy(renderState.memory, buffer, renderState.width * renderState.height * sizeof(uint32_t));
    free(buffer);
}
} // namespace PostProcess