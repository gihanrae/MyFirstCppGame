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
#include <iostream>
#include <entities/slime.h>
#include <entityIdHolder.h>
#include <entities/droppedItem.h>
#include <player.h>
#include <audio.h>
#include <settings.h>
#include <drawBackground.h>
#include <ui.h>


struct  GameData
{
	GameMap gameMap;
	Camera2D camera;
	DrawBackground background;

	int creativeSelectedBlock = Block::dirt;

	Vector2 selectionStart = {};
	Vector2 selectionEnd = {};
	Structure copyStructure;

	char saveName[100] = {};

	Player player;
	EntityHolder entities;

	bool insideInventory = false;

}gameData;

AssetManager assetManager;

bool showImgui = false;

void spawnSlime(Vector2 position)
{
	Slime slime;

	slime.physics.teleport(position);

	auto id = gameData.entities.idHolder.getEntityIdAndIncrement();

	gameData.entities.entities[id] = std::make_unique<Slime>(slime);
}

void spawnDroppedItem(Vector2 position, int type)
{
	DroppedItem droppedItem;

	droppedItem.teleport(position);
	droppedItem.itemType = type;

	auto id = gameData.entities.idHolder.getEntityIdAndIncrement();

	gameData.entities.entities[id] = (std::make_unique<DroppedItem>(droppedItem));

}

Rectangle getInventoryRectangle(float w, float h)
{
	Rectangle inventoryRectangle;

	inventoryRectangle.height = h * 0.30;
	inventoryRectangle.width = inventoryRectangle.height * 3;

	inventoryRectangle = placeRectangleTopLeftCorner(inventoryRectangle, w);

	inventoryRectangle.x += w * 0.01;
	inventoryRectangle.y += h * 0.01;
	return inventoryRectangle;
}


bool initGame()
{
	Audio::init();
	assetManager.loadAll();
	loadSettings();

	generateWorld(gameData.gameMap);

	gameData.camera.target = { 20, 120 }; // world space center of view, we will use this as the camera position
	gameData.camera.rotation = 0.0f;
	gameData.camera.zoom = 100.0f;

	gameData.player.physics.teleport({ 20, 60 });
	gameData.player.physics.transform.w = 0.9f;
	gameData.player.physics.transform.h = 1.8f;

	spawnSlime({18, 60});

	return true;
}

bool updateGame()
{
	Audio::update();
	updateSettings();

	float deltaTime{ GetFrameTime() };
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

	gameData.camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	ClearBackground({ 75, 75, 150, 255 });

	if (IsKeyPressed(KEY_I)) { showImgui = !showImgui; }

#pragma region camera movement

	static float CAMERA_SPEED = 10;
	static bool creative = false;

	{
		bool moving = 0;
		bool falling = 0;

		if (IsKeyDown(KEY_A))
		{
			gameData.player.physics.transform.pos.x -= CAMERA_SPEED * GetFrameTime();
			moving = true;
			gameData.player.animations.movingLeft = true;
		}
		if (IsKeyDown(KEY_D))
		{
			gameData.player.physics.transform.pos.x += CAMERA_SPEED * GetFrameTime();
			moving = true;
			gameData.player.animations.movingLeft = false;
		}
		if (creative)
		{
			if (IsKeyDown(KEY_W)) { gameData.player.physics.transform.pos.y -= CAMERA_SPEED * GetFrameTime(); moving = true; }
			if (IsKeyDown(KEY_S)) { gameData.player.physics.transform.pos.y += CAMERA_SPEED * GetFrameTime(); moving = true; }
		};

		if (IsKeyDown(KEY_SPACE)) gameData.player.physics.jump(10);

		if (gameData.player.physics.downTouch) 
		{ 
			falling = 0; 
		}
		else
		{ 
			falling = 0; 
		}

		if (falling)
		{
			gameData.player.animations.setAnimation(2);
		}
		else if (moving)
		{
			gameData.player.animations.setAnimation(1);
		}
		else
		{
			gameData.player.animations.setAnimation(0);
		}

		gameData.player.animations.update(deltaTime, 0.08, 7);


	}



#pragma endregion

#pragma region entities

	auto updateEntityPhysics = [&](Entity& entity, bool applyGravity = true)
	{
			if (applyGravity) { entity.physics.applyGravity(); }
			
			entity.physics.updateForces(deltaTime);
			entity.physics.resolveConstrains(gameData.gameMap);
			entity.physics.updateFinal();
	};
	
	updateEntityPhysics(gameData.player, !creative);

	gameData.camera.target = gameData.player.physics.transform.pos;

	//clamp camera
	{
		float zoom = gameData.camera.zoom;

		float screenWidth = GetScreenWidth();
		float screenHeight = GetScreenHeight();

		// half of the visible area (adjusted for zoom)
		float halfViewWidth = (screenWidth * 0.5f) / zoom;
		float halfViewHeight = (screenHeight * 0.5f) / zoom;

		float minX = halfViewWidth;
		float maxX = gameData.gameMap.w - halfViewWidth;
		float minY = halfViewHeight;
		float maxY = gameData.gameMap.h - halfViewHeight;

		// if the map is smaller than the view (zoomed out a lot), we just set the camera to the center of the world
		if (maxX < minX)
		{
			gameData.camera.target.x = gameData.gameMap.w * 0.5f;
		}
		else
		{
			gameData.camera.target.x = Clamp(gameData.camera.target.x, minX, maxX);
		}

		if (maxY < minY)
		{
			gameData.camera.target.y = gameData.gameMap.h * 0.5f;
		}
		else
		{
			gameData.camera.target.y = Clamp(gameData.camera.target.y, minY, maxY);
		}

	}

	
	std::ranlux24_base rng(std::random_device{}());

	//entities

	//update all entities
	for (auto it = gameData.entities.entities.begin(); it != gameData.entities.entities.end();)
	{
		EntityUpdateData updateData
		{
			gameData.player.getPosition(),
			rng,
			gameData.entities,
			it->first
		};

		bool shouldKill = false;

		if (!it->second->update(deltaTime, updateData) || it->second->life <= 0)
		{
			shouldKill = true;
		}

		if (shouldKill)
		{
			// erase returns the next valid iterator
			it = gameData.entities.entities.erase(it);
		}
		else
		{
			//physics
			updateEntityPhysics(*it->second);

			++it;
		}
	}

#pragma endregion

	bool insideInventoryMenu = 0;
	Rectangle inventoryRectangle = getInventoryRectangle(GetScreenWidth(), GetScreenHeight());

	if (gameData.insideInventory &&
		CheckCollisionPointRec(GetMousePosition(), inventoryRectangle))
	{
		insideInventoryMenu = true;
	}


	Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), gameData.camera);
	int blockX = (int)floor(worldPos.x);
	int blockY = (int)floor(worldPos.y);

	if (gameData.creativeSelectedBlock < 0) { gameData.creativeSelectedBlock = 0; }
	if (gameData.creativeSelectedBlock >= Block::BLOCKS_COUNT) { gameData.creativeSelectedBlock = Block::BLOCKS_COUNT - 1; }

	if (showImgui)
	{
		if (IsKeyPressed(KEY_ONE)) { gameData.selectionStart = Vector2{ (float)blockX, (float)blockY }; }
		if (IsKeyPressed(KEY_TWO)) { gameData.selectionEnd = Vector2{ (float)blockX, (float)blockY }; }
		if (IsKeyPressed(KEY_THREE))
		{
			gameData.copyStructure.pasteIntoMap(gameData.gameMap, Vector2{(float)blockX, (float)blockY});
		}

		if (gameData.selectionStart.x > gameData.selectionEnd.x)
		{
			std::swap(gameData.selectionStart.x, gameData.selectionEnd.x);
		}

		if (gameData.selectionStart.y > gameData.selectionEnd.y)
		{
			std::swap(gameData.selectionStart.y, gameData.selectionEnd.y);
		}
	}

	if (!showImgui)
	{
		if (!insideInventoryMenu)
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
			if (b)
			{
				if (b->type)
				{
					spawnDroppedItem({(float)blockX + 0.5f, (float)blockY + 0.5f}, b->type);
				}
				*b = {};
			}
		}

		if (!insideInventoryMenu)
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
			if (b)
			{
				b->type = gameData.creativeSelectedBlock;
			}
		}

		if (IsKeyPressed(KEY_TAB))
		{
			gameData.insideInventory = !gameData.insideInventory;
		}
	}

#pragma region draw world

	//background
	{

		int backgroundType = DrawBackground::forest;

		if (gameData.player.getPosition().x > gameData.gameMap.desertStart &&
			gameData.player.getPosition().x < gameData.gameMap.desertEnd
			)
		{
			backgroundType = DrawBackground::desert;
		}

		if (gameData.player.getPosition().y > 130)
		{
			backgroundType = DrawBackground::cave;
		}

		gameData.background.setBackground(backgroundType);

		gameData.background.draw(deltaTime, assetManager, gameData.camera,
			{ (float)gameData.gameMap.w, (float)gameData.gameMap.h });

	}

	BeginMode2D(gameData.camera);

	Vector2 topLeftView = GetScreenToWorld2D({ 0, 0 }, gameData.camera);
	Vector2 bottomRightView = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, gameData.camera);

	int startXView = (int)floorf(topLeftView.x - 1);
	int endXView = (int)ceilf(bottomRightView.x + 1);
	int startYView = (int)floorf(topLeftView.y - 1);
	int endYView = (int)ceilf(bottomRightView.y + 1);

	startXView = Clamp(startXView, 0, gameData.gameMap.w - 1);
	endXView = Clamp(endXView, 0, gameData.gameMap.w - 1);

	startYView = Clamp(startYView, 0, gameData.gameMap.h - 1);
	endYView = Clamp(endYView, 0, gameData.gameMap.h - 1);

	for (int y = startYView; y < endYView; y++)
		for (int x = startXView; x < endXView; x++)
		{
			auto& b = gameData.gameMap.getBlockUnsafe(x, y);

			if (b.type != Block::air)
			{
				Rectangle textureUV;
				textureUV.width = 32;
				textureUV.height = 32;
				textureUV.x = b.type * 32;
				textureUV.y = 0;

				DrawTexturePro(
					assetManager.textures,
					getTextureAtlas(b.type, 0, 32, 32), // source
					{(float)x, (float)y, 1, 1}, // dest
					{0, 0}, // origin (top left corner)
					0.0f, // rotation
					WHITE // tint 
				);
			}
		}

	//draw selected block
	if (!insideInventoryMenu)
	{
		DrawTexturePro(
			assetManager.frame,
			{ 0,0, (float)assetManager.frame.width, (float)assetManager.frame.height },
			{ (float)blockX, (float)blockY, 1, 1 }, // dest
			{ 0, 0 },
			0.0f,
			WHITE
		);
	}


	if (showImgui)
	{
		Rectangle rect;
		rect.x = gameData.selectionStart.x;
		rect.y = gameData.selectionStart.y;
		rect.width = gameData.selectionEnd.x - gameData.selectionStart.x;
		rect.height = gameData.selectionEnd.y - gameData.selectionStart.y;

		rect.width++;
		rect.height++;

		DrawRectangleLinesEx(rect, 0.05, {20, 101, 250, 145});
	}

	for (auto& e : gameData.entities.entities)
	{
		e.second->render(assetManager);
	}


	gameData.player.render(assetManager);

	DrawRectangleLinesEx(gameData.player.physics.transform.getAABB(), 0.1,
		{ 20, 101, 250, 120 });

	EndMode2D();

#pragma endregion

#pragma region ui
	{

		float w = GetScreenWidth();
		float h = GetScreenHeight();

		Rectangle hearthRectangle;

		hearthRectangle.height = h * 0.05;
		hearthRectangle.width = hearthRectangle.height * 5;

		hearthRectangle = placeRectangleTopRightCorner(hearthRectangle, w);

		//test the drawing area
		//DrawRectangle(hearthRectangle.x, hearthRectangle.y, hearthRectangle.width, hearthRectangle.height, RED);


		for (int i = 0; i < 5; i++)
		{

			Rectangle oneHearthRectangle = hearthRectangle;
			oneHearthRectangle.width = oneHearthRectangle.height;
			oneHearthRectangle.x += oneHearthRectangle.width * i;

			//shrink hearths
			oneHearthRectangle = shrinkRectanglePercentage(oneHearthRectangle, 0.1, 0.1);

			DrawTexturePro(
				assetManager.hearts,
				getTextureAtlas(0, 0, assetManager.hearts.width / 3, assetManager.hearts.height), //source
				oneHearthRectangle, //dest
				{ 0, 0 },// origin (top-left corner)
				0.0f, // rotation
				WHITE // tint
			);

		}

		if (gameData.insideInventory)
		{

			Rectangle inventoryRectangle = getInventoryRectangle(w, h);

			DrawRectangle(inventoryRectangle.x, inventoryRectangle.y, inventoryRectangle.width, inventoryRectangle.height,
				{ 100, 100, 100, 100 });

			inventoryRectangle = shrinkRectanglePercentage(inventoryRectangle, 0.01, 0.01);

			Rectangle oneCellRectangle;
			oneCellRectangle.height = inventoryRectangle.height / 3;
			oneCellRectangle.width = oneCellRectangle.height;
			oneCellRectangle.x = inventoryRectangle.x;
			oneCellRectangle.y = inventoryRectangle.y;

			for (int i = 0; i < 9; i++)
				for (int j = 0; j < 3; j++)
				{

					Rectangle r = oneCellRectangle;
					r.x += i * oneCellRectangle.width;
					r.y += j * oneCellRectangle.height;

					r = shrinkRectanglePercentage(r, 0.1, 0.1);

					if (CheckCollisionPointRec(GetMousePosition(), r))
					{
						DrawTexturePro(
							assetManager.frame,
							{ 0,0,(float)assetManager.frame.width, (float)assetManager.frame.height }, //source
							r, //dest
							{ 0, 0 },// origin (top-left corner)
							0.0f, // rotation
							{ 220,250,220,250 } // tint
						);
					}
					else
					{
						DrawTexturePro(
							assetManager.frame,
							{ 0,0,(float)assetManager.frame.width, (float)assetManager.frame.height }, //source
							r, //dest
							{ 0, 0 },// origin (top-left corner)
							0.0f, // rotation
							{ 180,180,200,240 } // tint
						);
					}

				}


		}
	}

#pragma endregion

#pragma region imgui

	if (showImgui)
	{
		ImGui::Begin("Game control");

		ImGui::SliderFloat("Camera zoom:", &gameData.camera.zoom, 10, 150);
		ImGui::SliderFloat("Camera speed:", &CAMERA_SPEED, 5, 30);
		ImGui::Checkbox("Creative", &creative);

		if (ImGui::Button("Hurt a slime"))
		{
			for (auto& e : gameData.entities.entities)
			{

				if (e.second->getEntityType() == EntityType_Slime)
				{
					e.second->life -= 3;
					break;
				}

			}
		}

		if (ImGui::Button("Spawn Slime"))
		{
			spawnSlime({18, 60});
		}

		if (ImGui::Button("Copy"))
		{
			gameData.copyStructure.copyFromMap(gameData.gameMap, gameData.selectionStart, gameData.selectionEnd);
		}

		ImGui::InputText("File Name", gameData.saveName, sizeof(gameData.saveName));

		if (ImGui::Button("Save to file"))
		{
			std::string path = RESOURCES_PATH "structures/";
			path += gameData.saveName;
			path += ".bin";

			saveBlockDataToFile(gameData.copyStructure.mapData, gameData.copyStructure.w, gameData.copyStructure.h, path.c_str());
		}

		if (ImGui::Button("Load from file"))
		{
			std::string path = RESOURCES_PATH "structures/";
			path += gameData.saveName;
			path += ".bin";

			loadBlockDataFromFile(gameData.copyStructure.mapData, gameData.copyStructure.w, gameData.copyStructure.h, path.c_str());
		}

		ImGui::Separator();

		ImGui::SliderFloat("master volume", &getSettings().masterVolume, 0, 1);
		ImGui::SliderFloat("sound volume", &getSettings().soundsVolume, 0, 1);
		ImGui::SliderFloat("music volume", &getSettings().musicVolume, 0, 1);

		if (ImGui::Button("Save settings"))
		{
			saveSettings();
		}

		if (ImGui::Button("Load settings"))
		{
			loadSettings();
		}

		ImGui::Separator();

		if (ImGui::Button("Play sound"))
		{
			Audio::playSound(Audio::placeBlock);
		}

		if (ImGui::Button("Play music forest"))
		{
			Audio::playMusic(Audio::musicForest);
		}

		if (ImGui::Button("Play music desert"))
		{
			Audio::playMusic(Audio::musicDesert);
		}

		ImGui::Separator();

		if (ImGui::Button("Save World"))
		{
			saveWorld(gameData.gameMap,
				gameData.entities, gameData.player);
		}

		if (ImGui::Button("Load World"))
		{
			if (!loadWorld(gameData.gameMap,
				gameData.entities, gameData.player))
			{
				return false;
			}
		}

		ImGui::Separator();

		for (int i = 0; i < Block::BLOCKS_COUNT; i++)
		{
			auto atlas = getTextureAtlas(i, 0, 32, 32);
			atlas.x /= assetManager.textures.width;
			atlas.width /= assetManager.textures.width;
			atlas.y /= assetManager.textures.height;
			atlas.height /= assetManager.textures.height;

			ImGui::PushID(i);

			ImTextureID tex = (ImTextureID)(intptr_t)assetManager.textures.id;
			if (ImGui::ImageButton(tex,
				{ 35,35 }, { atlas.x, atlas.y },
				{ atlas.x + atlas.width, atlas.y + atlas.height }))
			{
				gameData.creativeSelectedBlock = i;
			}

			ImGui::PopID();

			if (i % 10 != 0)
			{
				ImGui::SameLine();
			}
		}



		ImGui::Separator();

		ImGui::End();
	}

#pragma endregion

	DrawFPS(10, 10);

	return true;
}

void closeGame()
{
	std::cout << "\n\nCLOSED!!!!!!!!!\n\n";

	std::ofstream f(RESOURCES_PATH "log.txt");
	f << "\nCLOSED\n";
	f.close();
}
