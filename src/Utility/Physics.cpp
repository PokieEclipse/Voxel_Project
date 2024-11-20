#include "Physics.h"

Physics::Raycast::Raycast(Minecraft::World* worldContext) : worldContext(worldContext)
{
}

void Physics::Raycast::ShootRay(Physics::RayHit& ray, glm::vec3 start, glm::vec3 direction, float stepAmount)
{
	for (float i = 0; i < stepAmount; i += 0.01)
	{
		glm::vec3 rayPos = start + (float)i * glm::normalize(direction);

		// Floor the position values to get the correct block coordinates
		glm::ivec3 blockPos(round(rayPos.x), round(rayPos.y), round(rayPos.z));

		if (worldContext->BlockExistsAt(blockPos)) {
			ray.hit = true;
			ray.hitBlock = worldContext->GetBlockAt(blockPos.x, blockPos.y, blockPos.z);
			ray.hitBlockPos = blockPos;


			// Start with a large distance and hit normal (initially none)
			float minDist = FLT_MAX;  // Start with the maximum float value
			glm::ivec3 hitNormal(0);

			// Compare distances for each face of the block
			float dist = glm::distance(rayPos, glm::vec3(blockPos) + glm::vec3(0.5, 0, 0));  // Right face
			if (dist < minDist) {
				minDist = dist;
				hitNormal = glm::ivec3(1, 0, 0);
			}

			dist = glm::distance(rayPos, glm::vec3(blockPos) + glm::vec3(-0.5, 0, 0));  // Left face
			if (dist < minDist) {
				minDist = dist;
				hitNormal = glm::ivec3(-1, 0, 0);
			}

			dist = glm::distance(rayPos, glm::vec3(blockPos) + glm::vec3(0, 0.5, 0));  // Top face
			if (dist < minDist) {
				minDist = dist;
				hitNormal = glm::ivec3(0, 1, 0);
			}

			dist = glm::distance(rayPos, glm::vec3(blockPos) + glm::vec3(0, -0.5, 0));  // Bottom face
			if (dist < minDist) {
				minDist = dist;
				hitNormal = glm::ivec3(0, -1, 0);
			}

			dist = glm::distance(rayPos, glm::vec3(blockPos) + glm::vec3(0, 0, 0.5));  // Front face
			if (dist < minDist) {
				minDist = dist;
				hitNormal = glm::ivec3(0, 0, 1);
			}

			dist = glm::distance(rayPos, glm::vec3(blockPos) + glm::vec3(0, 0, -0.5));  // Back face
			if (dist < minDist) {
				minDist = dist;
				hitNormal = glm::ivec3(0, 0, -1);
			}

			glm::ivec3 newBlockPos = blockPos + hitNormal;
			Minecraft::BlockData* adjBlock = worldContext->GetBlockAt(newBlockPos.x, newBlockPos.y, newBlockPos.z);

			if (adjBlock)
			{
				ray.adjacentBlock = adjBlock;
				ray.adjacentHitBlockPos = newBlockPos;
			}

			return;
		}
	}

	ray.hit = false;
	
}
