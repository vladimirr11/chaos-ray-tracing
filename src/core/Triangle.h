#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <array>
#include <cstdint>
#include <vector>
#include "AABBox.h"

using TriangleIndices = std::array<int, 3>;

class Material;

/// @brief Keeps data for ray-triangle intersection
struct Intersection {
    Vector3f pos;           ///< Intersection position
    Normal3f faceNormal;    ///< Face normal
    Normal3f smoothNormal;  ///< Computed smooth normal using barycentric coordinates
    float t = MAX_FLOAT;    ///< Distance from the origin of the ray to the intersection point
    float u, v;             ///< Barycentric coordinates
    int32_t materialIdx;    ///< Material index of the intersection
};

struct TriangleMesh;

struct Triangle {
    const int* indices;        ///< Indices of the triangle's vertices in the mesh
    const TriangleMesh* mesh;  ///< The triangle's owner mesh

    Triangle() = delete;

    Triangle(const TriangleIndices& _indices, const TriangleMesh* _mesh)
        : indices(_indices.data()), mesh(_mesh){};

    /// @brief Verifies if ray intersect with the triangle
    bool intersect(const Ray& ray, Intersection& isect) const;

    /// @brief Verifies if ray intersect with the triangle using Moller-Trumbor method
    bool intersectMT(const Ray& ray, Intersection& isect) const;
};

/// @brief Triangle mesh class that stores information for each object in the scene
struct TriangleMesh {
    std::vector<Point3f> vertPositions;        ///< Positions of the vertices in world space
    std::vector<TriangleIndices> vertIndices;  ///< Keeps indices for each triangle in the mesh
    std::vector<Normal3f> vertNormals;         ///< Pre-computed normals for each vertex in the mesh
    int32_t materialIdx;  ///< Index from the materials list that characterise current object (mesh)
    BBox bounds;          ///< The bounding box of the mesh

    TriangleMesh() = delete;

    /// @brief Initializes triangle mesh from vertex positions, vertex indices, and material index
    TriangleMesh(const std::vector<Point3f>& _vertPositions,
                 const std::vector<TriangleIndices>& _vertIndices, const int32_t _materialIdx);

    /// @brief Retrieves list of all triangles in the mesh upon request
    std::vector<Triangle> getTriangles() const;

    /// @brief Intersects ray with the mesh and records closest intersection point if any
    bool intersect(const Ray& ray, Intersection& isect) const;

    /// @brief Verifies if ray intersects with the mesh. Returns true on first intersection, false
    /// if no ray-triangle intersection found
    bool intersectPrim(const Ray& ray, Intersection& isect) const;
};

#endif  // !TRIANGLE_H
