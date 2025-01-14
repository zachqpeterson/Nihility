#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Containers\Vector.hpp"
#include "Containers\Freelist.hpp"

#include <xaudio2.h>

enum NH_API AudioEffect
{
	AUDIO_EFFECT_REVERB = 0x01,
	AUDIO_EFFECT_ECHO = 0x02,
	AUDIO_EFFECT_LIMITER = 0x04,
	AUDIO_EFFECT_EQUALIZER = 0x08,
};

struct NH_API EffectsParameters
{
	U32 effectFlags;
};

struct NH_API AudioParameters
{
	F32 volume = 1.0f;
	F32 speed = 1.0f;
	F32 leftPan = 1.0f;
	F32 rightPan = 1.0f;
	bool looping = false;

	U32 effectChainIndex = U32_MAX;
};

struct AudioPlayback
{
	U32 index;
	IXAudio2SourceVoice* voice;
	AudioParameters parameters;
	ResourceRef<AudioClip> clip;
};

struct NH_API AudioChannelParameters
{
	F32 volume = 1.0f;

	U32 effectChainIndex = U32_MAX;
};

struct AudioChannel
{
	IXAudio2SubmixVoice* mixer;
	AudioChannelParameters parameters;
};

//Samples: https://github.com/walbourn/directx-sdk-samples/tree/main/XAudio2
class NH_API Audio
{
public:
	static U32 CreateChannel(const AudioChannelParameters& parameters = {});
	static U32 CreateEffectChain(const EffectsParameters& parameters);
	static U32 PlayAudio(U32 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters = {});

	//TODO: Edit audio playback with index
	static void ChangeAudioVolume(U32 index, F32 volume);
	static F32 GetAudioVolume(U32 index);
	static void ChangeAudioPanning(U32 index, F32 leftPan, F32 rightPan);
	static void ChangeAudioEffectChain(U32 index, U32 chainIndex);
	static U32 GetAudioEffectChain(U32 index);

	static void ChangeChannelVolume(U32 channelIndex, F32 volume);
	static F32 GetChannelVolume(U32 channelIndex);
	static void ChangeChannelEffectChain(U32 channelIndex, U32 chainIndex);
	static U32 GetChannelEffectChain(U32 channelIndex);

	static bool GetUnfocusedAudio();
	static void SetUnfocusedAudio(bool b);
	static F32 GetMasterVolume();
	static void SetMasterVolume(F32 volume);

private:
	static bool Initialize();
	static void Update();
	static void Shutdown();

	static void Unfocus();
	static void Focus();

	static void EndAudio(U32 index);

	static IXAudio2* audioHandle;
	static IXAudio2MasteringVoice* masterVoice;
	static Vector<AudioChannel> channels;
	static Vector<XAUDIO2_EFFECT_CHAIN> effectChains;
	static Vector<XAUDIO2_EFFECT_DESCRIPTOR> effectDescriptors;
	static U32 sampleRate;

	static AudioPlayback* audioPlaybacks;
	static Freelist freePlaybacks;

	static bool unfocusedAudio;
	static F32 masterVolume;

	STATIC_CLASS(Audio);
	friend class Engine;
	friend class Platform;
	friend struct AudioCallbacks;
};