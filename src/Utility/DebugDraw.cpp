#include "DebugDraw.h"
#include <vector>

#include "Camera.h"

#include <iostream>

#include <glfw/glfw3.h>

#include "Core/game.h"
#include "Core/World/world.h"
#include "Core/Entity/Player/Player.h"


Utility::WireframeCube::WireframeCube(glm::vec3 cubePosition, Minecraft::World* world) : cubePosition(cubePosition), worldContext(world)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

    wireframeCubeShader.BuildFiles("shaders/wireframe_cube.vert", "shaders/wireframe_cube.frag");

}

Utility::WireframeCube::~WireframeCube()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Utility::WireframeCube::Render()
{

    std::vector<float> vertices = {
        -0.51f + cubePosition.x, -0.51f + cubePosition.y, -0.51f + cubePosition.z, // Vertex 0
        0.51f + cubePosition.x, -0.51f + cubePosition.y, -0.51f + cubePosition.z,  // Vertex 1
        0.51f + cubePosition.x, 0.51f + cubePosition.y, -0.51f + cubePosition.z,   // Vertex 2
        -0.51f + cubePosition.x, 0.51f + cubePosition.y, -0.51f + cubePosition.z,  // Vertex 3
        -0.51f + cubePosition.x, -0.51f + cubePosition.y, 0.51f + cubePosition.z,  // Vertex 4
        0.51f + cubePosition.x, -0.51f + cubePosition.y, 0.51f + cubePosition.z,   // Vertex 5
        0.51f + cubePosition.x, 0.51f + cubePosition.y, 0.51f + cubePosition.z,    // Vertex 6
        -0.51f + cubePosition.x, 0.51f + cubePosition.y, 0.51f + cubePosition.z    // Vertex 7
    };

    std::vector<unsigned int> indices = {
        // Bottom face edges
        0, 1, 1, 2, 2, 3, 3, 0,

        // Top face edges
        4, 5, 5, 6, 6, 7, 7, 4,

        // Vertical edges connecting bottom and top faces
        0, 4, 1, 5, 2, 6, 3, 7
    };

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    glm::mat4 identity(1.0f);

	wireframeCubeShader.Bind();

    glUniformMatrix4fv(glGetUniformLocation(wireframeCubeShader.GetID(), "modelMatrix"), 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(glGetUniformLocation(wireframeCubeShader.GetID(), "viewMatrix"), 1, GL_FALSE, glm::value_ptr(worldContext->game->GetPlayerReference()->GetCameraReference().viewSpace));
    glUniformMatrix4fv(glGetUniformLocation(wireframeCubeShader.GetID(), "perspectiveMatrix"), 1, GL_FALSE, glm::value_ptr(perspective));

    float timeVar = glfwGetTime();
    //std::cout << timeVar << std::endl;
    glUniform1f(glGetUniformLocation(wireframeCubeShader.GetID(), "time"), timeVar);

    //glDisable(GL_DEPTH_TEST);

    glLineWidth(5.0f);
	glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
    //glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0);
}

