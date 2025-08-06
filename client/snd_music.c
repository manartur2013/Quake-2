#include "../client/client.h"
#include "snd_loc.h"

static qboolean playing;
static qboolean paused;
static ALuint	currentSource;

void S_PlayTrack(int track, qboolean looping)
{
	char trackPath[MAX_QPATH];
	float volume;

	volume = cd_volume->value;

	if (playing)
		S_StopTrack();

	if (track < 2 || track > 21)
	{
		S_StopTrack();
		return;
	}

	if (track < 10)
		Com_sprintf(trackPath, sizeof(trackPath), "baseq2/music/track0%i.ogg", track);
	else
		Com_sprintf(trackPath, sizeof(trackPath), "baseq2/music/track%i.ogg", track);

	currentSource = QAL_PlaySoundGlobal(trackPath, 1.0, volume, looping);

	if (qal_ssources[currentSource].sourceID == 0)
		return;

	playing = true;
}

void S_StopTrack(void)
{
	if (!playing)
		return;

	alSourceStop(qal_ssources[currentSource].sourceID);
	QAL_FreeSource(currentSource);

	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{
		Com_Printf("Error %d\n", err);
	}
	playing = false;
}

void S_PauseTrack(void)
{
	if (paused)
		return;

	QAL_PauseSource(qal_ssources[currentSource].sourceID);
	qal_ssources[currentSource].paused = true;
	paused = true;
}

void S_ResumeTrack(void)
{
	if (!paused)
		return;

	QAL_ResumeSource(qal_ssources[currentSource].sourceID);
	qal_ssources[currentSource].paused = false;
	paused = false;
}

void PlayTrack_f(void)
{
	char* command;

	if (Cmd_Argc() < 2)
		return;

	command = Cmd_Argv(1);

	if (Q_strcasecmp(command, "play") == 0)
	{
		S_PlayTrack(atoi(Cmd_Argv(2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		S_PlayTrack(atoi(Cmd_Argv(2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		S_StopTrack();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		S_PauseTrack();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		S_ResumeTrack();
		return;
	}
}