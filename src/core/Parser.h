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
    int width{};
    int height{};
};

/// @brief Records global scene settings
struct SceneSettings {
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
    /// @brief Retrieves scene objects from given input json
    static int32_t parseSceneObjects(std::string_view inputFile,
                                     std::vector<TriangleMesh>& sceneObjects) {
        Document doc = getJsonDocument(inputFile);

        const Value& objects = doc.FindMember(SceneDefines::sceneObjects)->value;
        if (!objects.IsArray()) {
            std::cerr << "Parser failed to parse scene objects." << std::endl;
            return EXIT_FAILURE;
        }

        sceneObjects.reserve(objects.Size());
        for (size_t i = 0; i < objects.Size(); ++i) {
            const Value& vertices = objects[i].FindMember(SceneDefines::vertices)->value;
            if (!vertices.IsArray()) {
                std::cerr << "Parser failed to parse triangle vertices." << std::endl;
                return EXIT_FAILURE;
            }

            const Value& triangleIndices =
                objects[i].FindMember(SceneDefines::triangleIndices)->value;
            if (!triangleIndices.IsArray()) {
                std::cerr << "Parser failed to parse triangle indices." << std::endl;
                return EXIT_FAILURE;
            }

            const Value& materialIdx = objects[i].FindMember(SceneDefines::materialIdx)->value;
            if (!materialIdx.IsInt()) {
                std::cerr << "Parser failed to parse material index." << std::endl;
                return EXIT_FAILURE;
            }

            sceneObjects.emplace_back(loadVertices(vertices.GetArray()),
                                      loadTriangleIndices(triangleIndices.GetArray()),
                                      materialIdx.GetInt());
        }

        return EXIT_SUCCESS;
    }

    /// @brief Retrieves camera settings from given input json
    static int32_t parseCameraParameters(std::string_view inputFile, Camera& camera) {
        SceneDimensions sceneDimens;
        Document doc = getJsonDocument(inputFile);

        const Value& cameraSettings = doc.FindMember(SceneDefines::cameraSettings)->value;
        if (!cameraSettings.IsObject() || cameraSettings.ObjectEmpty()) {
            std::cerr << "Parser failed to parse camera settings." << std::endl;
            return EXIT_FAILURE;
        }

        const Value& cameraPos = cameraSettings.FindMember(SceneDefines::cameraPos)->value;
        if (!cameraPos.IsArray()) {
            std::cerr << "Parser failed to parse camera position." << std::endl;
            return EXIT_FAILURE;
        }
        const Value& cameraRotationM =
            cameraSettings.FindMember(SceneDefines::cameraRotationM)->value;
        if (!cameraRotationM.IsArray()) {
            std::cerr << "Parser failed to parse camera rotation matrix." << std::endl;
            return EXIT_FAILURE;
        }

        /// get scene width & height
        sceneDimens = parseSceneDimensions(inputFile);

        camera.init(loadVector(cameraPos.GetArray()), loadMatrix(cameraRotationM.GetArray()),
                    sceneDimens.width, sceneDimens.height);

        return EXIT_SUCCESS;
    }

    /// @brief Retrieves scene settings from given input json
    static int32_t parseSceneSettings(std::string_view inputFile, SceneSettings& settings) {
        Document doc = getJsonDocument(inputFile);

        /// set background color
        const Value& sceneSettings = doc.FindMember(SceneDefines::sceneSettings)->value;
        if (!sceneSettings.IsObject()) {
            std::cerr << "Parser failed to parse scene settings." << std::endl;
            return EXIT_FAILURE;
        }

        const Value& backgrColor = sceneSettings.FindMember(SceneDefines::backgroundColor)->value;
        if (!backgrColor.IsArray()) {
            std::cerr << "Parser failed to parse scene background color." << std::endl;
            return EXIT_FAILURE;
        }

        settings.backgrColor = loadVector(backgrColor.GetArray());

        /// set scene width & height
        settings.sceneDimensions = parseSceneDimensions(inputFile);

        return EXIT_SUCCESS;
    }

    /// @brief Retrieves scene lights from given input json
    static int32_t parseSceneLights(std::string_view inputFile, std::vector<Light>& sceneLights) {
        Document doc = getJsonDocument(inputFile);

        const Value& lightSettings = doc.FindMember(SceneDefines::sceneLights)->value;
        if (!lightSettings.IsArray() &&
            !lightSettings.IsObject()) {  // workaround for scenes without lights
            std::cerr << "Parser has not found scene lights." << std::endl;
            return EXIT_SUCCESS;
        }

        if (!lightSettings.IsArray()) {
            std::cerr << "Parser failed to parse scene lights." << std::endl;
            return EXIT_FAILURE;
        }

        sceneLights.reserve(lightSettings.Size());
        for (size_t i = 0; i < lightSettings.Size(); ++i) {
            const Value& lightPos = lightSettings[i].FindMember(SceneDefines::lightPosition)->value;
            if (!lightPos.IsArray()) {
                std::cerr << "Parser failed to parse light position." << std::endl;
                return EXIT_FAILURE;
            }

            const Value& lightIntensity =
                lightSettings[i].FindMember(SceneDefines::lightIntensity)->value;
            if (!lightIntensity.IsInt()) {
                std::cerr << "Parser failed to parse light intensity." << std::endl;
                return EXIT_FAILURE;
            }

            sceneLights.emplace_back(loadVector(lightPos.GetArray()), lightIntensity.GetInt());
        }

        return EXIT_SUCCESS;
    }

    /// @brief Retrieves scene materials fron given input json
    static int32_t parseMaterials(std::string_view inputFile, std::vector<Material>& materials) {
        Document doc = getJsonDocument(inputFile);

        const Value& materialsInfo = doc.FindMember(SceneDefines::materialsInfo)->value;
        if (!materialsInfo.IsArray()) {
            std::cerr << "Parser failed to parse materials information." << std::endl;
            return EXIT_FAILURE;
        }

        materials.reserve(materialsInfo.Size());
        for (size_t i = 0; i < materialsInfo.Size(); i++) {
            const Value& mType = materialsInfo[i].FindMember(SceneDefines::materialType)->value;
            if (!mType.IsString()) {
                std::cerr << "Parser failed to parse material type." << std::endl;
                return EXIT_FAILURE;
            }

            const Value& albedo = materialsInfo[i].FindMember(SceneDefines::materialAlbedo)->value;
            const Value& ior = materialsInfo[i].FindMember(SceneDefines::materialIOR)->value;

            MaterialProperty materialProp;
            if (albedo.IsArray())
                LIKELY { materialProp.albedo = loadVector(albedo.GetArray()); }
            else if (ior.IsFloat()) {
                materialProp.ior = ior.GetFloat();
            } else {
                std::cerr << "Parser failed to parse material albedo and ior." << std::endl;
                return EXIT_FAILURE;
            }

            const Value& smooth = materialsInfo[i].FindMember(SceneDefines::materialSmootSh)->value;
            if (!smooth.IsBool()) {
                std::cerr << "Parser failed to parse material smooth shading." << std::endl;
                return EXIT_FAILURE;
            }

            materials.emplace_back(makeMaterial(mType.GetString(), materialProp, smooth.GetBool()));
        }

        return EXIT_SUCCESS;
    }

private:
    /// @brief Retrieves json document from input stream
    static Document getJsonDocument(std::string_view inputFile) {
        std::ifstream inputFileStream(inputFile.data());
        if (!inputFileStream.good()) {
            std::cout << "Input file stream " << inputFile << " not good\n";
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
    static SceneDimensions parseSceneDimensions(std::string_view inputFile) {
        SceneDimensions sceneDimens;
        Document doc = getJsonDocument(inputFile);

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
