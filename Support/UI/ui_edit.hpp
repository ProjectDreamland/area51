//==============================================================================
//  
//  ui_edit.hpp
//  
//==============================================================================

#ifndef UI_EDIT_HPP
#define UI_EDIT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_control.hpp"

#ifdef TARGET_PC

enum{ KEYBOARD_LSHIFT, KEYBOARD_RSHIFT, KEYBOARD_CAPS, KEYBOARD_NUM };

#define LSHIFT       (1<<KEYBOARD_LSHIFT)
#define RSHIFT       (1<<KEYBOARD_RSHIFT)
#define CAPSLOCK    (1<<KEYBOARD_CAPS)
#define NUMLOCK     (1<<KEYBOARD_NUM)

#endif
//==============================================================================
//  ui_edit
//==============================================================================

extern ui_win* ui_edit_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_edit : public ui_control
{
public:
                            ui_edit                 ( void );
    virtual                ~ui_edit                 ( void );

    xbool                   Create                  ( s32           UserID,
                                                    ui_manager*   pManager,
                                                    const irect&  Position,
                                                    ui_win*       pParent,
                                                    s32           Flags );
    void                    Configure               ( xbool bName ) { m_bName = bName; }

    virtual void            Render                  ( s32 ox=0, s32 oy=0 );

    virtual void            OnPadSelect             ( ui_win* pWin );
    virtual void            OnKeyDown               ( ui_win* pWin, s32 Key );
    virtual void            OnKeyUp                 ( ui_win* pWin, s32 Key );
    virtual void            OnCursorEnter           ( ui_win* pWin );

    void                    SetLabelWidth           ( s32 Width );
    void                    SetBufferSize           ( s32 BufferSize );
    void                    SetVirtualKeyboardTitle ( const xwstring& Title );

//    virtual void            SetText             ( const xstring& Text );
//    virtual void            SetText             ( const char*    Text );
//    virtual const xstring&  GetText             ( void ) const;

protected:
    s32             m_iElement1;
    s32             m_iElement2;
    s32             m_LabelWidth;
    s32             m_BufferSize;
    xwstring        m_VirtualKeyboardTitle;
    xbool           m_bName;  // Whether this dialogue exists to enter in a name (as opposed to a password).

#ifdef TARGET_PC
    s32             KeyFlags;
    s32             m_CursorPosition;
    s32             m_LastKey;
#endif
//    xstring         m_Text;
};

//==============================================================================
#endif // UI_EDIT_HPP
//==============================================================================
