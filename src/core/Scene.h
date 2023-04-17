#ifndef SCENE_H
#define SCENE_H

#include "PPMImage.h"
#include "Parser.h"

class Scene {
public:
    Scene() = delete;

    /// @brief Initialize scene settings from input json
    Scene(const std::string& fileName) : settings(Parser::parseJsonSceneFile(fileName)) {}

    const std::vector<TriangleMesh> getGeometryObjects() const { return settings.sceneObjects; }

    const Camera getCamera() const { return settings.camera; }

    const SceneDimesions getSceneDimensions() const { return settings.sceneDimensions; }

    const Color3f getBackgroundColor() const { return settings.backgrColor; }

private:
    const Settings settings;  ///< Global scene settings
};

#endif  // !SCENE_H
