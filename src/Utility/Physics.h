#pragma once
#include "Core/World/world.h"

namespace Physics {
	
	struct RayHit {
		bool hit = false;

		struct Minecraft::BlockData* hitBlock;
		glm::vec3 hitBlockPos;

		struct Minecraft::BlockData* adjacentBlock;
		glm::vec3 adjacentHitBlockPos;

		operator bool() const {
			return hit;
		}
	};

	void ShootRay(class Minecraft::World* worldContext, RayHit& ray, glm::vec3 start, glm::vec3 direction, float stepAmount);

}