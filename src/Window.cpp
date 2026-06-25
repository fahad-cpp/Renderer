#include "Timer.h"
#include "Logging.h"
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
// #define NOMINMAX

#include "Window.h"
#include "Globals.h"
#include "Main.h"
#include "resource.h"
#include <FSWindow.h>

FS::Vector2 getMouseDiff(FS::Window &window) {
    if (!window.isFocused()) {
        return { 0.f, 0.f };
    }
    FS::RenderState &renderState = window.getRenderState();
    static FS::Vector2 prevPoint = { 0, 0 };
    FS::Vector2 mousePoint;
    int windowX = window.getWindowPos().x;
    int windowY = window.getWindowPos().y;

    mousePoint = window.getCursorPos();
    FS::Vector2 mousePrev = { float(prevPoint.x) - windowX, float(prevPoint.y) - windowY };
    if (sceneSettings.lockMouse) {
        prevPoint = { (windowX + (renderState.width * 0.5f)), (windowY + (renderState.height * 0.5f)) };
        window.setCursorPos((windowX + (renderState.width * 0.5f)), (windowY + (renderState.height * 0.5f)));
    } else {
        prevPoint = mousePoint;
    }
    FS::Vector2 mouseNow = { float(mousePoint.x) - windowX, float(mousePoint.y) - windowY };
    FS::Vector2 mouseDiff = mouseNow - mousePrev;
    
    return mouseDiff;
}
int main() {
    // Random seed
    srand(uint32_t(time(NULL)));
    Timer timer;
    FS::Window window("Renderer!", 720, 720);
    FS::RenderState &renderState = window.getRenderState();
    window.showCursor(false);
    canvas = { float(renderState.width), float(renderState.height) };
    float aspectratio = float(renderState.width) / float(renderState.height);
    vpWidth = aspectratio;
    vpHeight = 1;
    //Hack : TODO:reconstruct on resize
    size_t mutexSize = 1920 * 1080 * sizeof(std::mutex);
    pixelLocks = (std::mutex *)malloc(mutexSize);
    timer.Stop();
    LOG_INFO("Initialization took " << timer.dtms << " ms\n");
    init();
    try {
        while (running) {
            // Update Loop
            update(window);
            window.processMessages();
            window.swapBuffers();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        std::cin.get();
    }
    free(pixelLocks);
    return 0;
}
