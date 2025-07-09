#include "Audio.hpp"

#include "Core/Logger.hpp"
#include "Platform/Platform.hpp"
#include "Resources/Settings.hpp"

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#include <xapofx.h>

#pragma comment(lib,"xaudio2.lib")

IXAudio2* Audio::audioHandle;
IXAudio2MasteringVoice* Audio::masterVoice;
U32 Audio::sampleRate;

bool Audio::unfocusedAudio;
F32 Audio::masterVolume;

Vector<EffectChain> Audio::effectChains;
Vector<AudioChannel> Audio::channels;
AudioPlayback* Audio::audioPlaybacks;
Freelist Audio::freePlaybacks;

struct AudioCallbacks : public IXAudio2VoiceCallback
{
	__declspec(nothrow) void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) final {}
	__declspec(nothrow) void __stdcall OnVoiceProcessingPassEnd() final {}
	__declspec(nothrow) void __stdcall OnStreamEnd() final {}

	__declspec(nothrow) void __stdcall OnBufferStart(void* pBufferContext) final {}
	__declspec(nothrow) void __stdcall OnBufferEnd(void* pBufferContext) final { Audio::EndAudioClip(*(U32*)pBufferContext); }
	__declspec(nothrow) void __stdcall OnLoopEnd(void* pBufferContext) final {}

	__declspec(nothrow) void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) final {}
} callbacks;

bool Audio::Initialize()
{
	Logger::Trace("Initializing Audio...");

	Platform::OnFocused += Focus;

	if (XAudio2Create(&audioHandle) < 0) { return false; }
	if (audioHandle->CreateMasteringVoice(&masterVoice) < 0) { return false; }

	FXMASTERINGLIMITER_PARAMETERS params{};
	params.Release = FXMASTERINGLIMITER_DEFAULT_RELEASE;
	params.Loudness = FXMASTERINGLIMITER_DEFAULT_LOUDNESS;

	IUnknown* pVolumeLimiter;
	if (CreateFX(__uuidof(FXMasteringLimiter), &pVolumeLimiter, &params, sizeof(params)) < 0) { return false; }

	XAUDIO2_VOICE_DETAILS details;
	masterVoice->GetVoiceDetails(&details);
	Settings::SetSetting(Settings::ChannelCount, &details.InputChannels, sizeof(U32), SettingType::NUM32);
	sampleRate = details.InputSampleRate;

	UL32 channelMask;
	masterVoice->GetChannelMask(&channelMask);

	Settings::GetSetting(Settings::UnfocusedAudio, &unfocusedAudio, sizeof(bool));
	Settings::GetSetting(Settings::MasterVolume, &masterVolume, sizeof(U32));

	freePlaybacks(256);
	Memory::Allocate(&audioPlaybacks, freePlaybacks.Capacity());

	return true;
}

void Audio::Shutdown()
{
	Logger::Trace("Cleaning Up Audio...");

	freePlaybacks.Destroy();
	Memory::Free(&audioPlaybacks);
}

bool Audio::Focus(bool value)
{
	if (masterVoice && !unfocusedAudio)
	{
		masterVoice->SetVolume(masterVolume * value);
	}

	return false;
}

U32 Audio::CreateEffectChain()
{
	U32 index = (U32)effectChains.Size();
	effectChains.Push({});

	return index;
}

void Audio::AddReverb(U32 index, F32 diffusion, F32 roomSize)
{
	EffectChain& chain = effectChains[index];

	FXREVERB_PARAMETERS params{};
	params.Diffusion = diffusion;
	params.RoomSize = roomSize;

	IUnknown* reverb;
	CreateFX(__uuidof(FXReverb), &reverb, &params, sizeof(params));

	Effect& effect = chain.effectDescriptors.Push({});
	effect.pEffect = reverb;
	effect.InitialState = true;
	effect.OutputChannels = 2;
}

void Audio::AddEcho(U32 index, F32 wetDryMix, F32 feedback, F32 delay)
{
	EffectChain& chain = effectChains[index];

	FXECHO_PARAMETERS params{};
	params.WetDryMix = wetDryMix;
	params.Feedback = feedback;
	params.Delay = delay;

	IUnknown* echo;
	CreateFX(__uuidof(FXEcho), &echo, &params, sizeof(params));

	XAUDIO2_EFFECT_DESCRIPTOR effect{};
	effect.pEffect = echo;
	effect.InitialState = true;
	effect.OutputChannels = 2;
}

void Audio::AddLimiter(U32 index, U32 release, U32 loudness)
{
	EffectChain& chain = effectChains[index];

	FXMASTERINGLIMITER_PARAMETERS params{};
	params.Release = release;
	params.Loudness = loudness;

	IUnknown* volumeLimiter;
	CreateFX(__uuidof(FXMasteringLimiter), &volumeLimiter, &params, sizeof(params));

	XAUDIO2_EFFECT_DESCRIPTOR effect{};
	effect.pEffect = volumeLimiter;
	effect.InitialState = true;
	effect.OutputChannels = 2;
}

void Audio::AddEqualizer(U32 index, F32 frequencyCenter0, F32 gain0, F32 bandwidth0,
	F32 frequencyCenter1, F32 gain1, F32 bandwidth1,
	F32 frequencyCenter2, F32 gain2, F32 bandwidth2,
	F32 frequencyCenter3, F32 gain3, F32 bandwidth3)
{
	EffectChain& chain = effectChains[index];

	FXEQ_PARAMETERS params{};
	params.FrequencyCenter0 = frequencyCenter0;
	params.Gain0 = gain0;
	params.Bandwidth0 = bandwidth0;
	params.FrequencyCenter1 = frequencyCenter1;
	params.Gain1 = gain1;
	params.Bandwidth1 = bandwidth1;
	params.FrequencyCenter2 = frequencyCenter2;
	params.Gain2 = gain2;
	params.Bandwidth2 = bandwidth2;
	params.FrequencyCenter3 = frequencyCenter3;
	params.Gain3 = gain3;
	params.Bandwidth3 = bandwidth3;

	IUnknown* equalizer;
	CreateFX(__uuidof(FXEQ), &equalizer, &params, sizeof(params));

	XAUDIO2_EFFECT_DESCRIPTOR effect{};
	effect.pEffect = equalizer;
	effect.InitialState = true;
	effect.OutputChannels = 2;
}

U32 Audio::CreateChannel(U32 effectChainIndex)
{
	U32 index = (U32)channels.Size();
	AudioChannel& channel = channels.Push({});

	XAUDIO2_EFFECT_CHAIN effectChain;

	bool validEffectChain = false;
	if (effectChainIndex != U32_MAX)
	{
		EffectChain& chain = effectChains[effectChainIndex];

		if (chain.effectDescriptors.Size())
		{
			validEffectChain = true;

			effectChain.EffectCount = (U32)chain.effectDescriptors.Size();
			effectChain.pEffectDescriptors = (XAUDIO2_EFFECT_DESCRIPTOR*)chain.effectDescriptors.Data();
		}
	}

	HRESULT result = audioHandle->CreateSubmixVoice(&channel.mixer, 2, sampleRate, 0, 0, nullptr, validEffectChain ? &effectChain : nullptr);

	if (result < 0) { channels.Pop(); return U32_MAX; }

	return index;
}

U32 Audio::PlayAudioClip(U32 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters)
{
	if (freePlaybacks.Full())
	{
		freePlaybacks.Resize(freePlaybacks.Capacity() * 2);
		Memory::Reallocate(&audioPlaybacks, freePlaybacks.Capacity());
	}

	U32 index = freePlaybacks.GetFree();
	AudioPlayback& audio = audioPlaybacks[index];
	audio.index = index;
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
	audioBuffer.pContext = &audio.index;

	XAUDIO2_SEND_DESCRIPTOR sfxSend = { 0, channels[channelIndex].mixer };
	XAUDIO2_VOICE_SENDS sfxSendList = { 1, &sfxSend };

	IXAudio2SourceVoice* voice;

	XAUDIO2_EFFECT_CHAIN effectChain;

	bool validEffectChain = false;
	if (parameters.effectChain != U32_MAX)
	{
		EffectChain& chain = effectChains[parameters.effectChain];

		if (chain.effectDescriptors.Size())
		{
			validEffectChain = true;

			effectChain.EffectCount = (U32)chain.effectDescriptors.Size();
			effectChain.pEffectDescriptors = (XAUDIO2_EFFECT_DESCRIPTOR*)chain.effectDescriptors.Data();
		}
	}

	if (audioHandle->CreateSourceVoice(&voice, (WAVEFORMATEX*)&clip->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callbacks, &sfxSendList, validEffectChain ? &effectChain : nullptr) < 0) { freePlaybacks.Release(index); return U32_MAX; }
	audio.voice = voice;

	F32 volumes[]{ parameters.leftPan * parameters.volume, parameters.rightPan * parameters.volume };

	voice->SetChannelVolumes(2, volumes);
	voice->SetFrequencyRatio(parameters.speed);
	voice->SubmitSourceBuffer(&audioBuffer, NULL);
	voice->Start(0, XAUDIO2_COMMIT_NOW);

	return index;
}

void Audio::EndAudioClip(U32 index)
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

void Audio::ChangeChannelVolume(U32 index, F32 volume)
{
	channels[index].volume = volume;
	channels[index].mixer->SetVolume(volume);
}

F32 Audio::GetChannelVolume(U32 index)
{
	return channels[index].volume;
}

F32 Audio::GetMasterVolume()
{
	return masterVolume;
}

void Audio::SetMasterVolume(F32 volume)
{
	masterVolume = volume;
	Settings::SetSetting(Settings::MasterVolume, &masterVolume, sizeof(F32), SettingType::NUM32);
	masterVoice->SetVolume(volume);
}