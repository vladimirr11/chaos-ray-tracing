#include "Triangle.h"
#include "Material.h"

TriangleMesh::TriangleMesh(const std::vector<Point3f>& _vertsPositions,
                           const std::vector<TriangleIndices>& _vertsIndices,
                           const int32_t _materialIdx)
    : vertsPositions(_vertsPositions), vertsIndices(_vertsIndices), materialIdx(_materialIdx) {
    vertsNormals.resize(vertsPositions.size());
    for (size_t i = 0; i < vertsIndices.size(); i++) {
        const Vector3f& v0 = vertsPositions[vertsIndices[i][0]];
        const Vector3f& v1 = vertsPositions[vertsIndices[i][1]];
        const Vector3f& v2 = vertsPositions[vertsIndices[i][2]];

        const Vector3f v0v1 = v1 - v0;
        const Vector3f v0v2 = v2 - v0;

        // compute face normal
        const Vector3f faceN = cross(v0v1, v0v2);

        // accumulate vertex normals for each triangle in the mesh
        vertsNormals[vertsIndices[i][0]] += faceN;
        vertsNormals[vertsIndices[i][1]] += faceN;
        vertsNormals[vertsIndices[i][2]] += faceN;
    }

    for (size_t i = 0; i < vertsNormals.size(); i++) {
        vertsNormals[i].normalize();
    }
}

std::vector<Triangle> TriangleMesh::getTriangles() const {
    std::vector<Triangle> triangles;
    triangles.reserve(vertsIndices.size());
    std::for_each(vertsIndices.begin(), vertsIndices.end(),
                  [&](const TriangleIndices& currTriangleIndices) -> void {
                      triangles.emplace_back(currTriangleIndices, this);
                  });
    return triangles;
}

bool TriangleMesh::intersect(const Ray& ray, Intersection& isect) const {
    Intersection closestPrim;
    bool hasIntersect = false;
    float tMin = FLT_MAX;
    for (size_t i = 0; i < vertsIndices.size(); i++) {
        const Triangle triangle(vertsIndices[i], this);
        if (triangle.intersect(ray, tMin, isect)) {
            if (isect.t < closestPrim.t) {
                closestPrim = isect;
                tMin = isect.t;
            }
            hasIntersect = true;
        }
    }

    if (hasIntersect)
        isect = closestPrim;

    return hasIntersect;
}

bool TriangleMesh::intersectPrim(const Ray& ray, Intersection& isect) const {
    Intersection closestPrim;
    float tMin = FLT_MAX;
    for (size_t i = 0; i < vertsIndices.size(); i++) {
        const Triangle triangle(vertsIndices[i], this);
        if (triangle.intersect(ray, tMin, closestPrim) && closestPrim.t < isect.t) {
            return true;
        }
    }
    return false;
}

bool Triangle::intersect(const Ray& ray, float tMin, Intersection& isect) const {
    // take out the triangle's vertices
    const Vector3f& A = mesh->vertsPositions[indices[0]];
    const Vector3f& B = mesh->vertsPositions[indices[1]];
    const Vector3f& C = mesh->vertsPositions[indices[2]];

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
    const Normal3f& v0N = mesh->vertsNormals[indices[0]];
    const Normal3f& v1N = mesh->vertsNormals[indices[1]];
    const Normal3f& v2N = mesh->vertsNormals[indices[2]];

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
