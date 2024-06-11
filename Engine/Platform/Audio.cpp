#include "Audio.hpp"

#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Resources\Settings.hpp"

#include <xaudio2.h>
#include <x3daudio.h>

IXAudio2* Audio::audioHandle;
IXAudio2MasteringVoice* Audio::masterVoice;
U32 Audio::sampleRate;

IXAudio2SourceVoice* Audio::musicSource;
IXAudio2SubmixVoice* Audio::musicVoice;
ResourceRef<AudioClip> Audio::currentMusic;
ResourceRef<AudioClip> Audio::nextMusic;
F32 Audio::fadeTimer;
bool Audio::fadingIn;
bool Audio::fadingOut;

IXAudio2SubmixVoice* Audio::sfxVoice;
Vector<SoundEffect> Audio::sfxSources(128, {});
Freelist Audio::freeSFX{ 128 };

struct AudioCallbacks : public IXAudio2VoiceCallback
{
	__declspec(nothrow) void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) final {}
	__declspec(nothrow) void __stdcall OnVoiceProcessingPassEnd() final {}
	__declspec(nothrow) void __stdcall OnStreamEnd() final {}

	__declspec(nothrow) void __stdcall OnBufferStart(void* pBufferContext) final {}
	__declspec(nothrow) void __stdcall OnBufferEnd(void* pBufferContext) final { Audio::EndSFX(*(U32*)pBufferContext); }
	__declspec(nothrow) void __stdcall OnLoopEnd(void* pBufferContext) final {}

	__declspec(nothrow) void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) final {}
} callbacks;

bool Audio::Initialize()
{
	Logger::Trace("Initializing Audio...");

	if (XAudio2Create(&audioHandle) < 0) { return false; }
	if (audioHandle->CreateMasteringVoice(&masterVoice) < 0) { return false; }

	XAUDIO2_VOICE_DETAILS details;
	masterVoice->GetVoiceDetails(&details);
	Settings::data.channelCount = details.InputChannels;
	sampleRate = details.InputSampleRate;

	if (audioHandle->CreateSubmixVoice(&sfxVoice, 2, sampleRate, 0, 0, nullptr, nullptr) < 0) { return false; }
	if (audioHandle->CreateSubmixVoice(&musicVoice, 2, sampleRate, 0, 0, nullptr, nullptr) < 0) { return false; }

	UL32 channelMask;
	masterVoice->GetChannelMask(&channelMask);

	//X3DAUDIO_HANDLE X3DInstance;
	//X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);

	return true;
}

void Audio::Update()
{
	if (fadingOut)
	{
		fadeTimer -= (F32)Time::DeltaTime();

		if (fadeTimer <= 0.0f)
		{
			fadeTimer = 0.0f;
			fadingOut = false;
			fadingIn = true;

			TransitionMusic();
		}

		musicSource->SetVolume(fadeTimer);
	}
	else if (fadingIn)
	{
		fadeTimer += (F32)Time::DeltaTime();
		if (fadeTimer >= 1.0f)
		{
			fadeTimer = 1.0f;
			fadingIn = false;
		}

		musicSource->SetVolume(fadeTimer);
	}
}

void Audio::Shutdown()
{
	Logger::Trace("Shutting Down Audio...");

	freeSFX.Destroy();
	sfxSources.Destroy();

	if (musicSource)
	{
		musicSource->Stop();
		musicSource->DestroyVoice();
		musicSource = nullptr;
	}
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

void Audio::TransitionMusic()
{
	musicSource->Stop();
	musicSource->DestroyVoice();

	XAUDIO2_BUFFER audioBuffer{};
	audioBuffer.AudioBytes = nextMusic->size;
	audioBuffer.pAudioData = nextMusic->buffer;
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

	XAUDIO2_SEND_DESCRIPTOR musicSend = { 0, musicVoice };
	XAUDIO2_VOICE_SENDS musicSendList = { 1, &musicSend };

	if (audioHandle->CreateSourceVoice(&musicSource, (WAVEFORMATEX*)&nextMusic->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &musicSendList, nullptr) < 0) { return; }

	musicSource->SubmitSourceBuffer(&audioBuffer, NULL);
	musicSource->Start(0, XAUDIO2_COMMIT_NOW);

	currentMusic = nextMusic;
	nextMusic = nullptr;
}

void Audio::EndSFX(U32 index)
{
	SoundEffect& sfx = sfxSources[index];

	sfx.source->Stop();
	sfx.source->DestroyVoice();
	sfx.source = nullptr;
	sfx.index = U32_MAX;

	freeSFX.Release(index);
}

void Audio::PlayMusic(const ResourceRef<AudioClip>& clip)
{
	if (currentMusic)
	{
		nextMusic = clip;

		if (fadingIn)
		{
			fadingOut = true;
		}
		else if (!fadingOut)
		{
			fadingOut = true;
			fadeTimer = 1.0f;
		}
	}
	else
	{
		currentMusic = clip;

		XAUDIO2_BUFFER audioBuffer{};
		audioBuffer.AudioBytes = clip->size;
		audioBuffer.pAudioData = clip->buffer;
		audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
		audioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

		XAUDIO2_SEND_DESCRIPTOR musicSend = { 0, musicVoice };
		XAUDIO2_VOICE_SENDS musicSendList = { 1, &musicSend };

		if (audioHandle->CreateSourceVoice(&musicSource, (WAVEFORMATEX*)&clip->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &musicSendList, nullptr) < 0) { return; }

		musicSource->SubmitSourceBuffer(&audioBuffer, NULL);
		musicSource->Start(0, XAUDIO2_COMMIT_NOW);
	}
}

void Audio::PlaySfx(const ResourceRef<AudioClip>& clip, const SfxParameters& parameters)
{
	//TODO: Resizing
	if (!freeSFX.Full())
	{
		U32 index = freeSFX.GetFree();
		SoundEffect& sfx = sfxSources[index];
		sfx.index = index;
		sfx.parameters = parameters;

		XAUDIO2_BUFFER audioBuffer{};
		audioBuffer.AudioBytes = clip->size;
		audioBuffer.pAudioData = clip->buffer;
		audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
		audioBuffer.pContext = &sfx.index;

		XAUDIO2_SEND_DESCRIPTOR sfxSend = { 0, sfxVoice };
		XAUDIO2_VOICE_SENDS sfxSendList = { 1, &sfxSend };

		IXAudio2SourceVoice* voice;

		if (audioHandle->CreateSourceVoice(&voice, (WAVEFORMATEX*)&clip->format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callbacks, &sfxSendList, nullptr) < 0) { freeSFX.Release(index); return; }
		sfx.source = voice;

		F32 volumes[]{ parameters.leftPan * parameters.volume, parameters.rightPan * parameters.volume };

		voice->SetChannelVolumes(2, volumes);
		voice->SetFrequencyRatio(parameters.speed);
		voice->SubmitSourceBuffer(&audioBuffer, NULL);
		voice->Start(0, XAUDIO2_COMMIT_NOW);
	}
}

void Audio::ChangeMasterVolume(F32 volume)
{
	Settings::data.masterVolume = volume;
	masterVoice->SetVolume(volume);
}

void Audio::ChangeMusicVolume(F32 volume)
{
	Settings::data.musicVolume = volume;
	musicVoice->SetVolume(volume);
}

void Audio::ChangeSfxVolume(F32 volume)
{
	Settings::data.sfxVolume = volume;
	sfxVoice->SetVolume(volume);
}

F32 Audio::GetMasterVolume()
{
	return Settings::MasterVolume();
}

F32 Audio::GetMusicVolume()
{
	return Settings::MusicVolume();
}

F32 Audio::GetSfxVolume()
{
	return Settings::SfxVolume();
}