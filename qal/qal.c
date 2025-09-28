#include "../qmedia/qmedia.h"
#include "qal.h"
#include "qal_sound.h"

qalstate_t qal_state;
cvar_t* cd_volume;

vec3_t vec3_origin = { 0,0,0 };

int QAL_Init (void)
{
	ALCdevice* pDevice;
	ALCcontext* pContext;
	const ALCchar* szName;

	mi.Com_Printf("QAL: Initializing...\n");

	pDevice = alcOpenDevice(NULL);
	if (!pDevice)
	{
		mi.Com_Printf ("Failed to open sound device\n");
		return -1;
	}

	pContext = alcCreateContext(pDevice, NULL);
	if (!pContext)
	{
		mi.Com_Printf ("Failed to set sound context\n");
		return -1;
	}

	if (!alcMakeContextCurrent(pContext))
	{
		mi.Com_Printf ("Failed to make context current\n");
		return -1;
	}

	mi.Com_Printf ("QAL initialized successfully\n");

	szName = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);

	if (szName)
		mi.Com_Printf ("Sound device is: %s\n", szName);

	alDistanceModel(AL_EXPONENT_DISTANCE);

	qal_state.qal_init = true;
	cd_volume = mi.Cvar_Get("cd_volume", "0.4", CVAR_ARCHIVE);
	
	mi.Cmd_AddCommand("cd", PlayTrack_f);
	mi.Cmd_AddCommand("stopsound", QAL_StopAllSounds);

	return 1;
}

ALuint QAL_CreateBuffer(char* path)
{
	ALuint sbuffer;
	SNDFILE* sf;
	SF_INFO sfi;
	ALenum err, fmt;
	short* buffdata;
	sf_count_t nframes;
	ALsizei nbytes;
	char sfPath[MAX_QPATH];

	mi.Com_sprintf(sfPath, sizeof(sfPath), "%s", path);

	sf = sf_open(sfPath, SFM_READ, &sfi);
	if (!sf)
	{
		mi.Com_Printf("QAL: SF_ERROR %s\n", sf_error_number(sf_error(sf)));
		return 0;
	}

	if (sfi.frames < 1 || sfi.frames >(sf_count_t)(INT_MAX / sizeof(short)) / sfi.channels)
	{
		mi.Com_Printf("QAL: Bad sample count in %s: %i\n", sfPath, sfi.frames);
		sf_close(sf);
		return 0;
	}

	fmt = AL_NONE;
	switch (sfi.channels)
	{
	case 1:
		fmt = AL_FORMAT_MONO16;
		break;
	case 2:
		fmt = AL_FORMAT_STEREO16;
		break;
	case 0:
	default:
		mi.Com_Printf("QAL: Unsupported channel count: %i\n", sfi.channels);
		sf_close(sf);
		return 0;
		break;
	}

	buffdata = malloc((size_t)(sfi.frames * sfi.channels) * sizeof(short));
	if (buffdata == 0)
	{
		mi.Com_Printf("QAL: Out of memory\n");
		return 0;
	}

	nframes = sf_readf_short(sf, buffdata, sfi.frames);
	if (nframes < 1)
	{
		free(buffdata);
		sf_close(sf);
		mi.Com_Printf("QAL: Failed to read samples in %s (%i)", sfPath, nframes);
		return 0;
	}

	nbytes = (ALsizei)(nframes * sfi.channels) * (ALsizei)sizeof(short);

	sbuffer = 0;
	alGenBuffers(1, &sbuffer);
	alBufferData(sbuffer, fmt, buffdata, nbytes, sfi.samplerate);

	free(buffdata);
	sf_close(sf);

	if ((err = alGetError()) != AL_NO_ERROR)
	{
		mi.Com_Printf("QAL: Error generating buffer %d\n", err);
		return 0;
	}

	return sbuffer;
}

void QAL_Activate(qboolean active)
{
	if (!qal_state.qal_init)
		return;

	qal_state.qal_active = active;

	sound_t *snd = s_head;

	while (snd != NULL)
	{
		if (active)
			alSourcePlay(snd->sourceID);
		else
			alSourcePause(snd->sourceID);

		snd = snd->next;
	};
}

void QAL_Update(float* pos, float* forward, float* up)
{
	if (!qal_state.qal_init)
		return;

	if (!qal_state.qal_active)
		return;

	if (s_head == NULL)
		return;

	char szNum[8];
	int i = 0;
	sound_t* snd = s_head;
	ALfloat *alPos, *alFwd, *alUp;

	alPos = QAL_QCoordsToAL(pos);
	alFwd = QAL_QCoordsToAL(forward);
	alUp = QAL_QCoordsToAL(up);

	ALfloat orientation[] = {
		alFwd[0],	alFwd[1],	alFwd[2],
		alUp[0],	alUp[1],	alUp[2]
	};

	alListener3f(AL_POSITION, pos[0], pos[1], pos[2]);
	alListenerfv(AL_ORIENTATION, orientation);

	while (snd != NULL)
	{
		ALint state;
		vec3_t srcPos;
		sound_t* sndnext = snd->next;

		alGetSourcei(snd->sourceID, AL_SOURCE_STATE, &state);

		if (state != AL_PLAYING)
		{
			S_FreeSound(snd);
			goto nextsound;
		}

		if (snd->soundType & SOUNDTYPE_CD)
			alSourcef(snd->sourceID, AL_GAIN, cd_volume->value);
		else
			alSourcef(snd->sourceID, AL_GAIN, mi.Cvar_VariableValue("s_volume") * snd->volume);

		if (!(snd->soundType & SOUNDTYPE_LOCAL))
		{
			if (snd->soundType & SOUNDTYPE_FIXED)
				alGetSource3f(snd->sourceID, AL_POSITION, &srcPos[0], &srcPos[1], &srcPos[2]);
			else if (snd->soundType & SOUNDTYPE_ENT)
			{
				ALfloat *entOrg;

				entOrg = QAL_GetEntOrigin(snd->ent);
				alSource3f(snd->sourceID, AL_POSITION, entOrg[0], entOrg[1], entOrg[2]);
			}
		}

		nextsound:
			snd = sndnext;

		i++;
	};

	mi.Com_sprintf(szNum, sizeof(szNum), "%i", i);
	mi.CenterPrint(szNum);
}

void QAL_StopAllSounds(void)
{
	sound_t *snd, *nextsnd;

	snd = s_head;

	while (snd != NULL)
	{
		nextsnd = snd->next;
		
		alSourceStop(snd->sourceID);
		S_FreeSound(snd);

		snd = nextsnd;
	};
}

void QAL_Shutdown(void)
{
	if (!qal_state.qal_init)
		return;

	QAL_StopAllSounds();

	alcMakeContextCurrent(NULL);
	alcDestroyContext(qal_state.ALC_context);
	alcCloseDevice(qal_state.ALC_device);

	qal_state.qal_init = false;
}

ALfloat* QAL_QCoordsToAL(float coords[3])
{
	ALfloat alcoords[3];

	alcoords[0] = coords[0];
	alcoords[1] = coords[2];
	alcoords[2] = -coords[1];

	return alcoords;
}

ALfloat* QAL_GetEntOrigin(int entnum)
{
	ALfloat origin[3];

	mi.CL_GetEntitySoundOrigin(entnum, origin);

	return QAL_QCoordsToAL(origin);
}