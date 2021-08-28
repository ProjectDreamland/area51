//==============================================================================
//  
//  ui_slider.hpp
//  
//==============================================================================

#ifndef UI_SLIDER_HPP
#define UI_SLIDER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_control.hpp"

//==============================================================================
//  ui_slider
//==============================================================================

extern ui_win* ui_slider_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_slider : public ui_control
{
public:
                    ui_slider           ( void );
    virtual        ~ui_slider           ( void );

    xbool           Create              ( s32           UserID,
                                          ui_manager*   pManager,
                                          const irect&  Position,
                                          ui_win*       pParent,
                                          s32           Flags );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );
    virtual void    OnUpdate            ( f32 DeltaTime );

    virtual void    OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void    OnPadSelect             ( ui_win* pWin );
    virtual void    OnLBDown                ( ui_win* pWin );
    virtual void    OnLBUp                  ( ui_win* pWin );
    virtual void    OnCursorMove            ( ui_win* pWin, s32 x, s32 y );
    virtual void    OnCursorExit            ( ui_win* pWin );

    void            SetRange            ( s32 Min, s32 Max );
    void            GetRange            ( s32& Min, s32& Max ) const;
    void            SetStep             ( s32 Step, s32 StepScalerMax = 1 );
    s32             GetStep             ( void ) const;
    void            SetValue            ( s32 Value );
    s32             GetValue            ( void ) const;

    void            UseDefaultSound     ( xbool val ) { m_UseSound = val; };

    void            SetParametric       ( xbool State = TRUE );

protected:
    s32             m_iElementBar;
    s32             m_iElementThumb;

#ifdef TARGET_PC
    xbool           m_MouseDown;
    irect           m_Thumb;
    s32             m_MouseX;
    s32             m_MouseY;
#endif
    s32             m_Min;
    s32             m_Max;
    s32             m_Step;
    s32             m_StepScaler;
    s32             m_StepScalerMax;
    s32             m_Value;

    xbool           m_IsParametric;
    f32             m_ValueParametric;
    xbool           m_UseSound;
};

//==============================================================================
#endif // UI_SLIDER_HPP
//==============================================================================
