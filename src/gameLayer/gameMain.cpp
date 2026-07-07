#include <raylib.h>
#include "gameMain.h"
#include <fstream>
#include <iostream>
#include "assert.h"
#include "assetManager.h"


struct  GameData
{
}gameData;

AssetManager assetManager;

bool initGame()
{
	assetManager.loadAll();

	return true;
}

bool updateGame()
{
	float deltaTime{ GetFrameTime() };
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

	DrawTexturePro(assetManager.dirt, {0,0,(float)assetManager.dirt.width, (float)assetManager.dirt.height},
		{ 50, 50, 100, 100 }, {}, 0, WHITE);

	return true;
}

void closeGame()
{
	std::cout << "\n\nCLOSED!!!!!!!!!\n\n";

	std::ofstream f(RESOURCES_PATH "log.txt");
	f << "\nCLOSED\n";
	f.close();
}
