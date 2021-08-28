//==============================================================================
// AUDIODEFINES.H
//==============================================================================

#ifndef AUDIO_DEFINES_H
#define AUDIO_DEFINES_H


//==============================================================================

#define		WM_USER_MSG_OPEN_SOUND_FILE         WM_USER+853
#define     WM_USER_MSG_CLEAR_ALL_SOUND_FILE    WM_USER+855
#define     WM_USER_MSG_UPDATE_OFFSET           WM_USER+859
#define     WM_USER_MSG_CHECK_OFFSET            WM_USER+861   

//==============================================================================
// MESSAGES SENT TO THE VIEWS.
//==============================================================================

#define     NEW_AUDIO_PACKAGE       1
#define     NEW_AUDIO_DESCRITPOR    2   
#define     NEW_AUDIO_ELEMENT       3
#define     REF_AUDIO_DESCRITPOR    4
#define     REFRESH_SAMPLE_VIEW     5
#define     GET_AUDIO_SOURCE_PATH   6
#define     SET_AUDIO_SOURCE_PATH   7
#define     ELEMENT_SEL_CHANGE      8
#define     ADD_AUDIO_ELEMENT       9
#define     SAMPLE_TO_DESCRIPTOR    10
#define     CREATE_DESC_FROM_SAMPLE 11
#define     OPEN_AUDIO_PROJECT      12
#define     CLOSE_AUDIO_PROJECT     13  
#define     NEW_AUDIO_PROJECT       14
#define     GET_SOUND_UNITS         15
#define     REFRESH_INTENSITY       16
#define     SORT_DESCENDING         17
#define     AUDIO_EDITOR_COPY       18
#define     AUDIO_EDITOR_PASTE      19

//==============================================================================
// DIFFRENT FOLDER TYPES.
//==============================================================================

#define     FOLDER      (1<<13)
#define     PACKAGE     (1<<14)
#define     DESCRIPTOR  (1<<15)
#define     ELEMENT     (1<<16)

//==============================================================================
// STRUCTURES
//==============================================================================

struct tree_item
{
    HTREEITEM           m_hItem;        
    DWORD               m_Data;
};

//------------------------------------------------------------------------------

struct tree_structure_info
{
    char     cPath[MAX_PATH];
    char     cWildcard[MAX_PATH];
    char     cForcedExt[MAX_PATH];
};

//------------------------------------------------------------------------------

struct fname
{
    char m_PathName[128];
    fname() { m_PathName[0] = 0; }
};

//==============================================================================

#endif