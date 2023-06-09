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
/// the inherent floating-point arithmetic rounding error
/// source https://github.com/mmp/pbrt-v3/blob/master/src/core/pbrt.h
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

    /// @brief Verifies if ray intersects with the box using Kay and Kajiya’s
    /// slab method
    /// source https://github.com/mmp/pbrt-v3/blob/master/src/core/geometry.h
    bool intersect(const Ray& ray) const {
        float t0 = 0, t1 = MAX_FLOAT;
        for (int i = 0; i < 3; i++) {
            float invRayDir = 1 / ray.dir[i];
            float tNear = (min[i] - ray.origin[i]) * invRayDir;
            float tFar = (max[i] - ray.origin[i]) * invRayDir;

            if (tNear > tFar) {
                std::swap(tNear, tFar);
            }

            tFar *= 1 + 2 * gamma(3);  // Update tFar to ensure robust ray–bbox intersection
            t0 = tNear > t0 ? tNear : t0;
            t1 = tFar < t1 ? tFar : t1;
            if (t0 > t1) {
                return false;
            }
        }

        return true;
    }
};

/// @brief Splits _box_ at _axis_ in _offset_ and returns the corresponding boxes as pair
inline static std::pair<BBox, BBox> splitBBox(const BBox& box, const int32_t axis,
                                              const float offset) {
    BBox L = box, R = box;
    L.max[axis] = R.min[axis] = offset;
    return std::make_pair(L, R);
}

/// @brief Verifies if _boxB_ intersects with _boxA_
inline static bool boxIntersect(const BBox& boxA, const BBox& boxB) {
    for (int32_t axis = 0; axis < 3; axis++) {
        if (boxB.min[axis] > boxA.max[axis] || boxB.max[axis] < boxA.min[axis]) {
            return false;
        }
    }
    return true;
}

/// @brief Finds the longest axis of _box_
inline static int32_t findMaxExtent(const BBox& box) {
    const Vector3f boxDiagonal = box.max - box.min;
    if (boxDiagonal.x > boxDiagonal.y && boxDiagonal.x > boxDiagonal.z)
        return 0;
    else if (boxDiagonal.y > boxDiagonal.z)
        return 1;
    else
        return 2;
}

#endif  // !AABBOX_H
