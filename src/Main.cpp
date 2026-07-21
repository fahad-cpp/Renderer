#include "Main.h"
#include "Colour.h"
#include "FSWindow.h"
#include "Globals.h"
#include "Logging.h"
#include "Object.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "SceneSettings.h"
#include "Timer.h"
#include "Vector2.h"
#include "Window.h"
#include <Input.h>
#include <chrono>
#include <thread>

// TODO(Fahad):
/*
 *	Software Structure:
 *		-Seperate Engine and Game
 *	Adding Features:
 *		-Add matrix transformation for renderer
 *		-render according to material(Rasterizer)
 *		-Add some ui
 *		-Frametime graph
 *		-read and display textures(Rasterizer)
 *	Optimizations:
 *      -Implement SIMD for Vector operations
 *		-Implement Occlusion culling(Rasterizer)
 *		-Implement BVH ray tracing(Ray tracer)
 */
// frame delta time
double fdt = 0.06;
float totalFrameTime = 0.f;
int frameCount = 0;
const float defSpeed = 2.f;
float speed = defSpeed;
const float boostSpeed = (5 * defSpeed);
static FS::Vector2 getMouseDiff(FS::Window &window) {
    if (!window.isFocused() || !sceneSettings.lockMouse) {
        return { 0.f, 0.f };
    }
    FS::RenderState renderState = window.getRenderState();
    FS::Vector2 windowPos = window.getWindowPos();
    FS::Vector2 mousePos = window.getCursorPos();
    static FS::Vector2 prevPoint = mousePos;
    prevPoint = { windowPos.x + (renderState.width * 0.5f), windowPos.y + (renderState.height * 0.5f) };
    window.setCursorPos(prevPoint.x, prevPoint.y);
    FS::Vector2 diff = mousePos - prevPoint;
    return diff;
}
void handleInput(FS::Window &window) {
    const FS::Input &input = window.getInput();
    FS::RenderState &renderState = window.getRenderState();
    const float sensitivity = 1.2f;
    FS::Vector2 mouseDiff = sceneSettings.lockMouse ? getMouseDiff(window) : 0;
    mouseDiff = mouseDiff * sensitivity;
    Vector velocity = { 0.f, 0.f, 0.f };
    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
    // Movement events
    if (isDown(FS::Buttons::BUTTON_W)) {
        velocity = velocity + Vector{ 0, 0, 1.f };
    }
    if (isDown(FS::Buttons::BUTTON_A)) {
        velocity = velocity + Vector{ -1.f, 0, 0 };
    }
    if (isDown(FS::Buttons::BUTTON_S)) {
        velocity = velocity + Vector{ 0, 0, -1.f };
    }
    if (isDown(FS::Buttons::BUTTON_D)) {
        velocity = velocity + Vector{ 1.f, 0, 0 };
    }
    if (isDown(FS::Buttons::BUTTON_CTRL)) {
        velocity = velocity + Vector{ 0, -1.f, 0 };
    }
    if (isDown(FS::Buttons::BUTTON_SPACE)) {
        velocity = velocity + Vector{ 0, 1.f, 0 };
    }

    if (isDown(FS::Buttons::BUTTON_SHIFT)) {
        speed += 10 * fdt;
        if (speed > boostSpeed) {
            speed = boostSpeed;
        }
    } else {
        speed -= 10 * fdt;
        if (speed < defSpeed) {
            speed = defSpeed;
        }
        speed = defSpeed;
    }

    // Normalize move vector and move the Camera
    if (!(velocity == Vector{ 0, 0, 0 })) {
        change = true;
        velocity = velocity / length(velocity);
        velocity = velocity * speed;
        velocity = rotate(velocity, { 0, camera.rotation.y, 0 });
        camera.position = camera.position + (velocity * fdt);
    }

    if (isDown(FS::Buttons::BUTTON_LEFT)) {
        mouseDiff.x -= 100.f * fdt * sensitivity;
    }
    if (isDown(FS::Buttons::BUTTON_RIGHT)) {
        mouseDiff.x += 100.f * fdt * sensitivity;
    }
    if (isDown(FS::Buttons::BUTTON_UP)) {
        mouseDiff.y -= 100.f * fdt * sensitivity;
    }
    if (isDown(FS::Buttons::BUTTON_DOWN)) {
        mouseDiff.y += 100.f * fdt * sensitivity;
    }
    if (pressed(FS::Buttons::BUTTON_L)) {
        LOG_INFO("STATS:\n");
        LOG_INFO("Position : " << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n");
        LOG_INFO("Rotation : " << camera.rotation.x << " " << camera.rotation.y << " " << camera.rotation.z << "\n");
        int c = 1;
        LOG_INFO("Model count " << scene.instances.size() << "\n");
        LOG_INFO("---\n");
        int totalTris = 0;
        for (const Instance &instance : scene.instances) {
            LOG_INFO("Model " << c << ":\n");
            // LOG_INFO("Triangle count : " << instance.mesh->triangles.size() << "\n");
            LOG_INFO("Face count : " << instance.mesh->faces.size() << "\n");
            LOG_INFO("Normal count : " << instance.mesh->normals.size() << "\n");
            // totalTris += instance.mesh->triangles.size();
            c++;
        }
        LOG_INFO("---\n");
        LOG_INFO("Total triangle count :" << totalTris << '\n');
        LOG_INFO(("Triangle seen :" + std::to_string(sceneSettings.triSeenCount) + "\n"));
    }

    // Show triangles of the mesh
    if (pressed(FS::Buttons::BUTTON_T)) {
        if (sceneSettings.debugState != DebugState::DS_WIREFRAME)
            LOG_INFO("Debug state set to wireframe triangle\n");
        sceneSettings.debugState = DebugState::DS_WIREFRAME;
        change = true;
    }
    // Show bounding box of the mesh
    if (pressed(FS::Buttons::BUTTON_B)) {
        if (sceneSettings.debugState != DebugState::DS_BOUNDING_BOX)
            LOG_INFO("Debug state set to bounding box\n");
        sceneSettings.debugState = DebugState::DS_BOUNDING_BOX;
        change = true;
    }
    // Turn off Debug view
    if (pressed(FS::Buttons::BUTTON_V)) {
        if (sceneSettings.debugState != DebugState::DS_OFF)
            LOG_INFO("Visual debugging off\n");
        sceneSettings.debugState = DebugState::DS_OFF;
        change = true;
    }
    // Change ray tracing to rasterization and vise versa
    if (pressed(FS::Buttons::BUTTON_R)) {
        sceneSettings.rayTraceMode = !sceneSettings.rayTraceMode;
        if (sceneSettings.rayTraceMode) {
            LOG_INFO("Ray tracing turned on\n");
        } else {
            LOG_INFO("Ray tracing turned off\n");
        }
        change = true;
    }
    // Exporting an image
    if (pressed(FS::Buttons::BUTTON_P)) {
        Renderer::printPPM("Image.ppm", renderState);
    }
    // Backface culling toggle
    if (pressed(FS::Buttons::BUTTON_C)) {
        sceneSettings.bfc = !sceneSettings.bfc;
        if (sceneSettings.bfc) {
            LOG_INFO("Backface culling turned on\n");
        } else
            LOG_INFO("Backface culling turned off\n");
        change = true;
    }

    // Reset camera Position and rotation
    if (pressed(FS::Buttons::BUTTON_Q)) {
        camera.rotation = { 0, 0, 0 };
        camera.position = { 0, 0, 0 };
        change = true;
    }

    // Slow down time
    fdt = 0.06;
    if (isDown(FS::Buttons::MOUSE_BUTTON_LEFT)) {
        fdt = 0.001;
    }
    if (isDown(FS::Buttons::MOUSE_BUTTON_RIGHT)) {
        Transform tf = { { 0, 0, 0 }, 1, { 0, float(100 * fdt), 0 } };
        if (scene.instances.size()) {
            scene.instances[0].applyTransform(tf);
        }
        change = true;
    }

    // Toggle Anti aliasing
    if (pressed(FS::Buttons::BUTTON_F)) {
        sceneSettings.antiAliasing = !sceneSettings.antiAliasing;
        if (sceneSettings.antiAliasing) {
            LOG_INFO("Anti aliasing turned on.\n");
        } else {
            LOG_INFO("Anti aliasing turned off.\n");
        }
        change = true;
    }
    // Lock / Unlock mouse
    if (pressed(FS::Buttons::BUTTON_G)) {
        window.showCursor(sceneSettings.lockMouse);
        sceneSettings.lockMouse = !sceneSettings.lockMouse;
    }
    if (pressed(FS::Buttons::BUTTON_M)) {
        sceneSettings.renderMode = ((sceneSettings.renderMode == RenderMode::RM_DEPTH) ? RenderMode::RM_COLOR : RenderMode::RM_DEPTH);
    }
    // Move according to mouse difference
    if (mouseDiff != FS::Vector2{ 0, 0 }) {
        camera.rotation.y -= mouseDiff.x * fdt;
        camera.rotation.x += mouseDiff.y * fdt;
        change = true;
    }

    // Change lighting modes
    if (pressed(FS::Buttons::BUTTON_X)) {
        LightingMode mode = sceneSettings.lightingMode;
        uint8_t modenumber = static_cast<uint8_t>(mode);
        modenumber++;
        if (modenumber >= static_cast<uint8_t>(LightingMode::MAX_ENUM)) {
            modenumber = 0;
        }
        mode = static_cast<LightingMode>(modenumber);
        sceneSettings.lightingMode = mode;
    }

    // Ambient Occlusion
    if (pressed(FS::Buttons::BUTTON_O)) {
        if (sceneSettings.renderMode == RenderMode::RM_AO) {
            sceneSettings.renderMode = RenderMode::RM_COLOR;
        } else {
            sceneSettings.renderMode = RenderMode::RM_AO;
        }
    }

    // Normal vis
    if (pressed(FS::Buttons::BUTTON_N)) {
        sceneSettings.debugState = DebugState::DS_NORMAL;
    }
}
void init() {
    // ZoneScopedN("init");
    const float shininess = 64.f;
    static Mesh model = loadOBJ("res/Models/sponza.obj", { shininess, 0.f, { 255, 255, 255 } });
    // static Mesh floor = loadOBJ("res/Models/surface.obj", { 233,234,231 }, 0.f, shininess);
    Vector p[3] = {
        { -1.f, 0.f, 1.f },
        { 0.f, 2.f, 1.f },
        { 1.f, 0.f, 1.f },
    };
    Vector n[3] = {
        { 0, 0, -1 },
        { 0, 0, -1 },
        { 0, 0, -1 },
    };
    Triangle tri{ { p[0], p[1], p[2] }, { n[0], n[1], n[2] } };
    scene = {
        .spheres = std::vector<Sphere>{
            {
                .center = Vector{ 0, 0, -3 },
                .radius = 1.f,
                .specular = shininess,
                .reflectiveness = 0.4f,
                .color = Colour{ 255, 0, 0 },
            },
            {
                .center = Vector{ -1, 0, -4 },
                .radius = 1.f,
                .specular = shininess,
                .reflectiveness = 0.4f,
                .color = Colour{ 0, 255, 0 },
            },
            {
                .center = Vector{ 1, 0, -4 },
                .radius = 1.f,
                .specular = shininess,
                .reflectiveness = 0.4f,
                .color = Colour{ 0, 0, 255 },
            },
            {
                .center = Vector{ 0, 1, 0 },
                .radius = .1f,
                .specular = 100.f,
                .reflectiveness = 0.4f,
                .color = Colour{ 255, 255, 255 },
            } },
        .triangles = std::vector<Triangle>{
            tri
            // empty
        },
        .instances = std::vector<Instance>{
            { .mesh = &model, .transform = { .position = { 0, 0, 0 }, .scale = .1f, .rotation = { 0, 0, 0 } } },
            //{ .mesh = &floor, .transform = { .position = { 0, -1.f, 0 }, .scale = 1.f, .rotation = { 0, 0, 0 } } },
        },
        .lights = std::vector<Light>{
            { .type = LT_AMBIENT, .pos = { 0, 0, 0 }, .direction = { 0, 0, 0 }, .intensity = 0.2f },
            { .type = LT_POINT, .pos = { 0, 1, 0 }, .direction = { 1, 2, 0 }, .intensity = 0.4f },
            { .type = LT_POINT, .pos = { -60, 1, 0 }, .direction = { 1, 2, 0 }, .intensity = 0.4f },
            { .type = LT_POINT, .pos = { 60, 1, 0 }, .direction = { 1, 2, 0 }, .intensity = 0.4f },
            { .type = LT_DIRECTIONAL, .pos = { 0, 0, 0 }, .direction = { 1, -4, 4 }, .intensity = 0.5f },
        }
    };
    for (Instance &ins : scene.instances) {
        ins.getBoundingBox();
    }
}
void update(FS::Window &window) {
    // Start counting frame time
    Timer timer;
    handleInput(window);
    FS::RenderState &renderState = window.getRenderState();
    if (sceneSettings.rayTraceMode && change) {
        Renderer::clearScreen(0x000000, renderState);
        // Ray tracing multithreaded
        static size_t threadCount = std::thread::hardware_concurrency();
        static std::vector<std::thread> rtThreads(threadCount);
        for (size_t i = 0; i < threadCount; i++) {
            rtThreads[i] = std::thread(Renderer::rayTraceThr, i, threadCount, std::ref(renderState));
        }
        for (size_t i = 0; i < rtThreads.size(); i++) {
            rtThreads[i].join();
        }
        if (sceneSettings.antiAliasing && (sceneSettings.debugState != DebugState::DS_WIREFRAME)) {
            PostProcess::FXAA(renderState);
        }
        timer.Stop();
        // LOG_INFO("RayTracing this frame took : "+std::to_string(timer.dtms)+"ms\n");
        change = false;
    } else if (change) {
        // Rasterizer
        Renderer::renderScene(renderState);
        if ((sceneSettings.renderMode == RenderMode::RM_AO) && (sceneSettings.debugState == DebugState::DS_OFF)) {
            PostProcess::renderAO(renderState);
        } else if (sceneSettings.renderMode == RenderMode::RM_DEPTH) {
            Renderer::renderDepthBuffer(renderState);
        }
        timer.Stop();
    }
    // Limit frame rate to 144
    if (timer.dtms < frameLimit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(int(frameLimit - timer.dtms)));
        timer.dtms += (frameLimit - timer.dtms);
    }
    fdt = timer.dtms / 90;
    // FPS count
    totalFrameTime += (timer.dtms * 0.001);
    frameCount++;
    printLive("CUR-FPS: " + std::to_string(1.f / (timer.dtms * 0.001)) + " AVG-FPS : " + std::to_string(1 / (totalFrameTime / frameCount)) + " CUR-FRAME: " + std::to_string(timer.dtms) + "ms" + " AVG-FRAME: " + std::to_string((totalFrameTime * 1000) / frameCount) + "ms");
    timer.dtms = 0;
}
