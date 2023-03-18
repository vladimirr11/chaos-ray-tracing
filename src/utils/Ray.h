#ifndef RAY_H
#define RAY_H

#include "Vector3.h"

struct Ray {
    Point3f origin;
    Vector3f dir;

    Ray() : origin(0), dir(0, 0, -1) {}

    Ray(const Point3f& _origin, const Vector3f& _dir) : origin(_origin), dir(_dir) {}

    Vector3f at(const float t) const { return origin + t * dir; }
};

#endif  // !RAY_H
