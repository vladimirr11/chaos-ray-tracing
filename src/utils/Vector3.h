#ifndef VECTOR3_H
#define VECTOR3_H

#include <cassert>
#include <cfloat>
#include <cmath>

template <typename T>
struct Vector3 {
    union {
        struct {
            T x, y, z;
        };
        T v3[3];
    };

    Vector3() : x(0), y(0), z(0) {}

    Vector3(const T p1) : x(p1), y(p1), z(p1) {}

    Vector3(const T _x, const T _y, const T _z) : x(_x), y(_y), z(_z) {}

    T& operator[](const int idx) {
        assert(idx >= 0 && idx < 3);
        return v3[idx];
    }

    const T& operator[](const int idx) const {
        assert(idx >= 0 && idx < 3);
        return v3[idx];
    }

    Vector3<T> operator+(const Vector3<T>& vec3) const {
        return Vector3(x + vec3.x, y + vec3.y, z + vec3.z);
    }

    Vector3<T>& operator+=(const Vector3<T>& vec3) {
        for (int i = 0; i < 3; i++) {
            v3[i] += vec3.v3[i];
        }
        return *this;
    }

    Vector3<T> operator-(const Vector3<T>& vec3) const {
        return Vector3(x - vec3.x, y - vec3.y, z - vec3.z);
    }

    Vector3<T>& operator-=(const Vector3<T>& vec3) {
        for (int i = 0; i < 3; i++) {
            v3[i] -= vec3.v3[i];
        }
        return *this;
    }

    template <typename U>
    Vector3<T> operator*(U s) const {
        return Vector3<T>(x * s, y * s, z * s);
    }

    template <typename U>
    Vector3<T>& operator*=(U s) {
        for (int i = 0; i < 3; i++) {
            v3[i] *= s;
        }
        return *this;
    }

    template <typename U>
    Vector3<T> operator/(U f) const {
        float invF = (float)1 / f;
        return Vector3(x * invF, y * invF, z * invF);
    }

    template <typename U>
    Vector3<T>& operator/=(U f) {
        float invF = (float)1 / f;
        for (int i = 0; i < 3; i++) {
            v3[i] *= invF;
        }
        return *this;
    }

    bool operator==(const Vector3<T>& vec3) const {
        return x == vec3.x && y == vec3.y && z == vec3.z;
    }

    bool operator!=(const Vector3<T>& vec3) const { return !(*this == vec3); }

    Vector3<T> operator-() const { return Vector3<T>(-x, -y, -z); }

    float lengthSquared() const { return x * x + y * y + z * z; }

    float length() const { return sqrtf(lengthSquared()); }

    Vector3<T> normalize() { return *this /= length(); }
};

template <typename T, typename U>
inline Vector3<T> operator*(U s, const Vector3<T>& v) {
    return v * s;
}

template <typename T>
inline Vector3<T> normalize(Vector3<T>& vec3) {
    return vec3 / vec3.length();
}

template <typename T>
inline T dot(const Vector3<T>& v1, const Vector3<T>& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
inline Vector3<T> cross(const Vector3<T>& v1, const Vector3<T>& v2) {
    return Vector3<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
                      v1.x * v2.y - v1.y * v2.x);
}

typedef Vector3<float> Vector3f;
typedef Vector3<float> Point3f;
typedef Vector3<float> Color3f;
typedef Vector3<float> Normal3f;
typedef Vector3<int> Vector3i;
typedef Vector3<int> Point3i;
typedef Vector3<int> Color3i;
typedef Vector3<int> Normal3i;

#endif  // !VECTOR3_H
