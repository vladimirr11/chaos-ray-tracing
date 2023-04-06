#include <fstream>
#include "utils/PPMImage.h"
#include "utils/TaskManager.h"
#include "utils/Timer.h"
#include "utils/Triangle.h"
#include "utils/Utils.h"

struct Scene : public ParallelTask {
    Scene() = delete;

    Scene(PPMImageI& _ppmImage, const TriangleMesh& _mesh) : ppmImage(_ppmImage), mesh(_mesh) {}

    void operator()(const size_t workerStartIdx, const size_t workerEndIdx) override {
        const auto& triangles = mesh.getTriangles();
        using std::abs;
        for (size_t i = workerStartIdx; i < workerEndIdx; i++) {
            const int row = i / IMG_WIDTH;
            const int col = i % IMG_WIDTH;
            const Ray ray = getScreenRay(row, col);
            bool hasIntersect = false;
            Intersection isect;
            for (const auto& triangle : triangles) {
                if (triangle.intersect(ray, isect)) {
                    hasIntersect = true;
                }
            }

            if (hasIntersect) {
                const int scaleFact = 255 / isect.t;
                ppmImage.data[i].color =
                    Color3i(abs(isect.normal.x + isect.position.x) * scaleFact,
                            abs(isect.normal.y + isect.position.y) * scaleFact,
                            abs(isect.normal.z + isect.position.z) * scaleFact);
            } else {
                ppmImage.data[i].color = Color3i(255, 255, 255);
            }
        }
    }

    PPMImageI& ppmImage;
    const TriangleMesh mesh;
};

int main() {
    std::vector<TriangleMesh> meshes;
    meshes.reserve(3);

    const std::vector<Point3f> singleTriangleVerts{Point3f{-1.75, -1.75, -3},
                                                   Point3f{1.75, -1.75, -3}, Point3f{0, 1.75, -3}};
    const std::vector<TriangleIndices> singleTriangleIndices{TriangleIndices{0, 1, 2}};

    const std::vector<Point3f> twoTrianglesVerts{
        Point3f{-2.625, -1.75, -3}, Point3f{0.875, -1.75, -3}, Point3f{-0.975, 1.75, -3},
        Point3f{1.075, -1.75, -3},  Point3f{-0.775, 1.75, -3}, Point3f{2.725, 1.75, -3}};
    const std::vector<TriangleIndices> twoTrianglesIndices{TriangleIndices{0, 1, 2},
                                                           TriangleIndices{4, 3, 5}};

    const std::vector<Point3f> simple3DFormVerts{
        Point3f{-1.35, -1.45, -3}, Point3f{1.35, -1.45, -3}, Point3f{0, 1.75, -3},
        Point3f{-2.15, -2.05, -2.8}, Point3f{2.15, -2.05, -2.8}};
    const std::vector<TriangleIndices> simple3DFormIndices{
        TriangleIndices{0, 1, 2}, TriangleIndices{3, 0, 2}, TriangleIndices{4, 2, 1},
        TriangleIndices{3, 1, 0}, TriangleIndices{3, 4, 1}};

    const std::vector<Point3f> isectedTrianglesVerts{
        Point3f{-1.75, -1.75, -3}, Point3f{1.75, -1.75, -3}, Point3f{0, 1.75, -3},
        Point3f{-2.05, -1.05, -1.8}, Point3f{1.95, -0.85, -2}, Point3f{2.15, 1.85, -2}};
    const std::vector<TriangleIndices> isectedTrianglesIndices{TriangleIndices{0, 1, 2},
                                                               TriangleIndices{3, 4, 5}};

    meshes.emplace_back(singleTriangleVerts, singleTriangleIndices);
    meshes.emplace_back(twoTrianglesVerts, twoTrianglesIndices);
    meshes.emplace_back(simple3DFormVerts, simple3DFormIndices);
    meshes.emplace_back(isectedTrianglesVerts, isectedTrianglesIndices);

    const char* sceneNames[] = {"SingleTriangle.ppm", "TwoTriangles.ppm", "Simple3DForm.ppm",
                                "IntersectedTriangles.ppm"};
    const size_t numScenes = std::size(sceneNames);

    const unsigned numThreads = std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1);
    printf("Scenes count [%zu]\n", numScenes);

    TaskManager taskManager(numThreads);
    taskManager.start();

    for (size_t i = 0; i < numScenes; i++) {
        std::ofstream ppmImageFile(sceneNames[i], std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            printf("Failed to open %s\n", sceneNames[i]);
            exit(EXIT_FAILURE);
        }

        PPMImageI currPPMImage(IMG_WIDTH, IMG_HEIGHT);

        printf("Loading %s scene...\n", sceneNames[i]);
        puts("Start generating data...");
        {
            Timer timer;
            Scene scene(currPPMImage, meshes[i]);
            taskManager.runParallelFor((size_t)0, currPPMImage.data.size(), scene);
            taskManager.completeTasks();

            printf("%s data generated in [%gms] on %d threads\n", sceneNames[i],
                   Timer::toMilliSec<float>(timer.getElapsedNanoSec()), numThreads);
        }

        serializePPMImage(ppmImageFile, currPPMImage);
        ppmImageFile.close();
    }

    taskManager.stop();

    return 0;
}
