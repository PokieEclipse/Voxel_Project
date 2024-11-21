#include "Chunk.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "Utility/Camera.h"

#include "FastNoiseLite.h"

#include "world.h"
#include "block.h"

#include <algorithm>

#include "Utility/Debug.h"

extern Camera Utility::camera;
extern glm::mat4 Utility::perspective;


Minecraft::Chunk::Chunk(Game* game, World* world, glm::vec3 chunkPos) : game(game), _world(world), blocks(World::CHUNK_SIZE * World::CHUNK_SIZE * World::CHUNK_HEIGHT), chunkPosition(chunkPos)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	chunkMatrix = glm::translate(chunkMatrix, glm::vec3(chunkPos.x - (chunkPosition.x), 0, chunkPos.z - chunkPosition.z));
}

Minecraft::Chunk::~Chunk()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

inline int Minecraft::Chunk::GetIndex(int x, int y, int z) const
{
	return x + World::CHUNK_SIZE * (z + World::CHUNK_SIZE * y);
}

void Minecraft::Chunk::GenerateChunk(int posX, int posY, int posZ)
{	
	ChunkGuard guard(this);

	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noise.SetFrequency(0.03f);
	noise.SetFractalOctaves(5.0f);
	noise.SetFractalLacunarity(2.0f);
	noise.SetFractalGain(0.5f);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);

	for (int x = 0; x < Minecraft::World::CHUNK_SIZE; x++)
	{
		for (int z = 0; z < Minecraft::World::CHUNK_SIZE; z++)
		{
			float height = 60.0f + noise.GetNoise((float)x + posX - 1, (float)z + posZ) * 20.0f;

			int nx = x; int nz = z;
			if (chunkPosition.x < 0)
			{
				nx = abs(x - World::CHUNK_SIZE - 1);
			}
			if (chunkPosition.z < 0)
			{
				nz = abs(z - World::CHUNK_SIZE - 1);
			}

			for (int y = 0; y < Minecraft::World::CHUNK_HEIGHT; y++)
			{
				BlockData block;

				if (y < 50 + (sin((x + chunkPosition.x * World::CHUNK_SIZE) * 0.1f) * 10.0f) + (sin((z + chunkPosition.z * World::CHUNK_SIZE) * 0.1f) * 10.0f))
				{
					block.blockType = BlockType::Grass;
				}

				chunkLock.lock();
				blocks[GetIndex(x, y, z)] = block;
				chunkLock.unlock();
			}
		}
	}

	//BlockData block;
	//block.blockType = BlockType::Air;
	//
	//for (int x = 0; x < Minecraft::World::CHUNK_SIZE; x++)
	//{
	//	for (int z = 0; z < Minecraft::World::CHUNK_SIZE; z++)
	//	{
	//
	//		for (int y = 0; y < Minecraft::World::CHUNK_HEIGHT; y++)
	//		{
	//			float density = noise.GetNoise((float)x + posX, (float)y + posY, (float)z + posZ) * 10;
	//
	//			if (density < -2)
	//			{
	//				GetBlockAt(x,y,z)->blockType = block.blockType;
	//			}
	//		}
	//	}
	//}

	ChunkReadyLighting = true;

}

void Minecraft::Chunk::SetupBoardingChunkVertices(World* world)
{
	ChunkGuard guard(this);

	const int directions[4][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };

	for (int i = 0; i < 4; ++i)
	{
		int nx = chunkPosition.x + directions[i][0];
		int nz = chunkPosition.z + directions[i][2];

		Chunk* chunk = world->GetChunkAtChunkPos(nx, 0, nz);
		if (chunk)
		{
			//std::cout << chunk << std::endl;
			chunk->SetupVertices(world);
			//ForceCPUVoxelSetup = true;
		}
	}
}

float Minecraft::Chunk::CalculateAO(bool side1, bool side2, bool corner, bool calculateAmbient)
{
	// - https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/

	if (!calculateAmbient) {
		return 3;
	}
	if (side1 && side2) {
		return 0;
	}
	return 3 - (side1 + side2 + corner);
}

void Minecraft::Chunk::SetupVertices(World* world)
{
	ChunkGuard guard(this);

	chunkLock.lock();

	vertices.clear();
	indices.clear();

	if (blocks.empty()) 
	{
		chunkLock.unlock();
		return;
	};

	chunkLock.unlock();

	unsigned int i = 0;
	for (int x = World::CHUNK_SIZE * chunkPosition.x; x < World::CHUNK_SIZE * (chunkPosition.x + 1); ++x)
	{
		for (int y = World::CHUNK_SIZE * chunkPosition.y; y < World::CHUNK_HEIGHT * (chunkPosition.y + 1); ++y)
		{
			for (int z = World::CHUNK_SIZE * chunkPosition.z; z < World::CHUNK_SIZE * (chunkPosition.z + 1); ++z)
			{
				//chunkLock.unlock();

				bool AmbientOcclusion = true;

				BlockData* block = GetBlockAt(x, y, z);
				if (!block || block->blockType == BlockType::Air) continue;

				BlockTexture texture = GetBlockTexture(block->blockType);

				float textureRow = texture.texture.row;
				float textureColumn = texture.texture.column;

				bool ShowFrontFace = !world->BlockExistsAt(x, y, z + 1);
				bool ShowBackFace = !world->BlockExistsAt(x, y, z - 1);

				bool ShowLeftFace = !world->BlockExistsAt(x - 1, y, z);
				bool ShowRightFace = !world->BlockExistsAt(x + 1, y, z);

				bool ShowTopFace = !world->BlockExistsAt(x, y + 1, z);
				bool ShowBottomFace = !world->BlockExistsAt(x, y - 1, z);

				// VERTICE POSTIONS // TEXTURE COORDINATES // TEXTURE POSITIONS IN ATLAS // LIGHT VALUE // AMBIENT OCCLUSION VALUE

				// FRONT FACE
				if (ShowFrontFace) {
					float lightValue = (world->GetBlockAt(x, y, z + 1) != nullptr && world->GetBlockAt(x, y, z + 1)->blockType == Minecraft::Air) ? world->GetBlockAt(x, y, z + 1)->blockLight : 0;

					float ao1 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z + 1), // Left neighbor
						world->BlockExistsAt(x, y - 1, z + 1), // Front neighbor
						world->BlockExistsAt(x - 1, y - 1, z + 1) // Diagonal
						, world->enableAO);

					float ao2 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z + 1), // Left neighbor
						world->BlockExistsAt(x, y - 1, z + 1), // Back neighbor
						world->BlockExistsAt(x + 1, y - 1, z + 1) // Diagonal
						, world->enableAO);

					float ao3 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z + 1), // Right neighbor
						world->BlockExistsAt(x, y + 1, z + 1), // Back neighbor
						world->BlockExistsAt(x + 1, y + 1, z + 1) // Diagonal
						, world->enableAO);

					float ao4 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z + 1), // Right neighbor
						world->BlockExistsAt(x, y + 1, z + 1), // Front neighbor
						world->BlockExistsAt(x - 1, y + 1, z + 1) // Diagonal
						, world->enableAO);

					vertices.insert(vertices.end(), {
							x - 0.5f, y - 0.5f, z + 0.5f,  0.0f, 1.0f, textureRow, textureColumn, lightValue, ao1,
							x + 0.5f, y - 0.5f, z + 0.5f,  1.0f, 1.0f, textureRow, textureColumn, lightValue, ao2,
							x + 0.5f, y + 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn, lightValue, ao3,
							x - 0.5f, y + 0.5f, z + 0.5f,  0.0f, 0.0f, textureRow, textureColumn, lightValue, ao4,
						});

					indices.insert(indices.end(), {
						0 + (i * 4), 1 + (i * 4), 2 + (i * 4),
						2 + (i * 4), 3 + (i * 4), 0 + (i * 4)
						});
					++i;
				}
				// BACK FACE
				if (ShowBackFace) {
					float lightValue = (world->GetBlockAt(x, y, z - 1) != nullptr && world->GetBlockAt(x, y, z - 1)->blockType == Minecraft::Air) ? world->GetBlockAt(x, y, z - 1)->blockLight : 0;

					float ao1 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z - 1), // Left neighbor
						world->BlockExistsAt(x, y - 1, z - 1), // Front neighbor
						world->BlockExistsAt(x - 1, y - 1, z - 1) // Diagonal
						, world->enableAO);

					float ao2 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z - 1), // Left neighbor
						world->BlockExistsAt(x, y + 1, z - 1), // Back neighbor
						world->BlockExistsAt(x - 1, y + 1, z - 1) // Diagonal
						, world->enableAO);

					float ao3 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z - 1), // Right neighbor
						world->BlockExistsAt(x, y + 1, z - 1), // Back neighbor
						world->BlockExistsAt(x + 1, y + 1, z - 1) // Diagonal
						, world->enableAO);

					float ao4 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z - 1), // Right neighbor
						world->BlockExistsAt(x, y - 1, z - 1), // Front neighbor
						world->BlockExistsAt(x + 1, y - 1, z - 1) // Diagonal
						, world->enableAO);

					vertices.insert(vertices.end(), {
							x - 0.5f, y - 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn,lightValue, ao1,
							x - 0.5f, y + 0.5f, z - 0.5f,  0.0f, 0.0f, textureRow, textureColumn,lightValue, ao2,
							x + 0.5f, y + 0.5f, z - 0.5f,  1.0f, 0.0f, textureRow, textureColumn,lightValue, ao3,
							x + 0.5f, y - 0.5f, z - 0.5f,  1.0f, 1.0f, textureRow, textureColumn,lightValue, ao4,
						});
					indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
						});
					++i;
				}
				// LEFT FACE
				if (ShowLeftFace) {
					float lightValue = (world->GetBlockAt(x - 1, y, z) != nullptr && world->GetBlockAt(x - 1, y, z)->blockType == Minecraft::Air) ? world->GetBlockAt(x - 1, y, z)->blockLight : 0;

					float ao1 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z - 1), // Left neighbor
						world->BlockExistsAt(x - 1, y - 1, z), // Front neighbor
						world->BlockExistsAt(x - 1, y - 1, z - 1) // Diagonal
						, world->enableAO);

					float ao2 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z + 1), // Left neighbor
						world->BlockExistsAt(x - 1, y - 1, z), // Back neighbor
						world->BlockExistsAt(x - 1, y - 1, z + 1) // Diagonal
						, world->enableAO);

					float ao3 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z + 1), // Right neighbor
						world->BlockExistsAt(x - 1, y + 1, z), // Back neighbor
						world->BlockExistsAt(x - 1, y + 1, z + 1) // Diagonal
						, world->enableAO);

					float ao4 = CalculateAO(
						world->BlockExistsAt(x - 1, y, z - 1), // Right neighbor
						world->BlockExistsAt(x - 1, y + 1, z), // Front neighbor
						world->BlockExistsAt(x - 1, y + 1, z - 1) // Diagonal
						, world->enableAO);

					vertices.insert(vertices.end(), {
							x - 0.5f, y - 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn, lightValue, ao1,
							x - 0.5f, y - 0.5f, z + 0.5f,  1.0f, 1.0f, textureRow, textureColumn, lightValue, ao2,
							x - 0.5f, y + 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn, lightValue, ao3,
							x - 0.5f, y + 0.5f, z - 0.5f,  0.0f, 0.0f, textureRow, textureColumn, lightValue, ao4,
						});
					indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
						});
					++i;
				}
				// RIGHT FACE
				if (ShowRightFace) {
					float lightValue = (world->GetBlockAt(x + 1, y, z) != nullptr && world->GetBlockAt(x + 1, y, z)->blockType == Minecraft::Air) ? world->GetBlockAt(x + 1, y, z)->blockLight : 0;

					float ao1 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z - 1), // Left neighbor
						world->BlockExistsAt(x + 1, y - 1, z), // Front neighbor
						world->BlockExistsAt(x + 1, y - 1, z - 1) // Diagonal
						, world->enableAO);

					float ao2 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z - 1), // Left neighbor
						world->BlockExistsAt(x + 1, y + 1, z), // Back neighbor
						world->BlockExistsAt(x + 1, y + 1, z - 1) // Diagonal
						, world->enableAO);

					float ao3 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z + 1), // Right neighbor
						world->BlockExistsAt(x + 1, y + 1, z), // Back neighbor
						world->BlockExistsAt(x + 1, y + 1, z + 1) // Diagonal
						, world->enableAO);

					float ao4 = CalculateAO(
						world->BlockExistsAt(x + 1, y, z + 1), // Right neighbor
						world->BlockExistsAt(x + 1, y - 1, z), // Front neighbor
						world->BlockExistsAt(x + 1, y - 1, z + 1) // Diagonal
						, world->enableAO);

					vertices.insert(vertices.end(), {
							x + 0.5f, y - 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn,lightValue, ao1,
							x + 0.5f, y + 0.5f, z - 0.5f,  0.0f, 0.0f, textureRow, textureColumn,lightValue, ao2,
							x + 0.5f, y + 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn,lightValue, ao3,
							x + 0.5f, y - 0.5f, z + 0.5f,  1.0f, 1.0f, textureRow, textureColumn,lightValue, ao4,
						});
					indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
						});
					++i;
				}
				// TOP FACE
				if (ShowTopFace) {

					float ao1 = CalculateAO(
						world->BlockExistsAt(x - 1, y + 1, z), // Left neighbor
						world->BlockExistsAt(x, y + 1, z - 1), // Front neighbor
						world->BlockExistsAt(x - 1, y + 1, z - 1) // Diagonal
						, world->enableAO);

					float ao2 = CalculateAO(
						world->BlockExistsAt(x - 1, y + 1, z), // Left neighbor
						world->BlockExistsAt(x, y + 1, z + 1), // Back neighbor
						world->BlockExistsAt(x - 1, y + 1, z + 1) // Diagonal
						, world->enableAO);

					float ao3 = CalculateAO(
						world->BlockExistsAt(x + 1, y + 1, z), // Right neighbor
						world->BlockExistsAt(x, y + 1, z + 1), // Back neighbor
						world->BlockExistsAt(x + 1, y + 1, z + 1) // Diagonal
						, world->enableAO);

					float ao4 = CalculateAO(
						world->BlockExistsAt(x + 1, y + 1, z), // Right neighbor
						world->BlockExistsAt(x, y + 1, z - 1), // Front neighbor
						world->BlockExistsAt(x + 1, y + 1, z - 1) // Diagonal
						, world->enableAO);

					if (texture.hasTop) {

						textureRow = texture.topTexture.row;
						textureColumn = texture.topTexture.column;

						float lightValue = (world->GetBlockAt(x, y + 1, z) != nullptr && world->GetBlockAt(x, y + 1, z)->blockType == Minecraft::Air) ? world->GetBlockAt(x, y + 1, z)->blockLight : 0;
						vertices.insert(vertices.end(), {
								x - 0.5f, y + 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn,lightValue, ao1,
								x - 0.5f, y + 0.5f, z + 0.5f,  0.0f, 0.0f, textureRow, textureColumn,lightValue, ao2,
								x + 0.5f, y + 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn,lightValue, ao3,
								x + 0.5f, y + 0.5f, z - 0.5f,  1.0f, 1.0f, textureRow, textureColumn,lightValue, ao4,
							});
						indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
							});
						++i;
					}
					else {
						textureRow = texture.texture.row;
						textureColumn = texture.texture.column;

						float lightValue = (world->GetBlockAt(x, y + 1, z) != nullptr && world->GetBlockAt(x, y + 1, z)->blockType == Minecraft::Air) ? world->GetBlockAt(x, y + 1, z)->blockLight : 0;
						vertices.insert(vertices.end(), {
								x - 0.5f, y + 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn,lightValue, ao1,
								x - 0.5f, y + 0.5f, z + 0.5f,  1.0f, 1.0f, textureRow, textureColumn,lightValue, ao2,
								x + 0.5f, y + 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn,lightValue, ao3,
								x + 0.5f, y + 0.5f, z - 0.5f,  0.0f, 0.0f, textureRow, textureColumn,lightValue, ao4,
							});
						indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
							});
						++i;
					}
				}
				// BOTTOM FACE
				if (ShowBottomFace) {

					float ao1 = CalculateAO(
						world->BlockExistsAt(x - 1, y - 1, z), // Left neighbor
						world->BlockExistsAt(x, y - 1, z - 1), // Front neighbor
						world->BlockExistsAt(x - 1, y - 1, z - 1) // Diagonal
						, world->enableAO);

					float ao2 = CalculateAO(
						world->BlockExistsAt(x + 1, y - 1, z), // Left neighbor
						world->BlockExistsAt(x, y - 1, z - 1), // Back neighbor
						world->BlockExistsAt(x + 1, y - 1, z - 1) // Diagonal
						, world->enableAO);

					float ao3 = CalculateAO(
						world->BlockExistsAt(x + 1, y - 1, z), // Right neighbor
						world->BlockExistsAt(x, y - 1, z + 1), // Back neighbor
						world->BlockExistsAt(x + 1, y - 1, z + 1) // Diagonal
						, world->enableAO);

					float ao4 = CalculateAO(
						world->BlockExistsAt(x - 1, y - 1, z), // Right neighbor
						world->BlockExistsAt(x, y - 1, z + 1), // Front neighbor
						world->BlockExistsAt(x - 1, y - 1, z + 1) // Diagonal
						, world->enableAO);

					if (texture.hasBottom) {

						textureRow = texture.bottomTexture.row;
						textureColumn = texture.bottomTexture.column;

						float lightValue = (world->GetBlockAt(x, y - 1, z) != nullptr && world->GetBlockAt(x, y - 1, z)->blockType == Minecraft::Air) ? world->GetBlockAt(x, y - 1, z)->blockLight : 0;
						vertices.insert(vertices.end(), {
								x - 0.5f, y - 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn,lightValue, ao1,
								x + 0.5f, y - 0.5f, z - 0.5f,  1.0f, 1.0f, textureRow, textureColumn,lightValue, ao2,
								x + 0.5f, y - 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn,lightValue, ao3,
								x - 0.5f, y - 0.5f, z + 0.5f,  0.0f, 0.0f, textureRow, textureColumn,lightValue, ao4,
							});
						indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
							});
						++i;
					}
					else
					{
						textureRow = texture.texture.row;
						textureColumn = texture.texture.column;

						float lightValue = (world->GetBlockAt(x, y - 1, z) != nullptr && world->GetBlockAt(x, y - 1, z)->blockType == Minecraft::Air) ? world->GetBlockAt(x, y - 1, z)->blockLight : 0;
						vertices.insert(vertices.end(), {
								x - 0.5f, y - 0.5f, z - 0.5f,  0.0f, 1.0f, textureRow, textureColumn,lightValue, ao1,
								x + 0.5f, y - 0.5f, z - 0.5f,  1.0f, 1.0f, textureRow, textureColumn,lightValue, ao2,
								x + 0.5f, y - 0.5f, z + 0.5f,  1.0f, 0.0f, textureRow, textureColumn,lightValue, ao3,
								x - 0.5f, y - 0.5f, z + 0.5f,  0.0f, 0.0f, textureRow, textureColumn,lightValue, ao4,
							});
						indices.insert(indices.end(), {
						(unsigned int)0 + (i * 4), (unsigned int)1 + (i * 4), (unsigned int)2 + (i * 4),
						(unsigned int)2 + (i * 4), (unsigned int)3 + (i * 4), (unsigned int)0 + (i * 4)
							});
						++i;
					}
				}
				//chunkLock.lock();
			}
		}

	}

	SetupRenderFlag = true;
	indiceSize = indices.size();
	
	//chunkLock.unlock();

}

void Minecraft::Chunk::SetupVoxels(World* world)
{
	// TODO - Implement Greedy Meshing

	ChunkGuard guard(this);

	if (!ChunkReady)
		return;

	SetupVertices(world);
	SetupRenderFlag = false;
	
	//game->GetWorldReference()->worldLock.unlock();

	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	vertices.clear();

	indiceSize = indices.size();
	ForceCPUVoxelSetup = false;

}

void Minecraft::Chunk::SetupRender()
{
	ChunkGuard guard(this);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	vertices.clear();

	SetupRenderFlag = false;
}

void Minecraft::Chunk::SetBlockPropagate(glm::ivec3 pos)
{	
	ChunkGuard guard(this);

	auto Wrap = [](int coord, int size) {
		return (coord % size + size) % size;
		};

	chunkLock.lock();
	lightQueue.push(GetIndex(Wrap(pos.x % World::CHUNK_SIZE, World::CHUNK_SIZE), pos.y, Wrap(pos.z % World::CHUNK_SIZE, World::CHUNK_SIZE)));

	//int index = GetIndex(Wrap(pos.x % World::CHUNK_SIZE, World::CHUNK_SIZE), pos.y, Wrap(pos.z % World::CHUNK_SIZE, World::CHUNK_SIZE));
	//std::cout << "Positions:\n" << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
	//std::cout << "Index:\n" << index << std::endl;
	//
	//int x_local = index % World::CHUNK_SIZE;
	//int y_local = (index / (World::CHUNK_SIZE * World::CHUNK_SIZE));
	//int z_local = (index / World::CHUNK_SIZE) % World::CHUNK_SIZE;
	//
	//// Convert to world positions
	//int x_world = x_local + World::CHUNK_SIZE * chunkPosition.x;
	//int y_world = y_local + World::CHUNK_SIZE * chunkPosition.y;
	//int z_world = z_local + World::CHUNK_SIZE * chunkPosition.z;
	//
	//std::cout << "Positions in index:\n" << x_world << ", " << y_local << ", " << z_world << std::endl;
	chunkLock.unlock();
}

void Minecraft::Chunk::PropagateLighting(World* world)
{
	ChunkGuard guard(this);

	if (!ChunkReadyLighting)
		return;

	std::vector<Chunk*> chunksToPropagate;

	const int directions[6][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1} };

	while (!lightQueue.empty())
	{
		LightNode light = lightQueue.front();
		lightQueue.pop();

		int x_local = light.index % World::CHUNK_SIZE;
		int y_local = (light.index / (World::CHUNK_SIZE * World::CHUNK_SIZE));
		int z_local = (light.index / World::CHUNK_SIZE) % World::CHUNK_SIZE;

		int x_world = x_local + World::CHUNK_SIZE * chunkPosition.x;
		int y_world = y_local + World::CHUNK_SIZE * chunkPosition.y;
		int z_world = z_local + World::CHUNK_SIZE * chunkPosition.z;

		BlockData* block = GetBlockAt(x_world, y_world, z_world);
		if (!block)
		{
			continue;
		}

		for (int i = 0; i < 6; ++i)
		{
			int nx = x_world + directions[i][0];
			int ny = y_world + directions[i][1];
			int nz = z_world + directions[i][2];

			BlockData* neighborBlock = world->GetBlockAt(nx, ny, nz);
			
			if (neighborBlock && neighborBlock->blockType == BlockType::Air && neighborBlock->blockLight < block->blockLight - 1)
			{
				neighborBlock->blockLight = block->blockLight - 1;

				Chunk* neighborChunk = world->GetChunkAt(nx, ny, nz);
				if (neighborChunk) {
					neighborChunk->SetBlockPropagate(glm::ivec3(nx, ny, nz));
				}

				if (std::find(chunksToPropagate.begin(), chunksToPropagate.end(), neighborChunk) == chunksToPropagate.end())
				{
					chunksToPropagate.push_back(neighborChunk);
				}
			}

		}
	}

	// Propagate the neighbor chunks in the vector
	for (Chunk* chunk : chunksToPropagate)
	{
		if (chunk)
		{
			chunk->PropagateLighting(world);
		}
	}
	//SetupVoxels(world);

}

void Minecraft::Chunk::SetBlockRemovePropagate(glm::vec3 pos)
{
	ChunkGuard guard(this);

	auto Wrap = [](int coord, int size) {
		return (coord % size + size) % size;
		};

	lightRemovalQueue.push(GetIndex(Wrap((int)pos.x % World::CHUNK_SIZE, World::CHUNK_SIZE), pos.y, Wrap((int)pos.z % World::CHUNK_SIZE, World::CHUNK_SIZE)));
}

void Minecraft::Chunk::RemovePropagateLighting(World* world, std::vector<Chunk*>* chunkVector)
{
	ChunkGuard guard(this);

	std::vector<Chunk*> chunksToPropagate;

	const int directions[6][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1} };

	while (!lightRemovalQueue.empty())
	{
		LightNode light = lightRemovalQueue.front();
		lightRemovalQueue.pop();

		int x_local = light.index % World::CHUNK_SIZE;
		int y_local = (light.index / (World::CHUNK_SIZE * World::CHUNK_SIZE));
		int z_local = (light.index / World::CHUNK_SIZE) % World::CHUNK_SIZE;

		int x_world = x_local + World::CHUNK_SIZE * chunkPosition.x;
		int y_world = y_local + World::CHUNK_SIZE * chunkPosition.y;
		int z_world = z_local + World::CHUNK_SIZE * chunkPosition.z;

		BlockData* block = GetBlockAt(x_world, y_world, z_world);
		if (!block) continue;

		int lightValue = block->blockLight;

		for (int i = 0; i < 6; ++i)
		{
			int nx = x_world + directions[i][0];
			int ny = y_world + directions[i][1];
			int nz = z_world + directions[i][2];

			if (nx < chunkPosition.x * world->CHUNK_SIZE || nx >= (chunkPosition.x + 1) * world->CHUNK_SIZE ||
				ny < chunkPosition.y * world->CHUNK_HEIGHT || ny >= (chunkPosition.y + 1) * world->CHUNK_HEIGHT ||
				nz < chunkPosition.z * world->CHUNK_SIZE || nz >= (chunkPosition.z + 1) * world->CHUNK_SIZE)
			{
				BlockData* neighbor = world->GetBlockAt(nx, ny, nz);
				if (neighbor && neighbor->blockType == Minecraft::Air) {

					Chunk* neighborChunk = world->GetChunkAt(nx, ny, nz);

					if (neighbor->blockLight != 0 && neighbor->blockLight < block->blockLight) 
					{ 
						neighborChunk->SetBlockRemovePropagate(glm::vec3(nx, ny, nz));
					}
					else 
					{
						neighborChunk->SetBlockPropagate(glm::vec3(nx, ny, nz));
					}

					// Add chunk to the propagation vector if it is not already there
					if (std::find(chunksToPropagate.begin(), chunksToPropagate.end(), neighborChunk) == chunksToPropagate.end())
					{
						chunksToPropagate.push_back(neighborChunk);
					}

					if (chunkVector)
					{
						if (std::find(chunkVector->begin(), chunkVector->end(), neighborChunk) == chunkVector->end())
						{
							chunkVector->push_back(neighborChunk);
						}
					}

					continue;
				}
				Chunk* neighborChunk = world->GetChunkAt(nx, ny, nz);
				if (neighborChunk)
				{
					if (std::find(chunksToPropagate.begin(), chunksToPropagate.end(), neighborChunk) == chunksToPropagate.end())
					{
						chunksToPropagate.push_back(neighborChunk);
					}
					if (chunkVector)
					{
						if (std::find(chunkVector->begin(), chunkVector->end(), neighborChunk) == chunkVector->end())
						{
							chunkVector->push_back(neighborChunk);
						}
					}
				}
			}


			BlockData* neighbor = world->GetBlockAt(nx, ny, nz);
			if (neighbor && neighbor->blockType == Minecraft::Air) {

				if (neighbor->blockLight != 0 && neighbor->blockLight < lightValue) {

					//neighbor->blockLight = 0;
					SetBlockRemovePropagate(glm::vec3(nx, ny, nz));
					continue;

				}
				else if (neighbor->blockLight >= lightValue)
				{
					SetBlockPropagate(glm::vec3(nx, ny, nz));
				}
			}
		}
		block->blockLight = 1;
	}

	// Propagate the neighbor chunks in the vector
	for (Chunk* chunk : chunksToPropagate)
	{
		if (chunk)
		{
			chunk->RemovePropagateLighting(world, chunkVector);
			chunk->SetupVoxels(world);
		}
	}

	SetupVoxels(world);
}

void Minecraft::Chunk::RenderVoxels()
{
	ChunkGuard guard(this);

	if (SetupRenderFlag)
	{
		SetupRender();
	}


	if (!ChunkReady)
		return;

	if (ForceCPUVoxelSetup)
	{
		SetupVoxels(_world);
	}

	glBindVertexArray(VAO);

	_world->shader.Bind();

	glUniformMatrix4fv(glGetUniformLocation(_world->shader.GetID(), "modelMatrix"), 1, GL_FALSE, glm::value_ptr(chunkMatrix));
	glUniformMatrix4fv(glGetUniformLocation(_world->shader.GetID(), "viewMatrix"), 1, GL_FALSE, glm::value_ptr(Utility::camera.viewSpace));
	glUniformMatrix4fv(glGetUniformLocation(_world->shader.GetID(), "perspectiveMatrix"), 1, GL_FALSE, glm::value_ptr(Utility::perspective));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, game->GetTextureAtlasID());

	glUniform1i(_world->textureUniformLocation, 0);

	//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glDrawElements(GL_TRIANGLES, indiceSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	
}

Minecraft::BlockData* Minecraft::Chunk::GetBlockAt(int x, int y, int z)
{
	ChunkGuard guard(this); // Ensure the guard locks the chunk

	{
		std::lock_guard<std::mutex> lock(chunkLock); // Automatically unlocks when going out of scope

		auto Wrap = [](int coord, int size) {
			return (coord % size + size) % size;
			};

		int index = GetIndex(Wrap(x % World::CHUNK_SIZE, World::CHUNK_SIZE), y, Wrap(z % World::CHUNK_SIZE, World::CHUNK_SIZE));
		if (index < 0 || index >= blocks.size()) {
			return nullptr; // Return nullptr if index is out of bounds
		}
		BlockData* block = &blocks[index];
		return block; // No need to unlock explicitly; std::lock_guard handles that
	}
}

Minecraft::BlockData* Minecraft::Chunk::GetBlockAt(glm::vec3 vec3) { return GetBlockAt(vec3.x, vec3.y, vec3.z); }

Minecraft::BlockData* Minecraft::Chunk::GetBlockAt(int index)
{
	ChunkGuard guard(this); // Ensure the guard locks the chunk

	{
		std::lock_guard<std::mutex> lock(chunkLock);

		if (index < 0 || index >= blocks.size()) {
			return nullptr; // Return nullptr if index is out of bounds
		}
		BlockData* block = &blocks[index];
		return block; // No need to unlock explicitly; std::lock_guard handles that
	}
}

void Minecraft::Chunk::acquire()
{
	threadCount.fetch_add(1);
}

void Minecraft::Chunk::release()
{
	threadCount.fetch_sub(1);
}


Minecraft::ChunkGuard::ChunkGuard(Chunk* chunk) : chunk(chunk)
{
	chunk->acquire();
}

Minecraft::ChunkGuard::~ChunkGuard()
{
	chunk->release();
}
