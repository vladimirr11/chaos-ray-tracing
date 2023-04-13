#ifndef CAMERA_H
#define CAMERA_H

#include "Matrix3x3.h"

class Camera {
public:
    /// @brief Initialize camera position, camera orientation, and rotation matrix
    Camera(const Point3f& _loofFrom, const Point3f& _lookAt, const Vector3f& _vUp = {0.f, 1.f, 0.f})
        : lookFrom(_loofFrom), lookAt(_lookAt), vUp(_vUp) {
        const Vector3f zVec = (lookFrom - lookAt).normalize();
        const Vector3f xVec = cross(vUp, zVec).normalize();
        const Vector3f yVec = cross(zVec, xVec);

        rotationM.m[0][0] = xVec.x;
        rotationM.m[0][1] = xVec.y;
        rotationM.m[0][2] = xVec.z;
        rotationM.m[1][0] = yVec.x;
        rotationM.m[1][1] = yVec.y;
        rotationM.m[1][2] = yVec.z;
        rotationM.m[2][0] = zVec.x;
        rotationM.m[2][1] = zVec.y;
        rotationM.m[2][2] = zVec.z;
    }

    /// @brief Generate ray for each pixel in the scene given _x_ and _y_ raster coordinates
    Ray getRay(const uint32_t x, const uint32_t y) const {
        const float ndcX = (y + 0.5f) / IMG_WIDTH;
        const float ndcY = (x + 0.5f) / IMG_HEIGHT;
        const float screenX = (2.f * ndcX - 1.f) * ASPECT_RATIO;
        const float screenY = (1.f - 2.f * ndcY);
        const Vector3f rayDir(screenX, screenY, -1);
        return Ray(lookFrom, (rayDir * rotationM).normalize());
    }

    /// @brief Move the camera in sideway direction
    void truck(const float sidewayStep) { lookFrom += rotationM * Vector3f(sidewayStep, 0.f, 0.f); }

    /// @brief Move the camera upward & downward
    void boom(const float upDownStep) { lookFrom += rotationM * Vector3f(0.f, upDownStep, 0.f); }

    /// @brief Move the camera forward & backward
    void dolly(const float frontBackStep) {
        lookFrom += rotationM * Vector3f(0.f, 0.f, frontBackStep);
    }

    /// @brief Rotate the camera around X-axis with _thetaDeg_ degrees 
    void tilt(const float thetaDeg) {
        const Matrix3x3 xAxisRotationMatrix = rotateX(thetaDeg);
        rotationM = rotationM * xAxisRotationMatrix;
    }

    /// @brief Rotate the camera around Y-axis with _thetaDeg_ degrees
    void pan(const float thetaDeg) {
        const Matrix3x3 yAxisRotationMatrix = rotateY(thetaDeg);
        rotationM = rotationM * yAxisRotationMatrix;
    }

    /// @brief Rotate the camera around Z-axis with _thetaDeg_ degrees
    void roll(const float thetaDeg) {
        const Matrix3x3 zAxisRotationMatrix = rotateZ(thetaDeg);
        rotationM = rotationM * zAxisRotationMatrix;
    }

    /// @brief Cancel last rotation
    void undoLastRotation() { rotationM = rotationM * inverse(rotationM); }

    Vector3f getLookFrom() const { return lookFrom; }

    Vector3f getLookAt() const { return lookAt; }

    Matrix3x3 getRotationMatrix() const { return rotationM; }

private:
    Point3f lookFrom;     ///< Camera position in world space
    Point3f lookAt;       ///< Camera orientation in world space
    Vector3f vUp;         ///< Up direction in world space
    Matrix3x3 rotationM;  ///< Rotation matrix of the camera's basis vectors
};

#endif  // !CAMERA_H
