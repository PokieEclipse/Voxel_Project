#pragma once

#include <glew.h>

#include "Shader.h"

class Crosshair {

public:
	Crosshair();

	void RenderCrosshair();
private:

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Utility::Shader shader;

};