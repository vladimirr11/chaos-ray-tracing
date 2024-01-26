#include "core/Renderer.h"
#include "core/Scene.h"
#include "core/Statistics.h"
#include "core/ThreadPool.h"
#include "core/Timer.h"

static int32_t runRenderer(const std::string& inputFile, ThreadPool& pool,
                           RenderSettings& settings) {
    const std::string ppmFileName = getFileName(inputFile);

    SceneParams sceneParams;
    if (parseSceneParams(inputFile, sceneParams) != EXIT_SUCCESS) {
        std::cerr << "Failed to parse " << inputFile << " file." << std::endl;
        return EXIT_FAILURE;
    }

    // initialize scene
    Scene scene(sceneParams);

    // initialize image
    const SceneDimensions dimens = scene.getSceneDimensions();
    PPMImageI ppmImage(dimens.width, dimens.height);

    // initialize renderer
    Renderer renderer(ppmImage, &scene);
    settings.numPixelsPerThread = scene.getSceneSettings().bucketSize;

    // camera pos to take an image from
    const std::vector<Vector3f> cameraPosVec{Vector3f{0.f, 14.f, 26.f}, Vector3f{0.f, 6.f, 10.f},
                                             Vector3f{4.f, 6.f, 10.f},  Vector3f{0.f, 6.f, -10.f},
                                             Vector3f{4.f, 6.f, -10.f}, Vector3f{-14.f, 14.f, 0.f}};

    std::cout << "Loading " << ppmFileName << ".crtscene ...\n";
    scene.createAccelTree();
    for (int32_t i = 0; i < (int32_t)cameraPosVec.size(); i++) {
        // set camera position and target
        Camera& sceneCamera = scene.getCamera();
        sceneCamera.setLookFrom(cameraPosVec[i]);
        sceneCamera.setLookAt(Vector3f{0.f, 0.f, 0.f});

        std::cout << "Start generating data...\n";
        Timer timer;
        timer.start();

#ifdef RENDER_STATIC
        for (size_t threadId = 0; threadId < settings.numThreads; threadId++) {
            auto renderTask = std::bind(&Renderer::renderStatic, &renderer, threadId,
                                        settings.numThreads, settings.numPixelsPerThread);
            pool.scheduleTask(renderTask);
        }
#else
        using namespace std::placeholders;
        auto renderTask = std::bind(&Renderer::renderRegion, &renderer, _1, _2, _3, _4);
        pool.parallelLoop2D(renderTask, (size_t)dimens.width, (size_t)dimens.height,
                            settings.numPixelsPerThread, settings.numPixelsPerThread);
#endif
        pool.completeTasks();

        std::cout << ppmFileName << i << " data generated in [" << std::fixed
                  << std::setprecision(2) << Timer::toMilliSec<float>(timer.getElapsedNanoSec())
                  << "ms] on " << settings.numThreads << " threads\n";

        flushStatistics();
        serializePPMImage2PNG(ppmFileName + std::to_string(i) + ".jpg", ppmImage);
    }

    return EXIT_SUCCESS;
}

int main() {
    const std::vector<std::string> inputFiles{"scenes/scene.crtscene"};
    RenderSettings renderSettings;

    ThreadPool pool(renderSettings.numThreads);
    pool.start();

    for (const auto& file : inputFiles) {
        if (runRenderer(file, pool, renderSettings) != EXIT_SUCCESS) {
            std::cerr << "Failed to render file - " << file << std::endl;
            return EXIT_FAILURE;
        }
    }

    pool.stop();

    return 0;
}
