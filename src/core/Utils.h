#include <iostream>
#include <thread>
#include "Triangle.h"

/// @brief Computes parallelogram area formed between vectors _v1_ and _v2_
inline static float calcParallelogramArea(const Vector3f& v1, const Vector3f& v2) {
    const Vector3f cProd = cross(v1, v2);
    return sqrtf(cProd.x * cProd.x + cProd.y * cProd.y + cProd.z * cProd.z);
}

/// @brief Computes area of _triangle_
inline static float calcTraingleArea(const Triangle& triangle) {
    const Vector3f& A = triangle.mesh->vertexPositions[triangle.indices[0]];
    const Vector3f& B = triangle.mesh->vertexPositions[triangle.indices[1]];
    const Vector3f& C = triangle.mesh->vertexPositions[triangle.indices[2]];
    const Vector3f E0 = B - A;
    const Vector3f E1 = C - A;
    return calcParallelogramArea(E0, E1) / 2;
}

/// @brief Computes the normalized surface normal of _triangle_ plane
inline static Normal3f calcSurfaceNormal(const Triangle& triangle) {
    const Vector3f& A = triangle.mesh->vertexPositions[triangle.indices[0]];
    const Vector3f& B = triangle.mesh->vertexPositions[triangle.indices[1]];
    const Vector3f& C = triangle.mesh->vertexPositions[triangle.indices[2]];
    const Vector3f E0 = B - A;
    const Vector3f E1 = C - A;
    return Normal3f(cross(E0, E1).normalize());
}

inline std::ostream& operator<<(std::ostream& out, const Vector3f& v) {
    out << "{" << v.x << ", " << v.y << ", " << v.z << "}";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const Triangle& triangle) {
    out << "{A" << triangle.mesh->vertexPositions[triangle.indices[0]] << ", B"
        << triangle.mesh->vertexPositions[triangle.indices[1]] << ", C"
        << triangle.mesh->vertexPositions[triangle.indices[2]] << "}";
    return out;
}

/// @brief Retrieves ray directed at the center of a pixel based on the coordinates of
/// that pixel in the scene
inline static Ray getScreenRay(const int row, const int col) {
    const float ndcX = (col + 0.5f) / IMG_WIDTH;
    const float ndcY = (row + 0.5f) / IMG_HEIGHT;
    const float screenX = (2.f * ndcX - 1.f) * ASPECT_RATIO;
    const float screenY = (1.f - 2.f * ndcY);
    return Ray{{0, 0, 0}, (Vector3f(screenX, screenY, -1)).normalize()};
}

/// @brief Converts degrees to radians
inline static float deg2Radians(const float degrees) { return degrees * (PI / 180.f); }

/// @brief Computes sphere area by given sphere radius
inline static float calcSphereArea(const float sphereR) { return 4 * PI * sphereR * sphereR; }

/// @brief Clamps a value between a pair of boundary values
inline static float clamp(const float low, const float high, const float value) {
    return std::max(low, std::min(high, value));
}

/// @brief Computes reflected ray given incident ray direction and hitted surface normal
template <typename T>
inline static Vector3<T> reflect(const Vector3<T>& incidentDir, const Vector3<T>& normal) {
    return incidentDir - 2.f * dot(incidentDir, normal) * normal;
}

/// @brief Returns the number of available hardware threads
inline static unsigned getHardwareThreads() {
    return std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1);
}

/// @brief Converts crtscene file name to ppm file name
inline static std::string getPpmFileName(const std::string& inputFile) {
    const size_t start = inputFile.rfind("/");
    const size_t end = inputFile.rfind(".");
    if (start > end)
        return inputFile.substr(0, end) + ".ppm";
    return inputFile.substr(start + 1, end - start - 1) + ".ppm";
}
