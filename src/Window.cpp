#define _CRT_SECURE_NO_WARNINGS
#include "Globals.h"
#include "Logging.h"
#include "Main.h"
#include "Renderer.h"
#include "Timer.h"
#include <FSWindow.h>

int main() {
    // Random seed
    srand(uint32_t(time(NULL)));
    Timer timer;
    FS::Window window("Renderer!", 720, 720);
    FS::RenderState &renderState = window.getRenderState();
    FS::Vector2 windowPos = window.getWindowPos();
    canvas = { float(renderState.width), float(renderState.height) };
    float aspectratio = float(renderState.width) / float(renderState.height);
    vpWidth = aspectratio;
    vpHeight = 1;
    // Hack : TODO:reconstruct on resize
    size_t mutexSize = 1920 * 1080 * sizeof(std::mutex);
    pixelLocks = (std::mutex *)malloc(mutexSize);
    window.focus();
    Renderer::drawNoise(renderState);
    window.processMessages();
    window.swapBuffers();
    init();
    window.showCursor(!sceneSettings.lockMouse);
    window.setCursorPos(windowPos.x + (renderState.width / 2.f), windowPos.y + (renderState.height / 2.f));
    timer.Stop();
    LOG_INFO("Initialization took " << timer.dtms << " ms\n");
    while (window.isOpen()) {
        // Update Loop
        update(window);
        window.processMessages();
        window.swapBuffers();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    window.close();
    free(pixelLocks);
    return 0;
}
