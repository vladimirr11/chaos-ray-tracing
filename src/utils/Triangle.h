#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vector3.h"

struct Triangle {
    Vector3f verts[3];

    Triangle() = delete;

    Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2) {
        verts[0] = v0;
        verts[1] = v1;
        verts[2] = v2;
    };
};

#endif  // !TRIANGLE_H
