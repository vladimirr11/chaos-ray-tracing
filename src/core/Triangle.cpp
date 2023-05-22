#include "Triangle.h"
#include "Material.h"

TriangleMesh::TriangleMesh(const std::vector<Point3f>& _vertexPositions,
                           const std::vector<TriangleIndices>& _vertexIndices,
                           const int32_t _materialIdx)
    : vertexPositions(_vertexPositions), vertexIndices(_vertexIndices), materialIdx(_materialIdx) {
    vertexNormals.resize(vertexPositions.size());
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        const Vector3f& A = vertexPositions[vertexIndices[i][0]];
        const Vector3f& B = vertexPositions[vertexIndices[i][1]];
        const Vector3f& C = vertexPositions[vertexIndices[i][2]];

        const Vector3f AB = B - A;
        const Vector3f AC = C - A;

        // compute face normal
        const Vector3f faceN = cross(AB, AC);

        // accumulate vertex normals for each triangle in the mesh
        vertexNormals[vertexIndices[i][0]] += faceN;
        vertexNormals[vertexIndices[i][1]] += faceN;
        vertexNormals[vertexIndices[i][2]] += faceN;
    }

    // normalize vertex normals and compute mesh bounds in the same loop
    for (size_t i = 0; i < vertexNormals.size(); i++) {
        vertexNormals[i].normalize();
        bounds.expandBy(vertexPositions[i]);
    }
}

std::vector<Triangle> TriangleMesh::getTriangles() const {
    std::vector<Triangle> triangles;
    triangles.reserve(vertexIndices.size());
    std::for_each(vertexIndices.begin(), vertexIndices.end(),
                  [&](const TriangleIndices& currTriangleIndices) -> void {
                      triangles.emplace_back(currTriangleIndices, this);
                  });
    return triangles;
}

bool TriangleMesh::intersect(const Ray& ray, Intersection& isect) const {
    // early return if ray does not intersect with the mesh bounds
    if (!bounds.intersect(ray))
        return false;

    Intersection* closestPrim = nullptr;  // copy only the address not the entire structure
    bool hasIntersect = false;
    float tMin = MAX_FLOAT;
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        const Triangle triangle(vertexIndices[i], this);
        if (triangle.intersect(ray, tMin, isect)) {
            if (isect.t < tMin) {
                closestPrim = &isect;
                tMin = isect.t;
            }
            hasIntersect = true;
        }
    }

    if (hasIntersect)
        isect = *closestPrim;

    return hasIntersect;
}

bool TriangleMesh::intersectPrim(const Ray& ray, Intersection& isect) const {
    Intersection closestPrim;
    float tMin = FLT_MAX;
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        const Triangle triangle(vertexIndices[i], this);
        if (triangle.intersect(ray, tMin, closestPrim) && closestPrim.t < isect.t) {
            return true;
        }
    }
    return false;
}

bool Triangle::intersect(const Ray& ray, float tMin, Intersection& isect) const {
    // take out the triangle's vertices
    const Vector3f& A = mesh->vertexPositions[indices[0]];
    const Vector3f& B = mesh->vertexPositions[indices[1]];
    const Vector3f& C = mesh->vertexPositions[indices[2]];

    const Vector3f AB = B - A;
    const Vector3f AC = C - A;

    // compute face normal
    Vector3f N = cross(AB, AC);

    // area of the triangle x2
    float areaX2 = N.length();

    // check if the ray and plane are parallel
    if (fabs(dot(N, ray.dir)) < EPSILON)
        return false;

    float D = -dot(N, A);

    // find distance from the ray's origin to the intersection point
    float t = -(dot(N, ray.origin) + D) / dot(N, ray.dir);

    // verify that the triangle is not behind the ray's origin and the triangle
    // is ahead of the closest found so far
    if (t < 0 || t > tMin)
        return false;

    // get intersection point
    const Vector3f P = ray.at(t);

    // verify if intersection point _P_ is on the left side of edge _E0_
    const Vector3f E0 = AB;
    const Vector3f AP = P - A;
    const Vector3f E0crossAP = cross(E0, AP);
    if (dot(N, E0crossAP) < 0)
        return false;

    // verify if intersection point _P_ is on the left side of edge _E1_
    const Vector3f E1 = C - B;
    const Vector3f BP = P - B;
    const Vector3f E1crossBP = cross(E1, BP);
    if (dot(N, E1crossBP) < 0)
        return false;

    // verify if intersection point _P_ is on the left side of edge _E2_
    const Vector3f E2 = A - C;
    const Vector3f CP = P - C;
    const Vector3f E2crossCP = cross(E2, CP);
    if (dot(N, E2crossCP) < 0)
        return false;

    // take out the triangle's vertex normals
    const Normal3f& v0N = mesh->vertexNormals[indices[0]];
    const Normal3f& v1N = mesh->vertexNormals[indices[1]];
    const Normal3f& v2N = mesh->vertexNormals[indices[2]];

    // record intersection data
    isect.pos = P;
    isect.faceN = N.normalize();
    isect.t = t;
    isect.u = E2crossCP.length() / areaX2;
    isect.v = E0crossAP.length() / areaX2;
    isect.smoothN = v1N * isect.u + v2N * isect.v + v0N * (1 - isect.u - isect.v);
    isect.materialIdx = mesh->materialIdx;

    return true;
}
