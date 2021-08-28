#ifndef FOCUS_OBJECT_HPP
#define FOCUS_OBJECT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"

enum view_type 
{
    VIEW_NONE,
    VIEW_BRACKETS,
    VIEW_SQUARE,
    VIEW_TRIANGLE,
    VIEW_CIRCLE
};

enum CIRCLE_SECTIONS
{
    CIRCLE_INNER = 0,
    CIRCLE_FLOATER,
    CIRCLE_OUTTER,
    NUM_CIRCLE_SECTIONS
};

//=========================================================================
// CLASS
//=========================================================================
class focus_object : public object
{
public:

    CREATE_RTTI( focus_object, object, object )

                            focus_object                        ( void );
                            ~focus_object                       ( void );
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnRender                            ( void );
    virtual void            OnRenderTransparent                 ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender                       ( void );
#endif // X_RETAIL

    virtual bbox            GetLocalBBox                        ( void ) const;      

	virtual	void	        OnEnumProp		                    ( prop_enum& list );
	virtual	xbool	        OnProperty		                    ( prop_query& rPropQuery );
            
    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );
            
    virtual void            OnActivate                          ( xbool Flag );
    void                    OnAdvanceLogic                      ( f32 DeltaTime );

    xbool                   HasFocus                            ( void );

    xbool                   TestPress                           ( void );


#ifdef X_EDITOR
    virtual s32             OnValidateProperties                ( xstring& ErrorMsg );
#endif


    struct state_vals
    {
        // Positive state values.
        guid        m_TriggerGuid;
        guid        m_SpatialGuid;

        bbox        m_FocusBox;

        float       m_ViewDist;

        s32         m_DisplayStrTable;
        s32         m_DisplayStrTitle;

        xcolor      m_Color1;
        xcolor      m_Color2;
        xcolor      m_SelectedColor1;
        xcolor      m_SelectedColor2;

        s32         m_InteractStrTable;
        s32         m_InteractStrTitle;

        float       m_TextDist;
        float       m_InteractDist;
        xbool       m_bInteractLOS;

        char*       m_pInteractAudio;
        view_type   m_ViewType;
        xbool       m_bAutoOff;
    };

    state_vals m_PosState;
    state_vals m_NegState;

protected:
    state_vals* m_pCurrState;

    s32         m_State;

    xbool       m_bActive;

    // Internal Stuff
    f32         m_AnimState;
    f32         m_TextAlphaState;
    xbool       m_bLookingAt;
    xbool       m_bRendered;

    xbool       m_bHasLOS;
    xbool       m_bInViewRange;
    xbool       m_bInTextRange;
    xbool       m_bInInteractRange;
    xbool       m_bStandingIn;

    u8          m_MaxAlpha;
    f32         m_ColorPhase;

    // this is to fix the same frame activation bug
    xbool       m_bActivationChanged;
    u8          m_ActivationFrameDelay;

    typedef struct _circleFocus
    {
        vector3 Position;
        rhandle<xbitmap> BMP;
        f32     Rotation;
    }circleFocus;

    static rhandle<xbitmap>     m_Bracket;
    circleFocus                 m_CircleFocusObj[3];
    xbool                       m_bLocked;
    f32                         m_FocusRotate;
    vector3                     m_PrevCircleFocusObjPos;
    xbool                       m_CircleFocusFadeIN;
    xbool                       m_CircleFocusFadeOUT;
    f32                         m_CircleFocusFadeStep;
    s32                         m_CircleFocusFadeStepIndex;
    s32                         m_CircleFocusColorAlpha;

//=========================================================================
};


//=========================================================================
// END
//=========================================================================

#endif