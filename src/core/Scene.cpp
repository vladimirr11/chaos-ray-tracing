#include "Scene.h"

Scene::Scene(const SceneParams& sceneParams)
    : camera(std::move(sceneParams.camera)),
      sceneObjects(std::move(sceneParams.objects)),
      sceneLights(std::move(sceneParams.lights)),
      materials(std::move(sceneParams.materials)),
      settings(std::move(sceneParams.settings)) {}

void Scene::createAccelTree() {
    std::vector<Triangle> sceneTriangles;
    for (const auto& object : sceneObjects) {
        sceneTriangles.reserve(sceneTriangles.size() + object.vertIndices.size());
        object.retrieveTriangles(sceneTriangles);
        sceneBBox.unionWith(object.bounds);
    }
    accelTree = std::make_unique<AccelTree>(std::move(sceneTriangles), sceneBBox);
}

bool Scene::intersect(const Ray& ray, Intersection& isect) const {
    if (accelTree) {
        if (!sceneBBox.intersect(ray))
            return false;
        return accelTree->intersect(ray, sceneBBox, isect);
    }

    bool hasIntersect = false;
    Intersection closestPrim;
    for (const auto& object : sceneObjects) {
        if (object.intersect(ray, isect)) {
            if (isect.t < closestPrim.t) {
                closestPrim = isect;
            }
            hasIntersect = true;
        }
    }

    if (hasIntersect)
        isect = closestPrim;

    return hasIntersect;
}

bool Scene::intersectPrim(const Ray& ray) const {
    Intersection closestPrim;
    if (accelTree) {
        if (!sceneBBox.intersect(ray))
            return false;
        return accelTree->intersectPrim(ray, sceneBBox, closestPrim) &&
               materials[closestPrim.materialIdx].type != MaterialType::REFRACTIVE;
    }

    for (const auto& object : sceneObjects) {
        if (object.intersectPrim(ray, closestPrim) &&
            materials[closestPrim.materialIdx].type != MaterialType::REFRACTIVE) {
            return true;
        }
    }

    return false;
}
