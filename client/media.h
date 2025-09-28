typedef struct
{
	int		(*Init)	(void);

	void	(*S_PlayTrack) (int track, qboolean looping);
	void	(*S_StopTrack) (void);
	void	(*S_PlaySoundLocal) (char* path);
	void	(*S_PlaySound) (char* path, float pitch, float volume, float* position, float attenuation, int loop);
	void	(*S_PlaySoundEntity) (char* path, float pitch, float volume, float attenuation, int entnum);
	
	void	(*S_StopAllSounds) (void);
	void	(*S_Shutdown) (void);

	void	(*QAL_Activate) (qboolean active);
	void	(*QAL_Update) (float* pos, float* forward, float* up);
} mexport_t;

typedef struct
{
	void	(*Sys_Error) (int err_level, char* str, ...);

	void	(*Cmd_AddCommand) (char* name, void(*cmd)(void));
	void	(*Cmd_RemoveCommand) (char* name);
	int		(*Cmd_Argc) (void);
	char*	(*Cmd_Argv) (int i);
	void	(*Cmd_ExecuteText) (int exec_when, char* text);

	void	(*Com_Printf) (char *fmt, ...);
	void	(*Com_sprintf) (char* dest, int size, char* fmt, ...);
	void	(*CenterPrint) (char* str);

	int		(*Q_strcasecmp) (char* s1, char* s2);

	int		(*FS_LoadFile) (char* name, void** buf);
	void	(*FS_FreeFile) (void* buf);

	char*	(*FS_Gamedir) (void);

	cvar_t* (*Cvar_Get) (char* name, char* value, int flags);
	cvar_t* (*Cvar_Set) (char* name, char* value);
	void	(*Cvar_SetValue) (char* name, float value);
	float	(*Cvar_VariableValue) (char* var_name);

	void	(*CL_GetEntitySoundOrigin) (int ent, vec3_t org);
	void	(*CL_ParticleEffect) (vec3_t org, vec3_t dir, int color, int count);
} mimport_t;

typedef struct
{
	qboolean media_init;
	qboolean sound_init;
} mstate_t;

extern mimport_t mi;
extern mstate_t media_state;

typedef mexport_t (*GetMAPI_t) (mimport_t);