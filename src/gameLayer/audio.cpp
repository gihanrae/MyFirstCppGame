#include "audio.h"
#include <raylib.h>
#include <vector>
#include <cmath>
#include <asserts.h>
#include <settings.h>
#include <raymath.h>

namespace Audio
{
	void init()
	{
		InitAudioDevice();
		SetMasterVolume(0.9);

		loadAllMusicSounds();
	}

	std::vector<Sound> allSounds;

	void loadAllMusicSounds()
	{
		auto loadSound = [&](const char* path)
			{
				Sound s;

				s = LoadSound(path);

				if (s.stream.buffer)
				{
					allSounds.push_back(s);
				}
				else
				{
					allSounds.push_back({});
				}
			};

		allSounds.push_back({});

		loadSound(RESOURCES_PATH "sounds/place.ogg");
		loadSound(RESOURCES_PATH "sounds/break.ogg");

		permaAssertComment(allSounds.size() == SOUNDS_COUNT, "Forgot to add a sound here!");
	}

	void update()
	{
	}

	void playSound(int sound, float volume)
	{

		if (sound <= noneSound || sound >= SOUNDS_COUNT) { return; }

		volume = Clamp(volume, 0, 1);

		volume *= getSettings().masterVolume * getSettings().masterVolume;
		volume *= getSettings().soundsVolume * getSettings().soundsVolume;

		SetSoundVolume(allSounds[sound], volume);
		PlaySound(allSounds[sound]);

	}
}




