#include "Audio.hpp"

#include "Core/Logger.hpp"
#include "Core/Settings.hpp"
#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Math/Math.hpp"
#include "Resources/Resources.hpp"
#include "Core/Time.hpp"

#pragma comment(lib,"xaudio2.lib")

#define MAX_VOICES 32

XAUDIO2FX_REVERB_I3DL2_PARAMETERS reverbTypes[REVERB_TYPE_COUNT] =
{
	XAUDIO2FX_I3DL2_PRESET_FOREST,
	XAUDIO2FX_I3DL2_PRESET_DEFAULT,
	XAUDIO2FX_I3DL2_PRESET_GENERIC,
	XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
	XAUDIO2FX_I3DL2_PRESET_ROOM,
	XAUDIO2FX_I3DL2_PRESET_BATHROOM,
	XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
	XAUDIO2FX_I3DL2_PRESET_STONEROOM,
	XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
	XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
	XAUDIO2FX_I3DL2_PRESET_CAVE,
	XAUDIO2FX_I3DL2_PRESET_ARENA,
	XAUDIO2FX_I3DL2_PRESET_HANGAR,
	XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
	XAUDIO2FX_I3DL2_PRESET_HALLWAY,
	XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
	XAUDIO2FX_I3DL2_PRESET_ALLEY,
	XAUDIO2FX_I3DL2_PRESET_CITY,
	XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
	XAUDIO2FX_I3DL2_PRESET_QUARRY,
	XAUDIO2FX_I3DL2_PRESET_PLAIN,
	XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
	XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
	XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
	XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
	XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
	XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
	XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
	XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
	XAUDIO2FX_I3DL2_PRESET_PLATE,
};

IXAudio2* Audio::xAudioEngine;
IXAudio2MasteringVoice* Audio::masterVoice;
SourceVoice* Audio::sfxSourceVoices;
IXAudio2SourceVoice* Audio::musicSourceVoice;
IXAudio2SubmixVoice* Audio::musicMixVoice;
IXAudio2SubmixVoice* Audio::sfxMixVoice;
XAUDIO2_VOICE_DETAILS Audio::masterDetails;
Transform2D* Audio::listener;
U8 Audio::sourceIndex;
U8 Audio::channelCount;
U32 Audio::sampleRate;

void AudioCallbacks::OnBufferEnd(void* pBufferContext) {}
void AudioCallbacks::OnStreamEnd() {}
void AudioCallbacks::OnVoiceProcessingPassEnd() {}
void AudioCallbacks::OnVoiceProcessingPassStart(U32 SamplesRequired) {}
void AudioCallbacks::OnBufferStart(void* pBufferContext) {}
void AudioCallbacks::OnLoopEnd(void* pBufferContext) {}
void AudioCallbacks::OnVoiceError(void* pBufferContext, HRESULT Error) {}

bool Audio::Initialize()
{
	Logger::Info("Initializing Audio System...");

	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		MessageBox(NULL, L"Failed to initialize COM!", L"FAILURE", MB_OK);
		return false;
	}

	if (FAILED(XAudio2Create(&xAudioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		MessageBox(NULL, L"Failed to initialize XAudio!", L"FAILURE", MB_OK);
		return false;
	}

	if (FAILED(xAudioEngine->CreateMasteringVoice(&masterVoice)))
	{
		MessageBox(NULL, L"Failed to initialize XAudio mastering voice!", L"FAILURE", MB_OK);
		return false;
	}

	masterVoice->GetVoiceDetails(&masterDetails);
	channelCount = masterDetails.InputChannels;
	sampleRate = masterDetails.InputSampleRate;

	if (FAILED(xAudioEngine->CreateSubmixVoice(&sfxMixVoice, 1, sampleRate, 0, 0, nullptr, nullptr)))
	{
		MessageBox(NULL, L"Failed to initialize XAudio submix voice!", L"FAILURE", MB_OK);
		return false;
	}

	if (FAILED(xAudioEngine->CreateSubmixVoice(&musicMixVoice, channelCount, sampleRate, 0, 0, nullptr, nullptr)))
	{
		MessageBox(NULL, L"Failed to initialize XAudio submix voice!", L"FAILURE", MB_OK);
		return false;
	}

	sfxSourceVoices = (SourceVoice*)malloc(sizeof(SourceVoice) * MAX_VOICES);
	memset(sfxSourceVoices, 0, sizeof(SourceVoice) * MAX_VOICES);

	return true;
}

void Audio::Shutdown()
{
	
}

void Audio::Update()
{
	
}

void Audio::PlaySFX(const String& path, F32 volume, F32 pitch, ReverbType reverbType)
{
	AudioData* audioData = Resources::LoadAudio(path);

	XAUDIO2_BUFFER audioBuffer{};
	audioBuffer.AudioBytes = audioData->size;
	audioBuffer.pAudioData = audioData->data;
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;

	SourceVoice& sourceVoice = sfxSourceVoices[sourceIndex];

	if (sourceVoice.sourceVoice)
	{
		for (U8 i = 0; i < sourceVoice.effectChain.EffectCount; ++i)
		{
			sourceVoice.effectChain.pEffectDescriptors[i].pEffect->Release();
		}

		sourceVoice.effectChain.EffectCount = 0;
		sourceVoice.effectChain.pEffectDescriptors = nullptr;

		sourceVoice.sourceVoice->Stop();
		sourceVoice.sourceVoice->DestroyVoice();
		sourceVoice.sourceVoice = nullptr;
	}

	if (reverbType != REVERB_TYPE_NONE)
	{
		if (FAILED(XAudio2CreateReverb(&sourceVoice.reverbEffect, 0)))
		{
			MessageBox(NULL, L"Failed to create reverb effect!", L"FAILURE", MB_OK);
			return;
		}

		sourceVoice.effects = { sourceVoice.reverbEffect, TRUE, 1 };
		++sourceVoice.effectChain.EffectCount;
		sourceVoice.effectChain.pEffectDescriptors = &sourceVoice.effects; //TODO: Use a vector
	}

	IXAudio2SourceVoice** sourceVoicePtr = &sourceVoice.sourceVoice;
	sourceVoice.sfxSend = { 0, sfxMixVoice };
	sourceVoice.sfxSendList = { 1, &sourceVoice.sfxSend };

	if (reverbType != REVERB_TYPE_NONE)
	{
		if (xAudioEngine->CreateSourceVoice(sourceVoicePtr, &audioData->format, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &sourceVoice.sfxSendList, &sourceVoice.effectChain) < 0)
		{
			return;
		}

		ReverbConvertI3DL2ToNative(&reverbTypes[reverbType], &sourceVoice.native);
		sourceVoice.sourceVoice->SetEffectParameters(0, &sourceVoice.native, sizeof(sourceVoice.native));
	}
	else
	{
		if (xAudioEngine->CreateSourceVoice(sourceVoicePtr, &audioData->format, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &sourceVoice.sfxSendList, nullptr) < 0)
		{
			return;
		}
	}

	sourceVoice.sourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
	sourceVoice.sourceVoice->SetVolume(volume);
	sourceVoice.sourceVoice->SetFrequencyRatio(pitch);

	if (sourceVoice.sourceVoice->SubmitSourceBuffer(&audioBuffer, NULL) < 0) { return; }
	if (++sourceIndex == MAX_VOICES) { sourceIndex = 0; }
}

void Audio::PlaySpatialSFX(const String& path, F32 volume, F32 pitch, ReverbType reverbType)
{

}

void Audio::PlayMusic(const String& path, F32 volume, F32 pitch)
{
	AudioData* audioData = Resources::LoadAudio(path);

	XAUDIO2_BUFFER audioBuffer{};
	audioBuffer.AudioBytes = audioData->size;
	audioBuffer.pAudioData = audioData->data;
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	XAUDIO2_SEND_DESCRIPTOR musicSend = { 0, musicMixVoice };
	XAUDIO2_VOICE_SENDS musicSendList = { 1, &musicSend };

	if (musicSourceVoice)
	{
		musicSourceVoice->Stop();
		musicSourceVoice->DestroyVoice();
		musicSourceVoice = nullptr;
	}

	if (xAudioEngine->CreateSourceVoice(&musicSourceVoice, &audioData->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &musicSendList, nullptr) < 0)
	{
		return;
	}

	musicSourceVoice->SetVolume(volume);
	musicSourceVoice->SetFrequencyRatio(pitch);
	musicSourceVoice->Start(0, XAUDIO2_COMMIT_NOW);

	musicSourceVoice->SubmitSourceBuffer(&audioBuffer, NULL);
}

void Audio::SetListener(Transform2D* newListener)
{
	listener = newListener;
}