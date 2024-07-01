module;

#include "Resources\Settings.hpp"

#include <sdkddkver.h>

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#include <xapofx.h>

#pragma comment(lib,"xaudio2.lib")

module Audio;

import Core;
import Memory;

IXAudio2* Audio::audioHandle;
IXAudio2MasteringVoice* Audio::masterVoice;
Vector<AudioChannel> Audio::channels;
Vector<EffectChain> Audio::effectChains;
U32 Audio::sampleRate;

AudioPlayback* Audio::audioPlaybacks;
Freelist Audio::freePlaybacks;

struct AudioCallbacks : public IXAudio2VoiceCallback
{
	__declspec(nothrow) void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) final {}
	__declspec(nothrow) void __stdcall OnVoiceProcessingPassEnd() final {}
	__declspec(nothrow) void __stdcall OnStreamEnd() final {}

	__declspec(nothrow) void __stdcall OnBufferStart(void* pBufferContext) final {}
	__declspec(nothrow) void __stdcall OnBufferEnd(void* pBufferContext) final { Audio::EndAudio(*(U32*)pBufferContext); }
	__declspec(nothrow) void __stdcall OnLoopEnd(void* pBufferContext) final {}

	__declspec(nothrow) void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) final {}
} callbacks;

bool Audio::Initialize()
{
	Logger::Trace("Initializing Audio...");

	if (XAudio2Create(&audioHandle) < 0) { return false; }
	if (audioHandle->CreateMasteringVoice(&masterVoice) < 0) { return false; }

	FXMASTERINGLIMITER_PARAMETERS params{};
	params.Release = FXMASTERINGLIMITER_DEFAULT_RELEASE;
	params.Loudness = FXMASTERINGLIMITER_DEFAULT_LOUDNESS;

	IUnknown* pVolumeLimiter;
	if (CreateFX(__uuidof(FXMasteringLimiter), &pVolumeLimiter, &params, sizeof(params)) < 0) { return false; }

	XAUDIO2_VOICE_DETAILS details;
	masterVoice->GetVoiceDetails(&details);
	Settings::data.channelCount = details.InputChannels;
	sampleRate = details.InputSampleRate;

	UL32 channelMask;
	masterVoice->GetChannelMask(&channelMask);

	//X3DAUDIO_HANDLE X3DInstance;
	//X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);

	freePlaybacks(256);
	Memory::AllocateArray(&audioPlaybacks, freePlaybacks.Capacity());

	return true;
}

void Audio::Update()
{

}

void Audio::Shutdown()
{
	Logger::Trace("Shutting Down Audio...");

	freePlaybacks.Destroy();
	Memory::Free(&audioPlaybacks);

	//TODO: Clean up channels
	//musicSource->Stop();
	//musicSource->DestroyVoice();
}

void Audio::Unfocus()
{
	if (masterVoice && !Settings::UnfocusedAudio())
	{
		masterVoice->SetVolume(0.0f);
	}
}

void Audio::Focus()
{
	if (masterVoice && !Settings::UnfocusedAudio())
	{
		masterVoice->SetVolume(Settings::MasterVolume());
	}
}

U32 Audio::CreateChannel(const AudioChannelParameters& parameters)
{
	U32 index = (U32)channels.Size();
	AudioChannel& channel = channels.Push({});

	XAUDIO2_EFFECT_CHAIN* chain = parameters.effectChainIndex == U32_MAX ? nullptr : (XAUDIO2_EFFECT_CHAIN*)&effectChains[parameters.effectChainIndex];

	if (audioHandle->CreateSubmixVoice(&channel.mixer, 2, sampleRate, 0, 0, nullptr, chain) < 0) { channels.Pop(); return U32_MAX; }

	return index;
}

U32 Audio::CreateEffectChain(const EffectsParameters& parameters)
{
	if (parameters.effectFlags == 0) { return U32_MAX; }

	XAUDIO2_EFFECT_DESCRIPTOR effects[4];

	U32 index = (U32)effectChains.Size();
	EffectChain& chain = effectChains.Push({});

	chain.effectCount = 0;
	chain.effectDescriptors = effects;

	if (parameters.effectFlags)
	{
		if (parameters.effectFlags & AUDIO_EFFECT_REVERB)
		{
			FXREVERB_PARAMETERS params{};
			params.Diffusion = FXREVERB_DEFAULT_DIFFUSION;
			params.RoomSize = FXREVERB_DEFAULT_ROOMSIZE;

			IUnknown* reverb;
			CreateFX(__uuidof(FXReverb), &reverb, &params, sizeof(params));

			XAUDIO2_EFFECT_DESCRIPTOR effect{};
			IUnknown* pEffect = reverb;
			BOOL InitialState = true;
			UINT32 OutputChannels = 2;

			effects[chain.effectCount++] = effect;
		}

		if (parameters.effectFlags & AUDIO_EFFECT_ECHO)
		{
			FXECHO_PARAMETERS params{};
			params.WetDryMix = FXECHO_DEFAULT_WETDRYMIX;
			params.Feedback = FXECHO_DEFAULT_FEEDBACK;
			params.Delay = FXECHO_DEFAULT_DELAY;

			IUnknown* echo;
			CreateFX(__uuidof(FXEcho), &echo, &params, sizeof(params));

			XAUDIO2_EFFECT_DESCRIPTOR effect{};
			IUnknown* pEffect = echo;
			BOOL InitialState = true;
			UINT32 OutputChannels = 2;

			effects[chain.effectCount++] = effect;
		}

		if (parameters.effectFlags & AUDIO_EFFECT_LIMITER)
		{
			FXMASTERINGLIMITER_PARAMETERS params{};
			params.Release = FXMASTERINGLIMITER_DEFAULT_RELEASE;
			params.Loudness = FXMASTERINGLIMITER_DEFAULT_LOUDNESS;

			IUnknown* volumeLimiter;
			CreateFX(__uuidof(FXMasteringLimiter), &volumeLimiter, &params, sizeof(params));

			XAUDIO2_EFFECT_DESCRIPTOR effect{};
			IUnknown* pEffect = volumeLimiter;
			BOOL InitialState = true;
			UINT32 OutputChannels = 2;

			effects[chain.effectCount++] = effect;
		}

		if (parameters.effectFlags & AUDIO_EFFECT_EQUALIZER)
		{
			FXEQ_PARAMETERS params{};
			params.FrequencyCenter0 = FXEQ_DEFAULT_FREQUENCY_CENTER_0;
			params.Gain0 = FXEQ_DEFAULT_GAIN;
			params.Bandwidth0 = FXEQ_DEFAULT_BANDWIDTH;
			params.FrequencyCenter1 = FXEQ_DEFAULT_FREQUENCY_CENTER_1;
			params.Gain1 = FXEQ_DEFAULT_GAIN;
			params.Bandwidth1 = FXEQ_DEFAULT_BANDWIDTH;
			params.FrequencyCenter2 = FXEQ_DEFAULT_FREQUENCY_CENTER_2;
			params.Gain2 = FXEQ_DEFAULT_GAIN;
			params.Bandwidth2 = FXEQ_DEFAULT_BANDWIDTH;
			params.FrequencyCenter3 = FXEQ_DEFAULT_FREQUENCY_CENTER_3;
			params.Gain3 = FXEQ_DEFAULT_GAIN;
			params.Bandwidth3 = FXEQ_DEFAULT_BANDWIDTH;

			IUnknown* equalizer;
			CreateFX(__uuidof(FXEQ), &equalizer, &params, sizeof(params));

			XAUDIO2_EFFECT_DESCRIPTOR effect{};
			IUnknown* pEffect = equalizer;
			BOOL InitialState = true;
			UINT32 OutputChannels = 2;

			effects[chain.effectCount++] = effect;
		}
	}

	return index;
}

U32 Audio::PlayAudio(U32 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters)
{
	if (freePlaybacks.Full())
	{
		freePlaybacks.Resize(freePlaybacks.Capacity() * 2);
		Memory::Reallocate(&audioPlaybacks, freePlaybacks.Capacity());
	}

	U32 index = freePlaybacks.GetFree();
	AudioPlayback& audio = audioPlaybacks[index];
	audio.parameters = parameters;
	audio.clip = clip;

	XAUDIO2_BUFFER audioBuffer{};
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.AudioBytes = clip->size;
	audioBuffer.pAudioData = clip->buffer;
	audioBuffer.PlayBegin = 0;
	audioBuffer.PlayLength = 0;
	audioBuffer.LoopBegin = 0;
	audioBuffer.LoopLength = 0;
	audioBuffer.LoopCount = parameters.looping ? XAUDIO2_LOOP_INFINITE : 0;
	audioBuffer.pContext = &index;

	XAUDIO2_SEND_DESCRIPTOR sfxSend = { 0, channels[channelIndex].mixer };
	XAUDIO2_VOICE_SENDS sfxSendList = { 1, &sfxSend };

	IXAudio2SourceVoice* voice;

	XAUDIO2_EFFECT_CHAIN* chain = parameters.effectChainIndex == U32_MAX ? nullptr : (XAUDIO2_EFFECT_CHAIN*)&effectChains[parameters.effectChainIndex];

	if (audioHandle->CreateSourceVoice(&voice, (WAVEFORMATEX*)&clip->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callbacks, &sfxSendList, chain) < 0) { freePlaybacks.Release(index); return U32_MAX; }
	audio.voice = voice;

	F32 volumes[]{ parameters.leftPan * parameters.volume, parameters.rightPan * parameters.volume };

	voice->SetChannelVolumes(2, volumes);
	voice->SetFrequencyRatio(parameters.speed);
	voice->SubmitSourceBuffer(&audioBuffer, NULL);
	voice->Start(0, XAUDIO2_COMMIT_NOW);

	return index;
}

void Audio::EndAudio(U32 index)
{
	AudioPlayback& audio = audioPlaybacks[index];
	if (!audio.parameters.looping)
	{
		audio.clip = nullptr;
		audio.voice->Stop();
		audio.voice->DestroyVoice();
		audio.voice = nullptr;

		freePlaybacks.Release(index);
	}
}

void Audio::ChangeAudioVolume(U32 index, F32 volume)
{
	AudioPlayback& audio = audioPlaybacks[index];
	AudioParameters& parameters = audio.parameters;

	parameters.volume = volume;

	F32 volumes[]{ parameters.leftPan * parameters.volume, parameters.rightPan * parameters.volume };

	audio.voice->SetChannelVolumes(2, volumes);
}

F32 Audio::GetAudioVolume(U32 index)
{
	AudioPlayback& audio = audioPlaybacks[index];

	return audio.parameters.volume;
}

void Audio::ChangeAudioPanning(U32 index, F32 leftPan, F32 rightPan)
{
	AudioPlayback& audio = audioPlaybacks[index];
	AudioParameters& parameters = audio.parameters;

	parameters.leftPan = leftPan;
	parameters.rightPan = rightPan;

	F32 volumes[]{ parameters.leftPan * parameters.volume, parameters.rightPan * parameters.volume };

	audio.voice->SetChannelVolumes(2, volumes);
}

void Audio::ChangeAudioEffectChain(U32 index, U32 chainIndex)
{
	AudioPlayback& audio = audioPlaybacks[index];
	AudioParameters& parameters = audio.parameters;

	if (chainIndex == U32_MAX && parameters.effectChainIndex != U32_MAX)
	{
		audio.voice->SetEffectChain(nullptr);
	}
	else if (chainIndex != U32_MAX)
	{
		XAUDIO2_EFFECT_CHAIN* chain = (XAUDIO2_EFFECT_CHAIN*)&effectChains[chainIndex];

		audio.voice->SetEffectChain(chain);
	}

	parameters.effectChainIndex = chainIndex;
}

U32 Audio::GetAudioEffectChain(U32 index)
{
	return audioPlaybacks[index].parameters.effectChainIndex;
}

void Audio::ChangeChannelVolume(U32 index, F32 volume)
{
	channels[index].parameters.volume = volume;
	channels[index].mixer->SetVolume(volume);
}

F32 Audio::GetChannelVolume(U32 index)
{
	return channels[index].parameters.volume;
}

void Audio::ChangeChannelEffectChain(U32 channelIndex, U32 chainIndex)
{
	AudioChannel& channel = channels[channelIndex];
	AudioChannelParameters& parameters = channel.parameters;

	if (chainIndex == U32_MAX && parameters.effectChainIndex != U32_MAX)
	{
		channel.mixer->SetEffectChain(nullptr);
	}
	else if (chainIndex != U32_MAX)
	{
		XAUDIO2_EFFECT_CHAIN* chain = (XAUDIO2_EFFECT_CHAIN*)&effectChains[chainIndex];

		channel.mixer->SetEffectChain(chain);
	}

	parameters.effectChainIndex = chainIndex;
}

U32 Audio::GetChannelEffectChain(U32 channelIndex)
{
	return channels[channelIndex].parameters.effectChainIndex;
}

void Audio::ChangeMasterVolume(F32 volume)
{
	Settings::data.masterVolume = volume;
	masterVoice->SetVolume(volume);
}

F32 Audio::GetMasterVolume()
{
	return Settings::MasterVolume();
}
