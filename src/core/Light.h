#ifndef LIGHT_H
#define LIGHT_H

#include "Vector3.h"

class Light {
public:
    Light(const Point3f& _position, const int32_t _intensity)
        : position(_position), intensity(_intensity) {}

    const Point3f getPosition() const { return position; }

    const int32_t getIntensity() const { return intensity; }

private:
    const Point3f position;   ///< Light source position in world space
    const int32_t intensity;  ///< Light source intensity
};

#endif  // !LIGHT_H
