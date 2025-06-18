#pragma once

#include "Resources/ResourceDefines.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Freelist.hpp"

struct IXAudio2;
struct IXAudio2SourceVoice;
struct IXAudio2SubmixVoice;
struct IXAudio2MasteringVoice;
struct XAUDIO2_EFFECT_DESCRIPTOR;

enum NH_API AudioEffect
{
	Reverb = 0x01,
	Echo = 0x02,
	Limiter = 0x04,
	Equalizer = 0x08,
};

struct AudioFormat
{
	U16 formatTag;
	U16 channelCount;
	UL32 samplesPerSec;
	UL32 avgBytesPerSec;
	U16 blockAlign;
	U16 bitsPerSample;
	U16 extraSize;
};

struct NH_API AudioClip
{
	String name;
	AudioFormat format;
	U32 size = 0;
	U8* buffer = nullptr;
};

struct NH_API AudioParameters
{
	F32 volume = 1.0f;
	F32 speed = 1.0f;
	F32 leftPan = 1.0f;
	F32 rightPan = 1.0f;
	bool looping = false;

	//TODO: effect chain
};

struct AudioPlayback
{
	U32 index;
	IXAudio2SourceVoice* voice;
	AudioParameters parameters;
	ResourceRef<AudioClip> clip;
};

struct EffectChain
{
	U32 effectCount = 0;
	XAUDIO2_EFFECT_DESCRIPTOR* effectDescriptors = nullptr;
};

struct AudioChannel
{
	IXAudio2SubmixVoice* mixer;
	F32 volume = 1.0f;
};

class NH_API Audio
{
public:
	static U32 CreateEffectChain();
	static U32 CreateChannel(const EffectChain& effectChain = {});
	static U32 PlayAudioClip(U32 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters = {});
	static void EndAudioClip(U32 index);

	static void ChangeAudioVolume(U32 index, F32 volume);
	static F32 GetAudioVolume(U32 index);
	static void ChangeAudioPanning(U32 index, F32 leftPan, F32 rightPan);
	static void ChangeAudioEffectChain(U32 index, U32 chainIndex);
	static U32 GetAudioEffectChain(U32 index);

	static void ChangeChannelVolume(U32 channelIndex, F32 volume);
	static F32 GetChannelVolume(U32 channelIndex);
	static void ChangeChannelEffectChain(U32 channelIndex, U32 chainIndex);
	static U32 GetChannelEffectChain(U32 channelIndex);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	static bool Focus(bool value);

	static IXAudio2* audioHandle;
	static IXAudio2MasteringVoice* masterVoice;
	static U32 sampleRate;

	static bool unfocusedAudio;
	static F32 masterVolume;

	static Vector<AudioChannel> channels;
	static AudioPlayback* audioPlaybacks;
	static Freelist freePlaybacks;

	STATIC_CLASS(Audio);
	friend class Engine;
};