#ifndef QAL
#include "qal.h"
#endif

#define SOUNDTYPE_GENERIC	0x01
#define SOUNDTYPE_CD		0x02
#define SOUNDTYPE_MENU		0x04
#define SOUNDTYPE_LOCAL		0x08
#define SOUNDTYPE_FIXED		0x10	// sound emitting from a fixed origin
#define SOUNDTYPE_ENT		0x20	// sound is being emitted from an entity

typedef struct sound_s
{
	struct sound_s *prev, *next;

	ALuint sourceID;
	int	ent;
	int soundType;

	float attenuation;
	float volume;
} sound_t;

sound_t *s_head, *s_tail;

void S_FreeSound(sound_t* snd);
void S_PlaySound(char* path, float pitch, float volume, float* position, float attenuation, int loop);
void S_PlaySoundLocal(char* path);
void S_PlaySoundEntity(char* path, float pitch, float volume, float attenuation, int entnum);

void S_PlayTrack(int track, qboolean looping);
void S_StopTrack(void);
void PlayTrack_f(void);

sound_t* QAL_PlaySound(char* path, float pitch, float volume, float* position, float attenuation, int loop);
sound_t* QAL_PlaySoundLocal(char* path, float pitch, float volume, int loop);
