#ifndef CAMERA_H
#define CAMERA_H

#include "Matrix3x3.h"

class Camera {
public:
    /// @brief Default constructor that allows initialization of camera through init method
    Camera() = default;

    /// @brief Initialize camera data members by given position and orientation in world space
    Camera(const Point3f& _loofFrom, const Point3f& _lookAt, const int _imageWidth = IMG_WIDTH,
           const int _imageHeight = IMG_HEIGHT);

    /// @brief Initialize camera data members by given position, rotation matrix, and scene
    /// dimensions
    void init(const Point3f& _lookFrom, const Matrix3x3& _rotationM, const int _imageWidth,
              const int _imageHeight);

    /// @brief Generate ray for each pixel in the scene by given _x_ and _y_ raster coordinates
    Ray getRay(const uint32_t x, const uint32_t y) const;

    /// @brief Move the camera in sideway direction
    void truck(const float sidewayStep);

    /// @brief Move the camera upward & downward
    void boom(const float upDownStep);

    /// @brief Move the camera forward & backward
    void dolly(const float frontBackStep);

    /// @brief Move the camera in world space by given vector with x, y, and z steps
    void move(const Vector3f& moveV);

    /// @brief Rotate the camera around X-axis with _thetaDeg_ degrees
    void tilt(const float thetaDeg);

    /// @brief Rotate the camera around Y-axis with _thetaDeg_ degrees
    void pan(const float thetaDeg);

    /// @brief Rotate the camera around Z-axis with _thetaDeg_ degrees
    void roll(const float thetaDeg);

    /// @brief Set up the camera position in world space
    void setLookFrom(const Vector3f& position);

    /// @brief Set camera orientation at _lookAt_ position
    void setLookAt(const Vector3f& lookAt);

    Vector3f getLookFrom() const { return lookFrom; }

    Matrix3x3 getRotationMatrix() const { return rotationM; }

private:
    Point3f lookFrom;     ///< Camera position in world space
    Matrix3x3 rotationM;  ///< Rotation matrix of the camera's basis vectors
    int imageWidth;       ///< Scene width
    int imageHeight;      ///< Scene height
    float aspectRatio;    ///< Aspect ratio of the scene
};

#endif  // !CAMERA_H
