#include "core/Scene.h"
#include "core/TaskManager.h"
#include "core/Timer.h"

inline static Color3f shade(const Scene* scene, Intersection& isectHit) {
    Color3f finalColor;
    const std::vector<Light>& lights = scene->getLights();
    for (const Light& light : lights) {
        Vector3f lightDir = light.getPosition() - isectHit.position;
        const float sphereR = lightDir.length();
        const Vector3f lightDirN = normalize(lightDir);
        const float cosTheta = std::max(0.f, dot(lightDirN, isectHit.normal));
        const Vector3f albedo(0.41f, 0.29f, 0.33f);
        const float sphereArea = calcSphereArea(sphereR);
        const Vector3f shadowRayOrig = isectHit.position + isectHit.normal * SHADOW_BIAS;
        const Ray shadowRay(shadowRayOrig, lightDirN);
        Color3f perLightColor;
        if (!scene->intersectPrim(shadowRay)) {
            perLightColor = Color3f(light.getIntensity() / sphereArea * albedo * cosTheta);
        }
        finalColor += perLightColor;
    }

    return finalColor;
}

inline static Color3f raytrace(const Ray& cameraRay, const Scene* scene) {
    Intersection isectData;
    if (scene->intersect(cameraRay, isectData)) {
        return shade(scene, isectData);
    }
    return scene->getBackground();
}

class Renderer : public ParallelTask {
public:
    Renderer() = delete;

    Renderer(PPMImageI& _ppmImage, const Scene* _scene) : ppmImage(_ppmImage), scene(_scene) {}

    void run(const size_t threadId, const size_t threadCount, const size_t chunkSize = 1) override {
        const SceneDimensions& dimens = scene->getSceneDimensions();
        const Camera& camera = scene->getCamera();
        for (size_t i = (chunkSize * threadId); i < ppmImage.data.size();
             i += (chunkSize * threadCount)) {
            for (size_t c = 0; c < chunkSize; c++) {
                const int row = (i + c) / dimens.width;
                const int col = (i + c) % dimens.width;
                const Ray cameraRay = camera.getRay(row, col);
                const Color3f pixelColor = raytrace(cameraRay, scene);
                ppmImage.data[i + c].color =
                    Color3i(pixelColor.x * 255, pixelColor.y * 255, pixelColor.z * 255);
            }
        }
    }

private:
    PPMImageI& ppmImage;
    const Scene* scene;
};

int main() {
    const char* inputScenes[]{"scenes/scene0.crtscene", "scenes/scene1.crtscene",
                              "scenes/scene2.crtscene", "scenes/scene3.crtscene"};

    const char* sceneNames[]{"scene0.ppm", "scene1.ppm", "scene2.ppm", "scene3.ppm"};

    const size_t numScenes = std::size(inputScenes);
    const unsigned numThreads = std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1);

    TaskManager taskManager(numThreads);
    taskManager.start();

    std::cout << "Number of scenes to render [" << numScenes << "]\n";
    for (size_t i = 0; i < numScenes; ++i) {
        std::ofstream ppmImageFile(sceneNames[i], std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            std::cout << "Input file " << sceneNames[i] << " not good\n";
            exit(EXIT_FAILURE);
        }

        Scene currScene(inputScenes[i]);
        const SceneDimensions dimens = currScene.getSceneDimensions();
        PPMImageI currPPMImage(dimens.width, dimens.height);

        Renderer renderer(currPPMImage, &currScene);

        std::cout << "Loading " << sceneNames[i] << " scene...\nStart generating data...\n";
        {
            Timer timer;

            const size_t chunkSize = 64;
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
