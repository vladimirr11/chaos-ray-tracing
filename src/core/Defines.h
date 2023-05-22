#ifndef DEFINES_H
#define DEFINES_H

#include <cassert>
#include <limits>

#define Assert(x) assert(x)
static constexpr float ASPECT_RATIO = 16.f / 9.f;
static constexpr int IMG_WIDTH = 1920;
static constexpr int IMG_HEIGHT = IMG_WIDTH / ASPECT_RATIO;
static constexpr int NUM_PIXELS = IMG_WIDTH * IMG_HEIGHT;
static constexpr int MAX_COLOR_COMP = 255;
static constexpr float EPSILON = std::numeric_limits<float>::epsilon() * 0.5f;
static constexpr float PI = 3.14159265358979323846;
static constexpr float SHADOW_BIAS = 1e-2f;
static constexpr int MAX_RAY_DEPTH = 5;
static constexpr size_t PIXELS_PER_THREAD = 16;
static constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
static constexpr float MIN_FLOAT = std::numeric_limits<float>::min();

namespace SceneDefines {
    inline const char* sceneSettings = "settings";
    inline const char* imageSettings = "image_settings";
    inline const char* backgroundColor = "background_color";
    inline const char* imageWidth = "width";
    inline const char* imageHeight = "height";
    inline const char* sceneLights = "lights";
    inline const char* lightIntensity = "intensity";
    inline const char* lightPosition = "position";
    inline const char* materialsInfo = "materials";
    inline const char* materialType = "type";
    inline const char* materialAlbedo = "albedo";
    inline const char* materialSmootSh = "smooth_shading";
    inline const char* cameraSettings = "camera";
    inline const char* cameraPos = "position";
    inline const char* cameraRotationM = "matrix";
    inline const char* sceneObjects = "objects";
    inline const char* materialIdx = "material_index";
    inline const char* vertices = "vertices";
    inline const char* triangleIndices = "triangles";
};  // namespace SceneDefines

#endif  // !DEFINES_H
