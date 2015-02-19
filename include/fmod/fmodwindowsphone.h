/* ======================================================================================================== */
/* FMOD Windows Phone Specific header file. Copyright (c), Firelight Technologies Pty, Ltd. 2005-2014.      */
/* ======================================================================================================== */

#ifndef _FMODWINDOWSPHONES_H
#define _FMODWINDOWSPHONES_H


#include "fmod.h"

typedef enum FMOD_WINDOWSPHONE_STREAMTYPE
{
    FMOD_WINDOWSPHONE_STREAMTYPE_OTHER = 0,          /* Default stream category */
    FMOD_WINDOWSPHONE_STREAMTYPE_FOREGROUNDONLY,     /* Sounds designed to work in foreground only, mutes existing background audio */
    FMOD_WINDOWSPHONE_STREAMTYPE_BACKGROUND,         /* Audio will continue to play when application is in the background */
    FMOD_WINDOWSPHONE_STREAMTYPE_COMMUNICATIONS,     /* VOIP or other voice chat technology, can continue in the background */
    FMOD_WINDOWSPHONE_STREAMTYPE_ALERT,              /* Looping or longer running alert sounds */
    FMOD_WINDOWSPHONE_STREAMTYPE_GAMEMEDIA,          /* Background music played by a game */
    FMOD_WINDOWSPHONE_STREAMTYPE_GAMEEFFECTS,        /* Game sound effects designed to mix with existing audio */
    FMOD_WINDOWSPHONE_STREAMTYPE_SOUNDEFFECTS,       /* Sound effects designed to mix with existing audio  */
    FMOD_WINDOWSPHONE_STREAMTYPE_FORCEINT = 65536,   
};

/*
[STRUCTURE] 
[
    [DESCRIPTION]
    Use this structure with System::init to set the information required for Windows Phone
    initialisation.

    Pass this structure in as the "extradriverdata" parameter in System::init.

    [REMARKS]
    

    [PLATFORMS]
    Windows Phone

    [SEE_ALSO]
    System::init
]
*/
typedef struct FMOD_WINDOWSPHONE_EXTRADRIVERDATA
{
    FMOD_WINDOWSPHONE_STREAMTYPE         stream_type;        /* Controls audio interaction with other applications and the system */
} FMOD_WINDOWSPHONE_EXTRADRIVERDATA;

/*
[
	[DESCRIPTION]
    Pause the mixer and output device.

	[PARAMETERS]
    'paused'    1 to pause the system, 0 to unpause.
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]
    This should be called when you application is placed in the background to avoid consuming CPU.

    [PLATFORMS]
    Windows Phone

	[SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_WindowsPhone_PauseSystem(FMOD_SYSTEM *system, FMOD_BOOL paused);

/*
[
	[DESCRIPTION]
    Return if the system is paused.

	[PARAMETERS]
    'paused'    Pointer to receive pause state: 1 for paused, 0 for unpaused.
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Windows Phone

	[SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_WindowsPhone_IsSystemPaused(FMOD_SYSTEM *system, FMOD_BOOL *paused);

#endif
