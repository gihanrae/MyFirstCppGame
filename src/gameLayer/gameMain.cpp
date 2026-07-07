#include <raylib.h>
#include "gameMain.h"
#include <fstream>
#include <iostream>
#include "platform/asserts.h"
#include "assetManager.h"
#include "gameMap.h"


struct  GameData
{
	GameMap gameMap;
}gameData;

AssetManager assetManager;

bool initGame()
{
	assetManager.loadAll();

	gameData.gameMap.create(30, 10);

	gameData.gameMap.getBlockUnsafe(0, 0).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(1, 1).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(2, 2).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(3, 3).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(4, 4).type = Block::dirt;

	return true;
}

bool updateGame()
{
	float deltaTime{ GetFrameTime() };
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

	for (int y = 0; y < gameData.gameMap.h; y++)
		for (int x = 0; x < gameData.gameMap.w; x++)
		{
			auto& b = gameData.gameMap.getBlockUnsafe(x, y);

			if (b.type != Block::air)
			{
				float size = 32;
				float posX = x * size;
				float posY = y * size;

				DrawTexturePro(
					assetManager.dirt,
					Rectangle{0.f, 0.f, (float)assetManager.dirt.width, (float)assetManager.dirt.height}, // source
					{posX, posY, size, size}, // dest
					{0, 0}, // origin (top left corner)
					0.0f, // rotation
					WHITE // tint 
				);
			}
		}

	return true;
}

void closeGame()
{
	std::cout << "\n\nCLOSED!!!!!!!!!\n\n";

	std::ofstream f(RESOURCES_PATH "log.txt");
	f << "\nCLOSED\n";
	f.close();
}
