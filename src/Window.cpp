#include <atomic>
#include <functional>
#include <thread>
#define _CRT_SECURE_NO_WARNINGS
#include "Globals.h"
#include "Logging.h"
#include "Main.h"
#include "Renderer.h"
#include "Timer.h"
#include <FSWindow.h>
void loadingScreen(FS::Window& window,std::atomic_bool& signal){
    FS::RenderState& renderState = window.getRenderState();
    while(!signal){
        Renderer::drawNoise(renderState);
        window.swapBuffers();
    }
}
int main() {
    // Random seed
    srand(uint32_t(time(NULL)));
    Timer timer;
    FS::Window window("Renderer!", 720, 720);
    window.focus();
    std::atomic_bool initialized = false;
    std::thread loadingThread(loadingScreen,std::ref(window),std::ref(initialized));
    FS::RenderState &renderState = window.getRenderState();
    FS::Vector2 windowPos = window.getWindowPos();
    
    // Hack : TODO:reconstruct on resize
    size_t mutexSize = 1920 * 1080 * sizeof(std::mutex);
    pixelLocks = (std::mutex *)malloc(mutexSize);
    Renderer::drawNoise(renderState);
    init();
    window.showCursor(!sceneSettings.lockMouse);
    window.setCursorPos(uint32_t(windowPos.x + (renderState.width / 2.f)), uint32_t(windowPos.y + (renderState.height / 2.f)));
    timer.Stop();
    initialized = true;
    loadingThread.join();
    LOG_INFO("Initialization took " << timer.dtms << " ms\n");
    while (window.isOpen()) {
        canvas = { float(renderState.width), float(renderState.height) };
        float aspectratio = float(renderState.width) / float(renderState.height);
        vpWidth = aspectratio;
        vpHeight = 1;
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
