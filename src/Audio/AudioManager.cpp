#include "AudioManager.h"
#include "WavFile.h"
#include "NFDriver/NFDriver.h"
#include "stb_vorbis.h"
#include <atomic>
#include "util/Utility.h"
#include <chrono>
#include <mutex>
#undef min
#undef max


#define NUM_AUDIO_FILES_IN_LIST sizeof(uintptr_t) * 8
using namespace nativeformat;
using namespace driver;





struct AudioPlaybackContext
{
	union File
	{
		WavFile* wav;
		stb_vorbis* ogg;
	}file = { nullptr };
	int index = 0;
	int remaining = 0;
	float volume = 1.0f;
	bool repeat = false;
	bool isPlaying = false;
	bool inUse = false;
	bool isOgg = false;
};

struct AudioFileList
{
	AudioFileList* next;
	uintptr_t fillMask;
	WavFile files[NUM_AUDIO_FILES_IN_LIST];
};

static WavFile* AllocWavFile(AudioFileList* list)
{
	for (uint64_t i = 0; i < NUM_AUDIO_FILES_IN_LIST; i++)
	{
		uint64_t cur = 1LL << i;
		if (!(cur & list->fillMask))
		{
			list->fillMask |= cur;
			memset(&list->files[i], 0, sizeof(WavFile));
			return &list->files[i];
		}
	}
	if (list->next)
	{
		return AllocWavFile(list);
	}
	list->next = new AudioFileList;
	memset(list->next, 0, sizeof(AudioFileList));
	list->next->fillMask = 1;
	return &list->next->files[0];
}
static void DeallocWavFile(AudioFileList* list, WavFile* file)
{
	const size_t idx = (file - &list->files[0]) / sizeof(WavFile);
	if (idx < NUM_AUDIO_FILES_IN_LIST)
	{
		if (list->fillMask & (1LL << idx))
		{
			list->fillMask &= ~(1LL << idx);
			list->files[idx].~WavFile();
			memset(&list->files[idx], 0, sizeof(WavFile));
		}
	}
	else if(list->next)
	{
		DeallocWavFile(list->next, file);
	}
}
static void CleanUpAllInList(AudioFileList* list)
{
	AudioFileList* child = list->next;
	while (child)
	{
		AudioFileList* cur = child;
		child = child->next;
		delete cur;
	}
	for (size_t i = 0; i < NUM_AUDIO_FILES_IN_LIST; i++)
	{
		if (list->fillMask & (1LL << i))
		{
			list->files[i].~WavFile();
		}
	}
	memset(list, 0, sizeof(AudioFileList));
}



struct AudioManager
{
	NFDriver* nfdriver = nullptr;
	float* tempBuffer = nullptr;
	AudioFileList stored;
	AudioPlaybackContext** active;
	AudioPlaybackContextList* tempStorage;
	int currentNumPlaying = 0;
	std::atomic_flag spinLock;
	int numConcurrent;
};

static void AU_Lock(AudioManager* manager)
{
	while(!manager->spinLock.test_and_set(std::memory_order_acquire)){ }
}
static void AU_Unlock(AudioManager* manager)
{
	manager->spinLock.clear(std::memory_order_release);
}

static bool AU_IsTempStorage(AudioManager* manager, AudioPlaybackContext* ctx)
{
	uintptr_t sz = (uintptr_t)ctx - (uintptr_t)manager->tempStorage;
	if (sz < manager->numConcurrent * sizeof(AudioPlaybackContext))
		return true;
	return false;
}

static void AudioStutterCallback(void* clientData)
{
}
static int AudioRenderCallback(void* clientData, float* frames, int numberOfFrames)
{
	AudioManager* manager = (AudioManager*)clientData;
	memset(frames, 0, sizeof(float) * numberOfFrames * 2);
	AU_Lock(manager);
	if (manager->currentNumPlaying > 0)
	{
		float max = 0.0f;
		for (int i = 0; i < manager->numConcurrent; i++)
		{
			AudioPlaybackContext* cur = manager->active[i];
			if (!cur) continue;
			if (cur->file.wav && cur->isPlaying)
			{
				if (!cur->isOgg)
				{
					const float* data = cur->file.wav->GetData();
					const int end = std::min(numberOfFrames, cur->remaining);
					for (int j = 0; j < end; j++)
					{
						frames[j * 2] += data[(j + cur->index) * 2] * cur->volume;
						frames[j * 2 + 1] += data[(j + cur->index) * 2 + 1] * cur->volume;
					}

					cur->index += end;
					cur->remaining -= end;
					if (cur->remaining <= 0)
					{
						if (cur->repeat)
						{
							cur->index = 0;
							cur->remaining = cur->file.wav->GetNumSamples();
						}
						else
						{
							cur->isPlaying = false;
							if (AU_IsTempStorage(manager, cur)) cur->inUse = false;
							manager->currentNumPlaying -= 1;
							manager->active[i] = nullptr;
						}
					}
				}
				else
				{
					stb_vorbis_seek(cur->file.ogg, cur->index);
					int added = stb_vorbis_get_samples_float_interleaved(cur->file.ogg, 2, manager->tempBuffer, numberOfFrames * 2);
					for (uint32_t i = 0; i < added * 2; i++)
					{
						frames[i] += manager->tempBuffer[i] * cur->volume;
					}
					cur->index += added;
					cur->remaining -= added;
					if (added == 0 || cur->remaining <= 0)
					{
						if (cur->repeat)
						{
							cur->index = 0;
							cur->remaining = cur->file.ogg->total_samples;
						}
						else
						{
							cur->isPlaying = false;
							if (AU_IsTempStorage(manager, cur)) cur->inUse = false;
							manager->currentNumPlaying -= 1;
							manager->active[i] = nullptr;
						}
					}
				}
			}
		}
		for (int i = 0; i < numberOfFrames * 2; i++)
		{
			if (std::abs(frames[i]) > max) max = std::abs(frames[i]);
		}
		if (max > 1.0f)
		{
			for (int i = 0; i < numberOfFrames * 2; i++)
			{
				frames[i] /= max;
			}
		}
	}
	AU_Unlock(manager);
	
	return numberOfFrames;
}
static void AudioErrorCallback(void* clientData, const char* errorMessage, int errorCode)
{

}
static void AudioWillRenderCallback(void* clientData)
{

}
static void AudioDidRenderCallback(void* clientData)
{

}

struct AudioManager* AU_CreateAudioManager(int numConcurrent)
{
	AudioManager* out = new AudioManager;
	NFDriver* driver = NFDriver::createNFDriver(out,
			AudioStutterCallback,AudioRenderCallback,
			AudioErrorCallback, AudioWillRenderCallback,
			AudioDidRenderCallback, OutputType::OutputTypeSoundCard);

	out->nfdriver = driver;
	out->currentNumPlaying = 0;
	memset(&out->stored, 0, sizeof(AudioFileList));


	out->active = new AudioPlaybackContext*[numConcurrent];
	out->tempStorage = AU_CreateAudioPlaybackContextList(numConcurrent);
	memset(out->active, 0, sizeof(AudioPlaybackContext*) * numConcurrent);
	out->numConcurrent = numConcurrent;

	out->tempBuffer = new float[NF_DRIVER_SAMPLE_BLOCK_SIZE * 2];

	out->nfdriver->setPlaying(true);
	return out;
}
void AU_ShutdownAudioManager(struct AudioManager* manager)
{
	manager->nfdriver->setPlaying(false);
	manager->currentNumPlaying = 0;
	CleanUpAllInList(&manager->stored);
	delete manager->nfdriver;
	manager->nfdriver = nullptr;
	delete manager;
	manager = nullptr;
}

struct WavFile* AU_LoadFile(struct AudioManager* manager, const char* file)
{
	WavFile* out = AllocWavFile(&manager->stored);
	if (!out->Load(file))
	{
		DeallocWavFile(&manager->stored, out);
		return nullptr;
	}
	return out;
}
struct OggFile* AU_LoadOggFile(struct AudioManager* manager, const char* file)
{
	int err = 0;
	stb_vorbis* out = stb_vorbis_open_filename(file, &err, nullptr);
	if (err)
	{
		return 0;
	}
	stb_vorbis_seek_frame(out, 0);	// loads the vorbis file data into memory
	return (struct OggFile*)out;
}
void AU_FreeFile(struct AudioManager* manager, struct WavFile* file)
{
	DeallocWavFile(&manager->stored, file);
}
void AU_FreeOggFile(struct AudioManager* manager, struct OggFile* file)
{
	stb_vorbis_close((stb_vorbis*)file);
}


struct AudioPlaybackContextList* AU_CreateAudioPlaybackContextList(int num)
{
	return (AudioPlaybackContextList*)new AudioPlaybackContext[num];
}
void AU_DestroyAudioPlaybackContextList(struct AudioPlaybackContextList* list)
{
	AudioPlaybackContext* l = (AudioPlaybackContext*)list;
	if (l) delete[] l;
}
struct AudioPlaybackContext* AU_AllocContext(AudioPlaybackContextList* list, int num)
{
	AudioPlaybackContext* result = nullptr;
	for (int i = 0; i < num; i++)
	{
		AudioPlaybackContext* cur = &((AudioPlaybackContext*)list)[i];
		if (!cur->inUse)
		{
			cur->file.wav = nullptr;
			cur->index = 0;
			cur->inUse = true;
			result = cur;
			cur->isPlaying = false;
			cur->isOgg = false;
			break;
		}
	}
	return result;
}
void AU_FreeContext(struct AudioManager* manager, AudioPlaybackContext* ctx)
{
	AU_Lock(manager);
	ctx->inUse = false;
	ctx->isPlaying = false;
	ctx->index = 0;
	AU_Unlock(manager);
}

static bool AU_InternalPlayAudio(struct AudioManager* manager, struct AudioPlaybackContext* ctx, void* file, float volume, bool isOgg, bool repeat)
{
	if (!file || manager->currentNumPlaying >= manager->numConcurrent) return false;
	AU_Lock(manager);
	bool found = false;

	for (int i = 0; i < manager->numConcurrent; i++)
	{
		if (!manager->active[i])
		{
			if (isOgg)
			{
				stb_vorbis* f = (stb_vorbis*)file;
				ctx->file.ogg = f;
				ctx->remaining = f->total_samples;
			}
			else
			{
				WavFile* f = (WavFile*)file;
				ctx->file.wav = f;
				ctx->remaining = f->GetNumSamples();
			}
			ctx->index = 0;
			ctx->volume = volume;
			ctx->repeat = repeat;
			ctx->isPlaying = true;
			ctx->isOgg = isOgg;

			found = true;
			manager->active[i] = ctx;
			manager->currentNumPlaying += 1;
			break;
		}
	}

	AU_Unlock(manager);

	return found;
}

bool AU_PlayAudio(struct AudioManager* manager, struct AudioPlaybackContext* ctx, struct WavFile* file, float volume)
{
	return AU_InternalPlayAudio(manager, ctx, file, volume, false, false);
}
bool AU_PlayOggAudio(struct AudioManager* manager, struct AudioPlaybackContext* ctx, struct OggFile* file, float volume)
{
	return AU_InternalPlayAudio(manager, ctx, file, volume, true, false);
}
bool AU_PlayAudioOnRepeat(struct AudioManager* manager, struct AudioPlaybackContext* ctx, struct WavFile* file, float volume)
{
	return AU_InternalPlayAudio(manager, ctx, file, volume, false, true);
}
bool AU_PlayOggAudioOnRepeat(struct AudioManager* manager, struct AudioPlaybackContext* ctx, struct OggFile* file, float volume)
{
	return AU_InternalPlayAudio(manager, ctx, file, volume, true, true);
}

AudioPlaybackContext* AU_PlayAudio(struct AudioManager* manager, WavFile* file, float volume)
{
	AudioPlaybackContext* out = AU_AllocContext(manager->tempStorage, manager->numConcurrent);
	if (!out) return nullptr;
	if (!AU_PlayAudio(manager, out, file, volume)) return nullptr;
	return out;
}
struct AudioPlaybackContext* AU_PlayOggAudio(struct AudioManager* manager, struct OggFile* file, float volume)
{
	AudioPlaybackContext* out = AU_AllocContext(manager->tempStorage, manager->numConcurrent);
	if (!out) return nullptr;
	if (!AU_PlayOggAudio(manager, out, file, volume)) return nullptr;
	return out;
}
struct AudioPlaybackContext* AU_PlayAudioOnRepeat(struct AudioManager* manager, WavFile* file, float volume)
{
	AudioPlaybackContext* out = AU_AllocContext(manager->tempStorage, manager->numConcurrent);
	if (!out) return nullptr;
	if (!AU_PlayAudioOnRepeat(manager, out, file, volume)) return nullptr;
	return out;
}
struct AudioPlaybackContext* AU_PlayOggAudioOnRepeat(struct AudioManager* manager, struct OggFile* file, float volume)
{
	AudioPlaybackContext* out = AU_AllocContext(manager->tempStorage, manager->numConcurrent);
	if (!out) return nullptr;
	if (!AU_PlayOggAudioOnRepeat(manager, out, file, volume)) return nullptr;
	return out;
}
void AU_StopAudio(struct AudioManager* manager, struct AudioPlaybackContext* audioCtx)
{
	AU_PauseAudio(manager, audioCtx);
	AU_FreeContext(manager, audioCtx);
}
void AU_PauseAudio(struct AudioManager* manager, struct AudioPlaybackContext* audioCtx)
{
	AU_Lock(manager);
	if (audioCtx->isPlaying)
	{
		for (int i = 0; i < manager->numConcurrent; i++)
		{
			if (manager->active[i] == audioCtx)
			{
				manager->active[i] = nullptr;
				manager->currentNumPlaying -= 1;
				break;
			}
		}
		audioCtx->isPlaying = false;
	}
	AU_Unlock(manager);
}
bool AU_ResumeAudio(struct AudioManager* manager, struct AudioPlaybackContext* audioCtx)
{
	AU_Lock(manager);
	bool succeeded = false;
	if (!audioCtx->isPlaying && manager->currentNumPlaying < manager->numConcurrent)
	{
		for (int i = 0; i < manager->numConcurrent; i++)
		{
			if (!manager->active[i])
			{
				manager->active[i] = audioCtx;
				audioCtx->isPlaying = true;
				manager->currentNumPlaying += 1;
				succeeded = true;
			}
		}
	}
	AU_Unlock(manager);
	return succeeded;
}

bool AU_IsPlaying(struct AudioPlaybackContext* ctx)
{
	if (!ctx) return false;
	return ctx->isPlaying;
}
int AU_GetSampleCount(struct AudioPlaybackContext* ctx)
{
	if (!ctx) return -1;
	if (ctx->file.wav)
	{
		if (ctx->isOgg)
		{
			return ctx->file.ogg->total_samples;
		}
		else
		{
			return ctx->file.wav->GetNumSamples();
		}
	}
	return -1;
}
void AU_SetSampleIndex(struct AudioManager* manager, struct AudioPlaybackContext* ctx, int index)
{
	int maxCount = AU_GetSampleCount(ctx);
	if (index >= 0 && index < maxCount)
	{
		AU_Lock(manager);
		ctx->remaining = (maxCount - index);
		ctx->index = index;
		AU_Unlock(manager);
	}
	
}
float AU_GetVolume(struct AudioPlaybackContext* ctx)
{
	if (!ctx) return 0.0f;
	return ctx->volume;
}
void AU_SetVolume(struct AudioPlaybackContext* ctx, float volume)
{
	if (!ctx) return;
	ctx->volume = volume;
}