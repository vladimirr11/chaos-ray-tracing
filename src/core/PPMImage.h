#ifndef PPMIMAGE_H
#define PPMIMAGE_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "Vector3.h"
// Trird-party includes
#include "external_libs/stb/stb_image_write.h"

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

    PPMImage(const int imageWidth, const int imageHeight)
        : data(imageWidth * imageHeight), width(imageWidth), height(imageHeight) {}

    std::vector<Pixel> data;
    const int32_t width, height;
};

typedef PPMImage<float> PPMImageF;
typedef PPMImage<int> PPMImageI;
typedef PPMImageF::Pixel PPMPixelF;
typedef PPMImageI::Pixel PPMPixelI;

/// @brief Writes pixel color data to the provided output stream in ppm format
inline static void serializePPMImage(std::ostream& outputStream, const PPMImageI& ppmImage) {
    outputStream << "P3\n";
    outputStream << ppmImage.width << " " << ppmImage.height << "\n";
    outputStream << MAX_COLOR_COMP << "\n";
    for (int c = 1, row = 1; const PPMPixelI& pixel : ppmImage.data) {
        outputStream << pixel.r << " " << pixel.g << " " << pixel.b << " ";
        if (c == ppmImage.width * row) {
            outputStream << "\n";
            row++;
        }
        c++;
    }
}

/// @brief Converts PPMImageI::Pixel data to buffer of chars
inline static std::vector<char> serializePPMImage2Buffer(const PPMImageI& ppmImage) {
    std::vector<char> buffer(ppmImage.width * ppmImage.height * 3);
    for (int i = 0; const PPMPixelI& pixel : ppmImage.data) {
        buffer[i] = pixel.r;
        buffer[i + 1] = pixel.g;
        buffer[i + 2] = pixel.b;
        i += 3;
    }
    return buffer;
}

/// @brief Writes pixel color data to jpeg format
inline static void serializePPMImage2PNG(std::string_view name, PPMImageI& ppmImage) {
    const std::vector<char> buffer = serializePPMImage2Buffer(ppmImage);
    stbi_write_jpg(name.data(), ppmImage.width, ppmImage.height, 3, buffer.data(), 100);
}

#endif  // !PPMIMAGE_H
