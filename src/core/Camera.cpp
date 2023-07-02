#include "Camera.h"

Camera::Camera(const Point3f& _loofFrom, const Point3f& _lookAt, const int _imageWidth,
               const int _imageHeight)
    : lookFrom(_loofFrom), lookAt(_lookAt), imageWidth(_imageWidth), imageHeight(_imageHeight) {
    const Vector3f vUp{0.f, 1.f, 0.f};  ///< Up direction in world space
    const Vector3f zVec = (lookFrom - lookAt).normalize();
    const Vector3f xVec = cross(vUp, zVec).normalize();
    const Vector3f yVec = cross(zVec, xVec);
    aspectRatio = imageWidth / (float)imageHeight;

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

void Camera::init(const Point3f& _lookFrom, const Matrix3x3& _rotationM, const int _imageWidth,
                  const int _imageHeight, const Point3f& _lookAt) {
    lookFrom = _lookFrom;
    lookAt = _lookAt;
    rotationM = _rotationM;
    imageWidth = _imageWidth;
    imageHeight = _imageHeight;
    aspectRatio = imageWidth / (float)imageHeight;
}

Ray Camera::getRay(const uint32_t x, const uint32_t y) const {
    const float ndcX = (y + 0.5f) / imageWidth;
    const float ndcY = (x + 0.5f) / imageHeight;
    const float screenX = (2.f * ndcX - 1.f) * aspectRatio;
    const float screenY = (1.f - 2.f * ndcY);
    const Vector3f rayDir(screenX, screenY, -1);
    return Ray(lookFrom, (rayDir * rotationM).normalize());
}

void Camera::truck(const float sidewayStep) {
    lookFrom += rotationM * Vector3f(sidewayStep, 0.f, 0.f);
}

void Camera::boom(const float upDownStep) {
    lookFrom += rotationM * Vector3f(0.f, upDownStep, 0.f);
}

void Camera::dolly(const float frontBackStep) {
    lookFrom += rotationM * Vector3f(0.f, 0.f, frontBackStep);
}

void Camera::move(const Vector3f& moveV) { lookFrom += rotationM * moveV; }

void Camera::tilt(const float thetaDeg) {
    const Matrix3x3 xAxisRotationMatrix = rotateX(thetaDeg);
    rotationM = rotationM * xAxisRotationMatrix;
}

void Camera::pan(const float thetaDeg) {
    const Matrix3x3 yAxisRotationMatrix = rotateY(thetaDeg);
    rotationM = rotationM * yAxisRotationMatrix;
}

void Camera::roll(const float thetaDeg) {
    const Matrix3x3 zAxisRotationMatrix = rotateZ(thetaDeg);
    rotationM = rotationM * zAxisRotationMatrix;
}
