#pragma once

#include "Utility/Camera.h"

#include "Utility/Crosshair.h"

#include "Core/World/block.h"

#include <vector>


namespace Minecraft {

	class World;

	class Chunk;

	class Player {

	public:

		Player(class World* world);

		void PlaceBlock();
		void BreakBlock();

		void RedoVoxels();

		void UpdatePlayer();

		void Input();

		Camera& GetCameraReference() { return camera; }

		glm::vec3 playerPosition = glm::vec3(0.0f, 65.0f, 0.0f);

		glm::ivec3 playerChunkPosition = glm::ivec3(0, 0, 0);

		BlockType currentBlock = BlockType::Wood;

		int BreakDistance = 10;
	private:
		class World* world;

		Crosshair crosshair;

		bool pressed = false;
		bool leftPressed = false;

		Camera camera;
	};
}


