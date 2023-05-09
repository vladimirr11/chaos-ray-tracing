#include "Material.h"
#include "Scene.h"

Color3f DiffuseM::shade([[maybe_unused]] const Ray& incidentR, const Scene* scene,
                        Intersection& isectHit) const {
    Color3f finalColor;
    const std::vector<Light>& lights = scene->getLights();
    const Normal3f isectN = smoothShading ? isectHit.smoothN : isectHit.faceN;
    for (const Light& light : lights) {
        Vector3f lightDir = light.getPosition() - isectHit.pos;
        const float lightDist = lightDir.length();
        const float lightArea = calcSphereArea(lightDist);
        const Vector3f lightDirN = normalize(lightDir);
        const float cosTheta = std::max(0.f, dot(lightDirN, isectN));
        const Vector3f shadowRayOrig = isectHit.pos + isectN * SHADOW_BIAS;
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

Color3f ReflectiveM::shade(const Ray& incidentR, const Scene* scene, Intersection& isectHit) const {
    return scatter(incidentR, scene, isectHit);
}

Color3f ReflectiveM::scatter(const Ray& incidentR, const Scene* scene, Intersection& isectData,
                             int rayDepth) const {
    const Vector3f reflectedDir = reflect(incidentR.dir, isectData.faceN).normalize();
    const Ray reflectedRay = Ray(isectData.pos + isectData.faceN * SHADOW_BIAS, reflectedDir);
    if (rayDepth < MAX_RAY_DEPTH && scene->intersect(reflectedRay, isectData)) {
        switch (scene->getMaterials()[isectData.materialIdx]->getType()) {
            case REFLECTIVE:
                return scatter(reflectedRay, scene, isectData, rayDepth + 1);
            case DIFFUSE:
                return albedo * scene->getMaterials()[isectData.materialIdx]->shade(
                                    reflectedRay, scene, isectData);
        }
    }
    return albedo * scene->getBackground();
}
