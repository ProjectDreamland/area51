#include "LightObject.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "Render\LightMgr.hpp"

//=========================================================================
// BASE LIGHT OBJECT
//=========================================================================

static struct light_obj_desc : public object_desc
{
    light_obj_desc( void ) : object_desc( 
            object::TYPE_LIGHT, 
            "Light",
            "RENDER",

            object::ATTR_RENDERABLE                  |
            object::ATTR_EDITOR_TEMP_OBJECT,

            FLAGS_GENERIC_EDITOR_CREATE ) {}

    //---------------------------------------------------------------------

    virtual object* Create( void ) { return new light_obj; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );

        if( Object.IsKindOf( light_obj::GetRTTI() ) )
        {
            light_obj& Light = light_obj::GetSafeType( Object );            

            EditorIcon_Draw(
                EDITOR_ICON_LIGHT, 
                Light.GetL2W(), 
                !!( Light.GetAttrBits() & object::ATTR_EDITOR_SELECTED ), 
                Light.GetColor() );
        }
        else
        {
            ASSERT( 0 );
        }

        return -1; 
    }

#endif // X_EDITOR

} s_LightObj_Desc;

//=========================================================================

const object_desc& light_obj::GetTypeDesc( void ) const
{
    return s_LightObj_Desc;
}

//=========================================================================

const object_desc& light_obj::GetObjectType( void )
{
    return s_LightObj_Desc;
}

//=========================================================================

light_obj::light_obj( void ) 
{
    m_Sphere.Pos.Set(0,0,0);
    m_Sphere.R=400;
    m_Color.Set(255,255,255,255);
    m_Ambient.Set(0,0,0,255);
    m_Intensity = 1;
    m_bAccentAngle = FALSE;
}

//=========================================================================

void light_obj::OnRender( void )
{
    CONTEXT( "light_obj::OnRender" );

    if ( IsDynamic() )
    {
        g_LightMgr.AddDynamicLight( GetPosition(), m_Color, m_Sphere.R, m_Intensity, TRUE );
    }
}

//=========================================================================

#ifndef X_RETAIL
void light_obj::OnDebugRender( void )
{
    CONTEXT( "light_obj::OnDebugRender" );

    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        draw_BBox( GetBBox(), m_Color );
    }
}
#endif // X_RETAIL

//=========================================================================

void light_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader( "Light", "Light Properties", 0 );
    List.PropEnumFloat ( "Light\\Radious",       "This is the radious of the light", 0 );
    List.PropEnumColor ( "Light\\LightColor",    "This is the color of the light", 0 );
    List.PropEnumColor ( "Light\\AmbientColor",  "This color gets added to the light object not matter what. It is the ambient color.", 0 );    
    List.PropEnumFloat ( "Light\\Intensity",     "Make the light start to saturate (powerfull) but still attenuates at the same distance. Note that maximun values are [-5,5]", 0 );    
    List.PropEnumBool  ( "Light\\AngleAccentuation", "Make the angle in which the light hits the surface be more contrasty.", 0 );    
}

//=============================================================================

xbool light_obj::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
    }
    else if( I.VarFloat("Light\\Radious", m_Sphere.R ) )
    {
        if( !I.IsRead() )
        {
            // This is to force recompute the bbox
            OnMove( GetPosition() );
        }
    }
    else if( I.VarColor("Light\\LightColor", m_Color ) )
    {
    }
    else if( I.VarColor("Light\\AmbientColor", m_Ambient ) )
    {
    }
    else if( I.VarFloat("Light\\Intensity", m_Intensity, -5, 5 ) )
    {
    }   
    else if( I.VarBool("Light\\AngleAccentuation", m_bAccentAngle ) )
    {
    }
    else
    {   
        return FALSE ;        
    }

    return TRUE;
}

//=========================================================================
// CHARACTER LIGHT OBJECT
//=========================================================================

static struct character_light_obj_desc : public object_desc
{
    character_light_obj_desc( void ) : object_desc( 
            object::TYPE_CHARACTER_LIGHT, 
            "Character Light", 
            "RENDER",

            object::ATTR_RENDERABLE,

            FLAGS_GENERIC_EDITOR_CREATE|
            FLAGS_IS_DYNAMIC) {}

    //---------------------------------------------------------------------

    virtual object* Create( void ) { return new character_light_obj; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );

        if( Object.IsKindOf( character_light_obj::GetRTTI() ) )
        {
            character_light_obj& Light = character_light_obj::GetSafeType( Object );            

            EditorIcon_Draw(
                EDITOR_ICON_LIGHT_CHARACTER, 
                Light.GetL2W(), 
                !!( Light.GetAttrBits() & object::ATTR_EDITOR_SELECTED ), 
                Light.GetColor() );
        }
        else
        {
            ASSERT( 0 );
        }

        return -1;
    }

#endif // X_EDITOR

} s_CharacterLightObj_Desc;

//=========================================================================

const object_desc& character_light_obj::GetTypeDesc( void ) const
{
    return s_CharacterLightObj_Desc;
}

//=========================================================================

const object_desc& character_light_obj::GetObjectType( void )
{
    return s_CharacterLightObj_Desc;
}

//=========================================================================

void character_light_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader( "Light", "Character Light Properties", 0 );
    List.PropEnumFloat ( "Light\\Radious",       "This is the radious of the light", 0 );
    List.PropEnumColor ( "Light\\LightColor",    "This is the color of the light", 0 );
    List.PropEnumFloat ( "Light\\Intensity",     "Make the light start to saturate (powerfull) but still attenuates at the same distance. Note that maximun values are [-5,5]", 0 );    
}


//=========================================================================
// DYNAMIC LIGHT OBJECT
//=========================================================================

static struct dynamic_light_obj_desc : public object_desc
{
    dynamic_light_obj_desc( void ) : object_desc( 
            object::TYPE_DYNAMIC_LIGHT, 
            "Dynamic Light", 
            "RENDER",

            object::ATTR_NEEDS_LOGIC_TIME            |
            object::ATTR_RENDERABLE,

            FLAGS_IS_DYNAMIC            |
            FLAGS_GENERIC_EDITOR_CREATE ) {}

    //---------------------------------------------------------------------

    virtual object* Create( void ) { return new dynamic_light_obj; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );

        if( Object.IsKindOf( dynamic_light_obj::GetRTTI() ) )
        {
            dynamic_light_obj& Light = dynamic_light_obj::GetSafeType( Object );            

            EditorIcon_Draw(
                EDITOR_ICON_LIGHT_DYNAMIC, 
                Light.GetL2W(), 
                !!( Light.GetAttrBits() & object::ATTR_EDITOR_SELECTED ), 
                Light.GetColor() );
        }
        else
        {
            ASSERT( 0 );
        }

        return -1;
    }

#endif // X_EDITOR

} s_DynamicLightObj_Desc;

//=========================================================================

const object_desc& dynamic_light_obj::GetTypeDesc( void ) const
{
    return s_DynamicLightObj_Desc;
}

//=========================================================================

const object_desc& dynamic_light_obj::GetObjectType( void )
{
    return s_DynamicLightObj_Desc;
}

//=========================================================================

dynamic_light_obj::dynamic_light_obj( void ) :
    light_obj(),
    m_LightType     (TYPE_CONSTANT),
    m_FlashRate     (1.0f),
    m_FadeInTime    (0.05f),
    m_FadeOutTime   (0.05f),
    m_RandTimeOffMin(0.0f),
    m_RandTimeOffMax(0.3f),
    m_RandTimeOnMin (0.0f),
    m_RandTimeOnMax (0.1f),
    m_LightFlags    (LIGHT_ACTIVE),
    m_LightState    (STATE_ON),
    m_FadeTimeLeft  (0.0f),
    m_FlashTimeLeft (0.0f),
    m_DoneAction    (ACTION_DESTROY)
{
}

//=========================================================================

void dynamic_light_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader( "Light",                               "Dynamic Light Properties", 0                        );
    List.PropEnumEnum  ( "Light\\Type", "CONSTANT\0FLASHING\0RANDOM\0ONESHOT FADE\0",
                                                           "The type of behavior for this light.", 0            );
    List.PropEnumFloat ( "Light\\Radious",                      "This is the radious of the light", 0                );
    List.PropEnumColor ( "Light\\LightColor",                   "This is the color of the light", 0                  );
    List.PropEnumFloat ( "Light\\Intensity",                    "Make the light start to saturate (powerfull) but still attenuates at the same distance. Note that maximun values are [-5,5]", 0 );
    List.PropEnumBool  ( "Light\\StartActive",                  "Does this light start as active or inactive?", 0    );
    List.PropEnumFloat ( "Light\\FlashRate",                    "How fast a flashing light toggles on/off states", 0 );
    List.PropEnumFloat ( "Light\\FadeInTime",                   "Time light takes to fade in after activation", 0    );
    List.PropEnumFloat ( "Light\\FadeOutTime",                  "Time light takes to fade out after deactivation", 0 );
    List.PropEnumFloat ( "Light\\RandTimeOffMin",               "Minimum time light is off when set to random", 0    );
    List.PropEnumFloat ( "Light\\RandTimeOffMax",               "Maximum time light is off when set to random", 0    );
    List.PropEnumFloat ( "Light\\RandTimeOnMin",                "Minimum time light is on when set to random", 0     );
    List.PropEnumFloat ( "Light\\RandTimeOnMax",                "Maximum time light is on when set to random", 0     );
    List.PropEnumEnum  ( "Light\\ActionWhenDone",               "DEACTIVATE\0DESTROY\0", "If the light finishes what it is doing, what action should be performed.", 0 );
}

//=========================================================================

xbool dynamic_light_obj::OnProperty( prop_query& I )
{
    if( light_obj::OnProperty( I ) )
    {
    }
    else
    if (I.IsVar("Light\\ActionWhenDone"))
    {
        if( I.IsRead() )
        {
            switch(m_DoneAction)
            {
            case ACTION_DESTROY:        I.SetVarEnum("DESTROY");    break ;
            case ACTION_DEACTIVATE:     I.SetVarEnum("DEACTIVATE"); break ;            
            }
        }
        else
        {
            if (!x_stricmp(I.GetVarEnum(), "DESTROY"))
                m_DoneAction = ACTION_DESTROY;
            else if (!x_stricmp(I.GetVarEnum(), "DEACTIVATE"))
                m_DoneAction = ACTION_DEACTIVATE;            
        }        
    }
    else if( I.IsVar("Light\\Type") )
    {
        if ( I.IsRead() )
        {
            switch ( m_LightType )
            {
            default:  ASSERT( FALSE );
            case TYPE_CONSTANT:     I.SetVarEnum("CONSTANT");       break;
            case TYPE_FLASHING:     I.SetVarEnum("FLASHING");       break;
            case TYPE_RANDOM:       I.SetVarEnum("RANDOM");         break;
            case TYPE_ONESHOT_FADE: I.SetVarEnum("ONESHOT FADE");   break;
            }
        }
        else
        {
            if ( !x_strcmp("CONSTANT", I.GetVarEnum()) )
                m_LightType = TYPE_CONSTANT;
            else
            if ( !x_strcmp("FLASHING", I.GetVarEnum()) )
                m_LightType = TYPE_FLASHING;
            else
            if ( !x_strcmp("RANDOM", I.GetVarEnum()) )
                m_LightType = TYPE_RANDOM;
            else
            if ( !x_strcmp("ONESHOT FADE", I.GetVarEnum()) )
                m_LightType = TYPE_ONESHOT_FADE;
        }
    }
    else if( I.IsVar("Light\\StartActive") )
    {
        if ( I.IsRead() )
        {
            xbool Active = (m_LightFlags&LIGHT_ACTIVE) ? TRUE : FALSE;
            I.SetVarBool(Active);
        }
        else
        {
            xbool Active = I.GetVarBool();
            if ( Active )
                m_LightFlags |= LIGHT_ACTIVE;
            else
                m_LightFlags &= ~LIGHT_ACTIVE;
            m_LightState = Active ? STATE_ON : STATE_OFF;
        }
    }
    else if ( I.VarFloat("Light\\FlashRate", m_FlashRate, 0.1f, 1000.0f) )
    {
    }
    else if ( I.VarFloat("Light\\FadeInTime", m_FadeInTime, 0.0f, 1000.0f) )
    {
    }
    else if ( I.VarFloat("Light\\FadeOutTime", m_FadeOutTime, 0.0f, 1000.0f) )
    {
    }
    else if ( I.VarFloat("Light\\RandTimeOffMin", m_RandTimeOffMin, 0.0f, 1000.0f) )
    {
    }
    else if ( I.VarFloat("Light\\RandTimeOffMax", m_RandTimeOffMax, 0.0f, 1000.0f) )
    {
    }
    else if ( I.VarFloat("Light\\RandTimeOnMin", m_RandTimeOnMin, 0.0f, 1000.0f) )
    {
    }
    else if ( I.VarFloat("Light\\RandTimeOnMax", m_RandTimeOnMax, 0.0f, 1000.0f) )
    {
    }
    else
    {   
        return FALSE ;        
    }

    return TRUE;
}

//=========================================================================

f32 dynamic_light_obj::CalcT( void )
{
    switch( m_LightState )
    {
    default: ASSERT( FALSE );
    
    case STATE_OFF:
        return 0.0f;
    
    case STATE_ON:
        return 1.0f;
    
    case STATE_FADING_OUT:
        if ( m_FadeOutTime > 0.0f )
            return m_FadeTimeLeft / m_FadeOutTime;
        else
            return 0.0f;

    case STATE_FADING_IN:
        if ( m_FadeInTime > 0.0f )
            return 1.0f - (m_FadeTimeLeft / m_FadeInTime);
        else
            return 0.0f;
    }
}

//=========================================================================

void dynamic_light_obj::StartFadeIn( void )
{
    switch ( m_LightState )
    {
    default:
        ASSERT( FALSE );
        break;

    case STATE_OFF:
        // we need the full fade in time, start it up
        m_FadeTimeLeft = m_FadeInTime;
        m_LightState   = STATE_FADING_IN;
        break;
    
    case STATE_ON:
        // no use fading in if you're already there
        m_FadeTimeLeft = 0;
        break;
    
    case STATE_FADING_IN:
        // already fading in...don't do it again
        break;
    
    case STATE_FADING_OUT:
        // start the fade-in, but we won't take as long because we're
        // partially there already
        m_FadeTimeLeft = m_FadeInTime * (1.0f-CalcT());
        m_LightState   = STATE_FADING_IN;
        break;
    }
}

//=========================================================================

void dynamic_light_obj::StartFadeOut( void )
{
    switch ( m_LightState )
    {
    default:
        ASSERT( FALSE );
        break;

    case STATE_OFF:
        // already off, don't bother fading out
        m_FadeTimeLeft = 0;
        break;

    case STATE_ON:
        // we need the full fade out time, start it up
        m_FadeTimeLeft = m_FadeOutTime;
        m_LightState   = STATE_FADING_OUT;
        break;

    case STATE_FADING_IN:
        // start the fade-out, but we won't take as long because we're
        // partially there already
        m_FadeTimeLeft = m_FadeOutTime * CalcT();
        m_LightState   = STATE_FADING_OUT;
        break;

    case STATE_FADING_OUT:
        // we are already in the process of fading out, don't do anything
        break;
    }
}

//=========================================================================

void dynamic_light_obj::ActivateConstant( xbool Flag )
{
    if ( Flag )
        StartFadeIn();
    else
        StartFadeOut();

    // we need logic time to fade in or out
    u32 AttrBits  = GetAttrBits();
    AttrBits     |= ATTR_NEEDS_LOGIC_TIME;
    SetAttrBits( AttrBits );
}

//=========================================================================

void dynamic_light_obj::ActivateFlashing( xbool Flag )
{
    if ( !Flag )
    {
        // turn out the lights and turn off any flashing
        StartFadeOut();
        m_FlashTimeLeft = 0.0f;
    }
}

//=========================================================================

void dynamic_light_obj::ActivateRandom( xbool Flag )
{
    if ( !Flag )
    {
        // turn out the lights and turn off any flashing
        StartFadeOut();
        m_FlashTimeLeft = 0.0f;
    }
}

//=========================================================================

void dynamic_light_obj::ActivateOneShotFade( xbool Flag )
{
    if (Flag)
        StartFadeIn();
    else
        StartFadeOut();

    u32 AttrBits  = GetAttrBits();
    AttrBits     |= ATTR_NEEDS_LOGIC_TIME;
    SetAttrBits( AttrBits );
}

//=========================================================================

void dynamic_light_obj::OnActivate( xbool Flag )
{
    if ( Flag && ((m_LightFlags&LIGHT_ACTIVE)==0) )
    {
        // activate the light
        m_LightFlags |= LIGHT_ACTIVE;
        switch( m_LightType )
        {
        default: ASSERT( FALSE );
        case TYPE_CONSTANT:     ActivateConstant(Flag);     break;
        case TYPE_FLASHING:     ActivateFlashing(Flag);     break;
        case TYPE_RANDOM:       ActivateRandom(Flag);       break;
        case TYPE_ONESHOT_FADE: ActivateOneShotFade(Flag);  break;
        }
    }
    else
    if ( !Flag && (m_LightFlags&LIGHT_ACTIVE) )
    {
        // deactivate the light
        m_LightFlags &= ~LIGHT_ACTIVE;
        switch( m_LightType )
        {
        default: ASSERT( FALSE );
        case TYPE_CONSTANT:     ActivateConstant(Flag);     break;
        case TYPE_FLASHING:     ActivateFlashing(Flag);     break;
        case TYPE_RANDOM:       ActivateRandom(Flag);       break;
        case TYPE_ONESHOT_FADE: ActivateOneShotFade(Flag);  break;
        }
    }
}

//=========================================================================

void dynamic_light_obj::FadingLogic( f32 DeltaTime )
{
    if ( (m_LightState == STATE_FADING_OUT) ||
         (m_LightState == STATE_FADING_IN) )
    {
        m_FadeTimeLeft -= DeltaTime;
        if ( m_FadeTimeLeft <= 0.0f )
        {
            m_FadeTimeLeft  = 0.0f;
            if ( m_LightState == STATE_FADING_OUT )
                m_LightState = STATE_OFF;
            else
                m_LightState = STATE_ON;
        }
    }
}

//=========================================================================

void dynamic_light_obj::ConstantLogic( f32 DeltaTime )
{
    // handle any fading
    FadingLogic(DeltaTime);

    // don't need logic for constant lights that aren't fading at the moment
    if ( (m_LightState == STATE_OFF) ||
         (m_LightState == STATE_ON) )
    {
        u32 AttrBits    = GetAttrBits();
        AttrBits       &= ~ATTR_NEEDS_LOGIC_TIME;
        SetAttrBits( AttrBits );
    }
}

//=========================================================================

void dynamic_light_obj::FlashingLogic( f32 DeltaTime )
{
    // handle any fading
    FadingLogic(DeltaTime);

    // handle any flashing
    if ( (m_LightFlags&LIGHT_ACTIVE) &&
         ((m_LightState == STATE_ON) || (m_LightState == STATE_OFF)) )
    {
        if ( m_FlashTimeLeft == 0.0f )
        {
            // we've yet to start the flash timer
            m_FlashTimeLeft = m_FlashRate;
        }
        else
        {
            // tick down the flash timer
            m_FlashTimeLeft -= DeltaTime;
            if ( m_FlashTimeLeft <= 0.0f )
            {
                m_FlashTimeLeft = 0.0f;
                if ( m_LightState == STATE_ON )
                    StartFadeOut();
                else
                    StartFadeIn();
            }
        }
    }
}

//=========================================================================

void dynamic_light_obj::RandomLogic( f32 DeltaTime )
{
    // handle any fading
    FadingLogic(DeltaTime);

    // handle any random flashing
    if ( (m_LightFlags&LIGHT_ACTIVE) &&
         ((m_LightState == STATE_ON) || (m_LightState == STATE_OFF)) )
    {
        if ( m_FlashTimeLeft == 0.0f )
        {
            // we've yet to start the flash timer
            if ( m_LightState == STATE_ON )
                m_FlashTimeLeft = x_frand( m_RandTimeOnMin, m_RandTimeOnMax );
            else
                m_FlashTimeLeft = x_frand( m_RandTimeOffMin, m_RandTimeOffMax );
        }
        else
        {
            // tick down the flash timer
            m_FlashTimeLeft -= DeltaTime;
            if ( m_FlashTimeLeft <= 0.0f )
            {
                m_FlashTimeLeft = 0.0f;
                if ( m_LightState == STATE_ON )
                    StartFadeOut();
                else
                    StartFadeIn();
            }
        }
    }
}

//=========================================================================

void dynamic_light_obj::OneShotFadeLogic( f32 DeltaTime )
{
    s32 oldstate = m_LightState;

    FadingLogic( DeltaTime );
    if (m_LightState == STATE_ON)
        StartFadeOut();

    // If the light is off, it's done.  Destroy it
    if (m_LightState == STATE_OFF) 
    {
        if (oldstate == STATE_FADING_OUT)
        {
            if (m_DoneAction == ACTION_DESTROY)
            {
                g_ObjMgr.DestroyObject( GetGuid() );
            }
            else
            {
                // Just turn off
                u32 AttrBits  = GetAttrBits();
                AttrBits     &= (~ATTR_NEEDS_LOGIC_TIME);
                SetAttrBits( AttrBits );
            }
        }    
        else
        {
            // No need to be running logic if we are off
            u32 AttrBits  = GetAttrBits();
            AttrBits     &= (~ATTR_NEEDS_LOGIC_TIME);
            SetAttrBits( AttrBits );
        }
    }
}

//=========================================================================

void dynamic_light_obj::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "dynamic_light_obj::OnAdvanceLogic" );

    switch ( m_LightType )
    {
    default: ASSERT( FALSE );
    case TYPE_CONSTANT:     ConstantLogic(DeltaTime);       break;
    case TYPE_FLASHING:     FlashingLogic(DeltaTime);       break;
    case TYPE_RANDOM:       RandomLogic(DeltaTime);         break;
    case TYPE_ONESHOT_FADE: OneShotFadeLogic(DeltaTime);    break;
    }
}

//=========================================================================

void dynamic_light_obj::OnRender( void )
{
    f32 T = CalcT();
    
    if ( T > 0.0f )
        g_LightMgr.AddDynamicLight( GetPosition(), m_Color, m_Sphere.R, T*m_Intensity, FALSE );
}

//=========================================================================


void light_obj::Setup( const    vector3&    Position,
                      xcolor      Color,
                      f32         Radius,
                      f32         Intensity,
                      xbool       bCharacterOnly /* = FALSE */ )
{
    OnMove( Position );
    m_Color = Color;
    m_Sphere.Pos = Position;
    m_Sphere.R = Radius;
    m_Intensity = Intensity;
    (void)bCharacterOnly;
}

//=========================================================================

// EOF