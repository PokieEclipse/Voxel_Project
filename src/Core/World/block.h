#pragma once
#include <cstdint>

namespace Minecraft {


	enum BlockType : uint8_t {
		Air = 0,
		Grass,
		Dirt,
		Stone,
		Wood,

		Torch
	};

	struct TextureID {
		uint8_t row = 0;
		uint8_t column = 0;

		void SetTexture(const uint8_t x, const uint8_t y) { row = x; column = y; }
	};

	struct BlockTexture {
		TextureID texture;

		bool hasTop = false;
		TextureID topTexture;

		bool hasBottom = false;
		TextureID bottomTexture;
		
	};

	struct BlockData {
		uint8_t blockType = 0;
		uint8_t blockLight = 1;
	};

	BlockTexture GetBlockTexture(uint8_t type);

}
