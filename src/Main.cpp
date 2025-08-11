#include "Main.h"
#include "Renderer.h"

//TODO(Fahad):
/*
*	Optimizations:
*		-Implement BVH ray tracing
*		-Implement proper Occlusion culling
*	Adding Features:
*		-Add matrix transformation for renderer
*		-interpolated normals(Rasterizer)
*		-read and display textures(Rasterizer)
*		-render according to material(Rasterizer)
*		-Frametime graph
*		-Add some ui
*/
//frame delta time
double fdt = 0.06;
float totalFrameTime = 0.f;
int frameCount = 0;
const float defSpeed = 2.f;
float speed = defSpeed;
const float boostSpeed = (5 * defSpeed);
void handleInput(const Input& input) {
	const float sensitivity = 1.2f;
	Vector mouseDiff = sceneSettings.lockMouse ? getMouseDiff() : 0;
	mouseDiff = mouseDiff * sensitivity;
	Vector velocity = { 0.f,0.f,0.f };
	if (isDown(BUTTON_ESC)) {
		running = false;
	}
	//Movement events
	if (isDown(BUTTON_W))
		velocity = velocity + Vector{ 0,0,1.f };
	if (isDown(BUTTON_A))
		velocity = velocity + Vector{ -1.f,0,0 };

	if (isDown(BUTTON_S))
		velocity = velocity + Vector{ 0,0,-1.f };
	if (isDown(BUTTON_D)) {
		velocity = velocity + Vector{ 1.f,0,0 };
	}
	if (isDown(BUTTON_CTRL))
		velocity = velocity + Vector{ 0,-1.f,0 };
	if (isDown(BUTTON_SPACE)) {
		velocity = velocity + Vector{ 0,1.f,0 };
	}

	if (isDown(BUTTON_SHIFT)) {
		speed += 10 * fdt;
		if (speed > boostSpeed) {
			speed = boostSpeed;
		}
	}
	else {
		speed -= 10 * fdt;
		if (speed < defSpeed) {
			speed = defSpeed;
		}
		speed = defSpeed;
	}

	//Normalize move vector and move the Camera
	if (!(velocity == Vector{ 0,0,0 })) {
		change = true;
		velocity = velocity / length(velocity);
		velocity = velocity * speed;
		velocity = rotate(velocity, { 0,camera.rotation.y,0 });
		camera.position = camera.position + (velocity * fdt);
	}
	if (pressed(BUTTON_L)) {
		LOG_INFO("STATS:\n");
		LOG_INFO("Position : " << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n");
		LOG_INFO("Rotation : " << camera.rotation.x << " " << camera.rotation.y << " " << camera.rotation.z << "\n");
		int c = 1;
		LOG_INFO("Model count " << scene.instances.size() << "\n");
		LOG_INFO("---\n");
		int totalTris = 0;
		for (const Instance& instance : scene.instances)
		{
			LOG_INFO("Model " << c << ":\n");
			LOG_INFO("Triangle count : " << instance.mesh->triangles.size() << "\n");
			LOG_INFO("Face count : " << instance.mesh->faces.size() << "\n");
			LOG_INFO("Normal count : " << instance.mesh->normals.size() << "\n");
			totalTris += instance.mesh->triangles.size();
			c++;
		}
		LOG_INFO("---\n");
		LOG_INFO("Total triangle count :" << totalTris << '\n');
		LOG_INFO(("Triangle seen :" + std::to_string(sceneSettings.triSeenCount) + "\n"));
	}

	//Show triangles of the mesh
	if (pressed(BUTTON_T)) {
		if (sceneSettings.debugState != DebugState::DS_TRIANGLE)LOG_INFO("Debug state set to wireframe triangle\n");
		sceneSettings.debugState = DebugState::DS_TRIANGLE;
		change = true;
	}
	//Show bounding box of the mesh
	if (pressed(BUTTON_B)) {
		if (sceneSettings.debugState != DebugState::DS_BOUNDING_BOX)LOG_INFO("Debug state set to bounding box\n");
		sceneSettings.debugState = DebugState::DS_BOUNDING_BOX;
		change = true;
	}
	//Turn off Debug view
	if (pressed(BUTTON_V)) {
		if (sceneSettings.debugState != DebugState::DS_OFF)LOG_INFO("Visual debugging off\n");
		sceneSettings.debugState = DebugState::DS_OFF;
		change = true;
	}
	//Change ray tracing to rasterization and vise versa
	if (pressed(BUTTON_R)) {
		rayTraceMode = !rayTraceMode;
		if (rayTraceMode) {
			LOG_INFO("Ray tracing turned on\n");
		}
		else {
			LOG_INFO("Ray tracing turned off\n");
		}
		change = true;
	}
	//Exporting an image
	if (pressed(BUTTON_P)) {
		Renderer::printPPM("Image.ppm");
	}
	//Backface culling toggle
	if (pressed(BUTTON_C)) {
		sceneSettings.bfc = !sceneSettings.bfc;
		if (sceneSettings.bfc) {
			LOG_INFO("Backface culling turned on\n");
		}
		else
			LOG_INFO("Backface culling turned off\n");
		change = true;
	}
	//Reset camera Position and rotation
	if (pressed(BUTTON_Q)) {
		camera.rotation = { 0,0,0 };
		camera.position = { 0,0,0 };
		change = true;
	}
	//Slow down time
	fdt = 0.06;
	if (isDown(MOUSE_BUTTON_LEFT)) {
		fdt = 0.001;
	}
	if (isDown(MOUSE_BUTTON_RIGHT)) {
		Transform tf = { {0,0,0},1,{0,float(100 * fdt),0} };
		if(scene.instances.size())
			scene.instances[0].applyTransform(tf);
		change = true;
	}
	if (pressed(BUTTON_F)) {
		//Toggle Anti aliasing
		sceneSettings.antiAliasing = !sceneSettings.antiAliasing;
		if (sceneSettings.antiAliasing) {
			LOG_INFO("Anti aliasing turned on.\n");
		}
		else {
			LOG_INFO("Anti aliasing turned off.\n");
		}
		change = true;
	}
	if (pressed(BUTTON_G)) {
		ShowCursor(sceneSettings.lockMouse);
		sceneSettings.lockMouse = !sceneSettings.lockMouse;
	}
	if (pressed(BUTTON_M)) {
		sceneSettings.renderMode = ((sceneSettings.renderMode == RenderMode::RM_DEPTH) ? RenderMode::RM_COLOR : RenderMode::RM_DEPTH);
	}
	//Move according to mouse difference
	if (mouseDiff != Vector{ 0,0,0 }) {
		camera.rotation.y -= mouseDiff.x * fdt;
		camera.rotation.x += mouseDiff.y * fdt;
		change = true;
	}
}
void init() {
	static Mesh model = Renderer::loadOBJ("res/Models/countrySide.obj", { 255,255,255 }, 0.f, 100.f);
	static Mesh floor = Renderer::loadOBJ("res/Models/surface.obj", { 255,255,255 }, 0.f, 100.f);
	scene = {
		.spheres = std::vector<Sphere>{
			{
				.center = Vector{0,0,-3},
				.radius = 1.f,
				.color = Colour{255,0,0},
				.specular = 100.f,
				.reflectiveness = 0.4f,
			},
			{
				.center = Vector{-1,0,-4},
				.radius = 1.f,
				.color = Colour{0,255,0},
				.specular = 100.f,
				.reflectiveness = 0.4f,
			},
			{
				.center = Vector{1,0,-4},
				.radius = 1.f,
				.color = Colour{0,0,255},
				.specular = 100.f,
				.reflectiveness = 0.4f,
			},
			/*{
				.center = Vector{2,1,0},
				.radius = .1f,
				.color = Colour{0,0,255},
				.specular = 100.f,
				.reflectiveness = 0.4f,
			}*/
		},
		.triangles = std::vector<Triangle>{
			//empty
		},
		.instances = std::vector<Instance>{
			{
				.mesh = &model,
				.transform = {
					.position = {0,-10,0},
					.scale = 10.f,
					.rotation = {0,0,0}
				}

			},
			{
				.mesh = &floor,
				.transform = {
					.position = {0,-10.f,0},
					.scale = 1.f,
					.rotation = {0,0,0}
				}
			}
		},
		.lights = std::vector<Light>{
			{
				.type = LT_AMBIENT,
				.pos = {0,0,0},
				.direction = {0,0,0},
				.intensity = 0.2f
			},
			{
				.type = LT_POINT,
				.pos = {2,1,0},
				.direction = {0,0,0},
				.intensity = 0.6f
			},
			{
				.type = LT_DIRECTIONAL,
				.pos = {0,0,0},
				.direction = {1,-4,4},
				.intensity = 0.2f
			},
		}
	};
}
void update(const Input& input) {
	//Start counting frame time
	Timer timer;
	handleInput(input);
	if (rayTraceMode && change) {
		Renderer::clearScreen(0x000000);
		//Ray tracing multithreaded
		static size_t threadCount = std::thread::hardware_concurrency();
		static std::vector<std::thread> rtThreads(threadCount);
		for (size_t i = 0; i < threadCount; i++)
		{
			rtThreads[i] = std::thread(Renderer::rayTraceThr, i, threadCount);
		}
		for (size_t i = 0; i < rtThreads.size(); i++) {
			rtThreads[i].join();
		}
		if (sceneSettings.antiAliasing && (sceneSettings.debugState != DebugState::DS_TRIANGLE)) {
			Renderer::FXAA();
		}
		timer.Stop();
		//LOG_INFO("RayTracing this frame took : "+std::to_string(timer.dtms)+"ms\n");
		change = false;
	}
	else if (change) {
		//Rasterize
		Renderer::renderScene();
		Renderer::renderAO();
		timer.Stop();
	}
	//Limit frame rate to 144
	if (timer.dtms < frameLimit) {
		Sleep(frameLimit - timer.dtms);
		timer.dtms += (frameLimit - timer.dtms);
	}
	fdt = timer.dtms / 90;
	//FPS count
	totalFrameTime += (timer.dtms * 0.001);
	frameCount++;
	printLive("CUR : " + std::to_string(1.f / (timer.dtms * 0.001)) + " AVG : " + std::to_string(1 / (totalFrameTime / frameCount)) + " FRAMETIME: " + std::to_string(timer.dtms) + "ms");
	timer.dtms = 0;
}