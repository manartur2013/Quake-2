#include "../qmedia/qmedia.h"
#include "qal.h"
#include "qal_sound.h"

static qboolean playing;
static qboolean paused;
sound_t *currentSound;

void S_PlayTrack(int track, qboolean looping)
{
	char trackPath[MAX_QPATH];
	float volume;
	sound_t *snd = NULL;

	volume = cd_volume->value;

	if (playing)
		S_StopTrack();

	if (track < 2 || track > 21)
	{
		S_StopTrack();
		return;
	}

	if (track < 10)
		mi.Com_sprintf(trackPath, sizeof(trackPath), "music/track0%i.ogg", track);
	else
		mi.Com_sprintf(trackPath, sizeof(trackPath), "music/track%i.ogg", track);

	if ( ( snd = QAL_PlaySoundLocal(trackPath, 1.0, volume, looping) ) == NULL )
		return;
	
	snd->soundType = SOUNDTYPE_CD;
	currentSound = snd;
	playing = true;
}

void S_StopTrack(void)
{
	if (!playing)
		return;

	alSourceStop(currentSound->sourceID);
	S_FreeSound(currentSound);

	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{
		mi.Com_Printf("Error %d\n", err);
	}
	playing = false;
}

void S_PauseTrack(void)
{
	if (paused)
		return;
	
	alSourcePause(currentSound->sourceID);
	paused = true;
}

void S_ResumeTrack(void)
{
	if (!paused)
		return;

	alSourcePlay(currentSound->sourceID);
	paused = false;
}

void PlayTrack_f(void)
{
	char* command;

	if (mi.Cmd_Argc() < 2)
		return;

	command = mi.Cmd_Argv(1);

	if (mi.Q_strcasecmp(command, "play") == 0)
	{
		S_PlayTrack(atoi(mi.Cmd_Argv(2)), false);
		return;
	}

	if (mi.Q_strcasecmp(command, "loop") == 0)
	{
		S_PlayTrack(atoi(mi.Cmd_Argv(2)), true);
		return;
	}

	if (mi.Q_strcasecmp(command, "stop") == 0)
	{
		S_StopTrack();
		return;
	}

	if (mi.Q_strcasecmp(command, "pause") == 0)
	{
		S_PauseTrack();
		return;
	}

	if (mi.Q_strcasecmp(command, "resume") == 0)
	{
		S_ResumeTrack();
		return;
	}
}