#ifndef SCENE_H
#define SCENE_H

#include "Parser.h"

/// @brief Stores parameters needed for initialization of scene object
struct SceneParams {
    Camera camera;
    std::vector<TriangleMesh> objects;
    std::vector<Light> lights;
    std::vector<Material> materials;
    SceneSettings settings;
};

class Scene {
public:
    Scene() = delete;

    /// @brief Initialize scene data members from input json
    Scene(const SceneParams& sceneParams)
        : camera(std::move(sceneParams.camera)),
          sceneObjects(std::move(sceneParams.objects)),
          sceneLights(std::move(sceneParams.lights)),
          materials(std::move(sceneParams.materials)),
          settings(std::move(sceneParams.settings)) {}

    /// @brief Intersects ray with the scene and finds the closest intersection point if any
    bool intersect(const Ray& ray, Intersection& isect) const {
        bool hasIntersect = false;
        Intersection closestPrim;
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

    /// @brief Verifies if ray intersects with any non transparent scene object.
    /// Returns true on first intersection, false if no ray-object intersection found
    bool intersectPrim(const Ray& ray) const {
        Intersection closestPrim;
        for (const auto& mesh : sceneObjects) {
            if (mesh.intersectPrim(ray, closestPrim) &&
                materials[closestPrim.materialIdx].type != MaterialType::REFRACTIVE) {
                return true;
            }
        }
        return false;
    }

    const Color3f& getBackground() const { return settings.backgrColor; }

    const SceneDimensions& getSceneDimensions() const { return settings.sceneDimensions; }

    const SceneSettings& getSceneSettings() const { return settings; }

    const Camera& getCamera() const { return camera; }

    const std::vector<Light>& getLights() const { return sceneLights; }

    const std::vector<TriangleMesh>& getObjects() const { return sceneObjects; }

    const std::vector<Material>& getMaterials() const { return materials; }

private:
    Camera camera;                                 ///< The scene's camera
    const std::vector<TriangleMesh> sceneObjects;  ///< List of the scene's objects
    const std::vector<Light> sceneLights;          ///< Lights in the scene
    const std::vector<Material> materials;         ///< List of the scene's materials
    const SceneSettings settings;                  ///< Global scene settings
};

/// @brief Retrieves scene parameters from given input json
inline static int32_t parseSceneParams(std::string_view inputFile, SceneParams& sceneParams) {
    if (Parser::parseCameraParameters(inputFile, sceneParams.camera) != EXIT_SUCCESS) {
        std::cerr << "Scene parser failed." << std::endl;
        return EXIT_FAILURE;
    } else if (Parser::parseSceneObjects(inputFile, sceneParams.objects) != EXIT_SUCCESS) {
        std::cerr << "Scene parser failed." << std::endl;
        return EXIT_FAILURE;
    } else if (Parser::parseSceneLights(inputFile, sceneParams.lights) != EXIT_SUCCESS) {
        std::cerr << "Scene parser failed." << std::endl;
        return EXIT_FAILURE;
    } else if (Parser::parseMaterials(inputFile, sceneParams.materials) != EXIT_SUCCESS) {
        std::cerr << "Scene parser failed." << std::endl;
        return EXIT_FAILURE;
    } else if (Parser::parseSceneSettings(inputFile, sceneParams.settings) != EXIT_SUCCESS) {
        std::cerr << "Scene parser failed." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif  // !SCENE_H
