cmake_minimum_required(VERSION 3.16)
project(VoxelProject)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin)

message(STATUS ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src include)

 # OpenGL
 find_package(OpenGL REQUIRED)
 target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

 # FetchContent for External Libraries
include(FetchContent)

# GLEW

set(GLEW_BUILD_DEMOS OFF CACHE BOOL "Disable GLEW demos" FORCE )
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "Build the shared glew library" FORCE )

FetchContent_Declare(
    GLEW
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG master # Change to the latest stable version
    )

FetchContent_MakeAvailable(GLEW)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
target_include_directories(${PROJECT_NAME} PRIVATE ${glew_SOURCE_DIR}/include/GL)
target_link_libraries(${PROJECT_NAME} PRIVATE ${glew_BINARY_DIR}/lib/Release/glew.lib)

    
# GLFW

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE )
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE )
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE )

FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8 # Change to the latest stable version
)


FetchContent_MakeAvailable(GLFW)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# GLM
FetchContent_Declare(
    GLM
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8 # Change to the latest stable version
)
FetchContent_MakeAvailable(GLM)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)

# ASSIMP
set(ASSIMP_BUILD_EXAMPLES OFF CACHE BOOL "Disable Assimp examples" FORCE )
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp tests" FORCE )

FetchContent_Declare(
    ASSIMP
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.2.5 # Change to the latest stable version
)

FetchContent_MakeAvailable(ASSIMP)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# IMGUI (example setup with GLFW/GL3 backend)
#

# Shaders and Textures

file(GLOB SHADER_VERT_SOURCES "${CMAKE_SOURCE_DIR}/resources/shaders/*.vert")
file(GLOB SHADER_FRAG_SOURCES "${CMAKE_SOURCE_DIR}/resources/shaders/*.frag")

file(GLOB TEXTURE_SOURCES "${CMAKE_SOURCE_DIR}/resources/textures/spritesheet/*.png")


#file(COPY ${SHADER_VERT_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
#file(COPY ${SHADER_FRAG_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

if(CMAKE_CONFIGURATION_TYPES) # Multi-config generator
    # Multi-config generators (Visual Studio, Xcode)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} CONFIG_UPPER)
        file(COPY ${SHADER_VERT_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}}/shaders)
        file(COPY ${SHADER_FRAG_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}}/shaders)

        file(COPY ${TEXTURE_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}}/textures)

        message(STATUS "Copying shaders and textrures to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}} for ${config} configuration.")
    endforeach()
else() # Single-config generator
    # Single-config generators (Makefiles, Ninja)
    file(COPY ${SHADER_VERT_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
    file(COPY ${SHADER_FRAG_SOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
    message(STATUS "Copying shaders to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders for ${CMAKE_BUILD_TYPE}.")
endif()



