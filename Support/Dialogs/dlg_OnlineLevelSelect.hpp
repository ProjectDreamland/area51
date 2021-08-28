//==============================================================================
//  
//  dlg_OnlineLevelSelect.hpp
//  
//==============================================================================

#ifndef DLG_ONLINE_LEVEL_SELECT_HPP
#define DLG_ONLINE_LEVEL_SELECT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_button.hpp"

#include "dlg_PopUp.hpp"
#include "StateMgr/maplist.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum map_select_type
{
    MAP_SELECT_MP,
    MAP_SELECT_ONLINE,
    MAP_SELECT_ONLINE_PAUSE,
};

enum map_fetch_state
{
    FETCH_IDLE,
    FETCH_START,
    FETCH_FINISH,
    FETCH_CHECK_CARDS,
    FETCH_WAIT_CHECK_CARDS,
    FETCH_WAIT_SET_DIRECTORY,
    FETCH_WAIT_ACQUIRE_MANIFEST,
};

//==============================================================================
//  dlg_online_level_select
//==============================================================================

extern void     dlg_online_level_select_register  ( ui_manager* pManager );
extern ui_win*  dlg_online_level_select_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;
struct map_settings;

class dlg_online_level_select : public ui_dialog
{
public:
                        dlg_online_level_select ( void );
    virtual            ~dlg_online_level_select ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnNotify                ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnPadActivate           ( ui_win* pWin );
    virtual void        OnPadDelete             ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );
    
    void                FillLevelList           ( void );
    void                FillMapCycleList        ( void );

    void                Configure               ( map_select_type Type );
    void                EnableBlackout          ( void )                    { m_bRenderBlackout = TRUE; }

    void                OnSaveSettingsCB        ( void );
    map_settings&       GetMapSettings          ( void );

protected:
    void                LaunchServer            ( void );

    ui_frame*           m_pFrame1;
    ui_listbox*         m_pLevelList;
    ui_listbox*         m_pLevelCycle;
    ui_button*          m_pLaunchButton;
    ui_text*            m_pNavText;
    s32                 m_ManifestCount[2];

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;
    map_fetch_state     m_ManifestState;
    s32                 m_CardIndex;

    s32                 m_CurrHL;
    xbool               m_bRenderBlackout;
    xbool               m_bInGame;
    map_select_type     m_Type;
    void*               m_pFileData;

};

//==============================================================================
#endif // DLG_ONLINE_LEVEL_SELECT_HPP
//==============================================================================
