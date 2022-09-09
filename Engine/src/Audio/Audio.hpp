#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include <Containers/Vector.hpp>
#include <Containers/List.hpp>

struct IDirectSoundBuffer;
struct AudioFull;
struct AudioChunk;
struct Timer;
struct Transform2D;

enum AudioType
{
	AUDIO_TYPE_MUSIC,
	AUDIO_TYPE_SFX,

	AUDIO_TYPE_MAX
};

struct AudioInfo
{
	AudioFull* audio;
	AudioChunk* chunk;
	F32 volume{ 1.0f };
	F32 pitch{ 1.0f };
	F32 samplesPlayed{ 0.0f };
	Vector2 position;
	bool global;
	bool loop;
	AudioType type;
};

class NH_API Audio
{
public:
	static void PlayAudio(const String& name, AudioType type, F32 volume = 1.0f, F32 pitch = 1.0f, bool loop = false);
	static void PlayAudioSpacial(const String& name, AudioType type, Vector2 position, F32 volume = 1.0f, F32 pitch = 1.0f, bool loop = false);
	static void SetListener(Transform2D* listener);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();
	static void Start();

	static void FillBuffer(U32 byteToLock, U32 bytesToWrite);
	static void ClearBuffer();
	static void OutputSound();
	
	static IDirectSoundBuffer* buffer;
	static I32 bufferSize;
	static I32 bytesPerSample;
	static I32 safetyBytes;
	static I32 sampleCount;
	static U32 runningSampleIndex;
	static I16* samples;
	static bool soundIsValid;
	static Transform2D* listener;

	static List<AudioInfo> playingAudio;

	friend class Engine;
};