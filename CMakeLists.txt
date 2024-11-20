cmake_minimum_required(VERSION 3.16)
project(VoxelProject)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the output directory for specific build configurations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src include)

# Find and Link Libraries
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# GLEW
set(GLEW_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/glew/include)
set(GLEW_LIBRARIES ${CMAKE_SOURCE_DIR}/libs/glew/lib/glew32s.lib)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLEW_LIBRARIES})

# Define glm static
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

# GLFW

set(GLFW_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/glfw/include)
set(GLFW_LIBRARIES ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3_mt.lib)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARIES})

# GLM

set(GLM_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/glm)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

# ASSIMP

set(ASSIMP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/assimp)
target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIRS})

# IMGUI

set(IMGUI_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/imgui/include)
set(IMGUI_LIBRARIES ${CMAKE_SOURCE_DIR}/libs/imgui/lib/assimp-vc143-mt.lib)

target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${IMGUI_LIBRARIES})


# SHADERS

file(GLOB SHADER_VERT_SOURCES "${CMAKE_SOURCE_DIR}/resources/shaders/*.vert")
file(GLOB SHADER_FRAG_SOURCES "${CMAKE_SOURCE_DIR}/resources/shaders/*.frag")

file(COPY ${SHADER_VERT_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
file(COPY ${SHADER_FRAG_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

# TEXTURES

file(GLOB SHADER_VERT_SOURCES "${CMAKE_SOURCE_DIR}/resources/textures/spritesheet/*.png")

file(COPY ${SHADER_VERT_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/textures)

