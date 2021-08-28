//==============================================================================
//  
//  dlg_SubMenu.hpp
//  
//==============================================================================

#ifndef DLG_SUBMENU_HPP
#define DLG_SUBMENU_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"

enum
{
    DLG_SUBMESSAGE_IDLE,
    DLG_SUBMESSAGE_ONE,
    DLG_SUBMESSAGE_TWO,
    DLG_SUBMESSAGE_THREE,
    DLG_SUBMESSAGE_FOUR,
    DLG_SUBMESSAGE_FIVE,
    DLG_SUBMESSAGE_BACK,
};

//==============================================================================
//  dlg_submenu
//==============================================================================

extern void     dlg_submenu_register( ui_manager* pManager );
extern ui_win*  dlg_submenu_factory ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;
class ui_text;

class dlg_submenu : public ui_dialog
{
public:
                    dlg_submenu         ( void );
    virtual        ~dlg_submenu         ( void );

    void            Configure           ( const xwchar*             Title,
                                          const xwchar*             ItemOne,
                                          const xwchar*             ItemTwo,
                                          const xwchar*             ItemThree,
                                          const xwchar*             Message,
                                          s32*                      pResult = NULL );

    void            Configure           ( const xwchar*             Title,
                                          const xwchar*             ItemOne,
                                          const xwchar*             ItemTwo,
                                          const xwchar*             ItemThree,
                                          const xwchar*             ItemFour,
                                          const xwchar*             Message,
                                          s32*                      pResult = NULL );

    void            Configure           ( const xwchar*             Title,
                                          const xwchar*             ItemOne,
                                          const xwchar*             ItemTwo,
                                          const xwchar*             ItemThree,
                                          const xwchar*             ItemFour,
                                          const xwchar*             ItemFive,
                                          const xwchar*             Message,
                                          s32*                      pResult = NULL );


	void			Close				( void );

    xbool           Create              ( s32                       UserID,
                                          ui_manager*               pManager,
                                          ui_manager::dialog_tem*   pDialogTem,
                                          const irect&              Position,
                                          ui_win*                   pParent,
                                          s32                       Flags,
                                          void*                     pUserData );

    virtual void    Destroy             ( void );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );

    virtual void    OnPadSelect         ( ui_win* pWin );
    virtual void    OnPadBack           ( ui_win* pWin );
    virtual void    OnUpdate            ( ui_win* pWin, f32 DeltaTime);

protected:
    ui_button*      m_pButtonOne;
    ui_button*      m_pButtonTwo;
    ui_button*      m_pButtonThree;
    ui_button*      m_pButtonFour;
    ui_button*      m_pButtonFive;

    xwstring        m_Message;
    s32             *m_pResult;
    xbool           m_bDoBlackout;
    s32             m_iElement;
    s32             m_CurrHL;
};



//==============================================================================
#endif // DLG_SUBMENU_HPP
//==============================================================================
