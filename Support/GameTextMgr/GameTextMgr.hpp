//=========================================================================
// GameTextMgr
//=========================================================================
#ifndef GAME_TEXT_MGR
#define GAME_TEXT_MGR

//=========================================================================
// INCLUDE
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// DEFINES
//=========================================================================
#define AUDIO_WARMING   (1<<1)
#define AUDIO_STARTING  (1<<2)
#define AUDIO_PLAYING   (1<<3)
#define AUDIO_RELEASING (1<<4)
#define AUDIO_FINISHED  (1<<5)

//=========================================================================
// GAME TEXT MGR
//=========================================================================
class game_text_mgr
{
public:    
    
    void    DisplayMessage  ( const char* pTableName, const char* pTitleString );
    void    DisplayMessage  ( const char* pTableName, const char* pTitleString, irect& RenderBox, 
                              xcolor Color = XCOLOR_WHITE );

    void    Init          ( void );
    void    Update        ( f32 DeltaTime );
    void    Kill          ( void );
    
    struct message_container
    {
        xwchar* pMainString;
        xwchar* pSubTitleString;
        xwchar* pSoundDescString;

        s32     SoundID;
        guid    Guid;
        u8      State;
    };

private:
    void    AddGoal             ( s32& GoalSeq, xwstring& pMessage, s32 HudIndex, f32 Time = -1.0f);
    void    ClearGoal           ( s32& GoalSeq, s32 HudIndex );

    message_container   m_Message;
    u32                 m_Flags;
};

extern game_text_mgr g_GameTextMgr;

//=========================================================================
// END
//=========================================================================
#endif