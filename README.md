# Chaos Ray Tracing course repo

Minimalistic C++ ray tracer developed during the Chaos Ray Tracing course 2023. It's a software renderer that does not depends on the GPU or any graphics API or windowing libs. It relies only on [rapidjson](https://github.com/Tencent/rapidjson) for parsing the input scene description and [stb](https://github.com/nothings/stb) lib to write the generated pixel data. The high computational cost inherent for the ray tracing is alleviated by assigning the work on multiple threads and through the use of Kd tree for accelerating the ray-primitive intersections.  
