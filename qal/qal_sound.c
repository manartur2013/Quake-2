#include "../qmedia/qmedia.h"
#include "qal_sound.h"

sound_t *S_AllocSound(void)
{
	sound_t *snd;

	snd = (sound_t *)malloc(sizeof(sound_t));

	snd->next = NULL;
	snd->prev = NULL;

	return snd;
}

void S_FreeSound(sound_t *snd)
{
	if (snd->prev == NULL && snd->next != NULL)
		s_head = snd->next;
	
	if (snd->prev != NULL && snd->next == NULL)
		s_tail = snd->prev;

	if (snd->next != NULL)
		snd->next->prev = snd->prev;

	if (snd->prev != NULL)
		snd->prev->next = snd->next;
	
	if (snd->prev == NULL && snd->next == NULL)
		s_head = s_tail = NULL;

	ALuint bufID;
	
	alGetSourcei(snd->sourceID, AL_BUFFER, &bufID);
	alDeleteSources(1, &snd->sourceID);
	alDeleteBuffers(1, &bufID);

	free(snd);
}

sound_t *QAL_PlaySound(char* path, float pitch, float volume, float* position, float attenuation, int loop)
{
	char sPath[MAX_QPATH];
	sound_t *snd = NULL;
	ALuint bufID, srcID;
	ALfloat *alPos;
	ALenum err;

	mi.Com_sprintf(sPath, sizeof(sPath), "%s/%s", mi.FS_Gamedir(), path);

//	mi.Com_Printf("Attempting to play %s, numsounds is %i\n", sPath, s_numsounds);

	if ( (bufID = QAL_CreateBuffer(sPath)) == 0 )
	{
		return NULL;
	}

	alGenSources(1, &srcID);

	if ( srcID == 0 )
	{
		return NULL;
	}

	if ((snd = S_AllocSound()) == NULL)
	{
		return NULL;
	}

	if (s_tail == NULL)
		s_head = s_tail = snd;
	else
	{
		s_tail->next = snd;
		snd->prev = s_tail;
		s_tail = snd;
	}
	snd->sourceID = srcID;
	snd->volume = volume;

	alGetError();

	alSourcef(srcID, AL_PITCH, pitch);
	alSourcef(srcID, AL_GAIN, volume * volume);	// if you don't set this up prematurely, you'll experience volume spikes. oh well...

	if (attenuation)
	{
		alSourcef(srcID, AL_REFERENCE_DISTANCE, 1.0);
		alSourcef(srcID, AL_MAX_DISTANCE, 512.0);
		alSourcef(srcID, AL_ROLLOFF_FACTOR, attenuation);
	}

	alPos = QAL_QCoordsToAL(position);

	alSource3f(srcID, AL_POSITION, alPos[0], alPos[1], alPos[2]);
	alSourcei(srcID, AL_LOOPING, loop);
	alSourcei(srcID, AL_BUFFER, bufID);

	alSourcePlay(srcID);
	
	if ( (err = alGetError()) != AL_NO_ERROR)
	{
		mi.Com_Printf("QAL: Error playing sound %d\n", err);

		S_FreeSound(snd);
		return NULL;
	}

	mi.Com_Printf("Playing %s\n", path);

	return snd;
}

sound_t *QAL_PlaySoundLocal(char* path, float pitch, float volume, int loop)
{
	float pos[3];

	pos[0] = pos[1] = pos[2] = 0.0;
	return QAL_PlaySound(path, pitch, volume, pos, 0.0, loop);
}

void S_PlaySound(char* path, float pitch, float volume, float* position, float attenuation, int loop)
{
	sound_t *snd;
	
	if ( ( snd = QAL_PlaySound(path, pitch, volume, position, attenuation, loop) ) == NULL )
		return;

	snd->soundType |= SOUNDTYPE_FIXED;
}

void S_PlaySoundLocal(char* path)
{
	char sPath[MAX_QPATH];
	sound_t* snd;

	mi.Com_sprintf(sPath, sizeof(sPath), "sound/%s", path);

	if ( ( snd = QAL_PlaySoundLocal(sPath, 1.0, mi.Cvar_VariableValue("s_volume"), false) ) == NULL )
		return;

	snd->soundType |= SOUNDTYPE_GENERIC;
	snd->soundType |= SOUNDTYPE_LOCAL;

	alSourcef(snd->sourceID, AL_ROLLOFF_FACTOR, 0.0);
	alSourcef(snd->sourceID, AL_REFERENCE_DISTANCE, 0.0);
	alSourcei(snd->sourceID, AL_SOURCE_RELATIVE, AL_TRUE);
}

void S_PlaySoundEntity(char* path, float pitch, float volume, float attenuation, int entnum)
{
	sound_t* snd;
	float pos[3];

	pos[0] = pos[1] = pos[2] = 0.0;

	if ((snd = QAL_PlaySound(path, pitch, volume, pos, attenuation, false)) == NULL)
		return;

	snd->ent = entnum;
	snd->soundType = SOUNDTYPE_ENT;
}