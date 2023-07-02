cmake_minimum_required(VERSION 3.10.2)

project(Acceleration LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-Wall -g -O2")
set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src) 
set(_EXTERNAL_LIBS ${_SRC_DIR}/external_libs)

add_executable(
    ${PROJECT_NAME} 
        ${_SRC_DIR}/core/Vector3.h
        ${_SRC_DIR}/core/Ray.h
        ${_SRC_DIR}/core/PPMImage.h
        ${_SRC_DIR}/core/Timer.h
        ${_SRC_DIR}/core/Triangle.h
        ${_SRC_DIR}/core/Triangle.cpp
        ${_SRC_DIR}/core/Defines.h
        ${_SRC_DIR}/core/ThreadPool.h
        ${_SRC_DIR}/core/Utils.h
        ${_SRC_DIR}/core/Matrix3x3.h
        ${_SRC_DIR}/core/Matrix3x3.cpp
        ${_SRC_DIR}/core/Camera.h
        ${_SRC_DIR}/core/Camera.cpp
        ${_SRC_DIR}/core/Parser.h
        ${_SRC_DIR}/core/Parser.cpp
        ${_SRC_DIR}/core/Scene.h
        ${_SRC_DIR}/core/Scene.cpp
        ${_SRC_DIR}/core/Light.h
        ${_SRC_DIR}/core/Material.h
        ${_SRC_DIR}/core/Material.cpp
        ${_SRC_DIR}/core/Renderer.h
        ${_SRC_DIR}/core/Renderer.cpp
        ${_SRC_DIR}/core/AABBox.h
        ${_SRC_DIR}/core/Statistics.h
        ${_SRC_DIR}/core/Statistics.cpp
        ${_SRC_DIR}/core/AccelerationTree.h
        ${_SRC_DIR}/core/AccelerationTree.cpp

        ${_SRC_DIR}/main.cpp
)

target_include_directories(
    ${PROJECT_NAME}
        PRIVATE 
            ${_SRC_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
        PRIVATE 
            rapidjson
)

add_subdirectory(${_EXTERNAL_LIBS})
