#pragma once

#include <glew.h>

#include <glm.hpp>
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "Shader.h"

namespace Utility {
	
	class DebugDrawer {
	public:
		virtual void Render() = 0;
	protected:
		unsigned int VAO, VBO, EBO;
	};

	class WireframeCube : DebugDrawer {
	public:
		WireframeCube(glm::vec3 cubePosition);
		~WireframeCube();

		glm::vec3 cubePosition;
		Shader wireframeCubeShader;

		void Render();
	};

}