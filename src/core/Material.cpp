#include "Material.h"
#include "Scene.h"

Color3f Material::shade(const Ray& ray, const Scene* scene, Intersection& isectData) const {
    Color3f shadeColor;
    const MaterialType hittedMaterial = scene->getMaterials()[isectData.materialIdx].type;
    if (hittedMaterial == MaterialType::DIFFUSE) {
        shadeColor = shadeDiffuseM(ray, scene, isectData);
    } else if (hittedMaterial == MaterialType::REFLECTIVE) {
        shadeColor = shadeReflectiveM(ray, scene, isectData);
    } else {
        Assert(false && "MaterialInstance::shade() received unsupported material type.");
    }
    return shadeColor;
}

Color3f shadeDiffuseM([[maybe_unused]] const Ray& ray, const Scene* scene,
                      Intersection& isectData) {
    Color3f finalColor;
    const std::vector<Light>& lights = scene->getLights();
    const Color3f& albedo = scene->getMaterials()[isectData.materialIdx].albedo;
    bool smoothShading = scene->getMaterials()[isectData.materialIdx].smoothShading;
    const Normal3f isectN = smoothShading ? isectData.smoothN : isectData.faceN;
    using std::max;
    for (const Light& light : lights) {
        Vector3f lightDir = light.getPosition() - isectData.pos;
        const float lightDist = lightDir.length();
        const float lightArea = calcSphereArea(lightDist);
        const Vector3f lightDirN = normalize(lightDir);
        const float cosTheta = max(0.f, dot(lightDirN, isectN));
        const Vector3f shadowRayOrig = isectData.pos + isectN * SHADOW_BIAS;
        const Ray shadowRay(shadowRayOrig, lightDirN);
        Color3f perLightColor;
        Intersection shadowRayIsect;
        shadowRayIsect.t = lightDist;
        if (!scene->intersectPrim(shadowRay, shadowRayIsect)) {
            perLightColor = Color3f(light.getIntensity() / lightArea * albedo * cosTheta);
        }
        finalColor += perLightColor;
    }

    return finalColor;
}

Color3f shadeReflectiveM(const Ray& ray, const Scene* scene, Intersection& isectData) {
    const Vector3f reflectedDir = reflect(ray.dir, isectData.faceN).normalize();
    Ray reflectedRay = Ray(isectData.pos + isectData.faceN * SHADOW_BIAS, reflectedDir);
    const Color3f& albedo = scene->getMaterials()[isectData.materialIdx].albedo;
    if (ray.depth < MAX_RAY_DEPTH && scene->intersect(reflectedRay, isectData)) {
        switch (scene->getMaterials()[isectData.materialIdx].type) {
            case MaterialType::REFLECTIVE:
                reflectedRay.depth = ray.depth + 1;
                return shadeReflectiveM(reflectedRay, scene, isectData);
            case MaterialType::DIFFUSE:
                return albedo * shadeDiffuseM(reflectedRay, scene, isectData);
            default:
                Assert(false && "shadeReflectiveM() received unsupported material type.");
        }
    }
    return albedo * scene->getBackground();
}
