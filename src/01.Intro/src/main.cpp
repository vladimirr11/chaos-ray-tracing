#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

static constexpr int IMG_WIDTH = 2048;
static constexpr int IMG_HEIGHT = 1080;
static constexpr int NUM_PIXELS = IMG_WIDTH * IMG_HEIGHT;
static constexpr int WIDTH_BLOCK_SIZE = IMG_WIDTH >> 2;
static constexpr int HEIGHT_BLOCK_SIZE = IMG_HEIGHT >> 2;
static constexpr int BLOCKS_PER_DIMENS = 4;
static constexpr int MAX_COLOR_COMP = 255;

#if __linux__ != 0
#include <time.h>

static uint64_t timer_nsec() {
#if defined(CLOCK_MONOTONIC_RAW)
    const clockid_t clockid = CLOCK_MONOTONIC_RAW;

#else
    const clockid_t clockid = CLOCK_MONOTONIC;

#endif
    timespec t;
    clock_gettime(clockid, &t);

    return t.tv_sec * 1000000000UL + t.tv_nsec;
}

#elif _WIN64 != 0
#include <Windows.h>

static struct TimerBase {
    LARGE_INTEGER freq;
    TimerBase() { QueryPerformanceFrequency(&freq); }
} timerBase;

static uint64_t timer_nsec() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);

    return 1000000000ULL * t.QuadPart / timerBase.freq.QuadPart;
}

#elif __APPLE__ != 0
#include <mach/mach_time.h>

static struct TimerBase {
    mach_timebase_info_data_t tb;
    TimerBase() { mach_timebase_info(&tb); }
} timerBase;

static uint64_t timer_nsec() {
    const uint64_t t = mach_absolute_time();
    return t * timerBase.tb.numer / timerBase.tb.denom;
}

#endif

struct Timer {
    Timer() : start(timer_nsec()) {}

    template <typename T>
    static T toMs(T ns) {
        return T(ns / 1e6);
    }

    int64_t elapsedNs() const {
        const uint64_t now = timer_nsec();
        return now - start;
    }

    uint64_t start;
};

inline static uint8_t getRandInt(const uint8_t from, const uint8_t to) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint8_t> dist(from, to);
    return dist(rng);
}

struct ColorPalette {
    enum Color : uint8_t { RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, NUM_COLORS };

    ColorPalette() : color(RED) {}

    ColorPalette(const int colorId) : color(static_cast<Color>(colorId)) {}

    Color getCurrentColor() { return color; }

    Color getNextColor() { return (Color)((color + 1) % NUM_COLORS); }

    Color getPrevRowStartColor() {
        return (Color)((color + std::abs(NUM_COLORS - BLOCKS_PER_DIMENS)) % NUM_COLORS);
    }

    operator uint8_t() const { return (uint8_t)color; }

    Color color;
};

struct PPMImage {
    struct Pixel {
        uint8_t r{}, g{}, b{};
    };

    PPMImage() = delete;

    PPMImage(const int imgWidth, const int imgHeight) : data(imgWidth * imgHeight) {}

    std::vector<Pixel> data;
};

typedef PPMImage::Pixel PPMPixel;

static PPMPixel calcPixelColor(const ColorPalette palette) {
    PPMPixel pixel;
    switch (palette.color) {
        case ColorPalette::RED:
            pixel.r = getRandInt(155, MAX_COLOR_COMP);
            break;
        case ColorPalette::GREEN:
            pixel.g = getRandInt(155, MAX_COLOR_COMP);
            break;
        case ColorPalette::YELLOW:
            pixel.r = getRandInt(155, MAX_COLOR_COMP);
            pixel.g = getRandInt(155, MAX_COLOR_COMP);
            break;
        case ColorPalette::BLUE:
            pixel.b = getRandInt(155, MAX_COLOR_COMP);
            break;
        case ColorPalette::MAGENTA:
            pixel.r = getRandInt(155, MAX_COLOR_COMP);
            pixel.b = getRandInt(155, MAX_COLOR_COMP);
            break;
        case ColorPalette::CYAN:
            pixel.g = getRandInt(155, MAX_COLOR_COMP);
            pixel.b = getRandInt(155, MAX_COLOR_COMP);
            break;
        default:
            std::cerr << "Recieved unsupported color"
                      << "\n";
            exit(1);
    }

    return pixel;
}

static void genRugImagePixelData(PPMImage& ppmImage, const int workerStartIdx,
                                 const int workerEndIdx, ColorPalette palette) {
    for (int i = workerStartIdx; i < workerEndIdx; i++) {
        const int col = i % IMG_WIDTH;
        if (col % WIDTH_BLOCK_SIZE == 0) {
            palette.color = palette.getNextColor();
            const int row = i / IMG_WIDTH;
            if (col == 0 && row % HEIGHT_BLOCK_SIZE != 0) {
                palette.color = palette.getPrevRowStartColor();
            }
        }

        ppmImage.data[i] = calcPixelColor(palette);
    }
}

inline static int calcThreadStartColor(const int workerStartIdx, const int threadId) {
    const int rowOffset = workerStartIdx / (IMG_WIDTH + 1);
    const int rowBlockOffset = rowOffset / HEIGHT_BLOCK_SIZE;
    const int startColor =
        ((BLOCKS_PER_DIMENS * (rowBlockOffset + 1)) - 1) % ColorPalette::NUM_COLORS;
    return (threadId > 0) ? startColor : ColorPalette::NUM_COLORS - 1;
}

static void generateRugImage(const int numThreads, PPMImage& ppmImageOut) {
    std::vector<std::thread> workers;

    const int workerRunDist = ppmImageOut.data.size() / numThreads;
    for (int i = 0; i < numThreads; i++) {
        const int workerStartIdx = i * workerRunDist;
        const int workerEndIdx = workerStartIdx + workerRunDist;
        const int startColor = calcThreadStartColor(workerStartIdx, i);
        ColorPalette palette(startColor);
        workers.emplace_back(&genRugImagePixelData, std::ref(ppmImageOut), workerStartIdx,
                             workerEndIdx, palette);
    }

    for (int i = 0; i < numThreads; i++) {
        workers[i].join();
    }
}

inline static bool inCircle(const int row, const int col, const int r, const int oriX,
                            const int oriY) {
    return ((col - oriX) * (col - oriX)) + ((row - oriY) * (row - oriY)) <= (r * r);
}

static void genCircleImagePixelData(PPMImage& ppmImage, const int workerStartIdx,
                                    const int workerEndIdx) {
    const int oriX = IMG_WIDTH >> 1;
    const int oriY = IMG_HEIGHT >> 1;
    const int r = IMG_HEIGHT >> 2;
    for (int i = workerStartIdx; i < workerEndIdx; i++) {
        const int row = i / IMG_WIDTH;
        const int col = i % IMG_WIDTH;
        if (inCircle(row, col, r, oriX, oriY)) {
            ppmImage.data[i] = PPMPixel(255, 0, 0);
        } else {
            ppmImage.data[i] = PPMPixel(255, 255, 255);
        }
    }
}

static void generateCircleImage(const int numThreads, PPMImage& ppmImageOut) {
    std::vector<std::thread> workers;

    const int workerRunDist = ppmImageOut.data.size() / numThreads;
    for (int i = 0; i < numThreads; i++) {
        const int workerStartIdx = i * workerRunDist;
        const int workerEndIdx = workerStartIdx + workerRunDist;
        workers.emplace_back(&genCircleImagePixelData, std::ref(ppmImageOut), workerStartIdx,
                             workerEndIdx);
    }

    for (int i = 0; i < numThreads; i++) {
        workers[i].join();
    }
}

static void serializePPMImage(std::ostream& outputStream, const PPMImage& ppmImage) {
    outputStream << "P3\n";
    outputStream << IMG_WIDTH << " " << IMG_HEIGHT << "\n";
    outputStream << MAX_COLOR_COMP << "\n";
    for (int c = 1, row = 1; const PPMPixel& pixel : ppmImage.data) {
        outputStream << std::to_string(pixel.r) << " " << std::to_string(pixel.g) << " "
                     << std::to_string(pixel.b) << " ";
        if (c == IMG_WIDTH * row) {
            outputStream << "\n";
            row++;
        }
        c++;
    }
}

int main() {
    void (*imageCreators[])(const int, PPMImage&) = {generateRugImage, generateCircleImage};
    const int numImages = std::size(imageCreators);

    const char* imagesNames[] = {"RugImage.ppm", "CircleImage.ppm"};

    const int numThreads = 3;
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
            PPMImage currPPMImage(IMG_WIDTH, IMG_HEIGHT);
            imageCreators[i](numThreads, currPPMImage);
            serializePPMImage(ppmImageFile, currPPMImage);
            printf("%s data generated in [%gms] on %d threads\n", imagesNames[i],
                   Timer::toMs<float>(timer.elapsedNs()), numThreads);
        }

        ppmImageFile.close();
    }

    return 0;
}
