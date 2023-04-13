cmake_minimum_required(VERSION 3.10.2)

project(Camera LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src) 

add_executable(
    ${PROJECT_NAME} 
        ${_SRC_DIR}/utils/Vector3.h
        ${_SRC_DIR}/utils/Ray.h
        ${_SRC_DIR}/utils/PPMImage.h
        ${_SRC_DIR}/utils/Timer.h
        ${_SRC_DIR}/utils/Triangle.h
        ${_SRC_DIR}/utils/Constants.h
        ${_SRC_DIR}/utils/TaskManager.h
        ${_SRC_DIR}/utils/Utils.h
        ${_SRC_DIR}/utils/Matrix3x3.h
        ${_SRC_DIR}/utils/Camera.h

        ${_SRC_DIR}/main.cpp
)

target_include_directories(
    ${PROJECT_NAME}
        PRIVATE 
            ${_SRC_DIR}
)
