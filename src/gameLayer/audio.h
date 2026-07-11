#pragma once

namespace Audio
{
	void init();

	void loadAllMusicSounds();

	void update();

	void playSound(int sound, float volume = 1);

	enum Sounds
	{
		noneSound = 0,
		placeBlock,
		breakBlock,

		SOUNDS_COUNT
	};
}