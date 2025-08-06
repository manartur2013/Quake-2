// q_al.h - Quake 2 OpenAL implementation

#ifndef QAL_H
#define QAL_H

#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>

#define QAL_MAX_SOUND_BUFFERS		16
#define QAL_MAX_SOUND_SOURCES		1

extern cvar_t* cd_volume;

int QAL_Init( void );
void QAL_Update( void );
void QAL_Activate(qboolean active);
void QAL_Shutdown( void );

ALuint QAL_PlaySound( char* path, float pitch, float volume, float* position, int loop );
ALuint QAL_PlaySoundGlobal( char* path, float pitch, float volume, int loop );

void QAL_PauseSource(ALuint source);
void QAL_ResumeSource(ALuint source);
void QAL_FreeSource(ALuint source);
void QAL_FreeBuffer(ALuint buffer);

typedef struct
{
	ALCdevice* ALC_device;
	ALCcontext* ALC_context;

	qboolean qal_init;
	qboolean qal_active;
} qalstate_t;

extern qalstate_t qal_state;

typedef struct qal_sbuffer_s
{
	ALuint bufferID;
	qboolean free;
} qal_sbuffer_t;

typedef struct qal_ssource_s
{
	ALuint sourceID;
	ALuint buffernum;

	qboolean paused;
	qboolean free;
} qal_ssource_t;

qal_sbuffer_t qal_sbuffers[QAL_MAX_SOUND_BUFFERS];
qal_ssource_t qal_ssources[QAL_MAX_SOUND_SOURCES];

static unsigned int qal_sbuffers_num;
static unsigned int qal_ssources_num;

#endif
