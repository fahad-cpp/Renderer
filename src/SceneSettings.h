#pragma once
enum class DebugState {
	DS_OFF,
	DS_BOUNDING_BOX,
	DS_TRIANGLE
};
enum class RenderMode {
	RM_COLOR = 0,
	RM_DEPTH
};
struct SceneSettings {
	bool bfc;
	bool antiAliasing;
	int triSeenCount;
	DebugState debugState;
	bool lockMouse;
	RenderMode renderMode;
	bool rayTraceMode;
};