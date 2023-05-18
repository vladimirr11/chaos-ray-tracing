#ifndef RENDERER_H
#define RENDERER_H

#include "PPMImage.h"
#include "Scene.h"
#include "ThreadPool.h"
#include "Timer.h"

/// @brief Dummy enum class to control if barycentric shading should be used (scenes 1 and 2)
/// or non barycentric (rest of the scenes)
enum class Shading { BARYCENTRIC, NONBARYCENTRIC, UNDEFINED };

/// @brief Stores global render settings. Only shading type can be changed at this point
struct RenderSettings {
    Shading shading = Shading::UNDEFINED;
    const unsigned numThreads = getHardwareThreads();
    const size_t numPixelsPerThread = PIXELS_PER_THREAD;
};

static Color3f raytrace(const Ray& ray, const Scene* scene, const Shading shading) {
    Intersection isectData;
    switch (shading) {
        case Shading::BARYCENTRIC:
            if (scene->intersect(ray, isectData)) {
                return Color3f(isectData.u, isectData.v, 0.f);
            }
            break;
        case Shading::NONBARYCENTRIC:
            if (scene->intersect(ray, isectData)) {
                return scene->getMaterials()[isectData.materialIdx].shade(ray, scene, isectData);
            }
            break;
        default:
            Assert(false && "Unsupported shading type received.");
    }

    return scene->getBackground();
}

class Renderer {
public:
    Renderer() = delete;

    Renderer(PPMImageI& _ppmImage, const Scene* _scene, const RenderSettings& _rSettings)
        : ppmImage(_ppmImage), scene(_scene), rSettings(_rSettings) {}

    void render(const size_t threadId, const size_t threadCount, const size_t chunkSize = 1) {
        const SceneDimensions& dimens = scene->getSceneDimensions();
        const Camera& camera = scene->getCamera();
        for (size_t i = (chunkSize * threadId); i < ppmImage.data.size();
             i += (chunkSize * threadCount)) {
            for (size_t c = 0; c < chunkSize; c++) {
                const int row = (i + c) / dimens.width;
                const int col = (i + c) % dimens.width;
                const Ray cameraRay = camera.getRay(row, col);
                const Color3f pixelColor = raytrace(cameraRay, scene, rSettings.shading);
                ppmImage.data[i + c].color = Color3i(clamp(0.f, 1.f, pixelColor.x) * 255,
                                                     clamp(0.f, 1.f, pixelColor.y) * 255,
                                                     clamp(0.f, 1.f, pixelColor.z) * 255);
            }
        }
    }

private:
    PPMImageI& ppmImage;
    const Scene* scene;
    const RenderSettings rSettings;
};

#endif  // !RENDERER_H
