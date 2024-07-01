#include "Audio.hpp"

#include "Resources\Settings.hpp"

#include <sdkddkver.h>

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#include <xapofx.h>

import Core;
import Memory;

IXAudio2* Audio::audioHandle;
IXAudio2MasteringVoice* Audio::masterVoice;
Vector<AudioChannel> Audio::channels;
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

U64 Audio::CreateChannel(const AudioChannelParameters& parameters)
{
	U64 index = channels.Size();
	AudioChannel& channel = channels.Push({});

	if (audioHandle->CreateSubmixVoice(&channel.mixer, 2, sampleRate, 0, 0, nullptr, nullptr) < 0) { channels.Pop(); return U64_MAX; }

	return index;
}

U64 Audio::PlayAudio(U64 channelIndex, const ResourceRef<AudioClip>& clip, const AudioParameters& parameters)
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
	audioBuffer.AudioBytes = clip->size;
	audioBuffer.pAudioData = clip->buffer;
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.pContext = &index;

	XAUDIO2_SEND_DESCRIPTOR sfxSend = { 0, channels[channelIndex].mixer };
	XAUDIO2_VOICE_SENDS sfxSendList = { 1, &sfxSend };

	IXAudio2SourceVoice* voice;

	if (audioHandle->CreateSourceVoice(&voice, (WAVEFORMATEX*)&clip->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callbacks, &sfxSendList, nullptr) < 0) { freePlaybacks.Release(index); return; }
	audio.voice = voice;

	F32 volumes[]{ parameters.leftPan * parameters.volume, parameters.rightPan * parameters.volume };

	voice->SetChannelVolumes(2, volumes);
	voice->SetFrequencyRatio(parameters.speed);
	voice->SubmitSourceBuffer(&audioBuffer, NULL);
	voice->Start(0, XAUDIO2_COMMIT_NOW);
}

void Audio::EndAudio(U32 index)
{
	AudioPlayback& audio = audioPlaybacks[index];

	audio.clip = nullptr;
	audio.voice->Stop();
	audio.voice->DestroyVoice();
	audio.voice = nullptr;

	freePlaybacks.Release(index);
}

void Audio::ChangeMasterVolume(F32 volume)
{
	Settings::data.masterVolume = volume;
	masterVoice->SetVolume(volume);
}

void Audio::ChangeChannelVolume(U64 index, F32 volume)
{
	channels[index].parameters.volume = volume;
	channels[index].mixer->SetVolume(volume);
}

F32 Audio::GetMasterVolume()
{
	return Settings::MasterVolume();
}

F32 Audio::GetChannelVolume(U64 index)
{
	return channels[index].parameters.volume;
}