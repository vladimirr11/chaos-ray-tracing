#include "Triangle.h"
#include "Material.h"
#include "Statistics.h"

STAT(NUM_TRIANGLE_ISECT_TESTS, numTriIsectTests, triIsectTestRegisterer);
STAT(NUM_TRIANGLE_ISECTS, numTriIsects, isectRegisterer);

TriangleMesh::TriangleMesh(const std::vector<Point3f>& _vertPositions,
                           const std::vector<TriangleIndices>& _vertIndices,
                           const int32_t _materialIdx)
    : vertPositions(_vertPositions), vertIndices(_vertIndices), materialIdx(_materialIdx) {
    vertNormals.resize(vertPositions.size());
    for (size_t i = 0; i < vertIndices.size(); i++) {
        const Vector3f& A = vertPositions[vertIndices[i][0]];
        const Vector3f& B = vertPositions[vertIndices[i][1]];
        const Vector3f& C = vertPositions[vertIndices[i][2]];

        const Vector3f AB = B - A;
        const Vector3f AC = C - A;

        // computes face normal
        const Vector3f faceNormal = cross(AB, AC);

        // accumulates vertex normals for each triangle in the mesh
        vertNormals[vertIndices[i][0]] += faceNormal;
        vertNormals[vertIndices[i][1]] += faceNormal;
        vertNormals[vertIndices[i][2]] += faceNormal;
    }

    // normalizes each vertex normal and computes mesh bounds
    for (size_t i = 0; i < vertNormals.size(); i++) {
        vertNormals[i].normalize();
        bounds.expandBy(vertPositions[i]);
    }
}

std::vector<Triangle> TriangleMesh::getTriangles() const {
    std::vector<Triangle> triangles;
    triangles.reserve(vertIndices.size());
    std::for_each(vertIndices.begin(), vertIndices.end(),
                  [&](const TriangleIndices& currTriangleIndices) -> void {
                      triangles.emplace_back(currTriangleIndices, this);
                  });
    return triangles;
}

bool TriangleMesh::intersect(const Ray& ray, Intersection& isect) const {
    // early return if ray does not intersect with the object bounds
    if (!bounds.intersect(ray))
        return false;

    bool hasIntersect = false;
    for (size_t i = 0; i < vertIndices.size(); i++) {
        const Triangle triangle(vertIndices[i], this);
        if (triangle.intersectMT(ray, isect)) {
            if (isect.t < ray.tMax) {
                ray.tMax = isect.t;
            }
            hasIntersect = true;
        }
    }

    return hasIntersect;
}

bool TriangleMesh::intersectPrim(const Ray& ray, Intersection& isect) const {
    for (size_t i = 0; i < vertIndices.size(); i++) {
        const Triangle triangle(vertIndices[i], this);
        if (triangle.intersectMT(ray, isect)) {
            return true;
        }
    }
    return false;
}

bool Triangle::intersect(const Ray& ray, Intersection& isect) const {
    ++numTriIsectTests;
    // takes out the triangle's vertices
    const Vector3f& A = mesh->vertPositions[indices[0]];
    const Vector3f& B = mesh->vertPositions[indices[1]];
    const Vector3f& C = mesh->vertPositions[indices[2]];

    const Vector3f AB = B - A;
    const Vector3f AC = C - A;

    // computes face normal
    Vector3f N = cross(AB, AC);

    // area of the triangle x2
    float areaX2 = N.length();

    // ray projection on the plane normal
    const float rayProj = dot(N, ray.dir);

    // checks if the ray and plane are parallel
    if (fabs(rayProj) < EPSILON)
        return false;

    float D = -dot(N, A);

    // finds the distance from the ray's origin to the intersection point
    float t = -(dot(N, ray.origin) + D) / rayProj;

    // verifies that the triangle is not behind the ray's origin and the triangle
    // is ahead of the closest found so far
    if (t < 0 || t > ray.tMax)
        return false;

    // gets intersection point
    const Vector3f P = ray.at(t);

    // verifies if intersection point _P_ is on the left side of edge _E0_
    const Vector3f AP = P - A;
    const Vector3f E0crossAP = cross(AB, AP);
    if (dot(N, E0crossAP) < 0)
        return false;

    // verifies if intersection point _P_ is on the left side of edge _E1_
    const Vector3f E1 = C - B;
    const Vector3f BP = P - B;
    const Vector3f E1crossBP = cross(E1, BP);
    if (dot(N, E1crossBP) < 0)
        return false;

    // verifies if intersection point _P_ is on the left side of edge _E2_
    const Vector3f E2 = A - C;
    const Vector3f CP = P - C;
    const Vector3f E2crossCP = cross(E2, CP);
    if (dot(N, E2crossCP) < 0)
        return false;

    // takes out the triangle's vertex normals
    const Normal3f& v0N = mesh->vertNormals[indices[0]];
    const Normal3f& v1N = mesh->vertNormals[indices[1]];
    const Normal3f& v2N = mesh->vertNormals[indices[2]];

    // records intersection data
    isect.pos = P;
    isect.faceNormal = N.normalize();
    isect.t = t;
    isect.u = E2crossCP.length() / areaX2;
    isect.v = E0crossAP.length() / areaX2;
    isect.smoothNormal = v1N * isect.u + v2N * isect.v + v0N * (1 - isect.u - isect.v);
    isect.materialIdx = mesh->materialIdx;

    ++numTriIsects;

    return true;
}

bool Triangle::intersectMT(const Ray& ray, Intersection& isect) const {
    ++numTriIsectTests;
    // takes out the triangle's vertices
    const Vector3f& A = mesh->vertPositions[indices[0]];
    const Vector3f& B = mesh->vertPositions[indices[1]];
    const Vector3f& C = mesh->vertPositions[indices[2]];

    const Vector3f AB = B - A;
    const Vector3f AC = C - A;

    const Vector3f pVec = cross(ray.dir, AC);
    const float det = dot(AB, pVec);

    // ray and triangle are parallel if determinant is close to 0
    if (fabs(det) < EPSILON)
        return false;

    const float invDet = 1 / det;

    // computes _u_ parameter and test if it's in bounds
    const Vector3f tVec = ray.origin - A;
    const float u = dot(tVec, pVec) * invDet;
    if (u < 0 || u > 1)
        return false;

    // computes _v_ parameter and test if it's in bounds
    const Vector3f qVec = cross(tVec, AB);
    const float v = dot(ray.dir, qVec) * invDet;
    if (v < 0 || u + v > 1)
        return false;

    // computes _t_ parameter and test that the triangle is not behind the
    // ray origin and the triangle is ahead of the closest found so far
    const float t = dot(AC, qVec) * invDet;
    if (t < 0 || t > ray.tMax)
        return false;

    // gets intersection point
    const Vector3f P = ray.at(t);

    // computes face normal
    Vector3f N = cross(AB, AC);

    // takes out the triangle's vertex normals
    const Normal3f& v0N = mesh->vertNormals[indices[0]];
    const Normal3f& v1N = mesh->vertNormals[indices[1]];
    const Normal3f& v2N = mesh->vertNormals[indices[2]];

    // records intersection data
    isect.pos = P;
    isect.faceNormal = N.normalize();
    isect.t = t;
    isect.u = u;
    isect.v = v;
    isect.smoothNormal = v1N * isect.u + v2N * isect.v + v0N * (1 - isect.u - isect.v);
    isect.materialIdx = mesh->materialIdx;

    ++numTriIsects;

    return true;
}
