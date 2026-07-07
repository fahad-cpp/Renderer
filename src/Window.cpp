#include "Logging.h"
#include "Timer.h"
#include <iostream>

#define _CRT_SECURE_NO_WARNINGS
// #define NOMINMAX

#include "Globals.h"
#include "Main.h"
#include "resource.h"
#include <FSWindow.h>

int main() {
    // Random seed
    srand(uint32_t(time(NULL)));
    Timer timer;
    FS::Window window("Renderer!", 720, 720);
    FS::RenderState &renderState = window.getRenderState();
    canvas = { float(renderState.width), float(renderState.height) };
    float aspectratio = float(renderState.width) / float(renderState.height);
    vpWidth = aspectratio;
    vpHeight = 1;
    // Hack : TODO:reconstruct on resize
    size_t mutexSize = 1920 * 1080 * sizeof(std::mutex);
    pixelLocks = (std::mutex *)malloc(mutexSize);
    timer.Stop();
    LOG_INFO("Initialization took " << timer.dtms << " ms\n");
    init();
    window.showCursor(!sceneSettings.lockMouse);
    window.focus();
    while (running) {
        // Update Loop
        update(window);
        window.processMessages();
        window.swapBuffers();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    free(pixelLocks);
    return 0;
}
