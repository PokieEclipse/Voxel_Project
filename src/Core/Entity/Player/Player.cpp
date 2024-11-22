#include "Player.h"

#include <iostream>
#include <algorithm>

#include <thread>
#include <mutex>

#include "Core/World/world.h"

#include "Utility/Physics.h"

#include "Utility/DebugDraw.h"



Minecraft::Player::Player(Minecraft::World* _world) : world(_world)
{
}

void Minecraft::Player::PlaceBlock()
{

	glm::vec3 pos = camera.cameraPosition;

	Physics::RayHit ray;

	ShootRay(world, ray, pos, camera.cameraForwardVector, BreakDistance);

	if (ray && ray.adjacentBlock)
	{
		world->PlaceBlockAt(currentBlock, ray.adjacentHitBlockPos);
	}
}

void Minecraft::Player::BreakBlock()
{
	glm::vec3 pos = camera.cameraPosition;

	Physics::RayHit ray;

	ShootRay(world, ray, pos, camera.cameraForwardVector, BreakDistance);

	if (ray)
	{
		world->DeleteBlockAt(ray.hitBlockPos);
	}

}

void Minecraft::Player::RedoVoxels()
{
	glm::vec3 pos = camera.cameraPosition;

	Physics::RayHit ray;

	ShootRay(world, ray, pos, camera.cameraForwardVector, BreakDistance);

	if (ray)
	{
		world->GetChunkAt(ray.hitBlockPos.x, ray.hitBlockPos.y, ray.hitBlockPos.z)->SetupVoxels(world);
	}
}

void Minecraft::Player::UpdatePlayer()
{
	crosshair.RenderCrosshair();

	Input();

	camera.cameraPosition = playerPosition;

	playerChunkPosition = glm::vec3(floor((float)playerPosition.x / world->CHUNK_SIZE), floor((float)playerPosition.y / world->CHUNK_SIZE), floor((float)playerPosition.z / world->CHUNK_SIZE));

	for (int x = -world->ChunksX; x <= world->ChunksX; ++x)
	{
		for (int z = -world->ChunksZ; z <= world->ChunksZ; ++z)
		{
			glm::vec3 chunkPos = playerChunkPosition + glm::ivec3(x, 0, z);

			world->MakeChunk(chunkPos.x, 0, chunkPos.z);

			world->UnloadChunks(glm::vec3(playerChunkPosition));
		}
		
	}

	Physics::RayHit ray;
	
	Physics::ShootRay(world, ray, camera.cameraPosition, camera.cameraForwardVector, BreakDistance);
	
	if (ray)
	{
		Utility::WireframeCube wCube(ray.hitBlockPos, world);
	
		wCube.Render();
	}
}

void Minecraft::Player::Input()
{
	float CameraSpeed = 0.5f;

	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		world->game->ShutdownGame();
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_Q) == GLFW_PRESS)
	{
		playerPosition.y += CameraSpeed;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_E) == GLFW_PRESS)
	{
		playerPosition.y -= CameraSpeed;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_W) == GLFW_PRESS)
	{
		playerPosition += CameraSpeed * camera.cameraForwardVector;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_S) == GLFW_PRESS)
	{
		playerPosition -= CameraSpeed * camera.cameraForwardVector;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_A) == GLFW_PRESS)
	{
		playerPosition -= CameraSpeed * glm::cross(camera.cameraForwardVector, camera.cameraUpVector);
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_D) == GLFW_PRESS)
	{
		playerPosition += CameraSpeed * glm::cross(camera.cameraForwardVector, camera.cameraUpVector);
	}

	if (glfwGetMouseButton(world->game->GetGameWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !leftPressed)
	{
		BreakBlock();
		leftPressed = true;
	}
	if (glfwGetMouseButton(world->game->GetGameWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		leftPressed = false;
	}

	if (glfwGetMouseButton(world->game->GetGameWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !pressed)
	{
		PlaceBlock();
		pressed = true;
	}
	if (glfwGetMouseButton(world->game->GetGameWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		pressed = false;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_O) == GLFW_PRESS)
	{
		currentBlock = Minecraft::BlockType::Dirt;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_P) == GLFW_PRESS)
	{
		currentBlock = Minecraft::BlockType::Torch;
	}
	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_T) == GLFW_PRESS)
	{
		RedoVoxels();
	}

	if (glfwGetKey(world->game->GetGameWindow(), GLFW_KEY_Y) == GLFW_PRESS)
	{
		for (const auto& chunk : world->chunks)
		{
			if (chunk.second->SetupRenderFlag == true)
			{
				
				chunk.second->PropagateLighting(world);
				chunk.second->SetupVoxels(world);
				chunk.second->SetupRenderFlag = false;
			}
		}
	}
}
