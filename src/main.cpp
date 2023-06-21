#include "core/Renderer.h"
#include "core/Statistics.h"

// #define staticRender

static int32_t runRenderer(const std::string& inputFile, ThreadPool& pool,
                           RenderSettings& settings) {
    const std::string ppmFileName = getPpmFileName(inputFile);
    std::ofstream ppmImageFile(ppmFileName, std::ios::out | std::ios::binary);
    if (!ppmImageFile.good()) {
        std::cout << "Input file " << inputFile << " not good.\n";
        return EXIT_FAILURE;
    }

    SceneParams sceneParams;
    if (parseSceneParams(inputFile, sceneParams) != EXIT_SUCCESS) {
        std::cerr << "Failed to parse " << inputFile << " file." << std::endl;
        return EXIT_FAILURE;
    }

    // initialize scene
    const Scene scene(sceneParams);

    // initialize image
    const SceneDimensions dimens = scene.getSceneDimensions();
    PPMImageI ppmImage(dimens.width, dimens.height);

    // initialize renderer
    Renderer renderer(ppmImage, &scene);

    settings.numPixelsPerThread = scene.getSceneSettings().bucketSize;

    std::cout << "Loading " << ppmFileName << " scene...\nStart generating data...\n";
    {
        Timer timer;
        timer.start();

#ifdef staticRender
        for (size_t threadId = 0; threadId < settings.numThreads; threadId++) {
            auto renderTask = std::bind(&Renderer::renderStatic, &renderer, threadId,
                                        settings.numThreads, settings.numPixelsPerThread);
            pool.scheduleTask(renderTask);
        }
#else
        using namespace std::placeholders;
        auto renderTask = std::bind(&Renderer::renderRegion, &renderer, _1, _2, _3, _4);
        pool.parallelLoop2D(renderTask, (size_t)dimens.width, (size_t)dimens.height,
                            settings.numPixelsPerThread);
#endif
        pool.completeTasks();

        std::cout << ppmFileName << " data generated in [" << std::fixed << std::setprecision(2)
                  << Timer::toMilliSec<float>(timer.getElapsedNanoSec()) << "ms] on "
                  << settings.numThreads << " threads\n";

        flushStatistics();
    }

    serializePPMImage(ppmImageFile, ppmImage);
    ppmImageFile.close();

    return EXIT_SUCCESS;
}

int main() {
    const std::vector<std::string> inputFiles = {"scenes/scene10.crtscene"};

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
