#include "world.h"
#include "Core/game.h"

#include "FastNoiseLite.h"

#include <iostream>

#include <queue>
#include <tuple>

#include "Utility/Debug.h"



Minecraft::World::World(Game* game) : game(game)
{
	shader.BuildFiles("shaders/block.vert", "shaders/block.frag");
	textureUniformLocation = glGetUniformLocation(shader.GetID(), "textureAtlas");
}

void Minecraft::World::GenerateWorld()
{
	Utility::Timer timer("Generate World");

	for (int chunkX = -ChunksX; chunkX <= ChunksX; chunkX++)
	{
		for (int chunkZ = -ChunksZ; chunkZ <= ChunksZ; chunkZ++)
		{
			Chunk* chunk = new Chunk(game, this, glm::vec3(chunkX, 0, chunkZ));
			chunk->GenerateChunk(chunkX * CHUNK_SIZE, 0, chunkZ * CHUNK_SIZE);
			chunks.insert({ glm::vec3(chunkX, 0, chunkZ), chunk });
			chunk->ChunkReady = true;

		}
	}

	timer.EndTimer();

	Utility::Timer sunTimer("SunPropagateWorld");
	for (int chunkX = -ChunksX; chunkX <= ChunksX; chunkX++)
	{
		for (int chunkZ = -ChunksZ; chunkZ <= ChunksZ; chunkZ++)
		{
			if (chunks.find(glm::vec3(chunkX,0,chunkZ)) != chunks.end())
			{
				PropagateSunlight(chunks[glm::vec3(chunkX, 0, chunkZ)]);
			}
		}
		
	}

	sunTimer.EndTimer();

	Utility::Timer propTimer("Propagate");
	for (int chunkX = -ChunksX; chunkX <= ChunksX; chunkX++)
	{
		for (int chunkZ = -ChunksZ; chunkZ <= ChunksZ; chunkZ++)
		{
			if (chunks.find(glm::vec3(chunkX, 0, chunkZ)) != chunks.end())
			{
				chunks[glm::vec3(chunkX, 0, chunkZ)]->PropagateLighting(this);
			}
		}
		
	}
	propTimer.EndTimer();
	
	for (auto chunk : chunks) {
		chunk.second->SetupVoxels(this);
	}

	chunkThread = std::thread(&World::ManageChunks, this);
	chunkThread.detach();

	//MakeChunk(3, 0, 0);
}

void Minecraft::World::RenderChunks()
{
	for (const auto& chunk : chunks) {
		chunk.second->RenderVoxels();
	}
}

Minecraft::BlockData* Minecraft::World::GetBlockAt(int x, int y, int z)
{
	game->GetWorldReference()->worldLock.lock();

	glm::vec3 chunkCoord = glm::vec3(floor((float)x / CHUNK_SIZE), 0, floor((float)z / CHUNK_SIZE));

	auto it = chunks.find(chunkCoord);
	if (it != chunks.end())
	{
		glm::vec3 LocalPos = glm::vec3(x, y, z);

		BlockData* block = it->second->GetBlockAt(LocalPos);
		game->GetWorldReference()->worldLock.unlock();

		if (!block)
			return nullptr;

		return block;
	}
	game->GetWorldReference()->worldLock.unlock();
	
	return nullptr;
}

//Get Chunk Position in World positions (x,y,z)
Minecraft::Chunk* Minecraft::World::GetChunkAt(int x, int y, int z)
{
	worldLock.lock();

	glm::vec3 chunkCoord = glm::vec3(floor((float)x / CHUNK_SIZE), 0, floor((float)z / CHUNK_SIZE));

	if (chunks.find(chunkCoord) == chunks.end())
	{
		worldLock.unlock();
		return nullptr;
	}
	Chunk* chunk = chunks.find(chunkCoord)->second;
	worldLock.unlock();
	return chunk;
}

Minecraft::Chunk* Minecraft::World::GetChunkAtChunkPos(int x, int y, int z)
{
	worldLock.lock();

	glm::vec3 chunkCoord = glm::vec3(x, y, z);

	if (chunks.find(chunkCoord) == chunks.end())
	{
		worldLock.unlock();
		return nullptr;
	}
	Chunk* chunk = chunks.find(chunkCoord)->second;
	worldLock.unlock();
	return chunk;
}

void Minecraft::World::ManageChunks()
{
	while (true)
	{
		int processedChunks = 0;
		while(!chunksToGenerate.empty())
		{
			Chunk* chunk = chunksToGenerate.front();
			chunksToGenerate.pop();

			// Average of 0.2ms per chunk
			chunk->GenerateChunk(chunk->chunkPosition.x * CHUNK_SIZE, 0, chunk->chunkPosition.z * CHUNK_SIZE);

			PropagateSunlight(chunk);

			Utility::Timer timerL("Lighting");
			chunk->PropagateLighting(this);
			timerL.EndTimer();


			//Utility::Timer timer("Generate World");
			chunk->SetupVertices(this);
			//chunk->ForceCPUVoxelSetup = true;
			//chunk->SetupBoardingChunkVertices(this);
			//timer.EndTimer();

			//chunk->SetupRenderFlag = true;
			chunk->ChunkReady = true;
			processedChunks++;

			if (chunksToGenerate.empty() || processedChunks >= 50)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				processedChunks = 0;
				break;
			}
		}

		if (game->IsShuttingDown())
		{
			finishedThread = true;
			return;
		}

	}
}

void Minecraft::World::MakeChunk(int x,int y,int z)
{
	LoadChunk(x, y, z);	
}

void Minecraft::World::SetupChunks()
{
	for (const auto& chunk : chunks)
	{
		if (chunk.second->SetupRenderFlag == true)
		{
			chunk.second->RenderVoxels();
			chunk.second->SetupRenderFlag = false;
		}
	}
}

bool Minecraft::World::LoadChunk(int x, int y, int z)
{
	if (chunks.find(glm::vec3(x, y, z)) != chunks.end())
	{
		return false;
	}

	Chunk* chunk = new Chunk(game, this, glm::vec3(x, y, z));

	worldLock.lock();
	chunks.insert({ glm::vec3(x, y, z), chunk });

	chunksToGenerate.push(chunk);
	worldLock.unlock();

	//chunk->SetupVoxels(this);
	return true;
}

void Minecraft::World::UnloadChunk(Chunk* chunk)
{
	//worldLock.lock();

	if (chunk->threadCount > 0)
	{
		return;
	}

	chunks.erase(chunk->chunkPosition);

	if (chunk)
		delete chunk;

	//worldLock.unlock();
}

void Minecraft::World::UnloadChunks(glm::vec3 playerPos)
{
	worldLock.lock();

	//glm::distance(glm::vec2(c.second->chunkPosition.x, c.second->chunkPosition.z), glm::vec2(playerPos.x,playerPos.z)) > ChunksX
	for (const auto& c : chunks)
	{
		if (glm::distance(c.second->chunkPosition.x, playerPos.x) > ChunksX)
		{
			if(c.second->ChunkReady)
				chunksToDelete.insert({ glm::vec3(c.second->chunkPosition.x,0,c.second->chunkPosition.z), c.second });
			//UnloadChunk(c.second);
		}
		if (glm::distance(c.second->chunkPosition.z, playerPos.z) > ChunksZ)
		{
			if (c.second->ChunkReady)
				chunksToDelete.insert({ glm::vec3(c.second->chunkPosition.x,0,c.second->chunkPosition.z), c.second });
			//UnloadChunk(c.second);
		}
	}

	for (const auto& c : chunksToDelete)
	{
		UnloadChunk(c.second);
	}

	chunksToDelete.clear();

	worldLock.unlock();
}

bool Minecraft::World::BlockExistsAt(int x, int y, int z)
{
	BlockData* block = GetBlockAt(x, y, z);
	return block && block->blockType != BlockType::Air;
}

bool Minecraft::World::BlockExistsAt(glm::ivec3 pos)
{
	return BlockExistsAt(pos.x, pos.y, pos.z);
}

bool Minecraft::World::PlaceBlockAt(BlockType type, glm::ivec3 pos)
{
	glm::vec3 chunkCoord = glm::vec3(floor((float)pos.x / CHUNK_SIZE), floor((float)pos.y / CHUNK_HEIGHT), floor((float)pos.z / CHUNK_SIZE));

	worldLock.lock();
	auto it = chunks.find(chunkCoord);
	worldLock.unlock();

	if (it != chunks.end())
	{
		glm::vec3 LocalPos = glm::vec3(pos.x, pos.y, pos.z);

		BlockData* block = GetBlockAt(pos.x, pos.y, pos.z);

		if (!block)
			return false;

		if (block->blockType == BlockType::Air) {
			BlockData block;
			block.blockType = type;

			if (block.blockType == Minecraft::BlockType::Torch)
			{
				BlockData* placedBlock = it->second->GetBlockAt(LocalPos);
				*placedBlock = block;
				placedBlock->blockLight = 7;
				it->second->SetBlockPropagate(LocalPos);
				it->second->PropagateLighting(this);
			}
			else
			{
				std::vector<Chunk*> chunksToPropagate;

				Utility::Timer timer("Remove Lighting");
				it->second->SetBlockRemovePropagate(LocalPos);
				it->second->RemovePropagateLighting(this, &chunksToPropagate);
				timer.EndTimer();

				*it->second->GetBlockAt(LocalPos) = block;

				Utility::Timer timerRS("Remove Sunlight");
				RemoveSunlight(it->second, pos.x, pos.y, pos.z);
				timerRS.EndTimer();
				
				Utility::Timer timerProp("Propagate Lighting");
				it->second->PropagateLighting(this);
				it->second->SetupVoxels(this);
				timerProp.EndTimer();

				for (Chunk* c : chunksToPropagate)
				{
					c->PropagateLighting(this);
				}
			}

			if (int(LocalPos.x) == (CHUNK_SIZE * (chunkCoord.x + 1) - 1)) { UpdateNeighborChunk(chunkCoord + glm::vec3(1, 0, 0)); }
			if (int(LocalPos.x) == CHUNK_SIZE * chunkCoord.x) { UpdateNeighborChunk(chunkCoord - glm::vec3(1, 0, 0)); }
			if (int(LocalPos.y) == (CHUNK_SIZE * (chunkCoord.y + 1) - 1)) { UpdateNeighborChunk(chunkCoord + glm::vec3(0, 1, 0)); }
			if (int(LocalPos.y) == CHUNK_SIZE * chunkCoord.y) { UpdateNeighborChunk(chunkCoord - glm::vec3(0, 1, 0)); }
			if (int(LocalPos.z) == (CHUNK_SIZE * (chunkCoord.z + 1) - 1)) { UpdateNeighborChunk(chunkCoord + glm::vec3(0, 0, 1)); }
			if (int(LocalPos.z) == CHUNK_SIZE * chunkCoord.z) { UpdateNeighborChunk(chunkCoord - glm::vec3(0, 0, 1)); }


			//std::cout << "Deleting at : " << LocalPos.x << ", " << LocalPos.y << ", " << LocalPos.z << std::endl;

			
			return true;
		}

	}
	return false;
}

bool Minecraft::World::DeleteBlockAt(glm::ivec3 pos)
{
	glm::vec3 chunkCoord = glm::vec3(floor((float)pos.x / CHUNK_SIZE), 0, floor((float)pos.z / CHUNK_SIZE));

	worldLock.lock();
	auto it = chunks.find(chunkCoord);
	worldLock.unlock();

	if (it != chunks.end())
	{
		glm::vec3 LocalPos = glm::vec3(pos.x, pos.y, pos.z);

		BlockData block;

		if (it->second->GetBlockAt(LocalPos)) {
			*it->second->GetBlockAt(LocalPos) = block;

			const int directions[6][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1} };

			for (int i = 0; i < 6; ++i)
			{
				int nx = LocalPos.x + directions[i][0];
				int ny = LocalPos.y + directions[i][1];
				int nz = LocalPos.z + directions[i][2];			
				Chunk* neighborChunk = GetChunkAt(nx, ny, nz);
				BlockData* neighborBlock = neighborChunk->GetBlockAt(nx, ny, nz);

				if (neighborChunk && neighborBlock && neighborBlock->blockType == BlockType::Air)
				{
					//std::cout << "Propagating" << std::endl;
					neighborChunk->SetBlockPropagate(glm::vec3(nx,ny,nz));
					neighborChunk->PropagateLighting(this);
				}
			}
			RecalculateSunlight(it->second, pos.x, pos.z);

			const int sunDirections[6][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };

			for (int i = 0; i < 4; ++i)
			{
				Chunk* neighborChunk = GetChunkAt(pos.x + sunDirections[i][0], 0, pos.z + sunDirections[i][2]);
				if (neighborChunk)
				{
					RecalculateSunlight(neighborChunk, pos.x + sunDirections[i][0], pos.z + sunDirections[i][2]);
					neighborChunk->PropagateLighting(this);
				}
			}
			it->second->PropagateLighting(this);
			it->second->SetupVoxels(this);


			if (int(LocalPos.x) == (CHUNK_SIZE * (chunkCoord.x + 1) - 1)) { UpdateNeighborChunk(chunkCoord + glm::vec3(1, 0, 0)); }
			if (int(LocalPos.x) == CHUNK_SIZE * chunkCoord.x) { UpdateNeighborChunk(chunkCoord - glm::vec3(1, 0, 0)); }
			if (int(LocalPos.y) == (CHUNK_SIZE * (chunkCoord.y + 1) - 1)) { UpdateNeighborChunk(chunkCoord + glm::vec3(0, 1, 0)); }
			if (int(LocalPos.y) == CHUNK_SIZE * chunkCoord.y) { UpdateNeighborChunk(chunkCoord - glm::vec3(0, 1, 0)); }
			if (int(LocalPos.z) == (CHUNK_SIZE * (chunkCoord.z + 1) - 1)) { UpdateNeighborChunk(chunkCoord + glm::vec3(0, 0, 1)); }
			if (int(LocalPos.z) == CHUNK_SIZE * chunkCoord.z) { UpdateNeighborChunk(chunkCoord - glm::vec3(0, 0, 1)); }

			//std::cout << "Deleting at : " << LocalPos.x << ", " << LocalPos.y << ", " << LocalPos.z << std::endl;
			
			return true;
		}

	}
	return false;
}

void Minecraft::World::UpdateNeighborChunk(glm::ivec3 neighborChunkCoords)
{
	auto neighborIt = chunks.find(neighborChunkCoords);
	if (neighborIt != chunks.end())
	{
		neighborIt->second->PropagateLighting(this);
		neighborIt->second->SetupVoxels(this);
	}
}

void Minecraft::World::PropagateSunlight(Chunk* chunk)
{
	//std::cout << "CHUNK POSITION : " << chunk->chunkPosition.x << " CHUNK POSITION Y : " << chunk->chunkPosition.y << " CHUNK POSITION Z : " << chunk->chunkPosition.z << std::endl;

	for (int x = chunk->chunkPosition.x * CHUNK_SIZE; x < (chunk->chunkPosition.x + 1) * CHUNK_SIZE; ++x)
	{
		for (int z = chunk->chunkPosition.z * CHUNK_SIZE; z < (chunk->chunkPosition.z + 1) * CHUNK_SIZE; ++z)
		{
			RecalculateSunlight(chunk, x, z);
		}
	}

}

void Minecraft::World::RecalculateSunlight(Chunk* chunk, int x, int z)
{

	Chunk* c = GetChunkAt(x, 0, z);

	for (int y = CHUNK_HEIGHT - 1; y > 0; --y)
	{
		BlockData* block = GetBlockAt(x, y, z);

		if (block != nullptr && block->blockType == Minecraft::Air)
		{
			block->blockLight = MAX_SKYLIGHT_VALUE; 
			c->SetBlockPropagate(glm::ivec3(x, y, z));
		}
		else
		{
			return;
		}
	}

	chunk->PropagateLighting(this);
}

void Minecraft::World::RemoveSunlight(Chunk* chunk, int x, int _y, int z)
{
	std::vector<Chunk*> chunksToPropagate;

	const int directions[4][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
	for (int y = _y - 1; y > 0; --y)
	{
		BlockData* block = GetBlockAt(x, y, z);
		if (block != nullptr && block->blockType == Minecraft::Air)
		{
			if (block->blockLight == 15)
			{
				block->blockLight = 0; // Reset the light value
				chunk->SetBlockRemovePropagate(glm::vec3(x, y, z));

				for (int i = 0; i < 4; ++i)
				{
					int nx = x + directions[i][0];
					int nz = z + directions[i][2];
				
					BlockData* neighborBlock = GetBlockAt(nx, y, nz);
					Chunk* neighborChunk = GetChunkAt(nx, y, nz);
					
					if (neighborChunk != nullptr && neighborBlock != nullptr)
					{
						neighborChunk->SetBlockRemovePropagate(glm::vec3(nx, y, nz));
						std::cout << "Propagating at " << nx << ", " << y << ", " << nz << std::endl;
						
						if (std::find(chunksToPropagate.begin(), chunksToPropagate.end(), neighborChunk) == chunksToPropagate.end())
						{
							chunksToPropagate.push_back(neighborChunk);
						}
				
					}
					else
					{
						std::cout << "ERROR" << std::endl;
					}
				}
			}
		}
		else
		{
			std::cout << "At " << x << ", " << y << ", " << z << std::endl;
			break;
		}
	}

	chunk->RemovePropagateLighting(this);

	RecalculateSunlight(chunk, x, z);
	for (Chunk* c : chunksToPropagate)
	{
		RecalculateSunlight(c, x,z);
	}
}
