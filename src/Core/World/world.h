#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

#include "chunk.h"
#include "block.h"

#include "Utility/Shader.h"

#include <thread>
#include <mutex>

namespace Minecraft {
	class World {
	public: 

		World(Game* game);
		Game* game;

		std::unordered_map<glm::vec3, Chunk*> chunks;
		std::queue<Chunk*> chunksToGenerate;
		std::unordered_map<glm::vec3, Chunk*> chunksToDelete;

		void GenerateWorld();

		void RenderChunks();

		Minecraft::BlockData* GetBlockAt(int x, int y, int z);

		Minecraft::Chunk* GetChunkAt(int x, int y, int z);
		Minecraft::Chunk* GetChunkAtChunkPos(int x, int y, int z);

		void ManageChunks();
		void MakeChunk(int x, int y, int z);
		void SetupChunks();

		bool LoadChunk(int x, int y, int z);
		void UnloadChunk(Chunk* chunk);
		void UnloadChunks(glm::vec3 playerPosChunk);

		bool BlockExistsAt(int x, int y, int z);
		bool BlockExistsAt(glm::ivec3 pos);

		bool PlaceBlockAt(BlockType type, glm::ivec3 pos);
		bool DeleteBlockAt(glm::ivec3 pos);

		void UpdateNeighborChunk(glm::ivec3 neighborChunkCoords);

		// Size of Chunk (eg. 16x256x16 or CHUNK_SIZE x CHUNK_HEIGHT x CHUNK_SIZE)
		static const int CHUNK_SIZE = 16;
		static const int CHUNK_HEIGHT = 256;

		// Maximum light sunlight can shine onto the world
		static const int MAX_SKYLIGHT_VALUE = 15;

		// Sunlight propagation
		void PropagateSunlight(Chunk* chunk);
		void RecalculateSunlight(Chunk* chunk, int x, int z);
		void RemoveSunlight(Chunk* chunk, int x, int _y, int z);

		int ChunksX = 8;
		int ChunksY = 1;
		int ChunksZ = 8;

		// Multithreading
		std::thread chunkThread;
		bool finishedThread = false;

		std::mutex worldLock;

		// Shader
		Utility::Shader shader;
		unsigned int textureUniformLocation;

		// Post Processing
		bool enableAO = true;

	private:

	};

}
