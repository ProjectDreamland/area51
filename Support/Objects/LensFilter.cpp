//==============================================================================
//  LensFilter.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class handles the logic for applying a pulsing lens color over the
//  screen (modulating the vertex colors, so that flashlight can still make it
//  brighter)
//==============================================================================

#include "LensFilter.hpp"
#include "Render\\Render.hpp"

//==============================================================================
// OBJECT DESCRIPTION
//==============================================================================

static struct lens_filter_desc : public object_desc
{
    lens_filter_desc( void ) : object_desc( 
        object::TYPE_LENS_FILTER, 
        "LensFilter",
        "RENDER",
        object::ATTR_NEEDS_LOGIC_TIME,
        
        FLAGS_IS_DYNAMIC| FLAGS_GENERIC_EDITOR_CREATE ) {}

        virtual object* Create( void )
        {
            return new lens_filter;
        }

#ifdef X_EDITOR

        virtual s32 OnEditorRender  ( object& Object ) const 
        { 
            (void)Object;
            return EDITOR_ICON_NOTE;
        }
#endif // X_EDITOR

} s_LensFilter_Desc;

//==============================================================================

const object_desc& lens_filter::GetTypeDesc( void ) const
{
    return s_LensFilter_Desc;
}

//==============================================================================

const object_desc& lens_filter::GetObjectType( void )
{
    return  s_LensFilter_Desc;
}

//==============================================================================
// IMPLEMENTATION
//==============================================================================

lens_filter::lens_filter( void ) :
    m_PulseColorMin     ( 64, 64, 64, 64 ),
    m_PulseColorMax     ( 128, 128, 128, 128 ),
    m_FadeInTime        ( 2.0f ),
    m_FadeOutTime       ( 2.0f ),
    m_FadeTimeElapsed   ( 0.0f ),
    m_PulseWaveLength   ( 3.0f ),
    m_CurrentState      ( STATE_INACTIVE ),
    m_CurrentWaveTime   ( 0.0f )
{
}

//==============================================================================

lens_filter::~lens_filter( void )
{
    render::EnableFilterLight( FALSE );
}

//==============================================================================

bbox lens_filter::GetLocalBBox( void ) const
{
    return bbox( vector3(0.0f,0.0f,0.0f), 25.0f );
}

//==============================================================================

void lens_filter::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );
    List.PropEnumHeader  ( "LensFilter",                "Lens filter - can be used for emergency lighting etc.", 0 );
    List.PropEnumBool    ( "LensFilter\\Active",        "Does this filter start active?", 0 );
    List.PropEnumColor   ( "LensFilter\\PulseColorMin", "Min strobe light color", 0 );
    List.PropEnumColor   ( "LensFilter\\PulseColorMax", "Max strobe light color", 0 );
    List.PropEnumFloat   ( "LensFilter\\FadeInTime",    "Time to turn on strobe light (in seconds)", 0 );
    List.PropEnumFloat   ( "LensFilter\\FadeOutTime",   "Time to turn off strobe light (in seconds)", 0 );
    List.PropEnumFloat   ( "LensFilter\\PulseTime",     "Time between strobe cycles (in seconds)", 0 );
}

//==============================================================================

xbool lens_filter::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return TRUE;

    if( I.IsVar( "LensFilter\\Active" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( (GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME) != 0 );
        }
        else
        {
            if( I.GetVarBool() )
                OnActivate( TRUE );
            else
                object::OnActivate( FALSE );
        }

        return TRUE;
    }

    if( I.VarColor( "LensFilter\\PulseColorMin", m_PulseColorMin ) )
        return TRUE;

    if( I.VarColor( "LensFilter\\PulseColorMax", m_PulseColorMax ) )
        return TRUE;

    if( I.VarFloat( "LensFilter\\FadeInTime", m_FadeInTime, 0.0f, 1000.0f ) )
        return TRUE;

    if( I.VarFloat( "LensFilter\\FadeOutTime", m_FadeOutTime, 0.0f, 1000.0f ) )
        return TRUE;

    if( I.VarFloat( "LensFilter\\PulseTime", m_PulseWaveLength, 0.1f, 1000.0f ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

void lens_filter::OnActivate( xbool bFlag )
{
    if( bFlag )
    {
        // we need logic time now
        object::OnActivate( bFlag );

        switch( m_CurrentState )
        {
        case STATE_INACTIVE:
            // start fading in
            m_CurrentState    = STATE_FADE_IN;
            m_FadeTimeElapsed = 0.0f;
            break;

        case STATE_ACTIVE:
        case STATE_FADE_IN:
            // already trying to be active
            break;

        case STATE_FADE_OUT:
            // switching states before we are fully faded out...set to
            // fade back in, but advance t based on how far we'd already
            // faded out
            f32 T;
            m_CurrentState = STATE_FADE_IN;
            if( m_FadeOutTime > 0.0f )
            {
                T = 1.0f - (m_FadeTimeElapsed / m_FadeOutTime);
                m_FadeTimeElapsed = T * m_FadeInTime;
            }
            else
            {
                m_FadeTimeElapsed = 0.0f;
            }
            break;
        }
    }
    else
    {
        switch( m_CurrentState )
        {
        case STATE_INACTIVE:
        case STATE_FADE_OUT:
            // already trying to be inactive
            break;

        case STATE_ACTIVE:
            // start fading out
            m_CurrentState    = STATE_FADE_OUT;
            m_FadeTimeElapsed = 0.0f;
            break;

        case STATE_FADE_IN:
            // switching states before we are fully faded in...set to
            // fade back out, but advance t based on how far we'd already
            // faded in
            f32 T;
            m_CurrentState = STATE_FADE_OUT;
            if( m_FadeInTime > 0.0f )
            {
                T = 1.0f - (m_FadeTimeElapsed / m_FadeInTime);
                m_FadeTimeElapsed = T * m_FadeOutTime;
            }
            else
            {
                m_FadeTimeElapsed = 0.0f;
            }
        }
    }
}

//==============================================================================

void lens_filter::OnAdvanceLogic( f32 DeltaTime )
{
    // determine if we have finished fading in or out
    if( m_CurrentState == STATE_FADE_IN )
    {
        m_FadeTimeElapsed += DeltaTime;
        if( m_FadeTimeElapsed > m_FadeInTime )
        {
            m_CurrentState = STATE_ACTIVE;
        }
    }
    else if( m_CurrentState == STATE_FADE_OUT )
    {
        m_FadeTimeElapsed += DeltaTime;
        if( m_FadeTimeElapsed > m_FadeOutTime )
        {
            m_CurrentState = STATE_INACTIVE;
            object::OnActivate( FALSE );
        }
    }

    // advance the pulsing filter
    //#### SPECIAL NOTE: We are using fmodf here rather than the standard x_fmod, because
    // x_fmod has a bug where it will get caught in an infinite loop if 0>m_PulseWaveLength<.5.
    // (Test with 0.1 to be sure) Once the bug is fixed, we'll switch it back to the x_files
    // version.
    m_CurrentWaveTime += DeltaTime;
    if( m_CurrentWaveTime > m_PulseWaveLength )
        m_CurrentWaveTime = fmodf( m_CurrentWaveTime, m_PulseWaveLength );

    // do a color blend between the low and high based on a sine wave
    f32 SinT = 1.0f + 0.5f*x_sin( (2.0f * PI * m_CurrentWaveTime) / m_PulseWaveLength );
    vector3 MinColor( m_PulseColorMin.R, m_PulseColorMin.G, m_PulseColorMin.B );
    vector3 MaxColor( m_PulseColorMax.R, m_PulseColorMax.G, m_PulseColorMax.B );
    vector3 CurrColor = MinColor + SinT * (MaxColor - MinColor);

    // and if we're fading in or out, do a color blend for that, too
    if( m_CurrentState == STATE_FADE_IN )
    {
        ASSERT( m_FadeInTime > 0.0f );
        
        f32 FadeT = 1.0f - (m_FadeTimeElapsed / m_FadeInTime);
        ASSERT( (FadeT>=0.0f) && (FadeT<=1.0f) );
        
        vector3 FadeColor( 128.0f, 128.0f, 128.0f );
        CurrColor = CurrColor + FadeT * (FadeColor - CurrColor);
    }
    else if( m_CurrentState == STATE_FADE_OUT )
    {
        ASSERT( m_FadeOutTime > 0.0f );

        f32 FadeT = m_FadeTimeElapsed / m_FadeOutTime;
        ASSERT( (FadeT>=0.0f) && (FadeT<=1.0f) );

        vector3 FadeColor( 128.0f, 128.0f, 128.0f );
        CurrColor = CurrColor + FadeT * (FadeColor - CurrColor);
    }

    // we shouldn't need to do this, but just to be safe, we'll clamp
    CurrColor.Min( 255.0f );
    CurrColor.Max( 0.0f );

    // let the render engine know the state of things
    if( m_CurrentState == STATE_INACTIVE )
    {
        render::EnableFilterLight( FALSE );
    }
    else
    {
        render::EnableFilterLight( TRUE );
        render::SetFilterLightColor( xcolor((u8)CurrColor.GetX(), (u8)CurrColor.GetY(), (u8)CurrColor.GetZ()) );
    }
}

//==============================================================================
