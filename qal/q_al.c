#include "../client/client.h"

qalstate_t qal_state;
cvar_t* cd_volume;

int QAL_Init (void)
{
	ALCdevice* pDevice;
	ALCcontext* pContext;
	const ALCchar* szName;

	Com_Printf("QAL: Initializing...\n");

	pDevice = alcOpenDevice(NULL);
	if (!pDevice)
	{
		Com_Printf ("Failed to open sound device\n");
		return -1;
	}

	pContext = alcCreateContext(pDevice, NULL);
	if (!pContext)
	{
		Com_Printf ("Failed to set sound context\n");
		return -1;
	}

	if (!alcMakeContextCurrent(pContext))
	{
		Com_Printf ("Failed to make context current\n");
		return -1;
	}

	for (int i = 0; i < QAL_MAX_SOUND_BUFFERS; i++)
	{
		qal_sbuffers[i].free = true;
	}

	for (int i = 0; i < QAL_MAX_SOUND_SOURCES; i++)
	{
		qal_ssources[i].free = true;
	}

	Com_Printf ("QAL initialized successfully\n");

	szName = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);

	if (szName)
		Com_Printf ("Sound device is: %s\n", szName);

	qal_state.qal_init = 1;

	cd_volume = Cvar_Get("cd_volume", "0.4", CVAR_ARCHIVE);

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

	if (qal_sbuffers_num >= QAL_MAX_SOUND_BUFFERS)
	{
		Com_Printf("QAL: Maximum sound buffers\n");
		return 0;
	}

	Com_sprintf(sfPath, sizeof(sfPath), "%s", path);

	sf = sf_open(sfPath, SFM_READ, &sfi);
	if (!sf)
	{
		Com_Printf("QAL: SF_ERROR %s\n", sf_error_number(sf_error(sf)));
		return 0;
	}

	if (sfi.frames < 1 || sfi.frames >(sf_count_t)(INT_MAX / sizeof(short)) / sfi.channels)
	{
		Com_Printf("QAL: Bad sample count in %s: %i\n", sfPath, sfi.frames);
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
		Com_Printf("QAL: Unsupported channel count: %i\n", sfi.channels);
		sf_close(sf);
		return 0;
		break;
	}

	buffdata = malloc((size_t)(sfi.frames * sfi.channels) * sizeof(short));
	if (buffdata == 0)
	{
		Com_Printf("QAL: Out of memory\n");
		return 0;
	}

	nframes = sf_readf_short(sf, buffdata, sfi.frames);
	if (nframes < 1)
	{
		free(buffdata);
		sf_close(sf);
		Com_Printf("QAL: Failed to read samples in %s (%i)", sfPath, nframes);
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
		Com_Printf("QAL: Error generating buffer %d\n", err);
		return 0;
	}

	int sbnum = 0;
	qal_sbuffers_num++;

	for (int i = 0; i < QAL_MAX_SOUND_BUFFERS; i++)
	{
		if (qal_sbuffers[i].free)
		{
			qal_sbuffers[i].bufferID = sbuffer;
			qal_sbuffers[i].free = false;

			sbnum = i;
		}
	}

	return sbnum;
}

ALuint QAL_PlaySound( char* path, float pitch, float volume, float* position, int loop )
{
	ALuint sbuffer, ssource;
	ALenum err;

	if (!qal_state.qal_init)
	{
		Com_Printf("QAL_PlaySound: QAL is not initialized!");
		return 0;
	}

	if (!qal_state.qal_active)
	{
		Com_Printf("QAL_PlaySound: QAL isn't active!");
		return 0;
	}

	if (qal_ssources_num >= QAL_MAX_SOUND_SOURCES)
	{
		Com_Printf("QAL: Maximum sound sources!");
		return 0;
	}

	alGetError();	// clear errors

	 ALuint sbnum = QAL_CreateBuffer(path);

	 sbuffer = qal_sbuffers[sbnum].bufferID;

	 if (!sbuffer)
	 {
		 Com_Printf("QAL: Bad buffer ID\n");
		 return 0;
	 }

	alGenSources(1, &ssource);
	alSourcef(ssource, AL_PITCH, pitch);
	alSourcef(ssource, AL_GAIN, volume);
	alSource3f(ssource, AL_POSITION, position[0], position[1], position[2]);
	alSource3f(ssource, AL_VELOCITY, 0.0, 0.0, 0.0);
	alSourcei(ssource, AL_LOOPING, loop);
	alSourcei(ssource, AL_BUFFER, sbuffer);

	alSourcePlay(ssource);

	err = alGetError();
	if (err != AL_NO_ERROR)
	{
		Com_Printf("QAL: Error playing sound %d\n", err);
		return 0;
	}

	Com_Printf("Playing %s\n", path);

	int ssnum = 0;
	qal_ssources_num++;

	for (int i = 0; i < QAL_MAX_SOUND_SOURCES; i++)
	{
		if (qal_ssources[i].free)
		{
			qal_ssources[i].sourceID = ssource;
			qal_ssources[i].buffernum = sbnum;
			qal_ssources[i].free = false;

			ssnum = i;
		}
	}

	return ssnum;
}

ALuint QAL_PlaySoundGlobal(char* path, float pitch, float volume, int loop)
{
	float pos[3];

	pos[0] = pos[1] = pos[2] = 0.0;
	return QAL_PlaySound(path, pitch, volume, pos, loop);
}

void QAL_FreeSource(ALuint source)
{
	if (!qal_ssources_num)
		return;

	alDeleteSources(1, &qal_ssources[source].sourceID);

	QAL_FreeBuffer(qal_ssources[source].buffernum);

	qal_ssources[source].sourceID = 0;
	qal_ssources[source].free = true;

	qal_ssources_num--;
}

void QAL_FreeBuffer(ALuint buffer)
{
	if (!qal_sbuffers_num)
		return;

	alDeleteBuffers(1, &qal_sbuffers[buffer].bufferID);

	qal_sbuffers[buffer].bufferID = 0;
	qal_sbuffers[buffer].free = true;

	qal_sbuffers_num--;
}

void QAL_PauseSource(ALuint source)
{ 
	ALenum err;
	
	alGetError();
	alSourcePause(source);

	if ((err = alGetError()) != AL_NO_ERROR)
	{
		Com_Printf("QAL: Error pausing source %d\n", err);
	}
}

void QAL_ResumeSource(ALuint source)
{
	ALenum err;

	alGetError();
	alSourcePlay(source);

	if ((err = alGetError()) != AL_NO_ERROR)
	{
		Com_Printf("QAL: Error resuming source %d\n", err);
	}
}

void QAL_Activate(qboolean active)
{
	for (int i = 0; i < qal_ssources_num; i++)
	{
		if (qal_ssources[i].free)
			continue;

		if (qal_ssources[i].paused)
			continue;

		if (active)
		{
			Com_Printf("Continue playing...\n");
			alSourcePlay(qal_ssources[i].sourceID);
		}
		else
			alSourcePause(qal_ssources[i].sourceID);
	}

	qal_state.qal_active = active;
}

void QAL_Update( void )
{
	if (!qal_state.qal_init)
		return;

	if (!qal_state.qal_active)
		return;

	if (!qal_ssources_num)
		return;

	for (int i = 0; i < qal_ssources_num; i++)
	{
		if (qal_ssources[i].free)
			continue;

		if (cd_volume->modified)
			alSourcef(qal_ssources[i].sourceID, AL_GAIN, cd_volume->value);
	}
}

void QAL_Shutdown(void)
{
	if (!qal_state.qal_init)
		return;

	for (int i = 0; i < qal_ssources_num; i++)
	{
		if (qal_ssources[i].free)
			continue;

		alSourceStop(qal_ssources[i].sourceID);
	}

	alcMakeContextCurrent(NULL);
	alcDestroyContext(qal_state.ALC_context);
	alcCloseDevice(qal_state.ALC_device);
}