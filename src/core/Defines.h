#ifndef DEFINES_H
#define DEFINES_H

#include <cassert>
#include <limits>

#define Assert(x) assert(x)

static const float ASPECT_RATIO = 16.f / 9.f;
static const int IMG_WIDTH = 1920;
static const int IMG_HEIGHT = IMG_WIDTH / ASPECT_RATIO;
static const int NUM_PIXELS = IMG_WIDTH * IMG_HEIGHT;
static const int MAX_COLOR_COMP = 255;
static const float EPSILON = std::numeric_limits<float>::epsilon();
static const float PI = 3.14159265358979323846;
static const float SHADOW_BIAS = 1e-2f;
static const int MAX_RAY_DEPTH = 5;
static const size_t PIXELS_PER_THREAD = 16;

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
