#include "Material.h"
#include "Scene.h"
#include "Statistics.h"

Color3f Material::shade(const Ray& ray, const Scene* scene, Intersection& isectData) const {
    Color3f shadeColor;
    const MaterialType hittedMaterialType = scene->getMaterials()[isectData.materialIdx].type;
    if (hittedMaterialType == MaterialType::DIFFUSE) {
        shadeColor = shadeDiffuse(ray, scene, isectData);
    } else if (hittedMaterialType == MaterialType::REFLECTIVE) {
        shadeColor = shadeReflective(ray, scene, isectData);
    } else if (hittedMaterialType == MaterialType::REFRACTIVE) {
        shadeColor = shadeRefractive(ray, scene, isectData);
    } else if (hittedMaterialType == MaterialType::CONSTANT) {
        shadeColor = shadeConstant(ray, scene, isectData);
    } else {
        Assert(false && "Material::shade() received unsupported material type.");
    }
    return shadeColor;
}

Color3f shadeDiffuse([[maybe_unused]] const Ray& ray, const Scene* scene, Intersection& isectData) {
    Color3f hitColor;
    const std::vector<Light>& lights = scene->getLights();
    const Color3f& albedo = scene->getMaterials()[isectData.materialIdx].property.albedo;
    bool smoothShading = scene->getMaterials()[isectData.materialIdx].smoothShading;
    const Normal3f isectNormal = smoothShading ? isectData.smoothNormal : isectData.faceNormal;
    for (const Light& light : lights) {
        Vector3f lightDir = light.getPosition() - isectData.pos;
        const float lightDist = lightDir.length();
        const float lightArea = calcSphereArea(lightDist);
        const Vector3f lightDirN = normalize(lightDir);
        const float cosTheta = std::max(0.f, dot(lightDirN, isectNormal));
        const Ray shadowRay(isectData.pos + isectNormal * SHADOW_BIAS, lightDirN);
        shadowRay.tMax = lightDist;
        Color3f perLightColor;
        if (!scene->intersectPrim(shadowRay)) {
            perLightColor = Color3f(light.getIntensity() / lightArea * albedo * cosTheta);
        }
        hitColor += perLightColor;
    }

    return hitColor;
}

Color3f shadeReflective(const Ray& ray, const Scene* scene, Intersection& isectData) {
    bool smoothShading = scene->getMaterials()[isectData.materialIdx].smoothShading;
    Normal3f surfNormal = smoothShading ? isectData.smoothNormal : isectData.faceNormal;
    const Vector3f reflectedDir = reflect(ray.dir, surfNormal);
    Ray reflectedRay = Ray(isectData.pos + surfNormal * REFLECTION_BIAS, reflectedDir);
    const Color3f& albedo = scene->getMaterials()[isectData.materialIdx].property.albedo;
    if (ray.depth <= MAX_RAY_DEPTH && scene->intersect(reflectedRay, isectData)) {
        const Material* hittedMaterial = &scene->getMaterials()[isectData.materialIdx];
        if (hittedMaterial->type == MaterialType::REFLECTIVE) {
            reflectedRay.depth = ray.depth + 1;
            return hittedMaterial->shade(reflectedRay, scene, isectData);
        } else {
            return albedo * hittedMaterial->shade(reflectedRay, scene, isectData);
        }
    }
    return albedo * scene->getBackground();
}

Color3f shadeRefractive(const Ray& ray, const Scene* scene, Intersection& isectData) {
    if (ray.depth <= MAX_RAY_DEPTH) {
        bool smoothShading = scene->getMaterials()[isectData.materialIdx].smoothShading;
        Normal3f surfNormal = smoothShading ? isectData.smoothNormal : isectData.faceNormal;
        float cosThetaI = clamp(-1.f, 1.f, dot(ray.dir, surfNormal));
        const float ior = scene->getMaterials()[isectData.materialIdx].property.ior;
        float etaI = 1.f, etaT = ior;
        bool rayLeaveTransparent = cosThetaI > 0.f;
        if (rayLeaveTransparent) {
            std::swap(etaI, etaT);
            surfNormal = -surfNormal;
        } else
            cosThetaI = -cosThetaI;

        Color3f refractColor, reflectColor;
        Vector3f refrRayDir;
        if (refract(ray.dir, surfNormal, etaI / etaT, cosThetaI, &refrRayDir)) {
            // construct and trace refraction ray in the scene
            Ray refractionRay = Ray(isectData.pos + (-surfNormal * REFRACTION_BIAS), refrRayDir);
            Intersection refrIsectData;
            if (scene->intersect(refractionRay, refrIsectData)) {
                refractionRay.depth = ray.depth + 1;
                const Material* hittedMaterial = &scene->getMaterials()[refrIsectData.materialIdx];
                refractColor = hittedMaterial->shade(refractionRay, scene, refrIsectData);
            } else {
                refractColor = scene->getBackground();
            }

            // construct and trace reflection ray in the scene
            const Vector3f reflRayDir = reflect(ray.dir, surfNormal);
            Ray reflectionRay = Ray(isectData.pos + (surfNormal * REFLECTION_BIAS), reflRayDir);
            Intersection reflIsectData;
            if (scene->intersect(reflectionRay, reflIsectData)) {
                reflectionRay.depth = ray.depth + 1;
                const Material* hittedMaterial = &scene->getMaterials()[reflIsectData.materialIdx];
                reflectColor = hittedMaterial->shade(reflectionRay, scene, reflIsectData);
            } else {
                reflectColor = scene->getBackground();
            }

            const float fres = fresnel(ray.dir, surfNormal);
            return fres * reflectColor + (1 - fres) * refractColor;
        } else {
            const Vector3f reflRayDir = reflect(ray.dir, surfNormal);
            Ray reflectionRay = Ray(isectData.pos + (surfNormal * REFLECTION_BIAS), reflRayDir);
            Intersection reflIsectData;
            if (scene->intersect(reflectionRay, reflIsectData)) {
                reflectionRay.depth = ray.depth + 1;
                const Material* hittedMaterial = &scene->getMaterials()[reflIsectData.materialIdx];
                return hittedMaterial->shade(reflectionRay, scene, reflIsectData);
            } else {
                return scene->getBackground();
            }
        }
    }

    return scene->getBackground();
}

Color3f shadeConstant([[maybe_unused]] const Ray& ray, const Scene* scene,
                      Intersection& isectData) {
    return scene->getMaterials()[isectData.materialIdx].property.albedo;
}
