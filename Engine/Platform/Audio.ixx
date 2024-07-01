module;

#include "Resources\ResourceDefines.hpp"

#include <xaudio2.h>

export module Audio;

import Containers;

export enum NH_API AudioEffect
{
	AUDIO_EFFECT_REVERB = 0x01,
	AUDIO_EFFECT_ECHO = 0x02,
	AUDIO_EFFECT_LIMITER = 0x04,
	AUDIO_EFFECT_EQUALIZER = 0x08,
};

export struct AudioFormat
{
	U16    formatTag;
	U16    channelCount;
	UL32   samplesPerSec;
	UL32   avgBytesPerSec;
	U16    blockAlign;
	U16    bitsPerSample;
	U16    extraSize;
};

export struct NH_API AudioClip : public Resource
{
	AudioFormat format;
	U32			size{ 0 };
	U8* buffer{ nullptr };
};

export struct NH_API EffectsParameters
{
	U32 effectFlags;
};

export struct EffectChain
{
	U32 effectCount;
	XAUDIO2_EFFECT_DESCRIPTOR* effectDescriptors;
};

export struct NH_API AudioParameters
{
	F32 volume = 1.0f;
	F32 speed = 1.0f;
	F32 leftPan = 1.0f;
	F32 rightPan = 1.0f;
	bool looping = false;

	U32 effectChainIndex = U32_MAX;
};

export struct AudioPlayback
{
	IXAudio2SourceVoice* voice;
	AudioParameters parameters;
	ResourceRef<AudioClip> clip;
};

export struct NH_API AudioChannelParameters
{
	F32 volume = 1.0f;

	U32 effectChainIndex = U32_MAX;
};

export struct AudioChannel
{
	IXAudio2SubmixVoice* mixer;
	AudioChannelParameters parameters;
};

//Samples: https://github.com/walbourn/directx-sdk-samples/tree/main/XAudio2
export class NH_API Audio
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