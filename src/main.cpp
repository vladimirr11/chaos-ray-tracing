#include "core/Renderer.h"
#include "core/Scene.h"
#include "core/Statistics.h"
#include "core/ThreadPool.h"
#include "core/Timer.h"

// #define RENDER_STATIC

static int32_t runRenderer(const std::string& inputFile, ThreadPool& pool, RenderSettings& settings,
                           const int32_t numFrames) {
    // parse scene parameters
    SceneParams sceneParams;
    if (parseSceneParams(inputFile, sceneParams) != EXIT_SUCCESS) {
        std::cerr << "Failed to parse " << inputFile << " file." << std::endl;
        return EXIT_FAILURE;
    }

    // initialize scene
    Scene scene(sceneParams);
    scene.createAccelTree();

    const SceneDimensions dimens = scene.getSceneDimensions();
    settings.numPixelsPerThread = scene.getSceneSettings().bucketSize;

    Camera& sceneCamera = scene.getCamera();

    const char* scenePrefix = {"scene"};
    int32_t moveCameraCloser = -1;
    const int32_t switchFrame = 43;
    for (int32_t i = 0; i < numFrames; i++) {
        const std::string fileName = scenePrefix + std::to_string(i) + ".ppm";
        std::ofstream ppmImageFile(fileName, std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            std::cout << "Input file " << inputFile << " not good.\n";
            return EXIT_FAILURE;
        }

        if (i % switchFrame == 0)
            ++moveCameraCloser;
        moveCameraCloser % 2 == 0
            ? sceneCamera.move(Vector3f{0.55f, 0.f, -0.5f})  // move camera closer
            : sceneCamera.move(Vector3f{0.55f, 0.f, 0.5f});  // move camera away

        sceneCamera.setLookAt(Vector3f{0.f, 0.f, 0.f});

        // initialize image and renderer
        PPMImageI ppmImage(dimens.width, dimens.height);
        Renderer renderer(ppmImage, &scene);
        {
            std::cout << "Loading " << fileName << " scene...\nStart generating data...\n";
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

            std::cout << fileName << " data generated in [" << std::fixed << std::setprecision(2)
                      << Timer::toMilliSec<float>(timer.getElapsedNanoSec()) << "ms] on "
                      << settings.numThreads << " threads\n";

            flushStatistics();
        }

        serializePPMImage(ppmImageFile, ppmImage);
        ppmImageFile.close();
    }

    return EXIT_SUCCESS;
}

int main() {
    const std::vector<std::string> inputFiles = {"scene1.crtscene"};

    RenderSettings renderSettings;

    ThreadPool pool(renderSettings.numThreads);
    pool.start();

    const int32_t numFrames = 180;

    for (const auto& file : inputFiles) {
        if (runRenderer(file, pool, renderSettings, numFrames) != EXIT_SUCCESS) {
            std::cerr << "Failed to render file - " << file << std::endl;
            return EXIT_FAILURE;
        }
    }

    pool.stop();

    return 0;
}
