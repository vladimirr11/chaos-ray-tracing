#include <fstream>
#include <string>
#include "utils/Camera.h"
#include "utils/PPMImage.h"
#include "utils/TaskManager.h"
#include "utils/Timer.h"
#include "utils/Triangle.h"

class Scene : public ParallelTask {
public:
    Scene() = delete;

    Scene(PPMImageI& _ppmImage, const TriangleMesh& _mesh, const Camera& _camera)
        : ppmImage(_ppmImage), mesh(_mesh), camera(_camera) {}

    void operator()(const size_t workerStartIdx, const size_t workerEndIdx) override {
        const auto& triangles = mesh.getTriangles();
        using std::abs;
        for (size_t i = workerStartIdx; i < workerEndIdx; i++) {
            const int row = i / IMG_WIDTH;
            const int col = i % IMG_WIDTH;
            const Ray ray = camera.getRay(row, col);
            bool hasIntersect = false;
            Intersection closestPrim;
            closestPrim.t = FLT_MAX;
            for (const auto& triangle : triangles) {
                Intersection isect;
                if (triangle.intersect(ray, isect)) {
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
                ppmImage.data[i].color = Color3i(255, 255, 255);
            }
        }
    }

private:
    PPMImageI& ppmImage;
    const TriangleMesh mesh;
    Camera camera;
};

int main() {
    std::vector<TriangleMesh> meshes;
    meshes.reserve(2);

    const std::vector<Point3f> singleTriangleVerts{Point3f{-1.75, -1.75, -3},
                                                   Point3f{1.75, -1.75, -3}, Point3f{0, 1.75, -3}};
    const std::vector<TriangleIndices> singleTriangleIndices{TriangleIndices{0, 1, 2}};

    const std::vector<Point3f> trianglePairVerts{
        Point3f{-1.75, -1.75, -3},   Point3f{1.75, -1.75, -3}, Point3f{0, 1.75, -3},
        Point3f{-2.05, -1.05, -1.8}, Point3f{1.95, -0.85, -2}, Point3f{2.15, 1.85, -2}};
    const std::vector<TriangleIndices> trianglePairIndices{TriangleIndices{0, 1, 2},
                                                           TriangleIndices{3, 4, 5}};

    meshes.emplace_back(singleTriangleVerts, singleTriangleIndices);
    meshes.emplace_back(trianglePairVerts, trianglePairIndices);

    const char* sceneNames[] = {"SingleTriangle", "TrianglePair"};
    const unsigned numThreads = std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1);

    TaskManager taskManager(numThreads);
    taskManager.start();

    // Tasks 1, 2, 3
    {
        const Vector3f cameraPos{0.f, 0.f, 0.f};
        const Vector3f cameraLookAt{0.f, 0.f, -1.f};
        Camera camera(cameraPos, cameraLookAt);

        std::cout << "Task [1] data:\nCamera position " << camera.getLookFrom()
                  << ", camera orientation " << camera.getLookAt() << "\nInitial rotation matrix\n"
                  << camera.getRotationMatrix() << "\n";

        camera.pan(30);
        std::cout << "Rotation matrix after pan 30 degrees\n"
                  << camera.getRotationMatrix()
                  << "\nTransformation of camera vector {0, 0, -1} to "
                  << camera.getRotationMatrix()[2] << "\n";

        camera.undoLastRotation();
        std::cout << "Rotation matrix after undo of last move\n"
                  << camera.getRotationMatrix() << "\n";

        camera.truck(-1.f);
        std::cout << "Camera position after move with -1 unit to the left " << camera.getLookFrom()
                  << "\n";

        const std::string fileName = sceneNames[0] + std::to_string(0) + ".ppm";
        std::ofstream ppmImageFile(fileName, std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            std::cout << "Failed to open " << fileName << "\n";
            exit(EXIT_FAILURE);
        }

        PPMImageI ppmImage(IMG_WIDTH, IMG_HEIGHT);

        Scene scene(ppmImage, meshes[0], camera);
        taskManager.runParallelFor((size_t)0, ppmImage.data.size(), scene);
        taskManager.completeTasks();

        serializePPMImage(ppmImageFile, ppmImage);
        ppmImageFile.close();
    }

    // // Task 4
    {
        const Vector3f cameraPos{0.f, 0.f, 0.f};
        const Vector3f cameraLookAt{0.f, 0.f, -1.f};
        Camera camera(cameraPos, cameraLookAt);

        camera.dolly(1.f);
        camera.tilt(-15.f);
        camera.pan(30);

        std::cout << "\nTask [4] data\nCamera look from position " << camera.getLookFrom()
                  << "\nCamera orientation " << camera.getLookAt() << "\nRotation matrix\n"
                  << camera.getRotationMatrix() << "\n"
                  << std::endl;

        const std::string fileName = sceneNames[0] + std::to_string(1) + ".ppm";
        std::ofstream ppmImageFile(fileName, std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            std::cout << "Failed to open " << fileName << "\n";
            exit(EXIT_FAILURE);
        }

        PPMImageI ppmImage(IMG_WIDTH, IMG_HEIGHT);

        Scene scene(ppmImage, meshes[0], camera);
        taskManager.runParallelFor((size_t)0, ppmImage.data.size(), scene);
        taskManager.completeTasks();

        serializePPMImage(ppmImageFile, ppmImage);
        ppmImageFile.close();
    }

    // Task 5
    {
        Camera camera({0, 0, 0}, {0, 0, -1});

        const size_t numFrames = 96;
        std::cout << "\nTask [5] data\nNumber of scenes " << numFrames;

        const float theta = 0.1f;
        const float dollyForw = -0.1f, dollyBack = 0.1f;
        for (size_t i = 0; i < numFrames; i++) {
            const std::string fileName = sceneNames[1] + std::to_string(i) + ".ppm";
            std::ofstream ppmImageFile(fileName, std::ios::out | std::ios::binary);
            if (!ppmImageFile.good()) {
                std::cout << "Failed to open " << fileName << "\n";
                exit(EXIT_FAILURE);
            }

            PPMImageI currPPMImage(IMG_WIDTH, IMG_HEIGHT);

            std::cout << "\nLoading " << fileName << " scene...\nStart generating data...\n";
            {
                Timer timer;

                camera.roll(theta);

                const float dollyMove = i < (numFrames / 2) ? dollyBack : dollyForw;
                camera.dolly(dollyMove);

                Scene scene(currPPMImage, meshes[1], camera);
                taskManager.runParallelFor((size_t)0, currPPMImage.data.size(), scene);
                taskManager.completeTasks();

                std::cout << fileName << " data generated in ["
                          << Timer::toMilliSec<float>(timer.getElapsedNanoSec()) << "ms] on "
                          << numThreads << " threads\n";
            }

            serializePPMImage(ppmImageFile, currPPMImage);
            ppmImageFile.close();
        }
    }

    taskManager.stop();

    return 0;
}
