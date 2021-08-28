#ifndef LIGHT_OBJECT_HPP
#define LIGHT_OBJECT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// LIGHT
//=========================================================================
class light_obj : public object
{
public:

    CREATE_RTTI( light_obj, object, object )
    
                            light_obj       ( void );
    virtual bbox            GetLocalBBox    ( void ) const { return m_Sphere.GetBBox(); }
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            Setup           (   const vector3&  Position,
                                                xcolor          Color,
                                                f32             Radius,
                                                f32             Intensity,
                                                xbool           bCharacterOnly = FALSE );


    xcolor                  GetAmbient          ( void );
    xcolor                  GetColor            ( void );
    f32                     GetRadious          ( void );
    f32                     GetLightIntensity   ( void );
    xbool                   GetAngleAccent      ( void );
    virtual xbool           IsDynamic           ( void ) { return FALSE; }


    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

protected:
    
    virtual void            OnRender        ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender   ( void );
#endif // X_RETAIL

    sphere                  m_Sphere;
    xcolor                  m_Color;
    xcolor                  m_Ambient;
    f32                     m_Intensity;
    xbool                   m_bAccentAngle;
    
// Make friends here
};

//=========================================================================
// CHARACTER LIGHT
//=========================================================================
class character_light_obj : public light_obj
{
public:
    CREATE_RTTI( character_light_obj, light_obj, object )

    virtual void            OnEnumProp          ( prop_enum& List );
    virtual xbool           IsDynamic           ( void ) { return TRUE; }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
};

//=========================================================================
// DYNAMIC LIGHT
//=========================================================================
class dynamic_light_obj : public light_obj
{
public:
    CREATE_RTTI( dynamic_light_obj, light_obj, object )

                            dynamic_light_obj   ( void );
    virtual void            OnEnumProp          ( prop_enum&  List      );
    virtual void            OnActivate          ( xbool       Flag      );
    virtual xbool           OnProperty          ( prop_query& I         );
    virtual void            OnAdvanceLogic      ( f32         DeltaTime );
    virtual void            OnRender            ( void );
    virtual xbool           IsDynamic           ( void ) { return TRUE; }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    enum flags
    {
        LIGHT_ACTIVE = 0x0001,
    };

    enum type
    {
        TYPE_CONSTANT = 0,
        TYPE_FLASHING,
        TYPE_RANDOM,
        TYPE_ONESHOT_FADE,
    };

    enum state
    {
        STATE_OFF = 0,
        STATE_ON,
        STATE_FADING_OUT,
        STATE_FADING_IN,
    };

    enum done_action
    {
        ACTION_DESTROY,
        ACTION_DEACTIVATE,
    };


protected:
    f32     CalcT               ( void );

    void    StartFadeIn         ( void );
    void    StartFadeOut        ( void );

    void    ActivateConstant    ( xbool Flag );
    void    ActivateFlashing    ( xbool Flag );
    void    ActivateRandom      ( xbool Flag );
    void    ActivateOneShotFade ( xbool Flag );

    void    FadingLogic         ( f32 DeltaTime );
    void    ConstantLogic       ( f32 DeltaTime );
    void    FlashingLogic       ( f32 DeltaTime );
    void    RandomLogic         ( f32 DeltaTime );
    void    OneShotFadeLogic    ( f32 DeltaTime );

    s32             m_LightType;
    f32             m_FlashRate;
    f32             m_FadeInTime;
    f32             m_FadeOutTime;
    f32             m_RandTimeOffMin;
    f32             m_RandTimeOffMax;
    f32             m_RandTimeOnMin;
    f32             m_RandTimeOnMax;
    
    u32             m_LightFlags;
    s32             m_LightState;
    f32             m_FadeTimeLeft;
    f32             m_FlashTimeLeft;

    done_action     m_DoneAction;

};

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================

inline xcolor  light_obj::GetAmbient( void )
{
    return m_Ambient;
}

//=========================================================================

inline xcolor  light_obj::GetColor  ( void )
{
    return m_Color;
}

//=========================================================================

inline f32 light_obj::GetRadious( void )
{
    return m_Sphere.R;
}

//=========================================================================

inline f32 light_obj::GetLightIntensity( void )
{
    return m_Intensity/2;
}

//=========================================================================

inline xbool light_obj::GetAngleAccent( void )
{
    return m_bAccentAngle;
}

//=========================================================================
// END
//=========================================================================
#endif

