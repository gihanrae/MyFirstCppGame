#include <raylib.h>
#include "gameMain.h"
#include <fstream>
#include <iostream>
#include "platform/asserts.h"
#include "assetManager.h"
#include "gameMap.h"
#include "helpers.h"
#include "worldGenerator.h"
#include <raymath.h>
#include <imgui.h>


struct  GameData
{
	GameMap gameMap;
	Camera2D camera;

	int creativeSelectedBlock = Block::dirt;

}gameData;

AssetManager assetManager;

bool showImgui = false;

bool initGame()
{
	assetManager.loadAll();

	generateWorld(gameData.gameMap);

	gameData.camera.target = { 0, 0 }; // world space center of view, we will use this as the camera position
	gameData.camera.rotation = 0.0f;
	gameData.camera.zoom = 100.0f;

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
	if (IsKeyDown(KEY_A)) gameData.camera.target.x -= CAMERA_SPEED * deltaTime;
	if (IsKeyDown(KEY_D)) gameData.camera.target.x += CAMERA_SPEED * deltaTime;
	if (IsKeyDown(KEY_W)) gameData.camera.target.y -= CAMERA_SPEED * deltaTime;
	if (IsKeyDown(KEY_S)) gameData.camera.target.y += CAMERA_SPEED * deltaTime;

#pragma endregion

	Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), gameData.camera);
	int blockX = (int)floor(worldPos.x);
	int blockY = (int)floor(worldPos.y);

	if (gameData.creativeSelectedBlock < 0) { gameData.creativeSelectedBlock = 0; }
	if (gameData.creativeSelectedBlock >= Block::BLOCKS_COUNT) { gameData.creativeSelectedBlock = Block::BLOCKS_COUNT - 1; }

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

	EndMode2D();

	if (showImgui)
	{
		ImGui::Begin("Game control");

		ImGui::SliderFloat("Camera zoom:", &gameData.camera.zoom, 10, 150);
		ImGui::SliderFloat("Camera speed:", &CAMERA_SPEED, 5, 30);

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
