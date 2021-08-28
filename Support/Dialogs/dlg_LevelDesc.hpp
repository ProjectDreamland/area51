//==============================================================================
//  
//  dlg_LevelDesc.hpp
//  
//==============================================================================

#ifndef DLG_LEVEL_DESC_HPP
#define DLG_LEVEL_DESC_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_text.hpp"

//==============================================================================
//  dlg_level_desc
//==============================================================================

enum level_desc_mode
{
    LEVEL_DESC_INITIAL,
    LEVEL_DESC_INTERLEVEL,
};

extern void     dlg_level_desc_register  ( ui_manager* pManager );
extern ui_win*  dlg_level_desc_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_frame;

class dlg_level_desc : public ui_dialog
{
public:
                        dlg_level_desc      ( void );
    virtual            ~dlg_level_desc      ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );

    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

            void        LoadingComplete     ( void );

            void        ForceExit           ( void )                            { m_ForceExit = TRUE; }
            xbool       GetForceExit        ( void )                            { return m_ForceExit; }

            void        Configure           ( level_desc_mode Mode );
protected:
    xbool           m_LoadingComplete;
    xbool           m_ForceExit;
    level_desc_mode m_Mode;
    f32             m_LoadTimeElapsed;

    ui_frame*       m_pFrameOuter;
    ui_frame*       m_pFrameGameTypeDesc;
    ui_frame*       m_pFrameLevelDesc;

    ui_text*        m_MapTitleText;
    ui_text*        m_MapDescText;
    ui_text*        m_GameTypeText;
    ui_text*        m_GameDescText;

    ui_text*        m_pLoadingText;
    ui_text*        m_pLoadingPips;
    ui_text*        m_pNavText;
};

//==============================================================================
#endif // DLG_LEVEL_DESC_HPP
//==============================================================================
