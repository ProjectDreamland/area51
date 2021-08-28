//==============================================================================
//
//  dlg_Credits.cpp
//
//  Copyright (c) 2005 Midway West All rights reserved.
//
//  define HERE
//
//==============================================================================

#ifndef __DLG_CREDITS__ 
#define __DLG_CREDITS__ 

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_bitmap.hpp"
#include "ui\ui_font.hpp"

#include "Obj_mgr\obj_mgr.hpp"

//==============================================================================
//  dlg_credits
//==============================================================================

extern void     dlg_credits_register  ( ui_manager* pManager );
extern ui_win*  dlg_credits_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

#define MAX_CREDIT_LINES_PER_PAGE      32
typedef struct _CreditLine_
{
    irect               m_Rect;
    char                m_Font[32];
    xwchar              m_String[128];
    xcolor              m_Color;
    u8                  m_Type;
    s32                 m_RenderFlags;
    s32                 m_StringLength;
    s32                 m_CharCount;
    CustomRenderStruct  m_CustomRenderStruct[128];
    f32                 m_FadeDelay[128];
}CreditLine;

class dlg_credits : public ui_dialog
{
public:

    enum _RETURN_CODES
    {
        CREDITS_NO_CODE = 0,
        CREDITS_NEW_PAGE = 1, 
        CREDITS_DONE,
        CREDITS_TITLE_CODE,
    };

    enum _LINE_TYPES
    {
        LINE_TYPE_TITLE = 0,
        LINE_TYPE_NORMAL
    };

    enum _FADE_MODE
    {
        ALPHA_FADE_IN,
        ALPHA_FADE_OUT,
        ALPHA_FADE_MODE_NONE,
        ALPHA_FADE_DONE
    };

                        dlg_credits     ( void );
    virtual            ~dlg_credits     ( void );

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
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                InitCreditLines     ( void );
    xbool               BuildPage           ( void );
    s32                 CheckLine           ( const xwchar* pString );

protected:
    s32                 m_LineIndex;
    s32                 m_PageLineCount;
    CreditLine          m_CreditLines[MAX_CREDIT_LINES_PER_PAGE];
    f32                 m_PageTimer;
    f32                 m_CurrentPageTime;
    xbool               m_CreditsDone;


};

#endif // DLG_PRESSSTART

