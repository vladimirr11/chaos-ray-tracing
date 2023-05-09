#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include "Triangle.h"

struct Scene;

enum MaterialType { DIFFUSE, REFLECTIVE };

/// @brief Abstract base class for a scene material
struct Material {
    /// @brief Shade an intersection
    /// @param ray The ray that created the intersection
    /// @param scene The scene that is rendered
    /// @param isectHit Data for the intersection
    /// @return Computed color for the intersection
    virtual Color3f shade(const Ray& ray, const Scene* scene, Intersection& isectHit) const = 0;

    /// @brief Returns the type of the material
    virtual MaterialType getType() const = 0;

    virtual ~Material() {}
};

struct DiffuseM : public Material {
    Color3f albedo;
    bool smoothShading = false;
    const MaterialType type = DIFFUSE;

    DiffuseM(const Color3f& _albedo, bool _smoothShading)
        : albedo(_albedo), smoothShading(_smoothShading) {}

    Color3f shade(const Ray& incidentR, const Scene* scene, Intersection& isectHit) const override;

    MaterialType getType() const override { return type; }
};

struct ReflectiveM : public Material {
    Color3f albedo;
    bool smoothShading = false;
    const MaterialType type = REFLECTIVE;

    ReflectiveM(const Color3f& _albedo, bool _smoothShading)
        : albedo(_albedo), smoothShading(_smoothShading) {}

    Color3f shade(const Ray& ray, const Scene* scene, Intersection& isectHit) const override;

    MaterialType getType() const override { return type; }

    /// @brief Scatter a ray in the scene (potentially recursively) until a background or other
    /// non-reflective material type is hitted
    /// @return The attenuated color for the last non-reflective intersection or the color of the
    /// background if max ray depth is reached
    Color3f scatter(const Ray& incidentR, const Scene* scene, Intersection& isectData,
                    int rayDepth = 0) const;
};

/// @brief Constructs the actual material on the heap
inline static std::unique_ptr<Material> makeMaterial(const std::string_view& materialType,
                                                     const Color3f& albedo, bool smoothShading) {
    std::unique_ptr<Material> materialPtr;
    if (materialType == "diffuse") {
        materialPtr = std::make_unique<DiffuseM>(albedo, smoothShading);
    } else if (materialType == "reflective") {
        materialPtr = std::make_unique<ReflectiveM>(albedo, smoothShading);
    }
    return materialPtr;
}

#endif  // !MATERIAL_H
