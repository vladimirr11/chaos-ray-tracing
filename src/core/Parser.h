#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <string>
#include "../external_libs/rapidjson/document.h"
#include "../external_libs/rapidjson/istreamwrapper.h"
#include "Camera.h"

using namespace rapidjson;

/// @brief Records global scene settings during parsing of the input json
struct Settings {
    Color3f backgrColor;
    SceneDimensions sceneDimensions;
    Camera camera;
    std::vector<TriangleMesh> sceneObjects;
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
    /// @brief Parse input json file and returns recorded scene settings
    static Settings parseJsonSceneFile(const std::string_view& fileName) {
        Settings settings;
        Document doc = getJsonDocument(fileName);

        /// set background color
        const Value& sceneSettings = doc.FindMember(SceneDifines::sceneSettings)->value;
        Assert(!sceneSettings.IsNull() && sceneSettings.IsObject());
        const Value& backgrColor = sceneSettings.FindMember(SceneDifines::backgroundColor)->value;
        Assert(!backgrColor.IsNull() && backgrColor.IsArray());

        settings.backgrColor = loadVector(backgrColor.GetArray());

        /// set image width & height
        const Value& imageSettings = sceneSettings.FindMember(SceneDifines::imageSettings)->value;
        Assert(!imageSettings.IsNull() && imageSettings.IsObject());
        const Value& imgWidth = imageSettings.FindMember(SceneDifines::imageWidth)->value;
        const Value& imgHeight = imageSettings.FindMember(SceneDifines::imageHeight)->value;
        Assert(!imgWidth.IsNull() && imgWidth.IsInt());
        Assert(!imgHeight.IsNull() && imgHeight.IsInt());

        settings.sceneDimensions.width = imgWidth.GetInt();
        settings.sceneDimensions.height = imgHeight.GetInt();

        /// initialize camera
        const Value& cameraSettings = doc.FindMember(SceneDifines::cameraSettings)->value;
        Assert(!cameraSettings.IsNull() && cameraSettings.IsObject());
        const Value& cameraPos = cameraSettings.FindMember(SceneDifines::cameraPos)->value;
        const Value& cameraRotationM =
            cameraSettings.FindMember(SceneDifines::cameraRotationM)->value;
        Assert(!cameraPos.IsNull() && cameraPos.IsArray());
        Assert(!cameraRotationM.IsNull() && cameraRotationM.IsArray());

        settings.camera.init(loadVector(cameraPos.GetArray()),
                             loadMatrix(cameraRotationM.GetArray()), settings.sceneDimensions.width,
                             settings.sceneDimensions.height);

        /// retrieve scene objects
        const Value& sceneObjects = doc.FindMember(SceneDifines::sceneObjects)->value;
        Assert(!sceneObjects.IsNull() && sceneObjects.IsArray());

        settings.sceneObjects.reserve(sceneObjects.Size());
        for (size_t i = 0; i < sceneObjects.Size(); ++i) {
            const Value& vertices = sceneObjects[i].FindMember(SceneDifines::vertices)->value;
            const Value& triangleIndices =
                sceneObjects[i].FindMember(SceneDifines::triangleIndices)->value;
            Assert(!vertices.IsNull() && vertices.IsArray());
            Assert(!triangleIndices.IsNull() && triangleIndices.IsArray());
            settings.sceneObjects.emplace_back(loadVertices(vertices.GetArray()),
                                               loadTriangleIndices(triangleIndices.GetArray()));
        }

        return settings;
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
};

#endif  // !PARSER_H
