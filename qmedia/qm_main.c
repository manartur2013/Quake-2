#include "qmedia.h"
#include "../qal/qal.h"
#include "../qal/qal_sound.h"

mimport_t mi;

int QM_Init(void)
{
	if ( QAL_Init() == -1 )
		return -1;
		
	return 0;
}

mexport_t GetMAPI(mimport_t qmimp)
{
	mexport_t me;

	mi = qmimp;

	me.Init = QM_Init;

	me.S_PlayTrack = S_PlayTrack;
	me.S_StopTrack = S_StopTrack;
	me.S_PlaySoundLocal = S_PlaySoundLocal;
	me.S_PlaySound = S_PlaySound;
	me.S_PlaySoundEntity = S_PlaySoundEntity;
	
	me.S_StopAllSounds = QAL_StopAllSounds;
	me.S_Shutdown = QAL_Shutdown;

	me.QAL_Activate = QAL_Activate;
	me.QAL_Update = QAL_Update;

	me.MED_BitmapToPNG = QM_BitmapToPNG;
	me.MED_BitmapToJPEG = QM_BitmapToJPEG;

	return me;
}