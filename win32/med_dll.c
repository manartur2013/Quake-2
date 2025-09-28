#include "../client/client.h"
#include "winquake.h"

HINSTANCE media_library;
mexport_t me;
mstate_t media_state;

qboolean MED_LoadLibrary (void)
{
	mimport_t mi;
	GetMAPI_t GetMAPI;

	Com_Printf("------- Loading Media -------\n");

	if ((media_library = LoadLibrary("qmedia.dll")) == 0)
	{
		return false;
	}

	mi.Cmd_AddCommand = Cmd_AddCommand;
	mi.Cmd_Argc = Cmd_Argc;
	mi.Cmd_Argv = Cmd_Argv;
	mi.Cmd_ExecuteText = Cbuf_ExecuteText;

	mi.Com_Printf = Com_Printf;
	mi.Com_sprintf = Com_sprintf;
	mi.CenterPrint = SCR_CenterPrintNoEcho;
	mi.Q_strcasecmp = Q_strcasecmp;

	mi.Cvar_Get = Cvar_Get;
	mi.Cvar_Set = Cvar_Set;
	mi.Cvar_SetValue = Cvar_SetValue;
	mi.Cvar_VariableValue = Cvar_VariableValue;

	mi.CL_GetEntitySoundOrigin = CL_GetEntitySoundOrigin;
	mi.CL_ParticleEffect = CL_ParticleEffect;

	mi.FS_Gamedir = FS_Gamedir;

	if ( ( GetMAPI = (void*)GetProcAddress(media_library, "GetMAPI") ) == 0 )
	{
		Com_Error(ERR_FATAL, "GetProcAddress failed on media");
	}

	me = GetMAPI(mi);

	return true;
}

void MED_Init (void)
{
	if (!MED_LoadLibrary())
		Com_Error(ERR_FATAL, "Failed to load media module!");

	if ( me.Init() == -1 )
		Com_Error(ERR_FATAL, "Failed to initialize media module!");

	media_state.media_init = true;
}