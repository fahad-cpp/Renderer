#pragma once
#include <cstdint>
enum class DebugState {
    DS_OFF,
    DS_BOUNDING_BOX,
    DS_TRIANGLE,
};
enum class LightingMode : uint8_t {
    NO_LIGHT,
    LIGHT_ONLY,
    LIGHT_SHADOWS,
    MAX_ENUM
};
enum class RenderMode {
    RM_COLOR = 0,
    RM_DEPTH,
    RM_AO
};
struct SceneSettings {
    bool bfc;
    bool antiAliasing;
    int triSeenCount;
    DebugState debugState;
    bool lockMouse;
    RenderMode renderMode;
    bool rayTraceMode;
    LightingMode lightingMode;
};
