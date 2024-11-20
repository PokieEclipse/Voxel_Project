#include "Crosshair.h"

#include <vector>

Crosshair::Crosshair()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	shader.BuildFiles("shaders/crosshair.vert", "shaders/crosshair.frag");

	std::vector<float> vertices = {
		 0.005f,  0.01f, 0.0f, // Top vertex
	   -0.005f, -0.01f, 0.0f, // Bottom left vertex
		0.005f, -0.01f, 0.0f,  // Bottom right vertex
		-0.005f, 0.01f, 0.0f  // Top Left vertex
	};

	std::vector<unsigned int> e = {
		0,1,2,
		1,0,3
	};

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, e.size() * sizeof(unsigned int), e.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

void Crosshair::RenderCrosshair()
{
	glBindVertexArray(VAO);
	shader.Bind();

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

}
