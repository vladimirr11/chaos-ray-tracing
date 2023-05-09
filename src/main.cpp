#include "core/Scene.h"
#include "core/TaskManager.h"
#include "core/Timer.h"

/// @brief Dummy enum class to control if barycentric shading should be used (scenes 1 and 2)
/// or non barycentric (rest of the scenes)
enum class Shading { BARYCENTRIC, NONBARYCENTRIC };

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
                return scene->getMaterials()[isectData.materialIdx]->shade(ray, scene, isectData);
            }
            break;
        default:
            std::cerr << "Unsupported shading received\n";
            exit(EXIT_FAILURE);
    }

    return scene->getBackground();
}

class Renderer : public ParallelTask {
public:
    Renderer() = delete;

    Renderer(PPMImageI& _ppmImage, const Scene* _scene, const Shading _shading)
        : ppmImage(_ppmImage), scene(_scene), shading(_shading) {}

    void run(const size_t threadId, const size_t threadCount, const size_t chunkSize = 1) override {
        const SceneDimensions& dimens = scene->getSceneDimensions();
        const Camera& camera = scene->getCamera();
        for (size_t i = (chunkSize * threadId); i < ppmImage.data.size();
             i += (chunkSize * threadCount)) {
            for (size_t c = 0; c < chunkSize; c++) {
                const int row = (i + c) / dimens.width;
                const int col = (i + c) % dimens.width;
                const Ray cameraRay = camera.getRay(row, col);
                const Color3f pixelColor = raytrace(cameraRay, scene, shading);
                ppmImage.data[i + c].color = Color3i(clamp(0.f, 1.f, pixelColor.x) * 255,
                                                     clamp(0.f, 1.f, pixelColor.y) * 255,
                                                     clamp(0.f, 1.f, pixelColor.z) * 255);
            }
        }
    }

private:
    PPMImageI& ppmImage;
    const Scene* scene;
    const Shading shading;
};

int main() {
    const char* inputScenes[]{"scenes/scene0.crtscene", "scenes/scene1.crtscene",
                              "scenes/scene2.crtscene", "scenes/scene3.crtscene",
                              "scenes/scene4.crtscene", "scenes/scene5.crtscene"};

    const char* sceneNames[]{"scene0.ppm", "scene1.ppm", "scene2.ppm",
                             "scene3.ppm", "scene4.ppm", "scene5.ppm"};

    const size_t numScenes = std::size(inputScenes);
    const unsigned numThreads = std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1);

    TaskManager taskManager(numThreads);
    taskManager.start();

    std::cout << "Number of scenes to render [" << numScenes << "]\n";

    for (size_t i = 0; i < numScenes; i++) {
        std::ofstream ppmImageFile(sceneNames[i], std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            std::cout << "Input file " << sceneNames[i] << " not good\n";
            exit(EXIT_FAILURE);
        }

        Scene currScene(inputScenes[i]);
        const SceneDimensions dimens = currScene.getSceneDimensions();
        PPMImageI currPPMImage(dimens.width, dimens.height);

        const Shading shading = (i < 2) ? Shading::BARYCENTRIC : Shading::NONBARYCENTRIC;
        Renderer renderer(currPPMImage, &currScene, shading);

        std::cout << "Loading " << sceneNames[i] << " scene...\nStart generating data...\n";
        {
            Timer timer;

            const size_t chunkSize = 32;
            for (size_t threadId = 0; threadId < numThreads; threadId++) {
                auto parallelRunTask =
                    std::bind(&Renderer::run, renderer, threadId, numThreads, chunkSize);
                taskManager.scheduleTask(parallelRunTask);
            }

            taskManager.completeTasks();

            std::cout << sceneNames[i] << " data generated in ["
                      << Timer::toMilliSec<float>(timer.getElapsedNanoSec()) << "ms] on "
                      << numThreads << " threads\n";
        }

        serializePPMImage(ppmImageFile, currPPMImage);
        ppmImageFile.close();
    }

    taskManager.stop();

    return 0;
}
