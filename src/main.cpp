#include <iostream>
#include "utils/Triangle.h"

inline static float calcParallelogramArea(const Vector3f& v1, const Vector3f& v2) {
    const Vector3f cProd = cross(v1, v2);
    return sqrtf(cProd.x * cProd.x + cProd.y * cProd.y + cProd.z * cProd.z);
}

inline static float calcTraingleArea(const Triangle& triangle) {
    const Vector3f u = triangle.verts[1] - triangle.verts[0];
    const Vector3f v = triangle.verts[2] - triangle.verts[0];
    return calcParallelogramArea(u, v) / 2;
}

inline static Normal3f calcSurfaceNormal(const Triangle& triangle) {
    const Vector3f u = triangle.verts[1] - triangle.verts[0];
    const Vector3f v = triangle.verts[2] - triangle.verts[0];
    return Normal3f(cross(u, v).normalize());
}

struct VectorPair {
    Vector3f A;
    Vector3f B;
};

std::ostream& operator<<(std::ostream& out, const Vector3f& v) {
    out << "{" << v.x << ", " << v.y << ", " << v.z << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const Triangle& triangle) {
    out << "A" << triangle.verts[0] << ", B" << triangle.verts[1] << ", C" << triangle.verts[2];
    return out;
}

int main() {
    const VectorPair vPairs[] = {VectorPair({3.5, 0, 0}, {1.75, 3.5, 0}),
                                 VectorPair({3, -3, 1}, {4, 9, 3}),
                                 VectorPair({3, -3, 1}, {-12, 12, -4})};

    const Triangle triangles[] = {
        Triangle({-1.75, -1.75, -3}, {1.75, -1.75, -3}, {0, 1.75, -3}),
        Triangle({0, 0, -1}, {1, 0, 1}, {-1, 0, 1}),
        Triangle({0.56, 1.11, 1.23}, {0.44, -2.368, -0.54}, {-1.56, 0.15, -1.92})};

    std::cout << "[Cross products and parallelograms area]\n";
    for (const auto& [vecA, vecB] : vPairs) {
        std::cout << "Cross A" << vecA << " x B" << vecB << " = C" << cross(vecA, vecB)
                  << " [parallelogram area: " << calcParallelogramArea(vecA, vecB) << "]\n";
    }

    std::cout << "\n[Normals and triangles area]\n";
    for (const Triangle& triang : triangles) {
        std::cout << "Normal of traingle " << triang << " = N" << calcSurfaceNormal(triang)
                  << " [triangle area: " << calcTraingleArea(triang) << "]\n";
    }

    return 0;
}
