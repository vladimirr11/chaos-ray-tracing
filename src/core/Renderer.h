#ifndef RENDERER_H
#define RENDERER_H

#include "PPMImage.h"
#include "Scene.h"
#include "ThreadPool.h"
#include "Timer.h"

/// @brief Stores global render settings
struct RenderSettings {
    const unsigned numThreads = getHardwareThreads();
    const size_t numPixelsPerThread = PIXELS_PER_THREAD;
};

/// @brief Trace a ray in the scene
/// @return Computed color of the closest found primitive if _ray_ intersects with the _scene_.
/// If no intersection found returns the color of the background
static Color3f rayTrace(const Ray& ray, const Scene* scene) {
    Intersection isectData;
    if (scene->intersect(ray, isectData)) {
        return scene->getMaterials()[isectData.materialIdx].shade(ray, scene, isectData);
    }
    return scene->getBackground();
}

class Renderer {
public:
    Renderer() = delete;

    Renderer(PPMImageI& _ppmImage, const Scene* _scene) : ppmImage(_ppmImage), scene(_scene) {}

    void render(const size_t threadId, const size_t threadCount, const size_t chunkSize = 1) {
        const SceneDimensions& dimens = scene->getSceneDimensions();
        const Camera& camera = scene->getCamera();
        for (size_t i = (chunkSize * threadId); i < ppmImage.data.size();
             i += (chunkSize * threadCount)) {
            for (size_t c = 0; c < chunkSize; c++) {
                const int row = (i + c) / dimens.width;
                const int col = (i + c) % dimens.width;
                const Ray cameraRay = camera.getRay(row, col);
                const Color3f currPixelColor = rayTrace(cameraRay, scene);
                ppmImage.data[i + c].color = Color3i(clamp(0.f, 1.f, currPixelColor.x) * 255,
                                                     clamp(0.f, 1.f, currPixelColor.y) * 255,
                                                     clamp(0.f, 1.f, currPixelColor.z) * 255);
            }
        }
    }

private:
    PPMImageI& ppmImage;  ///< Output image
    const Scene* scene;   ///< Scene to render
};

#endif  // !RENDERER_H
