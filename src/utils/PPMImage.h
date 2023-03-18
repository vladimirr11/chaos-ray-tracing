#ifndef PPMIMAGE_H
#define PPMIMAGE_H

#include <cstdint>
#include <vector>
#include "Vector3.h"

template <typename T>
struct PPMImage {
    union Pixel {
        Vector3<T> color;
        struct {
            T r, g, b;
        };

        Pixel() : color() {}
    };

    PPMImage() = delete;

    PPMImage(const int imgWidth, const int imgHeight) : data(imgWidth * imgHeight) {}

    std::vector<Pixel> data;
};

typedef PPMImage<float> PPMImageF;
typedef PPMImage<int> PPMImageI;
typedef PPMImageF::Pixel PPMPixelF;
typedef PPMImageI::Pixel PPMPixelI;

#endif  // !PPMIMAGE_H
