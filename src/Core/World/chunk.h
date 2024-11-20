#pragma once

#include <glew.h>

#include <glm.hpp>
#include <gtx/hash.hpp>

#include <unordered_map>
#include <vector>

#include "../game.h"

#include "block.h"

#include <mutex>
#include <atomic>

#include <queue>
#include <tuple>

namespace Minecraft {

	class ChunkGuard
	{
	public:
		ChunkGuard(class Chunk* chunk);
		~ChunkGuard();
	private:
		class Chunk* chunk;
	};

	struct LightNode {

		LightNode(int x) : index(x) {}

		int index;
	};

	class Chunk {
	public:

		Chunk(Game* game, World* world, glm::vec3 chunkPos);
		~Chunk();

		std::vector<BlockData> blocks;

		std::queue<LightNode> lightQueue;
		std::queue<LightNode> lightRemovalQueue;

		bool SetupRenderFlag = false;
		bool ForceCPUVoxelSetup = false;
		bool ChunkReady = false;
		bool ChunkReadyLighting = false;

		unsigned int VAO, VBO, EBO;
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		unsigned int indiceSize = 0;

		// Generates the chunk terrain. Parameters are put into noise function.
		void GenerateChunk(int posX, int posY, int posZ);

		// Sets up vertices of the chunk.
		void SetupVertices(World* world);

		// Sets up vertices of the chunk on the main thread.
		void SetupVoxels(class World* world);

		// Calculates the ambient occlusion value for the vertex.
		float CalculateAO(bool side1, bool side2, bool corner, bool calculateAmbient = true);

		// Sets up the vertices for every chunk that borders the current chunk.
		void SetupBoardingChunkVertices(World* world);
		
		// Provides the vertices to OpenGL.
		void SetupRender();

		// Puts the provided block into the lighting queue.
		void SetBlockPropagate(glm::ivec3 pos);

		// Goes through the lighting queue to light up the chunk.
		void PropagateLighting(World* world);

		// Puts the provided block into the remove lighting queue.
		void SetBlockRemovePropagate(glm::vec3 pos);

		// Goes through the remove lighting queue to removing lighting in the chunk.
		void RemovePropagateLighting(World* world, std::vector<Chunk*>* chunkVector = nullptr);

		// Renders the chunk.
		void RenderVoxels();

		// Get chunk data functions
		inline int GetIndex(int x, int y, int z) const;

		// Gets block at index in local space.
		Minecraft::BlockData* GetBlockAt(int x, int y, int z);

		// Gets block at position in local space.
		Minecraft::BlockData* GetBlockAt(glm::vec3 vec3);
		// Gets block at index in local space.
		Minecraft::BlockData* GetBlockAt(int index);

		glm::mat4 chunkMatrix = glm::mat4(1.0f);
		glm::vec3 chunkPosition;

		class World* _world;

		// Multi-Threading

		std::mutex chunkLock;
		std::atomic<int> threadCount{ 0 };

		void acquire();
		void release();

		size_t GetChunkSize() const {
			size_t size = sizeof(*this);  // Base size of the Chunk object

			// Add size of dynamically allocated memory in vectors
			size += blocks.capacity() * sizeof(BlockData);
			//size += vertices.capacity() * sizeof(float);
			//size += indices.capacity() * sizeof(unsigned int);

			// For the light queues, we can approximate based on their current size
			//size += lightQueue.size() * sizeof(LightNode);
			//size += lightRemovalQueue.size() * sizeof(LightNode);

			return size;
		}


	private:
		Game* game;
	};

}