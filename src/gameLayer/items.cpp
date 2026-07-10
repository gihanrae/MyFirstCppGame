#include "items.h"
#include <assetManager.h>
#include <helpers.h>

Texture getTextureForItemType(int itemType, AssetManager& assetManager)
{
	if (itemType < Block::BLOCKS_COUNT)
	{
		return assetManager.textures;
	}
	else
	{
		return assetManager.items;
	}
}

Rectangle getTextureCoordinatesForItemTypes(int itemType)
{
	if (itemType < Block::BLOCKS_COUNT)
	{
		return getTextureAtlas(itemType, 4, 32, 32);
	}
	else
	{
		return getTextureAtlas(itemType - Block::BLOCKS_COUNT, 0, 32, 32);
	}
}

bool isItem(int type)
{
	return type >= Block::BLOCKS_COUNT;
}

bool isBlock(int type)
{
	return !isBlock(type);
}