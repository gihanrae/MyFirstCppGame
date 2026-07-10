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


struct  GameData
{
	GameMap gameMap;
	Camera2D camera;

	int creativeSelectedBlock = Block::dirt;

	Vector2 selectionStart = {};
	Vector2 selectionEnd = {};
	Structure copyStructure;

	char saveName[100] = {};

	PhysicalEntity player;
	EntityHolder entities;

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

bool initGame()
{
	assetManager.loadAll();

	generateWorld(gameData.gameMap);

	gameData.camera.target = { 20, 120 }; // world space center of view, we will use this as the camera position
	gameData.camera.rotation = 0.0f;
	gameData.camera.zoom = 100.0f;

	gameData.player.teleport({ 20, 60 });
	gameData.player.transform.w = 0.9f;
	gameData.player.transform.h = 1.8f;

	spawnSlime({18, 60});

	return true;
}

bool updateGame()
{
	float deltaTime{ GetFrameTime() };
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

	gameData.camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	ClearBackground({ 75, 75, 150, 255 });

	if (IsKeyPressed(KEY_I)) { showImgui = !showImgui; }

#pragma region camera movement

	static float CAMERA_SPEED = 10;

	if (IsKeyDown(KEY_A)) gameData.player.transform.pos.x -= CAMERA_SPEED * GetFrameTime();
	if (IsKeyDown(KEY_D)) gameData.player.transform.pos.x += CAMERA_SPEED * GetFrameTime();
	if (IsKeyDown(KEY_W)) gameData.player.transform.pos.y -= CAMERA_SPEED * GetFrameTime();
	if (IsKeyDown(KEY_S)) gameData.player.transform.pos.y += CAMERA_SPEED * GetFrameTime();

	if (IsKeyDown(KEY_SPACE)) gameData.player.jump(10);

#pragma endregion

#pragma region entities

	gameData.player.applyGravity();

	gameData.player.updateForces(deltaTime);

	gameData.player.resolveConstrains(gameData.gameMap);

	gameData.camera.target = gameData.player.transform.pos;

	gameData.player.updateFinal();


	
	std::ranlux24_base rng(std::random_device{}());

	//entities

	EntityUpdateData updateData
	{
		gameData.player.getPosition(),
		rng
	};

	for (auto& e : gameData.entities.entities)
	{
		e.second->update(deltaTime, updateData);

		e.second->physics.applyGravity();

		e.second->physics.updateForces(deltaTime);
		e.second->physics.resolveConstrains(gameData.gameMap);
		e.second->physics.updateFinal();
	}

#pragma endregion

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
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
			if (b)
			{
				*b = {};
			}
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
			if (b)
			{
				b->type = gameData.creativeSelectedBlock;
			}
		}
	}

#pragma region draw world

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

	BeginMode2D(gameData.camera);

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
	DrawTexturePro(
		assetManager.frame,
		{0,0, (float)assetManager.frame.width, (float)assetManager.frame.height},
		{(float)blockX, (float)blockY, 1, 1}, // dest
		{0, 0},
		0.0f,
		WHITE
	);

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

	Transform2D playerSprite = gameData.player.transform;
	playerSprite.w = 1;
	playerSprite.h = 2;
	//move the sprite so that the bottom of the sprite matches the bottom of the collider
	playerSprite.pos.y -= (playerSprite.h - gameData.player.transform.h) / 2;

	DrawTexturePro(
		assetManager.player,
		{ 0, 0, (float)assetManager.player.width, (float)assetManager.player.height },
		playerSprite.getAABB(),
		{ 0, 0 }, //origin
		0.0f, // rotation
		WHITE
	);

	DrawRectangleLinesEx(gameData.player.transform.getAABB(), 0.1,
		{ 20, 101, 250, 120 });

	EndMode2D();

	if (showImgui)
	{
		ImGui::Begin("Game control");

		ImGui::SliderFloat("Camera zoom:", &gameData.camera.zoom, 10, 150);
		ImGui::SliderFloat("Camera speed:", &CAMERA_SPEED, 5, 30);

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
