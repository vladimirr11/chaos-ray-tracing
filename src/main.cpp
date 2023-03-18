#include <fstream>
#include <iostream>
#include <thread>
#include "utils/PPMImage.h"
#include "utils/Ray.h"
#include "utils/Timer.h"

static constexpr float ASPECT_RATIO = 16.f / 9.f;
static constexpr int IMG_WIDTH = 2048;
static constexpr int IMG_HEIGHT = IMG_WIDTH / ASPECT_RATIO;
static constexpr int NUM_PIXELS = IMG_WIDTH * IMG_HEIGHT;
static constexpr int MAX_COLOR_COMP = 255;

static void calcColorGradient(const int workerStartIdx, const int workerEndIdx,
                              PPMImageI& ppmImageOut) {
    for (int i = workerStartIdx; i < workerEndIdx; i++) {
        const int row = i / IMG_WIDTH;
        const int col = i % IMG_WIDTH;
        const float ndcX = (col + 0.5f) / IMG_WIDTH;
        const float ndcY = (row + 0.5f) / IMG_HEIGHT;
        const float screenX = (2.f * ndcX - 1.f) * ASPECT_RATIO;
        const float screenY = (1.f - 2.f * ndcY);
        const Ray ray{{0, 0, 0}, (Vector3f(screenX, screenY, -1)).normalize()};
        ppmImageOut.data[i].color = Color3i(std::abs(ray.dir.x) * 255, std::abs(ray.dir.y) * 255,
                                            std::abs(ray.dir.z) * 255);
    }
}

static void generateColorGradient(const int numThreads, PPMImageI& ppmImageOut) {
    std::vector<std::thread> workers;

    const int workerRunDist = ppmImageOut.data.size() / numThreads;
    for (int i = 0; i < numThreads; i++) {
        const int workerStartIdx = i * workerRunDist;
        const int workerEndIdx = workerStartIdx + workerRunDist;
        workers.emplace_back(&calcColorGradient, workerStartIdx, workerEndIdx,
                             std::ref(ppmImageOut));
    }

    for (int i = 0; i < numThreads; i++) {
        workers[i].join();
    }
}

static void serializePPMImage(std::ostream& outputStream, const PPMImageI& ppmImage) {
    outputStream << "P3\n";
    outputStream << IMG_WIDTH << " " << IMG_HEIGHT << "\n";
    outputStream << MAX_COLOR_COMP << "\n";
    for (int c = 1, row = 1; const PPMPixelI& pixel : ppmImage.data) {
        outputStream << pixel.r << " " << pixel.g << " " << pixel.b << " ";
        if (c == IMG_WIDTH * row) {
            outputStream << "\n";
            row++;
        }
        c++;
    }
}

int main() {
    void (*imageCreators[])(const int, PPMImageI&) = {generateColorGradient};
    const int numImages = std::size(imageCreators);

    const char* imagesNames[] = {"ColorGradient.ppm"};

    const int numThreads = 4;
    printf("Image count [%d]\n", numImages);

    for (int i = 0; i < numImages; i++) {
        std::ofstream ppmImageFile(imagesNames[i], std::ios::out | std::ios::binary);
        if (!ppmImageFile.good()) {
            printf("Failed to open %s\n", imagesNames[i]);
            exit(EXIT_FAILURE);
        }

        printf("Loading %s...\n", imagesNames[i]);
        puts("Start generating data...");
        {
            Timer timer;
            PPMImageI currPPMImage(IMG_WIDTH, IMG_HEIGHT);
            imageCreators[i](numThreads, currPPMImage);
            serializePPMImage(ppmImageFile, currPPMImage);
            printf("%s data generated in [%gms] on %d thread\n", imagesNames[i],
                   Timer::toMilliSec<float>(timer.getElapsedNanoSec()), numThreads);
        }

        ppmImageFile.close();
    }

    return 0;
}
