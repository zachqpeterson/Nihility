#include "Audio.hpp"

#include "Core/Logger.hpp"
#include "Core/Settings.hpp"
#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Math/Math.hpp"
#include "Resources/Resources.hpp"
#include "Core/Time.hpp"
#include "SIMD.h"

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(CreateDirectSound);

#define SAMPLES_PER_SECOND 48000
#define MAX_POSSIBLE_OVERRUN 32
#define FALLOFF_SCALE 0.2f

IDirectSoundBuffer* Audio::buffer;
I32 Audio::bufferSize;
I32 Audio::bytesPerSample;
I32 Audio::safetyBytes;
I32 Audio::sampleCount;
U32 Audio::runningSampleIndex;
I16* Audio::samples;
bool Audio::soundIsValid;
Transform2D* Audio::listener;

List<AudioInfo> Audio::playingAudio;

bool Audio::Initialize()
{
	Logger::Info("Initializing Audio System...");

	HMODULE dSoundLibrary = LoadLibraryA("dsound.dll");
	if (!dSoundLibrary)
	{
		Logger::Fatal("Failed to load Direct Sound!");
		return false;
	}

	CreateDirectSound* directSoundCreate = (CreateDirectSound*)GetProcAddress(dSoundLibrary, "DirectSoundCreate");
	LPDIRECTSOUND directSound = nullptr;

	if (!directSoundCreate || !SUCCEEDED(directSoundCreate(nullptr, &directSound, nullptr)))
	{
		Logger::Fatal("Failed to initialize Direct Sound!");
		return false;
	}

	if (!SUCCEEDED(directSound->SetCooperativeLevel((HWND)Platform::Handle(), DSSCL_PRIORITY)))
	{
		Logger::Fatal("Failed to set Direct Sound cooperative level!");
		return false;
	}

	bytesPerSample = sizeof(I16) * Settings::ChannelCount;
	bufferSize = SAMPLES_PER_SECOND * bytesPerSample;
	safetyBytes = (I32)(bufferSize * Settings::TargetFrametime / 2.0);
	soundIsValid = false;

	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = Settings::ChannelCount;
	waveFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) >> 3;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	DSBUFFERDESC primaryBufferDesc{};
	primaryBufferDesc.dwSize = sizeof(primaryBufferDesc);
	primaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

	LPDIRECTSOUNDBUFFER primaryBuffer;
	if (!SUCCEEDED(directSound->CreateSoundBuffer(&primaryBufferDesc, &primaryBuffer, nullptr)))
	{
		Logger::Fatal("Failed to create primary sound buffer!");
		return false;
	}

	if (!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
	{
		Logger::Fatal("Failed to set primary sound buffer format!");
		return false;
	}

	DSBUFFERDESC secondaryBufferDesc{};
	secondaryBufferDesc.dwSize = sizeof(secondaryBufferDesc);
	//TODO: Setting for global focus or not
	secondaryBufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	secondaryBufferDesc.dwBufferBytes = bufferSize;
	secondaryBufferDesc.lpwfxFormat = &waveFormat;

	if (!SUCCEEDED(directSound->CreateSoundBuffer(&secondaryBufferDesc, &buffer, nullptr)))
	{
		Logger::Fatal("Failed to create secondary sound buffer!");
		return false;
	}

	samples = (I16*)Memory::Allocate(bufferSize + MAX_POSSIBLE_OVERRUN, MEMORY_TAG_DATA_STRUCT);

	return true;
}

void Audio::Shutdown()
{
	playingAudio.Destroy();
}

void Audio::Start()
{
	if (!SUCCEEDED(buffer->Play(0, 0, DSBPLAY_LOOPING)))
	{
		Logger::Debug("Audio buffer failed to play!");
	}
}

void Audio::Update()
{
	F32 beginAudioTime = Time::TimeSinceLastFrame();
	F32 deltaTime = Time::DeltaTime();

	DWORD playCursor;
	DWORD writeCursor;

	if (buffer->GetCurrentPosition(&playCursor, &writeCursor) == DS_OK)
	{
		if (!soundIsValid)
		{
			soundIsValid = true;
			runningSampleIndex = writeCursor / bytesPerSample;
		}

		U32 byteToLock = ((runningSampleIndex * bytesPerSample) % bufferSize);
		U32 expectedBytesPerFrame = (I32)((F32)(SAMPLES_PER_SECOND * bytesPerSample) * Settings::TargetFrametime);
		F32 secondsLeftUntilFlip = Math::Max((F32)(deltaTime - beginAudioTime), 0.0f);
		U32 expectedBytesUntilFlip = (U32)((secondsLeftUntilFlip / deltaTime) * (F32)expectedBytesPerFrame);
		U32 expectedFrameBoundaryBytes = playCursor + expectedBytesUntilFlip;
		U32 safeWriteCursor = writeCursor + bufferSize * (writeCursor < playCursor) + safetyBytes;

		U32 targetCursor = 0;
		if (safeWriteCursor < expectedFrameBoundaryBytes) { targetCursor = (expectedFrameBoundaryBytes + expectedBytesPerFrame); }
		else { targetCursor = (writeCursor + expectedBytesPerFrame + safetyBytes); }

		targetCursor %= bufferSize;

		U32 bytesToWrite = targetCursor + bufferSize * (byteToLock > targetCursor) - byteToLock;

		sampleCount = Align8(bytesToWrite / bytesPerSample);
		bytesToWrite = sampleCount * bytesPerSample;

		OutputSound();
		FillBuffer(byteToLock, bytesToWrite);
	}
	else { soundIsValid = false; }
}

void Audio::FillBuffer(U32 byteToLock, U32 bytesToWrite)
{
	void* region1;
	DWORD region1Size;
	void* region2;
	DWORD region2Size;

	if (SUCCEEDED(buffer->Lock(byteToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0)))
	{
		U32 region1SampleCount = region1Size / bytesPerSample;
		U32 region2SampleCount = region2Size / bytesPerSample;

		Memory::Copy(region1, samples, region1Size);
		if (region2) { Memory::Copy(region2, samples + (region1Size >> 1), region2Size); }

		runningSampleIndex += region1SampleCount + region2SampleCount;

		ASSERT_MSG(SUCCEEDED(buffer->Unlock(region1, region1Size, region2, region2Size)), "Could Not Unlock The Secondary Buffer!");
	}
}

void Audio::ClearBuffer()
{
	void* region1;
	DWORD region1Size;
	void* region2;
	DWORD region2Size;

	ASSERT_MSG(SUCCEEDED(buffer->Lock(0, bufferSize, &region1, &region1Size, &region2, &region2Size, 0)), "Could Not Lock The Secondary Buffer!");

	Memory::Set(region1, 0, region1Size);
	if (region2) { Memory::Set(region2, 0, region2Size); }

	ASSERT_MSG(SUCCEEDED(buffer->Unlock(region1, region1Size, region2, region2Size)), "Could Not Unlock The Secondary Buffer!");
}

void Audio::OutputSound()
{
	static constexpr F32 secondsPerSample = 1.0f / (F32)SAMPLES_PER_SECOND;
	static const M128 zero = _mm_set1_ps(0.0f);
	static const M128 one = _mm_set1_ps(1.0f);

	U32 chunkCount = sampleCount >> 2;

	Vector<Vector<M128>> realChannel{ Settings::ChannelCount, Vector<M128>{chunkCount, zero} };
	M128** destination = (M128**)Memory::Allocate(sizeof(M128*) * Settings::ChannelCount, MEMORY_TAG_AUDIO);

	auto it = playingAudio.begin();

	for (auto it = playingAudio.begin(); it != playingAudio.end(); ++it)
	{
		AudioInfo& info = *it;
		bool finished = false;
		U32 totalChunksToMix = chunkCount;

		if (info.chunk && !info.chunk->last && !info.chunk->next)
		{
			Resources::LoadAudioChunk(info.audio, info.chunk);
		}

		for (U32 channelIndex = 0; channelIndex < Settings::ChannelCount; ++channelIndex)
		{
			destination[channelIndex] = realChannel[channelIndex].Data();
		}

		while (totalChunksToMix && !finished)
		{
			if (info.chunk)
			{
				F32 volume = 1.0f;
				M128 mixedVolume;
				switch (info.type)
				{
				case AUDIO_TYPE_MUSIC: {volume = Settings::MusicVolume; } break;
				case AUDIO_TYPE_SFX: {volume = Settings::SfxVolume; } break;
				}

				Vector<F32> balance;
				if (info.global || !listener)
				{
					balance = { Settings::ChannelCount, info.volume };
					mixedVolume = _mm_set1_ps(Settings::MasterVolume * volume);
				}
				else
				{
					Vector2 v = info.position - listener->Position();
					balance.Resize(2);
					balance[0] = 1.0f - (v.x >  1.0f) * 0.5f;
					balance[1] = 1.0f - (v.x < -1.0f) * 0.5f;

					mixedVolume = _mm_set1_ps(Settings::MasterVolume * volume / Math::Max(v.SqrMagnitude() * FALLOFF_SCALE, 1.0f));
				}

				F32 deltaSampleChunk = info.pitch * 4.0f;

				U32 chunksToMix = totalChunksToMix;
				F32 fChunksRemaining = (info.chunk->sampleCount - (U32)Math::Round(info.samplesPlayed)) / deltaSampleChunk;
				U32 ChunksRemaining = (U32)Math::Round(fChunksRemaining);

				if (chunksToMix > ChunksRemaining) { chunksToMix = ChunksRemaining; }

				F32 beginSamplePosition = info.samplesPlayed;
				F32 endSamplePosition = beginSamplePosition + chunksToMix * deltaSampleChunk;

				for (U32 channelIndex = 0; channelIndex < Settings::ChannelCount; ++channelIndex)
				{
					M128 balanceChannel = _mm_setr_ps(
						balance[channelIndex],
						balance[channelIndex],
						balance[channelIndex],
						balance[channelIndex]);

					for (U32 i = 0; i < chunksToMix; ++i)
					{
						F32 samplePosition = beginSamplePosition + deltaSampleChunk * (F32)i;
#if 1 //Bilinear
						M128 samplePos = _mm_setr_ps(
							samplePosition + 0.0f * info.pitch,
							samplePosition + 1.0f * info.pitch,
							samplePosition + 2.0f * info.pitch,
							samplePosition + 3.0f * info.pitch);
						I128 sampleIndex = _mm_cvttps_epi32(samplePos);
						M128 frac = _mm_sub_ps(samplePos, _mm_cvtepi32_ps(sampleIndex));

						M128 sampleValueF = _mm_setr_ps(
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[0]],
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[1]],
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[2]],
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[3]]);
						M128 sampleValueC = _mm_setr_ps(
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[0] + 1],
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[1] + 1],
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[2] + 1],
							info.chunk->samples[channelIndex][((I32*)&sampleIndex)[3] + 1]);

						M128 sampleValue = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, frac), sampleValueF), _mm_mul_ps(frac, sampleValueC));
#else
						M128 sampleValue = _mm_setr_ps(
							info.chunk->samples[channelIndex][(I32)Math::Round(samplePosition + 0.0f * info.pitch)],
							info.chunk->samples[channelIndex][(I32)Math::Round(samplePosition + 1.0f * info.pitch)],
							info.chunk->samples[channelIndex][(I32)Math::Round(samplePosition + 2.0f * info.pitch)],
							info.chunk->samples[channelIndex][(I32)Math::Round(samplePosition + 3.0f * info.pitch)]);
#endif
						M128 d = _mm_load_ps((F32*)&destination[channelIndex][0]);

						d = _mm_add_ps(d, _mm_mul_ps(_mm_mul_ps(mixedVolume, balanceChannel), sampleValue));

						_mm_store_ps((F32*)&destination[channelIndex][0], d);

						++(destination[channelIndex]);
					}
				}

				info.samplesPlayed = endSamplePosition;
				totalChunksToMix -= chunksToMix;

				if (chunksToMix == ChunksRemaining)
				{
					if (info.chunk->next)
					{
						info.samplesPlayed -= info.chunk->sampleCount;
						info.chunk = info.chunk->next;
						if (info.samplesPlayed < 0.0f) { info.samplesPlayed = 0.0f; }
					}
					else
					{
						finished = true;
					}
				}
			}
			else
			{
				break;
				//TODO: handle not loaded sound
			}
		}

		if (finished)
		{
			if (info.loop)
			{
				info.chunk = info.audio->chunks;
				info.samplesPlayed = 0;
			}
			else
			{
				playingAudio.Erase(it);
				if (playingAudio.Size() == 0) { break; }
			}
		}
	}

	Memory::Free(destination, sizeof(M128*) * Settings::ChannelCount, MEMORY_TAG_AUDIO);

	//TODO: do this dynamically with channel count
	M128* source0 = realChannel[0].Data();
	M128* source1 = realChannel[1].Data();

	Memory::Zero(samples, bufferSize + MAX_POSSIBLE_OVERRUN);
	I128* SampleOut = (I128*)samples;

	for (U32 sampleIndex = 0; sampleIndex < chunkCount; ++sampleIndex)
	{
		M128 s0 = _mm_load_ps((F32*)source0++);
		M128 s1 = _mm_load_ps((F32*)source1++);

		I128 l = _mm_cvtps_epi32(s0);
		I128 r = _mm_cvtps_epi32(s1);

		I128 lr0 = _mm_unpacklo_epi32(l, r);
		I128 lr1 = _mm_unpackhi_epi32(l, r);

		I128 s01 = _mm_packs_epi32(lr0, lr1);

		*SampleOut++ = s01;
	}
}

void Audio::PlayAudio(const String& name, AudioType type, F32 volume, F32 pitch, bool loop)
{
	AudioInfo info{};
	info.audio = Resources::LoadAudio(name);
	if (!info.audio) { return; }

	info.chunk = info.audio->chunks;
	info.volume = volume;
	info.pitch = pitch;
	info.global = true;
	info.loop = loop;
	info.type = type;

	playingAudio.PushBack(info);
}

void Audio::PlayAudioSpacial(const String& name, AudioType type, Vector2 position, F32 volume, F32 pitch, bool loop)
{
	AudioInfo info{};
	info.audio = Resources::LoadAudio(name);
	if (!info.audio) { return; }

	info.chunk = info.audio->chunks;
	info.volume = volume;
	info.pitch = pitch;
	info.position = position;
	info.global = false;
	info.loop = loop;
	info.type = type;

	playingAudio.PushBack(info);
}

void Audio::SetListener(Transform2D* newListener)
{
	listener = newListener;
}