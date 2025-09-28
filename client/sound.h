void MED_Init (void);
void MED_Shutdown (void);

// TEMP function to test music logic
void S_PlayTrack(int track, qboolean looping);
void S_StopTrack(void);

void S_Init(void);
void S_Update(vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up);
void S_Activate(qboolean active);
void S_Shutdown(void);

void S_StopAllSounds(void);
char* S_RegisterSound(char* name);

void S_StartSound(vec3_t origin, int entnum, int entchannel, char* path, float fvol, float attenuation, float timeofs);
void S_StartLocalSound(char* sound);

void CL_GetEntitySoundOrigin(int ent, vec3_t org);
