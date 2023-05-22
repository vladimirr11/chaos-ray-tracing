#ifndef AABBOX_H
#define AABBOX_H

#include "Ray.h"

/// @brief Finds and returns the component-wise minimum values of points _p1_ and _p2_
template <typename T>
inline static Vector3<T> minPoint(const Vector3<T>& p1, const Vector3<T>& p2) {
    using std::min;
    return Vector3<T>(min(p1.x, p2.x), min(p1.y, p2.y), min(p1.z, p2.z));
}

/// @brief Finds and returns the component-wise maximum values of points _p1_ and _p2_
template <typename T>
inline static Vector3<T> maxPoint(const Vector3<T>& p1, const Vector3<T>& p2) {
    using std::max;
    return Vector3<T>(max(p1.x, p2.x), max(p1.y, p2.y), max(p1.z, p2.z));
}

/// @brief Computes at compile time gamma coefficient that is used to bound
/// the inherent IEEE floating-point arithmetic rounding error
inline constexpr float gamma(int n) { return (n * EPSILON) / (1 - n * EPSILON); }

/// @brief Axis aligned bounding box represented by min & max corner points in 3D
struct BBox {
    Vector3f min;  ///< Vertex with minimum coordinate values
    Vector3f max;  ///< Vertex with maximum coordinate values

    BBox() : min(MAX_FLOAT), max(MIN_FLOAT) {}

    BBox(const Vector3f& p1, const Vector3f& p2) : min(minPoint(p1, p2)), max(maxPoint(p1, p2)) {}

    /// @brief Extends the bounds of the box by another box _otherBox_
    void unionWith(const BBox& otherBox) {
        min = minPoint(min, otherBox.min);
        max = maxPoint(max, otherBox.max);
    }

    /// @brief Extends the bounds of the box by point _p_
    template <typename T>
    void expandBy(const Vector3<T>& p) {
        min = minPoint(min, p);
        max = maxPoint(max, p);
    }

    /// @brief Verifies if ray intersects with the box
    /// source https://github.com/mmp/pbrt-v3/blob/master/src/core/geometry.h
    bool intersect(const Ray& ray) const {
        float t0 = 0, t1 = MAX_FLOAT;
        for (int i = 0; i < 3; i++) {
            // Update interval for ith bounding box slab
            float invRayDir = 1 / ray.dir[i];
            float tNear = (min[i] - ray.origin[i]) * invRayDir;
            float tFar = (max[i] - ray.origin[i]) * invRayDir;

            if (tNear > tFar) {
                std::swap(tNear, tFar);
            }

            tFar *= 1 + 2 * gamma(3);  // Update tFar to ensure robust ray–bounds intersection
            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar < t1 ? tFar : t1;
            if (t0 > t1) {
                return false;
            }
        }

        return true;
    }
};

#endif  // !AABBOX_H
