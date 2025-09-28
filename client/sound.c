#include "client.h"

cvar_t* s_volume;

void S_PlayTrack(int track, qboolean looping)
{
	me.S_PlayTrack(track, looping);
}

void S_StopTrack(void)
{
	me.S_StopTrack();
}

void S_Init(void)
{
	if (!media_state.sound_init)
	{
		s_volume = Cvar_Get("s_volume", "0.7", CVAR_ARCHIVE);

		media_state.sound_init = true;
	}

	S_StopAllSounds();
}

void S_Update(vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up)
{
	me.QAL_Update(origin, v_forward, v_up);
}

void S_Activate(qboolean active)
{
	if (!media_state.media_init)
		return;

	me.QAL_Activate(active);
}

void S_Shutdown(void)
{
	media_state.sound_init = false;
	me.S_Shutdown;
}

void S_StopAllSounds(void)
{
	me.S_StopAllSounds;
}

void S_StartLocalSound(char* sound)
{
	me.S_PlaySoundLocal(sound);
}

void S_RawSamples(int samples, int rate, int width, int channels, byte* data)
{
}

void S_StartSound(vec3_t origin, int entnum, int entchannel, char* path, float fvol, float attenuation, float timeofs)
{
	char sPath[MAX_QPATH];
	float pos[3], attn = .1;

	Com_sprintf(sPath, sizeof(sPath), "sound/%s", path);

	if (attenuation != ATTN_STATIC)
		attn /= 2;

	if (origin)
	{
		pos[0] = origin[0];
		pos[1] = origin[1];
		pos[2] = origin[2];
		me.S_PlaySound(sPath, 1.0, fvol, pos, attn, false);
	}
	else me.S_PlaySoundEntity(sPath, 1.0, fvol, attn, entnum);
}

char* S_RegisterSound(char* name)
{
	return name;
}