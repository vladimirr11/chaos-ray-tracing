#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <string>
#include "../external_libs/rapidjson/document.h"
#include "../external_libs/rapidjson/istreamwrapper.h"
#include "Camera.h"
#include "Light.h"

using namespace rapidjson;

/// @brief Records global scene settings
struct Settings {
    Color3f backgrColor;
    SceneDimensions sceneDimensions;
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
    /// @brief Retrieves scene objects
    static std::vector<TriangleMesh> parseSceneObjects(const std::string_view& fileName) {
        std::vector<TriangleMesh> sceneObjects;
        Document doc = getJsonDocument(fileName);

        const Value& objects = doc.FindMember(SceneDefines::sceneObjects)->value;
        Assert(!objects.IsNull() && objects.IsArray());

        sceneObjects.reserve(objects.Size());
        for (size_t i = 0; i < objects.Size(); ++i) {
            const Value& vertices = objects[i].FindMember(SceneDefines::vertices)->value;
            const Value& triangleIndices =
                objects[i].FindMember(SceneDefines::triangleIndices)->value;
            Assert(!vertices.IsNull() && vertices.IsArray());
            Assert(!triangleIndices.IsNull() && triangleIndices.IsArray());
            sceneObjects.emplace_back(loadVertices(vertices.GetArray()),
                                      loadTriangleIndices(triangleIndices.GetArray()));
        }

        return sceneObjects;
    }

    /// @brief Retrieves camera settings
    static Camera parseCameraParameters(const std::string_view& fileName) {
        Camera camera;
        SceneDimensions sceneDimens;
        Document doc = getJsonDocument(fileName);

        const Value& cameraSettings = doc.FindMember(SceneDefines::cameraSettings)->value;
        Assert(!cameraSettings.IsNull() && cameraSettings.IsObject());

        const Value& cameraPos = cameraSettings.FindMember(SceneDefines::cameraPos)->value;
        const Value& cameraRotationM =
            cameraSettings.FindMember(SceneDefines::cameraRotationM)->value;
        Assert(!cameraPos.IsNull() && cameraPos.IsArray());
        Assert(!cameraRotationM.IsNull() && cameraRotationM.IsArray());

        /// get scene width & height
        sceneDimens = parseSceneDimensions(fileName);

        camera.init(loadVector(cameraPos.GetArray()), loadMatrix(cameraRotationM.GetArray()),
                    sceneDimens.width, sceneDimens.height);

        return camera;
    }

    /// @brief Retrieves scene background color and scene dimensions
    static Settings parseSceneSettings(const std::string_view& fileName) {
        Settings settings;
        Document doc = getJsonDocument(fileName);

        /// set background color
        const Value& sceneSettings = doc.FindMember(SceneDefines::sceneSettings)->value;
        Assert(!sceneSettings.IsNull() && sceneSettings.IsObject());
        
        const Value& backgrColor = sceneSettings.FindMember(SceneDefines::backgroundColor)->value;
        Assert(!backgrColor.IsNull() && backgrColor.IsArray());
        settings.backgrColor = loadVector(backgrColor.GetArray());

        /// set scene width & height
        settings.sceneDimensions = parseSceneDimensions(fileName);

        return settings;
    }

    /// @brief Retrieves scene lights parameters
    static std::vector<Light> parseSceneLights(const std::string_view& fileName) {
        std::vector<Light> sceneLights;
        Document doc = getJsonDocument(fileName);

        const Value& lightSettings = doc.FindMember(SceneDefines::sceneLights)->value;
        Assert(!lightSettings.IsNull() && lightSettings.IsArray());

        sceneLights.reserve(lightSettings.Size());
        for (size_t i = 0; i < lightSettings.Size(); ++i) {
            const Value& lightPos = lightSettings[i].FindMember(SceneDefines::lightPosition)->value;
            const Value& lightIntensity =
                lightSettings[i].FindMember(SceneDefines::lightIntensity)->value;
            Assert(!lightPos.IsNull() && lightPos.IsArray());
            Assert(!lightIntensity.IsNull() && lightIntensity.IsInt());
            sceneLights.emplace_back(loadVector(lightPos.GetArray()), lightIntensity.GetInt());
        }

        return sceneLights;
    }

private:
    /// @brief Retrieves json document from input stream
    static Document getJsonDocument(const std::string_view& fileName) {
        std::ifstream inputFileStream(fileName.data());
        if (!inputFileStream.good()) {
            std::cout << "Input file stream " << fileName << " not good\n";
            exit(EXIT_FAILURE);
        }

        BasicIStreamWrapper<std::ifstream> istreamWrapper(inputFileStream);

        Document doc;
        doc.ParseStream(istreamWrapper);
        if (doc.HasParseError()) {
            std::cout << "Parse error " << doc.GetParseError() << "\n";
            std::cout << "Offset " << doc.GetErrorOffset() << "\n";
            Assert(false);
        }

        Assert(doc.IsObject());
        return doc;
    }

    /// @brief Retrieves scene width & height
    static SceneDimensions parseSceneDimensions(const std::string_view& fileName) {
        SceneDimensions sceneDimens;
        Document doc = getJsonDocument(fileName);

        const Value& sceneSettings = doc.FindMember(SceneDefines::sceneSettings)->value;
        Assert(!sceneSettings.IsNull() && sceneSettings.IsObject());

        const Value& imageSettings = sceneSettings.FindMember(SceneDefines::imageSettings)->value;
        Assert(!imageSettings.IsNull() && imageSettings.IsObject());
        const Value& imgWidth = imageSettings.FindMember(SceneDefines::imageWidth)->value;
        const Value& imgHeight = imageSettings.FindMember(SceneDefines::imageHeight)->value;
        Assert(!imgWidth.IsNull() && imgWidth.IsInt());
        Assert(!imgHeight.IsNull() && imgHeight.IsInt());

        sceneDimens.width = imgWidth.GetInt();
        sceneDimens.height = imgHeight.GetInt();

        return sceneDimens;
    }
};

#endif  // !PARSER_H
