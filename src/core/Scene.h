#ifndef SCENE_H
#define SCENE_H

#include "AccelerationTree.h"
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
    Scene(const SceneParams& sceneParams);

    /// @brief Constructs the acceleration tree
    void createAccelTree();

    /// @brief Intersects ray with the scene and finds the closest intersection point if any
    bool intersect(const Ray& ray, Intersection& isect) const;

    /// @brief Verifies if ray intersects with any non transparent scene object.
    /// Returns true on first intersection, false if no ray-object intersection found
    bool intersectPrim(const Ray& ray) const;

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
    std::unique_ptr<AccelTree> accelTree;          ///< The acceleration tree of the scene
    BBox sceneBBox;  ///< AABB of the entire scene. Computed only when acceleration tree is build
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
