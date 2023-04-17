#ifndef PPMIMAGE_H
#define PPMIMAGE_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "Vector3.h"

/// @brief Stores the color data for each pixel in the final image
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

/// @brief Writes pixels color data to the provided output stream in ppm format
inline void serializePPMImage(std::ostream& outputStream, const PPMImageI& ppmImage) {
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

#endif  // !PPMIMAGE_H
