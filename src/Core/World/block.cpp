#include "block.h"
#include <iostream>


Minecraft::BlockTexture Minecraft::GetBlockTexture(uint8_t type)
{
	BlockTexture block;

	switch (type)
	{
	case Minecraft::BlockType::Grass:
		block.texture.SetTexture(1, 0);
		block.hasTop = true;
		block.topTexture.SetTexture(2, 0);
		block.hasBottom = true;
		block.bottomTexture.SetTexture(0, 0);
		break;
	case Minecraft::BlockType::Dirt:
		block.texture.SetTexture(0, 0);
		block.hasTop = false;
		block.hasBottom = false;
		break;
	case Minecraft::BlockType::Stone:
		block.texture.SetTexture(3, 0);
		block.hasTop = false;
		block.hasBottom = false;
		break;
	case Minecraft::BlockType::Wood:
		block.texture.SetTexture(4, 0);
		break;
	default:
		block.texture.SetTexture(0, 0);
		block.hasTop = false;
		block.hasBottom = false;
		break;
	}

	return block;
}
