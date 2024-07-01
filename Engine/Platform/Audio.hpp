#pragma once

import Containers;

#include "Resources\ResourceDefines.hpp"

enum NH_API AudioEffect
{
	AUDIO_EFFECT_REVERB = 0x01,
	AUDIO_EFFECT_ECHO = 0x02,
	AUDIO_EFFECT_LIMITER = 0x04,
	AUDIO_EFFECT_EQUALIZER = 0x08,
};

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
struct XAUDIO2_EFFECT_DESCRIPTOR;

struct NH_API EffectsParameters
{
	U32 effectFlags;
};

struct EffectChain
{
	U32 effectCount;
	XAUDIO2_EFFECT_DESCRIPTOR* effectDescriptors;
};

struct NH_API AudioParameters
{
	F32 volume = 1.0f;
	F32 speed = 1.0f;
	F32 leftPan = 1.0f;
	F32 rightPan = 1.0f;
	bool looping = false;

	U64 effectChainIndex = U64_MAX;
};

struct AudioPlayback
{
	IXAudio2SourceVoice* voice;
	AudioParameters parameters;
	ResourceRef<AudioClip> clip;
};

struct NH_API AudioChannelParameters
{
	F32 volume = 1.0f;

	U64 effectChainIndex = U64_MAX;
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
	static U64 CreateChannel(const AudioChannelParameters& parameters = {});
	static U64 CreateEffectChain(const EffectsParameters& parameters);
	static U64 PlayAudio(U64 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters = {});

	//TODO: Edit audio playback with index
	static void ChangeAudioVolume(U64 index, F32 volume);
	static F32 GetAudioVolume(U64 index);
	static void ChangeAudioPanning(U64 index, F32 leftPan, F32 rightPan);
	static void ChangeAudioEffectChain(U64 index, U64 chainIndex);
	static U64 GetAudioEffectChain(U64 index);

	static void ChangeChannelVolume(U64 channelIndex, F32 volume);
	static F32 GetChannelVolume(U64 channelIndex);
	static void ChangeChannelEffectChain(U64 channelIndex, U64 chainIndex);
	static U64 GetChannelEffectChain(U64 channelIndex);

	static void ChangeMasterVolume(F32 volume);
	static F32 GetMasterVolume();

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
	static Vector<EffectChain> effectChains;
	static U32 sampleRate;

	static AudioPlayback* audioPlaybacks;
	static Freelist freePlaybacks;

	STATIC_CLASS(Audio);
	friend class Engine;
	friend class Platform;
	friend struct AudioCallbacks;
};