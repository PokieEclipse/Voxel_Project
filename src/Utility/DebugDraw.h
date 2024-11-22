#pragma once

#include <glew.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"

namespace Minecraft {
	class World;
	class Player;
}

namespace Utility {
	
	class DebugDrawer {
	public:
		virtual void Render() = 0;
	protected:
		unsigned int VAO, VBO, EBO;
	};

	class WireframeCube : DebugDrawer {
	public:
		WireframeCube(glm::vec3 cubePosition, class Minecraft::World* worldContext);
		~WireframeCube();


		void Render();

	private:
		glm::vec3 cubePosition;
		Shader wireframeCubeShader;

		class Minecraft::World* worldContext;

	};

}