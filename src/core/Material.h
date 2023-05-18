#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include "Triangle.h"

struct Scene;

enum class MaterialType { DIFFUSE, REFLECTIVE, UNDEFINED };

/// @brief Class for a scene material
struct Material {
    Color3f albedo;
    bool smoothShading = false;
    const MaterialType type = MaterialType::UNDEFINED;

    Material(const Color3f& _albedo, bool _smoothShading, const MaterialType _type)
        : albedo(_albedo), smoothShading(_smoothShading), type(_type) {}

    /// @brief Shade an intersection
    /// @param ray The ray that created the intersection
    /// @param scene The scene that is rendered
    /// @param isectData Data for the intersection
    /// @return Computed color for the intersection
    Color3f shade(const Ray& ray, const Scene* scene, Intersection& isectData) const;
};

inline static Material makeMaterial(std::string_view materialType, const Color3f& albedo,
                                    bool smoothShading) {
    MaterialType mtype = MaterialType::UNDEFINED;
    if (materialType == "diffuse") {
        mtype = MaterialType::DIFFUSE;
    } else if (materialType == "reflective") {
        mtype = MaterialType::REFLECTIVE;
    } else {
        Assert(mtype != MaterialType::UNDEFINED && "Recieved unsupported material type.");
    }
    return Material(albedo, smoothShading, mtype);
}

Color3f shadeDiffuseM(const Ray& ray, const Scene* scene, Intersection& isectData);

Color3f shadeReflectiveM(const Ray& ray, const Scene* scene, Intersection& isectData);

#endif  // !MATERIAL_H
