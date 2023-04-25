#ifndef DEFINES_H
#define DEFINES_H

#include <cassert>
#include <limits>

static constexpr float ASPECT_RATIO = 16.f / 9.f;
static constexpr int IMG_WIDTH = 1920;
static constexpr int IMG_HEIGHT = IMG_WIDTH / ASPECT_RATIO;
static constexpr int NUM_PIXELS = IMG_WIDTH * IMG_HEIGHT;
static constexpr int MAX_COLOR_COMP = 255;
static constexpr float EPSILON = std::numeric_limits<float>::epsilon();
static constexpr float PI = 3.14159265358979323846;
static constexpr float SHADOW_BIAS = 1e-6f;

#define Assert(x) assert(x)

struct SceneDimensions {
    int width;
    int height;
};

namespace SceneDefines {
    const char* sceneSettings = "settings";
    const char* imageSettings = "image_settings";
    const char* backgroundColor = "background_color";
    const char* imageWidth = "width";
    const char* imageHeight = "height";
    const char* sceneLights = "lights";
    const char* lightIntensity = "intensity";
    const char* lightPosition = "position";
    const char* cameraSettings = "camera";
    const char* cameraPos = "position";
    const char* cameraRotationM = "matrix";
    const char* sceneObjects = "objects";
    const char* vertices = "vertices";
    const char* triangleIndices = "triangles";
};  // namespace SceneDefines

#endif  // !DEFINES_H
