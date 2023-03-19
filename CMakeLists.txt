cmake_minimum_required(VERSION 3.10.2)

project(CRTRays LANGUAGES CXX)

set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src) 
set(CMAKE_CXX_STANDARD 20)

add_executable(
    ${PROJECT_NAME} 
        ${_SRC_DIR}/utils/PPMImage.h
        ${_SRC_DIR}/utils/Ray.h
        ${_SRC_DIR}/utils/Timer.h
        ${_SRC_DIR}/utils/Vector3.h

        ${_SRC_DIR}/main.cpp
)

target_include_directories(
    ${PROJECT_NAME}
        PRIVATE 
            ${_SRC_DIR}
)
