#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <array>
#include <vector>
#include "Constants.h"
#include "Ray.h"

using TriangleIndices = std::array<int, 3>;

/// @brief Keeps data for ray-triangle intersection
struct Intersection {
    Vector3f position;
    Vector3f normal;
    float t = 0.001f;  ///< Distance from the origin of the ray to the intersection point
};

struct TriangleMesh;

struct Triangle {
    const int* indices;        ///< Holds the indices of the triangle's vertices in the mesh
    const TriangleMesh* mesh;  ///< The triangle's owner mesh

    Triangle() = delete;

    Triangle(const TriangleIndices& _indices, const TriangleMesh* _mesh)
        : indices(_indices.data()), mesh(_mesh){};

    /// @brief Verifies if ray intersect with the triangle.
    bool intersect(const Ray& ray, Intersection& isect) const;
};

/// @brief Triangle mesh class that stores each vertex position in 3D and their corresponding
/// indices in the mesh.
struct TriangleMesh {
    std::vector<Point3f> vertsPositions;        ///< Vertices positions in 3D
    std::vector<TriangleIndices> vertsIndices;  ///< Keeps indices for each triangle in the mesh

    TriangleMesh() {}

    TriangleMesh(const std::vector<Point3f>& _vertsPositions,
                 const std::vector<TriangleIndices>& _vertsIndices)
        : vertsPositions(_vertsPositions), vertsIndices(_vertsIndices) {}

    /// @brief Retrieves list of all triangles in the mesh.
    std::vector<Triangle> getTriangles() const {
        std::vector<Triangle> triangles;
        std::for_each(vertsIndices.begin(), vertsIndices.end(),
                      [&](const TriangleIndices& currTriangleIndices) -> void {
                          triangles.emplace_back(currTriangleIndices, this);
                      });

        return triangles;
    }
};

bool Triangle::intersect(const Ray& ray, Intersection& isect) const {
    // take out the triangle's vertices
    const Vector3f& A = mesh->vertsPositions[indices[0]];
    const Vector3f& B = mesh->vertsPositions[indices[1]];
    const Vector3f& C = mesh->vertsPositions[indices[2]];

    const Vector3f AB = B - A;
    const Vector3f AC = C - A;

    // get normal not normalized
    const Vector3f N = cross(AB, AC);

    // check if the ray and plane are parallel
    if (fabs(dot(N, ray.dir)) < EPSILON)
        return false;

    float D = -dot(N, A);

    // find distance from the ray's origin to the intersection point
    float t = -(dot(N, ray.origin) + D) / dot(N, ray.dir);

    // verify if the triangle is behind the ray
    if (t < 0)
        return false;

    // get intersection point
    const Vector3f P = ray.at(t);

    // verify if intersection point _P_ is on the left side of edge _E0_
    const Vector3f E0 = AB;
    const Vector3f AP = P - A;
    if (dot(N, cross(E0, AP)) < 0)
        return false;

    // verify if intersection point _P_ is on the left side of edge _E1_
    const Vector3f E1 = C - B;
    const Vector3f BP = P - B;
    if (dot(N, cross(E1, BP)) < 0)
        return false;

    // verify if intersection point _P_ is on the left side of edge _E2_
    const Vector3f E2 = A - C;
    const Vector3f CP = P - C;
    if (dot(N, cross(E2, CP)) < 0)
        return false;

    // record intersection data
    isect.position = P;
    isect.normal = N;
    isect.t = t;

    return true;
}

#endif  // !TRIANGLE_H
