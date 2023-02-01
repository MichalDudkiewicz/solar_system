cmake_minimum_required (VERSION 3.8)

project (OpenGL)
set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF)

add_executable (OpenGL
        src/Application.cpp
        src/IndexBuffer.cpp
        src/Renderer.cpp
        src/Shader.cpp
        src/Texture.cpp
        src/VertexArray.cpp
        src/VertexBuffer.cpp

        vendor/include/stb_image/stb_image.cpp
)

find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)

target_include_directories(OpenGL PUBLIC include vendor/include)
target_link_libraries(OpenGL PUBLIC GLEW::GLEW glfw Qt6::Multimedia)

file(COPY res DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")