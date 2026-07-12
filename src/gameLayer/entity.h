#pragma once
#include <physics.h>
#include <unordered_map>
#include <random>
#include <nlohmann/json.hpp>

struct AssetManager;
struct EntityHolder;

using Json = nlohmann::json;

enum EntityType
{
	EntityType_Player = 0,
	EntityType_Slime,
	EntityType_DroppedItem,
};

struct EntityUpdateData
{
	Vector2 playerPosition = {};

	std::ranlux24_base& rng;
	
	EntityHolder& entityHolder;

	std::uint64_t ownId = 0;
	
};

struct Entity
{
	PhysicalEntity  physics;
	float life = 1;

	Vector2& getPosition()
	{
		return physics.transform.pos;
	}

	void teleport(Vector2 pos)
	{
		physics.teleport(pos);
	}

	virtual void render(AssetManager& assetManager) = 0;

	virtual bool update(float deltaTime, EntityUpdateData entityUpdateData) = 0;

	virtual int getEntityType() = 0;

	virtual float getMaxLife() = 0;

	virtual Json formatToJson() { return {}; }

	virtual bool loadFromJson(Json& j) { return true;  }
};