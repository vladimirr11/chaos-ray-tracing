#ifndef SCENE_H
#define SCENE_H

#include "PPMImage.h"
#include "Parser.h"

class Scene {
public:
    Scene() = delete;

    /// @brief Initialize scene data members from input json
    Scene(const std::string& fileName)
        : camera(Parser::parseCameraParameters(fileName)),
          sceneObjects(Parser::parseSceneObjects(fileName)),
          sceneLights(Parser::parseSceneLights(fileName)),
          settings(Parser::parseSceneSettings(fileName)) {}

    /// @brief Intersects ray with the scene and finds the closest intersection point if any
    bool intersect(const Ray& ray, Intersection& isect) const {
        bool hasIntersect = false;
        Intersection closestPrim;
        closestPrim.t = FLT_MAX;
        for (const auto& mesh : sceneObjects) {
            if (mesh.intersect(ray, isect)) {
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

    /// @brief Verifies if ray intersects with any scene object. Returns true on first intersection,
    /// false if no ray-object intersection found
    bool intersectPrim(const Ray& ray) const {
        for (const auto& mesh : sceneObjects) {
            if (mesh.intersectPrim(ray)) {
                return true;
            }
        }
        return false;
    }

    const Color3f getBackground() const { return settings.backgrColor; }

    const SceneDimensions getSceneDimensions() const { return settings.sceneDimensions; }

    const Camera getCamera() const { return camera; }

    const std::vector<Light> getLights() const { return sceneLights; }

    const std::vector<TriangleMesh> getObjects() const { return sceneObjects; }

private:
    Camera camera;                                 ///< The scene's camera
    const std::vector<TriangleMesh> sceneObjects;  ///< List of the scene's objects
    const std::vector<Light> sceneLights;          ///< Lights in the scene
    const Settings settings;                       ///< Global scene settings
};

#endif  // !SCENE_H
