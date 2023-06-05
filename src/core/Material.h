#ifndef MATERIAL_H
#define MATERIAL_H

#include "Triangle.h"
#include <string>

struct Scene;

enum class MaterialType : uint8_t { DIFFUSE, REFLECTIVE, REFRACTIVE, CONSTANT, UNDEFINED };

/// @brief Union that holds the distinctive reflective/refractive property
/// of the concrete material type. Index of refraction for refractive materials
/// and albedo for the rest
union MaterialProperty {
    Color3f albedo = Color3f(0);
    float ior;
};

/// @brief Class for a scene material
struct Material {
    const MaterialProperty property;
    bool smoothShading = false;
    const MaterialType type = MaterialType::UNDEFINED;

    Material(const MaterialProperty& _property, bool _smoothShading, const MaterialType _type)
        : property(_property), smoothShading(_smoothShading), type(_type) {}

    /// @brief Shade an intersection
    /// @param ray The ray that created the intersection
    /// @param scene The scene that is rendered
    /// @param isectData Data for the intersection
    /// @return Computed color for the intersection
    Color3f shade(const Ray& ray, const Scene* scene, Intersection& isectData) const;
};

inline static Material makeMaterial(std::string_view materialType, const MaterialProperty& property,
                                    bool smoothShading) {
    MaterialType mtype = MaterialType::UNDEFINED;
    if (materialType == "diffuse") {
        mtype = MaterialType::DIFFUSE;
    } else if (materialType == "reflective") {
        mtype = MaterialType::REFLECTIVE;
    } else if (materialType == "refractive") {
        mtype = MaterialType::REFRACTIVE;
    } else if (materialType == "constant") {
        mtype = MaterialType::CONSTANT;
    } else {
        Assert(mtype != MaterialType::UNDEFINED &&
               "makeMaterial() recieved unsupported material type.");
    }
    return Material(property, smoothShading, mtype);
}

Color3f shadeDiffuse(const Ray& ray, const Scene* scene, Intersection& isectData);

Color3f shadeReflective(const Ray& ray, const Scene* scene, Intersection& isectData);

Color3f shadeRefractive(const Ray& ray, const Scene* scene, Intersection& isectData);

Color3f shadeConstant(const Ray& ray, const Scene* scene, Intersection& isectData);

#endif  // !MATERIAL_H
