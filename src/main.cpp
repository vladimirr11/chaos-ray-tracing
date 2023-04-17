#include "core/Scene.h"
#include "core/TaskManager.h"
#include "core/Timer.h"

class Renderer : public ParallelTask {
public:
    Renderer() = delete;

    Renderer(PPMImageI& _ppmImage, const Scene& _scene)
        : ppmImage(_ppmImage),
          objects(_scene.getGeometryObjects()),
          camera(_scene.getCamera()),
          sceneDimens(_scene.getSceneDimensions()),
          background(_scene.getBackgroundColor()) {}

    void run(const size_t threadIdx, const size_t threadCount) override {
        const Color3i backgrColor =
            Color3i((int)background.x * 255, background.y * 255, background.z * 255);
        using std::abs;
        for (size_t i = threadIdx; i < ppmImage.data.size(); i += threadCount) {
            const int row = i / sceneDimens.width;
            const int col = i % sceneDimens.width;
            const Ray ray = camera.getRay(row, col);
            bool hasIntersect = false;
            Intersection closestPrim;
            closestPrim.t = FLT_MAX;
            for (const auto& mesh : objects) {
                Intersection isect;
                if (mesh.intersect(ray, isect)) {
                    if (isect.t < closestPrim.t) {
                        closestPrim = isect;
                    }
                    hasIntersect = true;
                }
            }

            if (hasIntersect) {
                const int scaleFact = 255 / closestPrim.t;
                ppmImage.data[i].color =
                    Color3i((int)abs(closestPrim.position.x) * scaleFact % 255,
                            (int)abs(closestPrim.position.y) * scaleFact % 255,
                            (int)abs(closestPrim.position.z) * scaleFact % 255);
            } else {
                ppmImage.data[i].color = backgrColor;
            }
        }
    }

private:
    PPMImageI& ppmImage;
    const std::vector<TriangleMesh> objects;
    const Camera camera;
    const SceneDimesions sceneDimens;
    const Color3f background;
};

int main() {
    const char* inputScenes[] = {"scenes/scene0.crtscene", "scenes/scene1.crtscene",
                                 "scenes/scene2.crtscene", "scenes/scene3.crtscene",
                                 "scenes/scene4.crtscene"};

    const char* sceneNames[] = {"scene0.ppm", "scene1.ppm", "scene2.ppm", "scene3.ppm",
                                "scene4.ppm"};

    const size_t numScenes = std::size(inputScenes);
    const unsigned numThreads = std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1);

    TaskManager taskManeger(numThreads);
    taskManeger.start();

    std::cout << "Number of scene to render [" << numScenes << "]\n";
    for (size_t i = 0; i < numScenes; ++i) {
        std::ofstream ppmImageFile(sceneNames[i], std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            std::cout << "Input file " << sceneNames[i] << " not good\n";
            exit(EXIT_FAILURE);
        }

        Scene currScene(inputScenes[i]);
        const SceneDimesions dimens = currScene.getSceneDimensions();
        PPMImageI currPPMImage(dimens.width, dimens.height);

        Renderer renderer(currPPMImage, currScene);

        std::cout << "Loading " << sceneNames[i] << " scene...\nStart generating data...\n";
        {
            Timer timer;

            for (size_t threadIdx = 0; threadIdx < numThreads; threadIdx++) {
                auto parallelRunTask = std::bind(&Renderer::run, renderer, threadIdx, numThreads);
                taskManeger.scheduleTask(parallelRunTask);
            }

            taskManeger.completeTasks();

            std::cout << sceneNames[i] << " data generated in ["
                      << Timer::toMilliSec<float>(timer.getElapsedNanoSec()) << "ms] on "
                      << numThreads << " threads\n";
        }

        serializePPMImage(ppmImageFile, currPPMImage);
        ppmImageFile.close();
    }

    taskManeger.stop();

    return 0;
}
