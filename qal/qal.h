// q_al.h - Quake 2 OpenAL implementation
#define QAL

#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>

extern cvar_t* cd_volume;

int QAL_Init( void );
void QAL_Update( float* pos, float* forward, float* up);
void QAL_Activate(qboolean active);
void QAL_StopAllSounds(void);
void QAL_Shutdown( void );

ALfloat* QAL_QCoordsToAL(float coords[3]);
ALfloat* QAL_GetEntOrigin(int entnum);

ALuint QAL_CreateBuffer(char* path);

typedef struct
{
	ALCdevice* ALC_device;
	ALCcontext* ALC_context;

	qboolean qal_init;
	qboolean qal_active;
} qalstate_t;

extern qalstate_t qal_state;
