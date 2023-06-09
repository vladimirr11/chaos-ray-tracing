#ifndef MATRIX3X3_H
#define MATRIX3X3_H

#include <cmath>
#include <cstring>
#include "Utils.h"

struct Matrix3x3 {
    Matrix3x3() = default;

    /// @brief Initialize diagonal matrix
    Matrix3x3(const float diagonal) { m[0][0] = m[1][1] = m[2][2] = diagonal; }

    /// @brief Initialize 3x3 matrix given 3 vectors each representing a corresponding
    /// row in the matrix
    Matrix3x3(const Vector3f& r0, const Vector3f& r1, const Vector3f& r2) {
        m[0][0] = r0.x, m[0][1] = r0.y, m[0][2] = r0.z;
        m[1][0] = r1.x, m[1][1] = r1.y, m[1][2] = r1.z;
        m[2][0] = r2.x, m[2][1] = r2.y, m[2][2] = r2.z;
    }

    /// @brief Subscripts row from the matrix by given index
    Vector3f operator[](const int idx) const {
        Assert(0 <= idx && idx < 3);
        return Vector3f(m[idx][0], m[idx][1], m[idx][2]);
    }

    friend Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2);

    friend Vector3f operator*(const Vector3f& v, const Matrix3x3& m);

    friend Matrix3x3 transpose(const Matrix3x3& m);

    friend float determinant(const Matrix3x3& m);

    friend Matrix3x3 inverse(const Matrix3x3& m);

    friend std::ostream& operator<<(std::ostream& out, const Matrix3x3& m);

    float m[3][3]{};  ///< Zero initialized 3x3 row major matrix
};

/// @brief Rotation transform matrix around X-axis with _theta_ degrees
inline static Matrix3x3 rotateX(const float theta) {
    const float thetaRad = deg2Radians(theta);
    const float cosThetaRad = cosf(thetaRad);
    const float sinThetaRad = sinf(thetaRad);
    const Vector3f r0{1.f, 0.f, 0.f};
    const Vector3f r1{0.f, cosThetaRad, -sinThetaRad};
    const Vector3f r2{0.f, sinThetaRad, cosThetaRad};
    return Matrix3x3{r0, r1, r2};
}

/// @brief Rotation transform matrix around Y-axis with _theta_ degrees
inline static Matrix3x3 rotateY(const float theta) {
    const float thetaRad = deg2Radians(theta);
    const float cosThetaRad = cosf(thetaRad);
    const float sinThetaRad = sinf(thetaRad);
    const Vector3f r0{cosThetaRad, 0.f, sinThetaRad};
    const Vector3f r1{0.f, 1.f, 0.f};
    const Vector3f r2{-sinThetaRad, 0.f, cosThetaRad};
    return Matrix3x3{r0, r1, r2};
}

/// @brief Rotation transform matrix around Z-axis with _theta_ degrees
inline static Matrix3x3 rotateZ(const float theta) {
    const float thetaRad = deg2Radians(theta);
    const float cosThetaRad = cosf(thetaRad);
    const float sinThetaRad = sinf(thetaRad);
    const Vector3f r0{cosThetaRad, -sinThetaRad, 0.f};
    const Vector3f r1{sinThetaRad, cosThetaRad, 0.f};
    const Vector3f r2{0.f, 0.f, 1.f};
    return Matrix3x3{r0, r1, r2};
}

#endif  // !MATRIX3X3_H
