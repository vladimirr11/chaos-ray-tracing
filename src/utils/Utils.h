#include <iostream>
#include "Triangle.h"

/// @brief Works out parallelogram area formed between vectors _v1_ and _v2_
inline static float calcParallelogramArea(const Vector3f& v1, const Vector3f& v2) {
    const Vector3f cProd = cross(v1, v2);
    return sqrtf(cProd.x * cProd.x + cProd.y * cProd.y + cProd.z * cProd.z);
}

/// @brief Works out area of _triangle_
inline static float calcTraingleArea(const Triangle& triangle) {
    const Vector3f& A = triangle.mesh->vertsPositions[triangle.indices[0]];
    const Vector3f& B = triangle.mesh->vertsPositions[triangle.indices[1]];
    const Vector3f& C = triangle.mesh->vertsPositions[triangle.indices[2]];
    const Vector3f E0 = B - A;
    const Vector3f E1 = C - A;
    return calcParallelogramArea(E0, E1) / 2;
}

/// @brief Works out the normalized surface normal of _triangle_ plane
inline static Normal3f calcSurfaceNormal(const Triangle& triangle) {
    const Vector3f& A = triangle.mesh->vertsPositions[triangle.indices[0]];
    const Vector3f& B = triangle.mesh->vertsPositions[triangle.indices[1]];
    const Vector3f& C = triangle.mesh->vertsPositions[triangle.indices[2]];
    const Vector3f E0 = B - A;
    const Vector3f E1 = C - A;
    return Normal3f(cross(E0, E1).normalize());
}

inline std::ostream& operator<<(std::ostream& out, const Vector3f& v) {
    out << "{" << v.x << ", " << v.y << ", " << v.z << "}";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const Triangle& triangle) {
    out << "{A" << triangle.mesh->vertsPositions[triangle.indices[0]] << ", B"
        << triangle.mesh->vertsPositions[triangle.indices[1]] << ", C"
        << triangle.mesh->vertsPositions[triangle.indices[2]] << "}";
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
