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

void Audio::Update()
{

}

bool Audio::Focus(bool value)
{
	if (masterVoice && !unfocusedAudio)
	{
		masterVoice->SetVolume(masterVolume * value);
	}

	return false;
}

U32 Audio::CreateChannel(const EffectChain& effectChain)
{
	U32 index = (U32)channels.Size();
	AudioChannel& channel = channels.Push({});

	XAUDIO2_EFFECT_CHAIN chain;
	chain.EffectCount = effectChain.effectCount;
	chain.pEffectDescriptors = effectChain.effectDescriptors;

	HRESULT result = audioHandle->CreateSubmixVoice(&channel.mixer, 2, sampleRate, 0, 0, nullptr, nullptr);

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

	//XAUDIO2_EFFECT_CHAIN* chain = parameters.effectChainIndex == U32_MAX ? nullptr : &effectChains[parameters.effectChainIndex];

	if (audioHandle->CreateSourceVoice(&voice, (WAVEFORMATEX*)&clip->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callbacks, &sfxSendList, nullptr) < 0) { freePlaybacks.Release(index); return U32_MAX; }
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