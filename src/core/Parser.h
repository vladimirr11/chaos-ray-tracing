#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <string>
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "external_libs/rapidjson/document.h"
#include "external_libs/rapidjson/istreamwrapper.h"

using namespace rapidjson;

/// @brief Stores scene's width and height
struct SceneDimensions {
    int32_t width = 0;
    int32_t height = 0;
};

/// @brief Records global scene settings
struct SceneSettings {
    Color3f backgrColor;
    SceneDimensions sceneDimensions;
    size_t bucketSize = 16;
};

inline static Vector3f loadVector(const Value::ConstArray& valArr) {
    Assert(valArr.Size() == 3);
    return Vector3f{valArr[0].GetFloat(), valArr[1].GetFloat(), valArr[2].GetFloat()};
}

inline static Matrix3x3 loadMatrix(const Value::ConstArray& valArr) {
    Assert(valArr.Size() == 9);
    const Vector3f r0 = Vector3f{valArr[0].GetFloat(), valArr[1].GetFloat(), valArr[2].GetFloat()};
    const Vector3f r1 = Vector3f{valArr[3].GetFloat(), valArr[4].GetFloat(), valArr[5].GetFloat()};
    const Vector3f r2 = Vector3f{valArr[6].GetFloat(), valArr[7].GetFloat(), valArr[8].GetFloat()};
    return Matrix3x3(r0, r1, r2);
}

inline static std::vector<Point3f> loadVertices(const Value::ConstArray& valArr) {
    Assert(valArr.Size() % 3 == 0);
    std::vector<Point3f> verts;
    verts.reserve(valArr.Size() / 3);
    for (size_t i = 0; i < valArr.Size(); i += 3) {
        const float v0 = valArr[i].GetFloat();
        const float v1 = valArr[i + 1].GetFloat();
        const float v2 = valArr[i + 2].GetFloat();
        verts.emplace_back(v0, v1, v2);
    }
    return verts;
}

inline static std::vector<TriangleIndices> loadTriangleIndices(const Value::ConstArray& valArr) {
    Assert(valArr.Size() % 3 == 0);
    std::vector<TriangleIndices> indices;
    indices.reserve(valArr.Size() / 3);
    for (size_t i = 0; i < valArr.Size(); i += 3) {
        const int i0 = valArr[i].GetInt();
        const int i1 = valArr[i + 1].GetInt();
        const int i2 = valArr[i + 2].GetInt();
        indices.emplace_back(TriangleIndices{i0, i1, i2});
    }
    return indices;
}

class Parser {
public:
    /// @brief Retrieves scene objects from given input json
    static int32_t parseSceneObjects(std::string_view inputFile,
                                     std::vector<TriangleMesh>& sceneObjects);

    /// @brief Retrieves camera settings from given input json
    static int32_t parseCameraParameters(std::string_view inputFile, Camera& camera);

    /// @brief Retrieves scene settings from given input json
    static int32_t parseSceneSettings(std::string_view inputFile, SceneSettings& settings);

    /// @brief Retrieves scene lights from given input json
    static int32_t parseSceneLights(std::string_view inputFile, std::vector<Light>& sceneLights);

    /// @brief Retrieves scene materials fron given input json
    static int32_t parseMaterials(std::string_view inputFile, std::vector<Material>& materials);

private:
    /// @brief Retrieves json document from input stream
    static Document getJsonDocument(std::string_view inputFile);

    /// @brief Retrieves scene width & height
    static SceneDimensions parseSceneDimensions(std::string_view inputFile);
};

#endif  // !PARSER_H
