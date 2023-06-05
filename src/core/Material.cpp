#include "Material.h"
#include "Scene.h"

Color3f Material::shade(const Ray& ray, const Scene* scene, Intersection& isectData) const {
    Color3f shadeColor;
    const MaterialType hittedMaterial = scene->getMaterials()[isectData.materialIdx].type;
    if (hittedMaterial == MaterialType::DIFFUSE) {
        shadeColor = shadeDiffuse(ray, scene, isectData);
    } else if (hittedMaterial == MaterialType::REFLECTIVE) {
        shadeColor = shadeReflective(ray, scene, isectData);
    } else if (hittedMaterial == MaterialType::REFRACTIVE) {
        shadeColor = shadeRefractive(ray, scene, isectData);
    } else if (hittedMaterial == MaterialType::CONSTANT) {
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
    using std::max;
    for (const Light& light : lights) {
        Vector3f lightDir = light.getPosition() - isectData.pos;
        const float lightDist = lightDir.length();
        const float lightArea = calcSphereArea(lightDist);
        const Vector3f lightDirN = normalize(lightDir);
        const float cosTheta = max(0.f, dot(lightDirN, isectNormal));
        const Vector3f shadowRayOrig = isectData.pos + isectNormal * SHADOW_BIAS;
        const Ray shadowRay(shadowRayOrig, lightDirN);
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
    const Vector3f reflectedDir = reflect(ray.dir, isectData.faceNormal);
    Ray reflectedRay = Ray(isectData.pos + isectData.faceNormal * REFLECTION_BIAS, reflectedDir);
    const Color3f& albedo = scene->getMaterials()[isectData.materialIdx].property.albedo;
    if (ray.depth < MAX_RAY_DEPTH && scene->intersect(reflectedRay, isectData)) {
        switch (scene->getMaterials()[isectData.materialIdx].type) {
        case MaterialType::REFLECTIVE:
            reflectedRay.depth = ray.depth + 1;
            return shadeReflective(reflectedRay, scene, isectData);
        case MaterialType::DIFFUSE:
            return albedo * shadeDiffuse(reflectedRay, scene, isectData);
        case MaterialType::REFRACTIVE:
            reflectedRay.depth = ray.depth + 1;
            return albedo * shadeRefractive(reflectedRay, scene, isectData);
        case MaterialType::CONSTANT:
            return albedo * shadeConstant(reflectedRay, scene, isectData);
        default:
            Assert(false && "shadeReflective() received unsupported material type.");
        }
    }
    return albedo * scene->getBackground();
}

Color3f shadeRefractive(const Ray& ray, const Scene* scene, Intersection& isectData) {
    if (ray.depth < MAX_RAY_DEPTH) {
        switch (scene->getMaterials()[isectData.materialIdx].type) {
        case MaterialType::REFLECTIVE:
        case MaterialType::DIFFUSE:
            return shadeDiffuse(ray, scene, isectData);
        case MaterialType::CONSTANT:
            return shadeConstant(ray, scene, isectData);
        case MaterialType::REFRACTIVE: {
            float cosThetaI = clamp(-1.f, 1.f, dot(ray.dir, isectData.faceNormal));
            const float ior = scene->getMaterials()[isectData.materialIdx].property.ior;
            float etaI = 1.f, etaT = ior;
            bool smoothShading = scene->getMaterials()[isectData.materialIdx].smoothShading;
            Normal3f surfNormal = smoothShading ? isectData.smoothNormal : isectData.faceNormal;
            bool rayLeaveTransparent = cosThetaI > 0.f;
            if (rayLeaveTransparent) {
                std::swap(etaI, etaT);
                surfNormal = -surfNormal;
            } else
                cosThetaI = -cosThetaI;

            Color3f refractColor, reflectColor;
            Vector3f refrRayDir;
            if (refract(ray.dir, surfNormal, etaI / etaT, cosThetaI, &refrRayDir)) {
                Ray refractionRay =
                    Ray(isectData.pos + (-surfNormal * REFRACTION_BIAS), refrRayDir);
                if (scene->intersect(refractionRay, isectData)) {
                    refractionRay.depth = ray.depth + 1;
                    refractColor = shadeRefractive(refractionRay, scene, isectData);
                } else {
                    refractColor = scene->getBackground();
                }

                const Vector3f reflRayDir = reflect(ray.dir, surfNormal);
                Ray reflectionRay = Ray(isectData.pos + (surfNormal * REFLECTION_BIAS), reflRayDir);
                if (scene->intersect(reflectionRay, isectData)) {
                    reflectionRay.depth = ray.depth + 1;
                    reflectColor = shadeRefractive(reflectionRay, scene, isectData);
                } else {
                    reflectColor = scene->getBackground();
                }

                const float fres = fresnel(ray.dir, surfNormal);
                return fres * reflectColor + (1 - fres) * refractColor;
            } else {
                const Vector3f reflRayDir = reflect(ray.dir, surfNormal);
                Ray reflectionRay = Ray(isectData.pos + (surfNormal * REFLECTION_BIAS), reflRayDir);
                if (scene->intersect(reflectionRay, isectData)) {
                    reflectionRay.depth = ray.depth + 1;
                    return shadeRefractive(reflectionRay, scene, isectData);
                } else {
                    return scene->getBackground();
                }
            }
        }
        default:
            Assert(false && "shadeRefractive() received unsupported material type.");
        }
    }

    return scene->getBackground();
}

Color3f shadeConstant([[maybe_unused]] const Ray& ray, const Scene* scene,
                      Intersection& isectData) {
    return scene->getMaterials()[isectData.materialIdx].property.albedo;
}
