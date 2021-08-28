//==============================================================================
//  
//  ui_button.hpp
//  
//==============================================================================

#ifndef UI_BUTTON_HPP
#define UI_BUTTON_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_control.hpp"

//==============================================================================
//  ui_button
//==============================================================================

extern ui_win* ui_button_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_button : public ui_control
{
public:
                    ui_button               ( void );
    virtual        ~ui_button               ( void );

    xbool           Create                  ( s32           UserID,
                                              ui_manager*   pManager,
                                              const irect&  Position,
                                              ui_win*       pParent,
                                              s32           Flags );

    virtual void    Render                  ( s32 ox=0, s32 oy=0 );
    virtual void    OnUpdate                ( f32 DeltaTime );

    void            SetBitmap               ( s32 ID )                  { m_BitmapID = ID; }
    s32             GetBitmap               ( void )                    { return m_BitmapID; }

    void            SetData                 ( s32 Data )                { m_Data = Data; }
    s32             GetData                 ( void )                    { return m_Data; }

    void            UseSmallText            ( const xbool useSmall )    { m_useSmallText = useSmall;    }
    void            UseNativeColor          ( const xbool useNative )   { m_useNativeColor = useNative; }

    void            EnablePulse             ( void )                    { m_bPulseOn = TRUE;  }
    void            DisablePulse            ( void )                    { m_bPulseOn = FALSE; }

    // global button color functions
    static void     SetTextColorNormal      ( xcolor color)             { m_TextColorNormal    = color; }
    static void     SetTextColorHightlight  ( xcolor color)             { m_TextColorHighlight = color; }
    static void     SetTextColorDisabled    ( xcolor color)             { m_TextColorDisabled  = color; }
    static void     SetTextColorShadow      ( xcolor color)             { m_TextColorShadow    = color; }

protected:

    xbool           m_useSmallText;
    xbool           m_useNativeColor;
    xbool           m_bPulseOn;
    xbool           m_bPulseUp;
    f32             m_PulseRate;
    f32             m_PulseValue;
    s32             m_iElement;
    s32             m_BitmapID;
    s32             m_Data;

    // set once for all buttons
    static xcolor   m_TextColorNormal;
    static xcolor   m_TextColorHighlight;
    static xcolor   m_TextColorDisabled;
    static xcolor   m_TextColorShadow;
};


// define the button colors
extern xcolor   m_TextColorNormal;
extern xcolor   m_TextColorHighlight;
extern xcolor   m_TextColorDisabled;
extern xcolor   m_TextColorShadow;



//==============================================================================
#endif // UI_BUTTON_HPP
//==============================================================================
