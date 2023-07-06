#ifndef RENDERER_H
#define RENDERER_H

#include "PPMImage.h"
#include "Utils.h"

struct Scene;
struct Ray;

/// @brief Stores global render settings
struct RenderSettings {
    const unsigned numThreads = getHardwareThreads();
    size_t numPixelsPerThread = DEFAULT_BUCKET_SIZE;
};

/// @brief Trace a ray in the scene
/// @return Computed color of the closest found primitive if _ray_ intersects with the _scene_.
/// If no intersection found returns the color of the background
Color3f rayTrace(const Ray& ray, const Scene* scene);

class Renderer {
public:
    Renderer() = delete;

    Renderer(PPMImageI& _ppmImage, Scene* _scene) : ppmImage(_ppmImage), scene(_scene) {}

    /// @brief Statically divides the scene into segments that are on [_chunkSize_ * _threadCount_]
    /// distance away for each thread
    void renderStatic(const size_t threadId, const size_t threadCount, const size_t chunkSize = 1);

    /// @brief Traces rays in precomputed 2D region of the scene
    void renderRegion(const int32_t startCol, const int32_t endCol, const int32_t startRow,
                      const int32_t endRow);

private:
    PPMImageI& ppmImage;  ///< Output image
    Scene* scene;         ///< Scene to render
};

#endif  // !RENDERER_H
