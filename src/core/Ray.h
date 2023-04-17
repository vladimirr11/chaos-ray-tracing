#ifndef RAY_H
#define RAY_H

#include "Vector3.h"

/// @brief Ray struct specified by its origin and direction
struct Ray {
    Point3f origin; 
    Vector3f dir;

    Ray() : origin(0), dir(0, 0, -1) {}

    Ray(const Point3f& org, const Vector3f& d) : origin(org), dir(d) {}
    
    /// @brief Calculate the position of point along the ray direction 
    /// @param t The distance from the ray origin along the ray direction
    /// @return The point at t
    Point3f at(const float t) const { return origin + t * dir; }
};

#endif  // !RAY_H
