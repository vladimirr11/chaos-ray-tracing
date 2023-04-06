#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <limits>

static constexpr float ASPECT_RATIO = 16.f / 9.f;
static constexpr int IMG_WIDTH = 1920;
static constexpr int IMG_HEIGHT = IMG_WIDTH / ASPECT_RATIO;
static constexpr int NUM_PIXELS = IMG_WIDTH * IMG_HEIGHT;
static constexpr int MAX_COLOR_COMP = 255;
static constexpr float EPSILON = std::numeric_limits<float>::epsilon();

#endif  // !CONSTANTS_H
