# Chaos Ray Tracing course repo

Minimalistic C++ ray tracer developed during the Chaos Ray Tracing course 2023. This is a software renderer that does not depend on the GPU or any graphics API or windowing libs. It relies only on [rapidjson](https://github.com/Tencent/rapidjson) for parsing the input scene and [stb](https://github.com/nothings/stb) to write the generated pixel data. The renderer is multithreaded and can use either region rendering to statically divide the work to the available threads or through the faster bucket rendering it can schedule the work in a task quaue for more dynamic rendering. The high computational cost inherent for the ray tracing is alleviated through the use of Kd tree for accelerating the ray-primitive intersections. The shading algorithm supports diffuse, reflective, and refractive materials.

## Getting Started
The project can be build either with cmake or a container instance can be run using docker.

### Getting the Code
To get the code clone the repo using the following command:
```bash
git clone https://github.com/vladimirr11/chaos-ray-tracing.git
```

### Build with CMake
To build the project run the following commands:
```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Docker
There is a Dockerfile so that a container can be run with the demo scene. In the root directory of this repo run:
```bash
docker build -t crt .
docker container run -it -v $(pwd):$(pwd) crt && docker cp $(docker ps -l -q):/crt_dir/build/ .
```
