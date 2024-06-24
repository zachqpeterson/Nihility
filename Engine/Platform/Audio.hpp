#pragma once

import Containers;

#include "Resources\ResourceDefines.hpp"

struct AudioFormat
{
	U16    formatTag;
	U16    channelCount;
	UL32   samplesPerSec;
	UL32   avgBytesPerSec;
	U16    blockAlign;
	U16    bitsPerSample;
	U16    extraSize;
};

struct NH_API AudioClip : public Resource
{
	AudioFormat format;
	U32			size{ 0 };
	U8* buffer{ nullptr };
};

struct IXAudio2;
struct IXAudio2MasteringVoice;
struct IXAudio2SourceVoice;
struct IXAudio2SubmixVoice;

struct NH_API SfxParameters
{
	F32 volume{ 1.0f };
	F32 speed{ 1.0f };
	F32 leftPan{ 1.0f };
	F32 rightPan{ 1.0f };

	//TODO: Effects
};

struct NH_API SoundEffect
{
private:
	IXAudio2SourceVoice* source{};
	U32 index{ U32_MAX };

	SfxParameters parameters{};

	friend class Audio;
};

//TODO: Better positional audio: https://github.com/microsoft/DirectXTK/wiki/Using-positional-audio
class NH_API Audio
{
public:
	static void PlayMusic(const ResourceRef<AudioClip>& clip);
	static void PlaySfx(const ResourceRef<AudioClip>& clip, const SfxParameters& parameters = {});

	static void ChangeMasterVolume(F32 volume);
	static void ChangeMusicVolume(F32 volume);
	static void ChangeSfxVolume(F32 volume);
	static F32 GetMasterVolume();
	static F32 GetMusicVolume();
	static F32 GetSfxVolume();

private:
	static bool Initialize();
	static void Update();
	static void Shutdown();

	static void Unfocus();
	static void Focus();

	static void TransitionMusic();
	static void EndSFX(U32 index);

	static IXAudio2* audioHandle;
	static IXAudio2MasteringVoice* masterVoice;
	static U32 sampleRate;

	//TODO: Don't hardcode channels
	static IXAudio2SourceVoice* musicSource;
	static IXAudio2SubmixVoice* musicVoice;
	static ResourceRef<AudioClip> currentMusic;
	static ResourceRef<AudioClip> nextMusic;
	static F32 fadeTimer;
	static bool fadingIn;
	static bool fadingOut;

	static IXAudio2SubmixVoice* sfxVoice;
	static Vector<SoundEffect> sfxSources;
	static Freelist freeSFX;

	STATIC_CLASS(Audio);
	friend class Engine;
	friend class Platform;
	friend struct AudioCallbacks;
};