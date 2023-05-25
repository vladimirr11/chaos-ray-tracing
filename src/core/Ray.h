#ifndef RAY_H
#define RAY_H

#include "Vector3.h"

enum class RayType {
    CAMERA,      ///< Ray comming from the camera
    SHADOW,      ///< Ray shading diffuse material
    REFLECTION,  ///< Ray scattered from reflective material
    UNDEFINED,
};

/// @brief Ray struct specified by its origin, direction, type and depth
struct Ray {
    Point3f origin;
    Vector3f dir;
    int depth = 0;
    mutable float tMax = MAX_FLOAT;

    Ray() : origin(0), dir(0, 0, -1) {}

    Ray(const Point3f& org, const Vector3f& d) : origin(org), dir(d) {}

    /// @brief Calculate the position of point along the ray direction
    /// @param t The distance from the ray origin along the ray direction
    /// @return The point at t
    Point3f at(const float t) const { return origin + t * dir; }
};

#endif  // !RAY_H
