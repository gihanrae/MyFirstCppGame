#include <raylib.h>
#include "gameMain.h"
#include <asserts.h>
#include <assetManager.h>
#include <gameMap.h>
#include <helpers.h>
#include <raymath.h>
#include <worldGenerator.h>
#include <imgui.h>
#include <structure.h>
#include <string>
#include <saveMap.h>
#include <physics.h>
#include <entities/slime.h>
#include <entities/droppedItem.h>
#include <entityIdHolder.h>
#include "player.h"
#include "items.h"
#include <audio.h>
#include <settings.h>
#include <drawBackground.h>
#include <ui.h>
#include <gameplay.h>

AssetManager assetManager;

Gameplay gameplay;



bool initGame()
{

	Audio::init();
	assetManager.loadAll();
	loadSettings();

	gameplay.init();

	return true;
}

bool updateGame()
{
	return gameplay.update(assetManager);

	//return true;
}

void closeGame()
{
}
