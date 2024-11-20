#pragma once
#include "../Core/World/world.h"

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

	class Raycast {

	public:
		Raycast(class Minecraft::World* worldContext);

		void ShootRay(RayHit& ray, glm::vec3 start, glm::vec3 direction, float stepAmount);

	private:
		class Minecraft::World* worldContext;

	};

}