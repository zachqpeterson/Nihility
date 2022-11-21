#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include <Containers/Vector.hpp>
#include <Containers/List.hpp>

#include <xaudio2.h>
#include <xaudio2fx.h>

enum ReverbType
{
	REVERB_TYPE_FOREST,
	REVERB_TYPE_DEFAULT,
	REVERB_TYPE_GENERIC,
	REVERB_TYPE_PADDEDCELL,
	REVERB_TYPE_ROOM,
	REVERB_TYPE_BATHROOM,
	REVERB_TYPE_LIVINGROOM,
	REVERB_TYPE_STONEROOM,
	REVERB_TYPE_AUDITORIUM,
	REVERB_TYPE_CONCERTHALL,
	REVERB_TYPE_CAVE,
	REVERB_TYPE_ARENA,
	REVERB_TYPE_HANGAR,
	REVERB_TYPE_CARPETEDHALLWAY,
	REVERB_TYPE_HALLWAY,
	REVERB_TYPE_STONECORRIDOR,
	REVERB_TYPE_ALLEY,
	REVERB_TYPE_CITY,
	REVERB_TYPE_MOUNTAINS,
	REVERB_TYPE_QUARRY,
	REVERB_TYPE_PLAIN,
	REVERB_TYPE_PARKINGLOT,
	REVERB_TYPE_SEWERPIPE,
	REVERB_TYPE_UNDERWATER,
	REVERB_TYPE_SMALLROOM,
	REVERB_TYPE_MEDIUMROOM,
	REVERB_TYPE_LARGEROOM,
	REVERB_TYPE_MEDIUMHALL,
	REVERB_TYPE_LARGEHALL,
	REVERB_TYPE_PLATE,

	REVERB_TYPE_COUNT,

	REVERB_TYPE_NONE
};

struct AudioCallbacks : IXAudio2VoiceCallback
{
	void OnBufferEnd(void* pBufferContext) final;
	void OnStreamEnd() final;
	void OnVoiceProcessingPassEnd() final;
	void OnVoiceProcessingPassStart(U32 SamplesRequired) final;
	void OnBufferStart(void* pBufferContext) final;
	void OnLoopEnd(void* pBufferContext) final;
	void OnVoiceError(void* pBufferContext, HRESULT Error) final;
};

struct SourceVoice
{
	IXAudio2SourceVoice* sourceVoice;
	XAUDIO2_SEND_DESCRIPTOR sfxSend;
	XAUDIO2_VOICE_SENDS sfxSendList;
	IUnknown* reverbEffect;
	XAUDIO2_EFFECT_DESCRIPTOR effects;
	XAUDIO2_EFFECT_CHAIN effectChain;
	XAUDIO2FX_REVERB_PARAMETERS native;
	AudioCallbacks callbacks;
};

struct Transform2D;

class NH_API Audio
{
public:
	static void PlaySFX(const String& path, F32 volume = 1.0f, F32 pitch = 1.0f, ReverbType reverbType = REVERB_TYPE_NONE);
	static void PlaySpatialSFX(const String& path, F32 volume = 1.0f, F32 pitch = 1.0f, ReverbType reverbType = REVERB_TYPE_NONE);
	static void PlayMusic(const String& path, F32 volume = 1.0f, F32 pitch = 1.0f);
	static void SetListener(Transform2D* listener);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

	static IXAudio2* xAudioEngine;
	static IXAudio2MasteringVoice* masterVoice;
	static SourceVoice* sfxSourceVoices;
	static IXAudio2SourceVoice* musicSourceVoice;
	static IXAudio2SubmixVoice* musicMixVoice;
	static IXAudio2SubmixVoice* sfxMixVoice;
	static XAUDIO2_VOICE_DETAILS masterDetails;
	static Transform2D* listener;
	static U8 sourceIndex;
	static U8 channelCount;
	static U32 sampleRate;

	friend struct AudioCallbacks;

	friend class Engine;
};