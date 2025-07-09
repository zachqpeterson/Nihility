#pragma once

#include "Resources/ResourceDefines.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Freelist.hpp"

struct IXAudio2;
struct IXAudio2SourceVoice;
struct IXAudio2SubmixVoice;
struct IXAudio2MasteringVoice;
struct IUnknown;

enum class NH_API AudioEffect
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

	U32 effectChain = U32_MAX;
};

struct AudioPlayback
{
	U32 index;
	IXAudio2SourceVoice* voice;
	AudioParameters parameters;
	ResourceRef<AudioClip> clip;
};

struct Effect
{
	IUnknown* pEffect;
	int InitialState;
	U32 OutputChannels;
};

struct EffectChain
{
	Vector<Effect> effectDescriptors;
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
	static void AddReverb(U32 index, F32 diffusion = 0.9f, F32 roomSize = 0.6f);
	static void AddEcho(U32 index, F32 wetDryMix = 0.5f, F32 feedback = 0.5f, F32 delay = 500.0f);
	static void AddLimiter(U32 index, U32 release = 6, U32 loudness = 1000);
	static void AddEqualizer(U32 index, F32 frequencyCenter0 = 100.0f, F32 gain0 = 1.0f, F32 bandwidth0 = 1.0f, 
		F32 frequencyCenter1 = 800.0f, F32 gain1 = 1.0f, F32 bandwidth1 = 1.0f, 
		F32 frequencyCenter2 = 2000.0f, F32 gain2 = 1.0f, F32 bandwidth2 = 1.0f,
		F32 frequencyCenter3 = 10000.0f, F32 gain3 = 1.0f, F32 bandwidth3 = 1.0f);
	static U32 CreateChannel(U32 effectChainIndex = U32_MAX);
	static U32 PlayAudioClip(U32 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters = {});
	static void EndAudioClip(U32 index);

	static void ChangeAudioVolume(U32 index, F32 volume);
	static F32 GetAudioVolume(U32 index);
	static void ChangeAudioPanning(U32 index, F32 leftPan, F32 rightPan);

	static void ChangeChannelVolume(U32 channelIndex, F32 volume);
	static F32 GetChannelVolume(U32 channelIndex);

	static F32 GetMasterVolume();
	static void SetMasterVolume(F32 volume);

private:
	static bool Initialize();
	static void Shutdown();

	static bool Focus(bool value);

	static IXAudio2* audioHandle;
	static IXAudio2MasteringVoice* masterVoice;
	static U32 sampleRate;

	static bool unfocusedAudio;
	static F32 masterVolume;

	static Vector<EffectChain> effectChains;
	static Vector<AudioChannel> channels;
	static AudioPlayback* audioPlaybacks;
	static Freelist freePlaybacks;

	STATIC_CLASS(Audio);
	friend class Engine;
};