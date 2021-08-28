#include "controller.hpp"
#include "element_spemitter.hpp"
#include "effect.hpp"
#include "x_context.hpp"
#include "convex_hull.hpp"

namespace fx_core
{

#define SPEM_VER    2

#define ELEMENT_EXPORT_TYPE     ( 'S' | ('P'<<8) | ('E'<<16) | ('M'<<24) )


//============================================================================
// element REGISTRATION
//============================================================================
element*         s_SpemitterFactory    ( void )  { return (element*)new element_spemitter; }
static elem_reg  s_SpemitterKeyReg               ( "spemitter", s_SpemitterFactory );


#define NUM_RANDOM_SEEDS    4096

s32 s_RandomSeeds[NUM_RANDOM_SEEDS];

class populate_random
{
public:
    populate_random()
    {
        for( s32 i=0 ; i<NUM_RANDOM_SEEDS ; i++ )
        {
            s_RandomSeeds[i] = x_rand();
        }
    }
} pop_random;

//============================================================================

void SwapColorEndian( xcolor& Color );

//============================================================================
// Constructor

element_spemitter::element_spemitter( )
: element()
{
//    m_StartFrame            = 0;
//    m_StopFrame             = 1000;
    m_nParticles            = 20;
    m_EmitInterval          = 0.05f;
    m_EmitIntervalVar       = 0.0f;
    m_LifeSpan              = 1.0f;
    m_IsBurst               = FALSE;
    m_Reverse               = FALSE;
    m_ScaleSprite           = FALSE;
    m_Oriented              = FALSE;
    m_UseEmissionVolume     = FALSE;
    m_WorldSpace            = TRUE;
    m_CombineMode           = COMBINEMODE_ALPHA;
    m_ZBias                 = 0.0f;

    m_Speed                 = 50.0f;

    m_MinVelocity.Set( -1.0f ,-1.0f ,-1.0f );
    m_MaxVelocity.Set(  1.0f,  1.0f,  1.0f );

    m_ShowVelocity          = FALSE;

    m_MinRotSpeed           = 0.0f;
    m_MaxRotSpeed           = 0.0f;

    m_Gravity               = 0.0f;
    m_Acceleration          = 0.0f;

    m_CycleTime             = m_nParticles * m_LifeSpan;

    m_nAllocatedParticles   = 0;
    m_pParticles            = NULL;
    m_iLastEmitted          = 0;

    m_Keys.SetCount( 1 );
    m_Keys[0].Color.Set( 255,255,255,255 );
    m_Keys[0].Scale = 10.0f;

    m_pType = "Spemitter";
}

element_spemitter::~element_spemitter()
{
    if( m_pParticles )
    {
        delete m_pParticles;
        m_pParticles = NULL;
    }
}

//============================================================================

void element_spemitter::Create( const char* pElementID, effect& Effect )
{
    // Assert that the ID not be NULL
    ASSERT( pElementID );

    m_ID        = pElementID;
    m_pEffect   = &Effect;

    m_pScale =       new ctrl_linear;
    m_pRotation =    new ctrl_linear;
    m_pTranslation = new ctrl_linear;
    m_pColor =       new ctrl_linear;
    m_pAlpha =       new ctrl_linear;

    // fix the color scalar
    m_pColor->SetScalar( 1.0f/255.0f );
    m_pAlpha->SetScalar( 1.0f/255.0f );

    m_pScale->SetNumFloats      (3);
    m_pRotation->SetNumFloats   (3);
    m_pTranslation->SetNumFloats(3);
    m_pColor->SetNumFloats      (3);
    m_pAlpha->SetNumFloats      (1);

    Effect.AddController( m_pScale       );
    Effect.AddController( m_pRotation    );
    Effect.AddController( m_pTranslation );
    Effect.AddController( m_pColor       );
    Effect.AddController( m_pAlpha       );

    // add myself to the element list
    Effect.AddElement( this );
}

//============================================================================

element* element_spemitter::Duplicate( void )
{
    element_spemitter* pNew = new element_spemitter( *this );

    pNew->m_pParticles = NULL;
    pNew->AllocateParticles( TRUE );

    pNew->m_pScale =        (ctrl_linear*)m_pScale->CopyOf();
    pNew->m_pRotation =     (ctrl_linear*)m_pRotation->CopyOf();
    pNew->m_pTranslation =  (ctrl_linear*)m_pTranslation->CopyOf();
    pNew->m_pColor =        (ctrl_linear*)m_pColor->CopyOf();
    pNew->m_pAlpha =        (ctrl_linear*)m_pAlpha->CopyOf();

    pNew->MakeDuplicateName( m_ID );

    m_pEffect->AddController( pNew->m_pScale       );
    m_pEffect->AddController( pNew->m_pRotation    );
    m_pEffect->AddController( pNew->m_pTranslation );
    m_pEffect->AddController( pNew->m_pColor       );
    m_pEffect->AddController( pNew->m_pAlpha       );
    

    return (element*)pNew;
}

//============================================================================

void element_spemitter::PostCopyPtrFix( void )
{
}

//============================================================================

xbool element_spemitter::ExistsAtTime( f32 T ) const
{
    return( (T >= m_LifeStartFrame) && (m_IsImmortal || (T <= (m_LifeStopFrame+(m_LifeSpan*30)))) );
}

//============================================================================

xbool element_spemitter::GetLocalBBoxAtTime( f32 T, bbox& BBox ) const
{
    if( ExistsAtTime( T ) )
    {
        // We can't compute for world space
        if( m_WorldSpace )
        {
            BBox.Set( vector3(0,0,0), 0.5f );
            return TRUE;
        }

        vector3 vMin    = m_MinVelocity * m_Speed;
        vector3 vMax    = m_MaxVelocity * m_Speed;
        f32     a       = m_Acceleration;
        vector3 g       = vector3(0,1,0)*(-m_Gravity);
        f32     t[8];
        vector3 Min( F32_MAX, F32_MAX, F32_MAX);
        vector3 Max(-F32_MAX,-F32_MAX,-F32_MAX);

        // Get times for zero velocities
        SolveForTimeAtZeroVelocity( vMin.GetX(), a, g.GetX(), t[0] );
        SolveForTimeAtZeroVelocity( vMax.GetX(), a, g.GetX(), t[1] );
        SolveForTimeAtZeroVelocity( vMin.GetY(), a, g.GetY(), t[2] );
        SolveForTimeAtZeroVelocity( vMax.GetY(), a, g.GetY(), t[3] );
        SolveForTimeAtZeroVelocity( vMin.GetZ(), a, g.GetZ(), t[4] );
        SolveForTimeAtZeroVelocity( vMax.GetZ(), a, g.GetZ(), t[5] );

        // Add in times for 0.0 and m_Lifespan
        t[6] = 0.0f;
        t[7] = m_LifeSpan;

        // Loop over times and get positions
        for( s32 i=0 ; i<(sizeof(t)/sizeof(t[0])) ; i++ )
        {
            f32 pMin;
            f32 pMax;

            pMin = SolveForPositionAtTime( vMin.GetX(), a, g.GetX(), t[i] );
            pMax = SolveForPositionAtTime( vMax.GetX(), a, g.GetX(), t[i] );
            if( pMin < Min.GetX() ) Min.GetX() = pMin;
            if( pMax < Min.GetX() ) Min.GetX() = pMax;
            if( pMin > Max.GetX() ) Max.GetX() = pMin;
            if( pMax > Max.GetX() ) Max.GetX() = pMax;

            pMin = SolveForPositionAtTime( vMin.GetY(), a, g.GetY(), t[i] );
            pMax = SolveForPositionAtTime( vMax.GetY(), a, g.GetY(), t[i] );
            if( pMin < Min.GetY() ) Min.GetY() = pMin;
            if( pMax < Min.GetY() ) Min.GetY() = pMax;
            if( pMin > Max.GetY() ) Max.GetY() = pMin;
            if( pMax > Max.GetY() ) Max.GetY() = pMax;
        
            pMin = SolveForPositionAtTime( vMin.GetZ(), a, g.GetZ(), t[i] );
            pMax = SolveForPositionAtTime( vMax.GetZ(), a, g.GetZ(), t[i] );
            if( pMin < Min.GetZ() ) Min.GetZ() = pMin;
            if( pMax < Min.GetZ() ) Min.GetZ() = pMax;
            if( pMin > Max.GetZ() ) Max.GetZ() = pMin;
            if( pMax > Max.GetZ() ) Max.GetZ() = pMax;
        }

        // Make bounding box
        BBox.Set( Min, Max );

        // Inflate if using emission volume
        if( m_UseEmissionVolume )
            BBox.Inflate( 0.5f, 0.5f, 0.5f );

        // Inflate for particle scale & rotation
        f32 Scale = 1.0f;
        for( i=0 ; i<m_Keys.GetCount() ; i++ )
        {
            if( m_Keys[i].Scale > Scale )
                Scale = m_Keys[i].Scale;
        }
        BBox.Inflate( Scale/2.0f*x_sqrt(2.0f), Scale/2.0f*x_sqrt(2.0f), Scale/2.0f*x_sqrt(2.0f));

        return TRUE;
    }

    return FALSE;
}

//============================================================================

void element_spemitter::SetWorldSpace( xbool UseWorldSpace )
{
    m_WorldSpace = UseWorldSpace;
}

//============================================================================
// Show properties

xbool element_spemitter::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    xcolor  HeaderColor     ( 119, 128, 144 );
    xcolor  KeyHeaderColor  ( 151, 160, 176 );
    xcolor  ItemColor       ( 176, 176, 176 );

    // give the base class a chance to deal with the basics
    if ( element::GetProperty( Idx, T, UIColor, Name, Value, IsDisabled, Type ) == FALSE )
    {
        // return our specific data here
        switch( Idx )
        {
            case 26:
                //============================================================================
                Name.Format( "Material\\Z Bias" );
                Value.Format( "%g", m_ZBias );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 27:
                //============================================================================
                Name.Format( "Material\\Draw Mode" );
                Value = CombineMode_ToString( m_CombineMode );
                Type        = PROP_COMBINEMODE;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 28:
                //============================================================================
                Name.Format( "Material\\Bitmap" );
                Value = m_BitmapName;
                Type        = PROP_FILENAME;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 29:
                //============================================================================
                Name.Format( "Emitter" );
                Value.Format( "" );
                Type        = PROP_HEADER;
                UIColor     = HeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 30:
                //============================================================================
                Name.Format( "Emitter\\Num Particles" );
                Value.Format( "%d", m_nParticles );
                Type        = PROP_INT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 31:
                //============================================================================
                Name.Format( "Emitter\\Emit Interval" );
                Value.Format( "%g", m_EmitInterval );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = TRUE;
                return TRUE;
            case 32:
                //============================================================================
                Name.Format( "Emitter\\Emit Variance" );
                Value.Format( "%g", m_EmitIntervalVar );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 33:
                //============================================================================
                Name.Format( "Emitter\\World Space" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;

                if( m_pEffect->m_Instanceable )
                {
                    Value       = "false";
                    IsDisabled  = TRUE;
                }
                else
                {
                    Value.Format( "%s", m_WorldSpace == TRUE ? "true" : "false" );
                    IsDisabled  = FALSE;
                }

                return TRUE;
            case 34:
                //============================================================================
                Name.Format( "Emitter\\Use Volume" );
                Value.Format( "%s", m_UseEmissionVolume==TRUE?"true":"false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 35:
                //============================================================================
                Name.Format( "Emitter\\Reverse" );
                Value.Format( "%s", m_Reverse==TRUE?"true":"false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 36:
                //============================================================================
                Name.Format( "Emitter\\Burst Mode" );
                Value.Format( "%s", m_IsBurst==TRUE?"true":"false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 37:
                //============================================================================
                Name.Format( "Emitter\\Gravity" );
                Value.Format( "%g", m_Gravity );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 38:
                //============================================================================
                Name.Format( "Emitter\\Acceleration" );
                Value.Format( "%g", m_Acceleration );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 39:
                //============================================================================
                Name.Format( "Particle" );
                Value.Format( "%d", m_Keys.GetCount() );
                Type        = PROP_HEADER;
                UIColor     = HeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 40:
                //============================================================================
                Name.Format( "Particle\\Life Span" );
                Value.Format( "%g", m_LifeSpan );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 41:
                //============================================================================
                Name.Format( "Particle\\Speed" );
                Value.Format( "%g", m_Speed );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 42:
                //============================================================================
                Name.Format( "Particle\\Min Velocity" );
                Value.Format( "%g, %g, %g", m_MinVelocity.GetX(), m_MinVelocity.GetY(), m_MinVelocity.GetZ() );
                Type        = PROP_V3;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 43:
                //============================================================================
                Name.Format( "Particle\\Max Velocity" );
                Value.Format( "%g, %g, %g", m_MaxVelocity.GetX(), m_MaxVelocity.GetY(), m_MaxVelocity.GetZ() );
                Type        = PROP_V3;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 44:
                //============================================================================
                Name.Format( "Particle\\Show Velocity" );
                Value.Format( "%s", m_ShowVelocity == TRUE ? "true" : "false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 45:
                //============================================================================
                Name.Format( "Particle\\Oriented" );
                Value.Format( "%s", m_Oriented == TRUE ? "true" : "false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 46:
                //============================================================================
                Name.Format( "Particle\\Min Rot Speed" );
                Value.Format( "%g", m_MinRotSpeed );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 47:
                //============================================================================
                Name.Format( "Particle\\Max Rot Speed" );
                Value.Format( "%g", m_MaxRotSpeed );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 48:
                //============================================================================
                Name.Format( "Particle\\Scale Size" );
                Value.Format( "%s", m_ScaleSprite == TRUE ? "true" : "false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 49:
                //============================================================================
                Name.Format( "Particle\\Num Keys" );
                Value.Format( "%d", m_Keys.GetCount() );
                Type        = PROP_INT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            default:
                {
                    s32 j = Idx - 50;
                    s32 k = 0;

                    for( s32 i=0 ; i<m_Keys.GetCount() ; i++ )
                    {
                        if ( j == (i * 3 + 0) )
                        {
                            //============================================================================
                            Name.Format( "Particle\\Key %d", i+1 );
                            Value.Clear();
                            Type        = PROP_HEADER;
                            UIColor     = KeyHeaderColor;
                            IsDisabled  = FALSE;
                            return TRUE;
                        }
                        else
                        if ( j == (i * 3 + 1) )
                        {
                            //============================================================================
                            Name.Format( "Particle\\Key %d\\KeyColor", i+1 );
                            Value.Format( "%d, %d, %d, %d", m_Keys[i].Color.R, m_Keys[i].Color.G, m_Keys[i].Color.B, m_Keys[i].Color.A );
                            Type        = PROP_COLOR;
                            UIColor     = ItemColor;
                            IsDisabled  = FALSE;
                            return TRUE;
                        }
                        else
                        if ( j == (i * 3 + 2) )
                        {
                            //============================================================================
                            Name.Format( "Particle\\Key %d\\KeySize", i+1 );
                            Value.Format( "%g", m_Keys[i].Scale );
                            Type        = PROP_FLOAT;
                            UIColor     = ItemColor;
                            IsDisabled  = FALSE;
                            return TRUE;
                        }
                    }

                }
                return FALSE;
        }
    }
    else
    {
        return TRUE;
    }

    return FALSE;
}

//============================================================================
// Update data fed back from the property bar

xbool element_spemitter::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
{
    xbool   Dirty = FALSE;

    // Strip out the header information from the Field, so we can check the property strings
    s32     PropLength  = Field.GetLength() - ( 17 + m_ID.GetLength() );
    xstring Property    = Field.Right( PropLength );

    //-------------------------------------------------------------------------
    // Over-ride the properties we want to
    //-------------------------------------------------------------------------
    if( x_strcmp( Property, "Object\\Immortal" ) == 0 )
    {
        m_IsImmortal    = x_strcmp( Value, "true" ) == 0 ? TRUE : FALSE;

        if( m_IsImmortal )
        {
            m_IsBurst = false;
        }

        Dirty           = TRUE;
    }
    else if( x_strcmp( Property, "Object\\Start Frame" ) == 0 )
    {
        m_LifeStartFrame =  x_atoi( Value );
        AllocateParticles   ( TRUE );
        Dirty            =  TRUE;
    }
    else if( x_strcmp( Property, "Object\\Stop Frame" ) == 0 )
    {
        m_LifeStopFrame =   x_atoi( Value );
        AllocateParticles   ( TRUE );
        Dirty           =   TRUE;
    }

    //-------------------------------------------------------------------------
    // Particle Properties
    //-------------------------------------------------------------------------
    else if( x_strcmp( Property, "Material\\Z Bias" ) == 0 )
    {
        m_ZBias = x_atof( Value );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Draw Mode" ) == 0 )
    {
        m_CombineMode = CombineMode_FromString( Value );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Bitmap" ) == 0 )
    {
        g_pTextureMgr->DeActivateBitmap( m_BitmapName );
        Value.IsEmpty() ? m_BitmapName = "fx_default.xbmp" : m_BitmapName = Value;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Num Particles" ) == 0 )
    {
        m_nParticles = x_atoi( Value );
        m_nParticles = MAX( 1, m_nParticles );
        m_nParticles = MIN( 20000, m_nParticles );
        m_EmitInterval = m_LifeSpan / m_nParticles;
        m_CycleTime  = m_nParticles * m_EmitInterval;
        AllocateParticles( TRUE );
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Emit Variance" ) == 0 )
    {
        m_EmitIntervalVar = (f32)atof(Value);
        m_EmitIntervalVar = MAX( m_EmitIntervalVar, 0.0f );
        m_EmitIntervalVar = MIN( m_EmitIntervalVar, 100.0f );
        AllocateParticles( TRUE );
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\World Space" ) == 0 )
    {
        m_WorldSpace = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Use Volume" ) == 0 )
    {
        m_UseEmissionVolume = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Reverse" ) == 0 )
    {
        m_Reverse = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Burst Mode" ) == 0 )
    {
        m_IsBurst = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;

        if( m_IsBurst )
        {
            m_IsImmortal = false;
        }

        AllocateParticles( TRUE );
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Gravity" ) == 0 )
    {
        m_Gravity = (f32)x_atof(Value);
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Emitter\\Acceleration" ) == 0 )
    {
        m_Acceleration = (f32)x_atof(Value);
        Dirty = TRUE;
    }

    //-------------------------------------------------------------------------
    // Particle Properties
    //-------------------------------------------------------------------------

    else if( x_strcmp( Property, "Particle\\Life Span" ) == 0 )
    {
        m_LifeSpan     = (f32)atof(Value);
        m_LifeSpan     = MAX( m_LifeSpan, 0.001f );
        m_EmitInterval = m_LifeSpan / m_nParticles;
        m_CycleTime    = m_nParticles * m_EmitInterval;
        AllocateParticles( TRUE );
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Speed" ) == 0 )
    {
        m_Speed = (f32)x_atof(Value);
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Min Velocity" ) == 0 )
    {
        String2V3( Value, m_MinVelocity );
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Max Velocity" ) == 0 )
    {
        String2V3( Value, m_MaxVelocity );
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Show Velocity" ) == 0 )
    {
        m_ShowVelocity = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Oriented" ) == 0 )
    {
        m_Oriented = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Min Rot Speed" ) == 0 )
    {
        m_MinRotSpeed = (f32)x_atof(Value);
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Max Rot Speed" ) == 0 )
    {
        m_MaxRotSpeed = (f32)x_atof(Value);
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Scale Size" ) == 0 )
    {
        m_ScaleSprite = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;
        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Particle\\Num Keys" ) == 0 )
    {
        s32 nKeys = x_atoi(Value);
        if( nKeys < m_Keys.GetCount() )
            m_Keys.SetCount( nKeys );
        else
        {
            while( m_Keys.GetCount() < nKeys )
            {
                key Key;
                Key.Color.Set(255,255,255,255);
                Key.Scale = 10.0f;
                m_Keys.Append( Key );
            }
            m_Keys.SetCount( nKeys );
        }

        Dirty = TRUE;
    }

    else if( strstr(Field, "KeyColor") )
    {
        const char* pStr = strstr(Field, "\\Key" );
        s32 iKey;
        iKey = x_atoi( &pStr[5] );
        String2Color( Value, m_Keys[iKey-1].Color );
        Dirty = TRUE;
    }

    else if( strstr(Field, "KeySize") )
    {
        const char* pStr = strstr(Field, "\\Key" );
        s32 iKey;
        iKey = x_atoi( &pStr[5] );
        m_Keys[iKey-1].Scale = (f32)x_atof( Value );
        Dirty = TRUE;
    }

    //-------------------------------------------------------------------------
    // Let the base class handle any properties we didn't over-ride

    else
    {
        Dirty = element::OnPropertyChanged( T, Field, Value );
    }

    return Dirty;
}


//============================================================================
void element_spemitter::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "SPEMITTER", "Type of element" );

        // add the base stuff
        element::Save( Igf );
 
        // now spemitter-specific
        Igf.AddS32    ( "SPEM_VER"      , SPEM_VER,                 "Spemitter Version Number" );
        Igf.AddBool   ( "Burst"         , m_IsBurst                 );
        Igf.AddBool   ( "Reverse"       , m_Reverse                 );
        Igf.AddBool   ( "UseVol"        , m_UseEmissionVolume       );
        Igf.AddBool   ( "WldSpace"      , m_WorldSpace              );
        Igf.AddS32    ( "CombineMode"   , m_CombineMode             );
        Igf.AddString ( "Bitmap"        , (const char*)m_BitmapName );
        Igf.AddS32    ( "nParticles"    , m_nParticles              );
        Igf.AddF32    ( "EmitInterval"  , m_EmitInterval            );
        Igf.AddF32    ( "EmitVariance"  , m_EmitIntervalVar         );
        Igf.AddF32    ( "Lifespan"      , m_LifeSpan                );
        Igf.AddV3     ( "Minvel"        , m_MinVelocity             );
        Igf.AddV3     ( "Maxvel"        , m_MaxVelocity             );
        Igf.AddF32    ( "MinRotSpeed"   , m_MinRotSpeed             );
        Igf.AddF32    ( "MaxRotSpeed"   , m_MaxRotSpeed             );
        Igf.AddF32    ( "Gravity"       , m_Gravity                 );
        Igf.AddF32    ( "Acceleration"  , m_Acceleration            );
        Igf.AddF32    ( "Speed"         , m_Speed                   );
        Igf.AddBool   ( "Oriented"      , m_Oriented                );
        Igf.AddF32    ( "ZBias"         , m_ZBias                   );
        Igf.AddBool   ( "ScaleSprite"   , m_ScaleSprite             );
    }
    
    // Add the individual keys
    NewGrp = Igf.AddGroup( "Keys" );
    Igf.EnterGroup( NewGrp );

    Igf.AddS32( "nKeys", m_Keys.GetCount(), "The number of keys" );
    for( s32 i=0 ; i<m_Keys.GetCount() ; i++ )
    {
        Igf.AddColor( "Color", m_Keys[i].Color );
        Igf.AddF32  ( "Scale", m_Keys[i].Scale );
    }
    // exit the key group
    Igf.ExitGroup();

    // set the group back to wherever we were
    Igf.ExitGroup();
}

//============================================================================
void element_spemitter::Load( igfmgr& Igf )
{
    s32 SpemitterVer = 0;

    // base data first
    element::Load( Igf );

    // This is for backwards compatibility
    if ( Igf.Find( "SPEM_VER" ) )
    {
        SpemitterVer = Igf.GetS32();
    }

    // spemitter-specific
    m_IsBurst           = Igf.GetBool   ( "Burst"         );
    m_Reverse           = Igf.GetBool   ( "Reverse"       );
    m_UseEmissionVolume = Igf.GetBool   ( "UseVol"        );
    m_WorldSpace        = Igf.GetBool   ( "WldSpace"      );


    m_BitmapName        = Igf.GetString ( "Bitmap"        );
    m_nParticles        = Igf.GetS32    ( "nParticles"    );
    m_EmitInterval      = Igf.GetF32    ( "EmitInterval"  );
    m_EmitIntervalVar   = Igf.GetF32    ( "EmitVariance"  );
    m_LifeSpan          = Igf.GetF32    ( "Lifespan"      );
    m_MinVelocity       = Igf.GetV3     ( "Minvel"        );
    m_MaxVelocity       = Igf.GetV3     ( "Maxvel"        );
    m_MinRotSpeed       = Igf.GetF32    ( "MinRotSpeed"   );
    m_MaxRotSpeed       = Igf.GetF32    ( "MaxRotSpeed"   );
    m_Gravity           = Igf.GetF32    ( "Gravity"       );
    m_Acceleration      = Igf.GetF32    ( "Acceleration"  );
    m_Speed             = Igf.GetF32    ( "Speed"         );
    m_Oriented          = Igf.GetBool   ( "Oriented"      );
    m_CombineMode       = Igf.GetS32    ( "CombineMode"   );

    m_ZBias             = ( SpemitterVer > 1 ) ? Igf.GetF32( "ZBias" ) : 0.0f;   // Version 2+
    
    if ( Igf.Find("ScaleSprite") )
        m_ScaleSprite = Igf.GetBool();
    else
        m_ScaleSprite = FALSE;


    hfield KeyGrp = Igf.GetGroup( "Keys" );
    Igf.EnterGroup( KeyGrp );
    {
        s32 Count, i;

        Count = Igf.GetS32( "nKeys" );
        m_Keys.SetCapacity( MAX(20, Count) );

        // move to the first color/scale pair
        Igf.Find( "Color" );

        m_Keys.Clear();

        for ( i = 0; i < Count; i++ )
        {
            element_spemitter::key Key;
            Key.Color = Igf.GetColor();
            Igf.Next();
            Key.Scale = Igf.GetF32();
            Igf.Next();
    
            m_Keys.Append( Key );
        }

        Igf.ExitGroup();
    }
}

//============================================================================
// Export data
void element_spemitter::ExportData( export::fx_elementhdr& ElemHdr, 
                                    xstring& Type,
                                    xbytestream& Stream, 
                                    s32 ExportTarget )
{

    struct spemitter_data
    {
        s32         BitmapIndex; 
        u32         Flags;
        s32         NParticles;
        f32         LifeSpan;
        f32         EmitVariance;
        radian      MinRotate;
        radian      MaxRotate;
        vector3p    MinVelocity;
        vector3p    MaxVelocity;
        f32         Gravity;
        f32         Acceleration;
        f32         ZBias;
        s32         NKeyFrames;
    };

    spemitter_data  Data;
    
    Type.Format( "SPEMITTER" );

    // Call base
    xbytestream     TempStream;
    TempStream.Clear();
    element::ExportData( ElemHdr, Type, TempStream, ExportTarget );
    ElemHdr.TypeIndex = ELEMENT_EXPORT_TYPE;
    
    switch( m_CombineMode )
    {
        case COMBINEMODE_SUBTRACTIVE:   ElemHdr.CombineMode = -1;    break;
        case COMBINEMODE_ALPHA:         ElemHdr.CombineMode =  0;    break;
        case COMBINEMODE_ADDITIVE:      ElemHdr.CombineMode =  1;    break;

        case COMBINEMODE_GLOW_SUB:      ElemHdr.CombineMode =  9;    break;
        case COMBINEMODE_GLOW_ALPHA:    ElemHdr.CombineMode = 10;    break;
        case COMBINEMODE_GLOW_ADD:      ElemHdr.CombineMode = 11;    break;

        case COMBINEMODE_DISTORT:       ElemHdr.CombineMode = 20;    break;

        default:                        ElemHdr.CombineMode =  0;    break;
    }

    // Insert the extra info
    Data.BitmapIndex  = g_pTextureMgr->GetTextureIndex( m_BitmapName );
    Data.Flags        = 0x00;
    Data.NParticles   = m_nParticles;
    Data.LifeSpan     = m_LifeSpan;
    Data.EmitVariance = m_EmitIntervalVar;
    Data.MinRotate    = DEG_TO_RAD(m_MinRotSpeed);
    Data.MaxRotate    = DEG_TO_RAD(m_MaxRotSpeed);
    Data.MinVelocity  = m_MinVelocity * m_Speed;
    Data.MaxVelocity  = m_MaxVelocity * m_Speed;
    Data.Gravity      = m_Gravity;
    Data.Acceleration = m_Acceleration;
    Data.NKeyFrames   = m_Keys.GetCount();
    Data.ZBias        = m_ZBias;

    if( m_WorldSpace        )   Data.Flags |= (1 <<  0);
    if( m_ScaleSprite       )   Data.Flags |= (1 <<  1);
    if( m_UseEmissionVolume )   Data.Flags |= (1 <<  2);
    if( m_IsBurst           )   Data.Flags |= (1 <<  3);
    if( m_Reverse           )   Data.Flags |= (1 <<  4);
    if( m_Oriented          )   Data.Flags |= (1 <<  5);

    // Write that data out
    TempStream.Append( (const byte*)&Data, sizeof(spemitter_data) );

    // Write out the color/size/alpha data
    for ( s32 i = 0; i < Data.NKeyFrames; i++ )
    {
        struct spemkey
        {
            xcolor  Color;
            f32     Scale;
        };

        spemkey Key;

        Key.Color = m_Keys[i].Color;
        Key.Scale = m_Keys[i].Scale;
        
        if( ExportTarget == EXPORT_TARGET_GCN )
        {
            // Must reverse the Endian of the colors.
            SwapColorEndian( Key.Color );
        }

        TempStream.Append( (const byte*)&Key, sizeof(spemkey) );
    }

    // Adjust the size
    ElemHdr.TotalSize = TempStream.GetLength() / sizeof(s32);
    TempStream.Replace( 0, (const byte*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // Append the new data onto our stream
    Stream.Append( TempStream );
}

//============================================================================

void element_spemitter::AllocateParticles( xbool Force )
{
    // Check for a forced allocation (usually to set new emission times) or a change in number of particles
    if( Force || (m_nParticles != m_nAllocatedParticles) )
    {
        // Set number of particles allocated and allocate them
        m_nAllocatedParticles   = m_nParticles;
        m_pParticles            = (particle*)realloc( m_pParticles, sizeof(particle)*m_nAllocatedParticles );
        ASSERT( m_pParticles );

        f32 EmitInterval;

        // Calculate time to emit all particles in the circular buffer
        if( m_IsBurst )
        {
            m_CycleTime  = 0.0f;
            EmitInterval = ((m_LifeStopFrame - m_LifeStartFrame) * (1.0f/30.0f)) / m_nAllocatedParticles;
        }
        else
        {
            m_CycleTime  = m_nAllocatedParticles * m_EmitInterval;
            EmitInterval = m_EmitInterval;
        }

        // Calculate variance in emission time
        f32 Variance = MIN( (EmitInterval*m_EmitIntervalVar)/100.0f, EmitInterval );

        // Set Emit times and age rate for all particles
        for( s32 i=0 ; i<m_nAllocatedParticles ; i++ )
        {
            m_pParticles[i].EmitTime = i*EmitInterval + x_frand( 0, Variance );
            m_pParticles[i].AgeRate  = 1.0f / m_LifeSpan;
        }
    }
}

//============================================================================

void element_spemitter::SetParticlePositions( f32 T )
{
    f32     Time        = T                * (1.0f / 30.0f);
    f32     StartTime   = m_LifeStartFrame * (1.0f / 30.0f);
    f32     StopTime    = m_LifeStopFrame  * (1.0f / 30.0f);
    s32     nKeys       = m_Keys.GetCount();
    random  r;

    // Controlled values
    f32     Pos  [3];
    f32     Scale[3];
    f32     Rot  [3];
    f32     Color[3];
    f32     Alpha;


    // Do we get a valid control node for this time?
    if( m_pTranslation->GetValue( T, Pos ) )
    {
        matrix4 L2W;
        vector3 m_EmitPoint;

        // Get controlled values
        m_pScale->GetValue   ( T, Scale  );
        m_pRotation->GetValue( T, Rot    );
        m_pColor->GetValue   ( T, Color  );
        m_pAlpha->GetValue   ( T, &Alpha );

        // Build matrix for local particle system to world
        L2W.Identity ();
        L2W.Scale    ( vector3(Scale[0], Scale[1], Scale[2]) );
        L2W.Rotate   ( radian3(  Rot[0],   Rot[1],   Rot[2]) );
        L2W.Translate( vector3(  Pos[0],   Pos[1],   Pos[2]) );

        // Clamp StopTime
        if( StopTime <= StartTime )
            StopTime = StartTime;

        // Transform time into emitter relative time
        Time     -= StartTime;
        StopTime -= StartTime;

        // Calculate cycle variables
        s32 iCycle          = (m_CycleTime > 0.0f) ? (s32)(Time / m_CycleTime) : 0;
        f32 CycleStartTime  = (iCycle*m_CycleTime);
        f32 CycleIntraTime  = Time - CycleStartTime;

        // Loop through all particles
        for( s32 i=0 ; i<m_nAllocatedParticles ; i++ )
        {
            // Determine index of this particle in the random seed table
            s32 iRand = iCycle*m_nAllocatedParticles + i;

            // Get pointer to particle
            particle* p = &m_pParticles[i];

            // Calculate time of emission & Correct iRand for cycle condition
            f32 EmitTime = p->EmitTime;
            if( !m_IsBurst )
            {
                EmitTime += CycleStartTime;
                if( p->EmitTime > CycleIntraTime )
                {
                    EmitTime -= m_CycleTime;
                    iRand    -= m_nAllocatedParticles;
                }
            }

            // Is this particle emitted?
            if( (EmitTime <= Time) && (EmitTime >= 0.0f) && ((EmitTime <= StopTime) || m_IsImmortal) )
            {
                // Calculate Age
                f32 Age = Time - EmitTime;
                p->Age  = Age * p->AgeRate;
                p->Active = p->Age < 1.0f;

                // Save last emitted index for rendering
                m_iLastEmitted = i;

                // Seed the random number generator
                r.srand(s_RandomSeeds[iRand&(NUM_RANDOM_SEEDS-1)] );

                // Set intial velocity
                p->Velocity.GetX() = r.frand( m_MinVelocity.GetX(), m_MaxVelocity.GetX() );
                p->Velocity.GetY() = r.frand( m_MinVelocity.GetY(), m_MaxVelocity.GetY() );
                p->Velocity.GetZ() = r.frand( m_MinVelocity.GetZ(), m_MaxVelocity.GetZ() );

                // Multiply the "speed" factor into the initial velocity
                p->Velocity *= m_Speed;

                // Set initial rotation & rotation speed
                // TODO: Set initial rotation?
                p->RotationSpeed = DEG_TO_RAD( r.frand( m_MinRotSpeed, m_MaxRotSpeed ) );

                // Calculate emission position
                if( m_UseEmissionVolume )
                {
                    // Get random emission point inside the volume
                    p->Position.Set( r.frand(-0.5f, 0.5f), 
                                     r.frand(-0.5f, 0.5f),
                                     r.frand(-0.5f, 0.5f) );
                }
                else
                {
                    // Emit from origin
                    p->Position.Zero();
                }

                // If a world space effect then transform the point now
                if( m_WorldSpace )
                {
                    // Get Keydata at time of particle emission
                    f32 EmitT = (StartTime+EmitTime)*30.0f;
                    m_pTranslation->GetValue( EmitT, Pos    );
                    m_pScale->GetValue      ( EmitT, Scale  );
                    m_pRotation->GetValue   ( EmitT, Rot    );

                    // Build matrix for velocity
                    L2W.Identity ();
                    L2W.Scale    ( vector3(Scale[0], Scale[1], Scale[2]) );
                    L2W.Rotate   ( radian3(  Rot[0],   Rot[1],   Rot[2]) );
                    p->Velocity = L2W.Transform( p->Velocity );

                    // Add translation into matrix and transform position
                    L2W.Translate( vector3(  Pos[0],   Pos[1],   Pos[2]) );
                    p->Position = L2W.Transform( p->Position );
                }

                // Advance position using Age
                if( m_Reverse )
                {
                    f32 ReverseAge = m_LifeSpan - Age;

                    p->Position += p->Velocity                  * ReverseAge +
                                   (p->Velocity*m_Acceleration) * (ReverseAge * ReverseAge * 0.5f) + 
                                   vector3(0,1,0)*(-m_Gravity)  * (ReverseAge * ReverseAge * 0.5f);

                    p->Velocity  = p->Velocity + ((p->Velocity*m_Acceleration) + vector3(0,1,0)*(-m_Gravity)) * ReverseAge;
                }
                else
                {
                    p->Position += p->Velocity * Age +
                                   ((p->Velocity*m_Acceleration) + vector3(0,1,0)*(-m_Gravity)) * (Age * Age * 0.5f);

                    p->Velocity  = p->Velocity + ((p->Velocity*m_Acceleration) + vector3(0,1,0)*(-m_Gravity)) * Age;
                }

                // Advance rotation using Age
                p->Rotation = p->RotationSpeed * Age;
            }
            else
            {
                p->Active = FALSE;
                p->Age    = 0.0f;
            }

            // Interpolate particle appearance from keyframes
            if( p->Active )
            {
                if( nKeys == 0 )
                {
                    // Set Color & Scale
                    p->Color.Set(255,255,255,255);
                    p->Scale = 10.0f;
                }
                else if( nKeys == 1 )
                {
                    // Set Color & Scale
                    p->Color = m_Keys[0].Color;
                    p->Scale = m_Keys[0].Scale;
                }
                else
                {
                    // Figure out which keyframes to use and a blend factor.
                    // TO DO - Rig the parametric age to be the frames.
                    f32 Max    = nKeys - 1.00001f;
                    f32 Frame  = p->Age * Max;
                    s32 Lo     = (s32)x_floor( Frame );
                    s32 Hi     = Lo + 1;
                    f32 LoMix  = Hi - Frame;
                    f32 HiMix  = Frame - Lo;

                    // Update the particle.
                    p->Scale   =     ((m_Keys[Lo].Scale   * LoMix) + 
                                      (m_Keys[Hi].Scale   * HiMix));

                    p->Color.R = (u8)((m_Keys[Lo].Color.R * LoMix) + 
                                      (m_Keys[Hi].Color.R * HiMix));

                    p->Color.G = (u8)((m_Keys[Lo].Color.G * LoMix) + 
                                      (m_Keys[Hi].Color.G * HiMix));

                    p->Color.B = (u8)((m_Keys[Lo].Color.B * LoMix) + 
                                      (m_Keys[Hi].Color.B * HiMix));

                    p->Color.A = (u8)((m_Keys[Lo].Color.A * LoMix) + 
                                      (m_Keys[Hi].Color.A * HiMix));
                }
            }
        }
    }
}

//============================================================================
// GetLocalStaticBBox

xbool element_spemitter::SolveForTimeAtZeroVelocity( f32 v, f32 a, f32 g, f32& t ) const
{
    f32 Numerator   = -v;
    f32 Denominator = v*a + g;

    if( x_abs(Denominator) > 0.000001f )
    {
        t = Numerator / Denominator;
        if( t <       0.0f ) t = 0.0f;
        if( t > m_LifeSpan ) t = m_LifeSpan;
        return TRUE;
    }
    else
    {
        t = 0.0f;
        return FALSE;
    }
}

f32 element_spemitter::SolveForPositionAtTime( f32 v, f32 a, f32 g, f32 t ) const
{
    return v*t + ((v*a) + g)*(t*t*0.5f);
}

// Solve for each of x,y,z independantly
// Solve Time for velocity == 0 for both min & max initial velocity
// Solve Position at Times 0.0, m_Lifespan, velocity == 0
// Expand for emission volume if used
// Expand for particle scale
// Send BBox through scale and rotation controllers

xbool element_spemitter::GetWorldStaticBBox( bbox& aBBox ) const
{
    if( m_WorldSpace )
        return FALSE;

    vector3 vMin    = m_MinVelocity * m_Speed;
    vector3 vMax    = m_MaxVelocity * m_Speed;
    f32     a       = m_Acceleration;
    vector3 g       = vector3(0,1,0)*(-m_Gravity);
    f32     t[8];
    vector3 Min( F32_MAX, F32_MAX, F32_MAX);
    vector3 Max(-F32_MAX,-F32_MAX,-F32_MAX);
    bbox    BBox;

    // Get times for zero velocities
    SolveForTimeAtZeroVelocity( vMin.GetX(), a, g.GetX(), t[0] );
    SolveForTimeAtZeroVelocity( vMax.GetX(), a, g.GetX(), t[1] );
    SolveForTimeAtZeroVelocity( vMin.GetY(), a, g.GetY(), t[2] );
    SolveForTimeAtZeroVelocity( vMax.GetY(), a, g.GetY(), t[3] );
    SolveForTimeAtZeroVelocity( vMin.GetZ(), a, g.GetZ(), t[4] );
    SolveForTimeAtZeroVelocity( vMax.GetZ(), a, g.GetZ(), t[5] );

    // Add in times for 0.0 and m_Lifespan
    t[6] = 0.0f;
    t[7] = m_LifeSpan;

    // Loop over times and get positions
    for( s32 i=0 ; i<8 ; i++ )
    {
        f32 pMin;
        f32 pMax;

        pMin = SolveForPositionAtTime( vMin.GetX(), a, g.GetX(), t[i] );
        pMax = SolveForPositionAtTime( vMax.GetX(), a, g.GetX(), t[i] );
        if( pMin < Min.GetX() ) Min.GetX() = pMin;
        if( pMax < Min.GetX() ) Min.GetX() = pMax;
        if( pMin > Max.GetX() ) Max.GetX() = pMin;
        if( pMax > Max.GetX() ) Max.GetX() = pMax;

        pMin = SolveForPositionAtTime( vMin.GetY(), a, g.GetY(), t[i] );
        pMax = SolveForPositionAtTime( vMax.GetY(), a, g.GetY(), t[i] );
        if( pMin < Min.GetY() ) Min.GetY() = pMin;
        if( pMax < Min.GetY() ) Min.GetY() = pMax;
        if( pMin > Max.GetY() ) Max.GetY() = pMin;
        if( pMax > Max.GetY() ) Max.GetY() = pMax;
        
        pMin = SolveForPositionAtTime( vMin.GetZ(), a, g.GetZ(), t[i] );
        pMax = SolveForPositionAtTime( vMax.GetZ(), a, g.GetZ(), t[i] );
        if( pMin < Min.GetZ() ) Min.GetZ() = pMin;
        if( pMax < Min.GetZ() ) Min.GetZ() = pMax;
        if( pMin > Max.GetZ() ) Max.GetZ() = pMin;
        if( pMax > Max.GetZ() ) Max.GetZ() = pMax;
    }

    // Make bounding box
    BBox.Set( Min, Max );

    // Inflate if using emission volume
    if( m_UseEmissionVolume )
        BBox.Inflate( 0.5f, 0.5f, 0.5f );

    // Inflate for particle scale & rotation
    f32 Scale = 1.0f;
    for( i=0 ; i<m_Keys.GetCount() ; i++ )
    {
        if( m_Keys[i].Scale > Scale )
            Scale = m_Keys[i].Scale;
    }
    BBox.Inflate( Scale/2.0f*x_sqrt(2.0f), Scale/2.0f*x_sqrt(2.0f), Scale/2.0f*x_sqrt(2.0f));

    // Determine time range to cover all controller keys
    s32 MinT = m_pTranslation->GetMinT();
    s32 MaxT = m_pTranslation->GetMaxT();
    MinT = MIN( m_pRotation->GetMinT(), MinT );
    MinT = MIN( m_pScale   ->GetMinT(), MinT );
    MaxT = MAX( m_pRotation->GetMaxT(), MaxT );
    MaxT = MAX( m_pScale   ->GetMaxT(), MaxT );

    // Apply controllers to BBox
    aBBox = BBox;
    s32 nTransforms = 0;
    for( s32 T = MinT ; T <= MaxT ; T++ )
    {
        f32 Pos  [3];
        f32 Scale[3];
        f32 Rot  [3];

        m_pTranslation->GetValue( (f32)T, Pos    );
        m_pRotation   ->GetValue( (f32)T, Rot    );
        m_pScale      ->GetValue( (f32)T, Scale  );

        // Build L2W for the controller
        matrix4 L2W;
        L2W.Identity ();
        L2W.Scale    ( vector3(Scale[0], Scale[1], Scale[2]) );
        L2W.Rotate   ( radian3(Rot  [0], Rot  [1], Rot  [2]) );
        L2W.Translate( vector3(Pos  [0], Pos  [1], Pos  [2]) );

        // Send BBox through L2W & apply to final BBox
        bbox TempBBox = BBox;
        TempBBox.Transform( L2W );
        if( nTransforms == 0 )
            aBBox = TempBBox;
        else
            aBBox += TempBBox;
        nTransforms++;
    }
/*
    // Inflate by 10% just to be safe
    vector3 Size = FinalBBox.GetSize() / 10.0f;
    FinalBBox.Inflate( Size.X, Size.Y, Size.Z );
*/
    // Return success
    return TRUE;
}

//============================================================================
// Render

void element_spemitter::Render( f32 T )
{
    CONTEXT( "element_spemitter::Render" );
    
    if ( m_Hide )
        return;

    vector2 WH;
    xcolor  ElementColor;
    xcolor  RenderColor;

    f32     Pos  [3];
    f32     Scale[3];
    f32     Rot  [3];
    f32     Color[3];
    f32     Alpha;
    f32     ParticleScale = 1.0f;

    // Read controllers
    m_pTranslation->GetValue( T, Pos    );
    m_pRotation   ->GetValue( T, Rot    );
    m_pScale      ->GetValue( T, Scale  );
    m_pColor      ->GetValue( T, Color  );
    m_pAlpha      ->GetValue( T, &Alpha );

    // Clamp color values
    if( Color[0] > 255.0f )     { Color[0] = 255.0f; }
    if( Color[1] > 255.0f )     { Color[1] = 255.0f; }
    if( Color[2] > 255.0f )     { Color[2] = 255.0f; }
    if( Alpha    > 255.0f )     { Alpha    = 255.0f; }

    if( Color[0] < 0.0f )       { Color[0] = 0.0f; }
    if( Color[1] < 0.0f )       { Color[1] = 0.0f; }
    if( Color[2] < 0.0f )       { Color[2] = 0.0f; }
    if( Alpha    < 0.0f )       { Alpha    = 0.0f; }

    // Set the color
    ElementColor.Set( (u8)Color[0], (u8)Color[1], (u8)Color[2], (u8)Alpha );

    // Build L2W and vector L2W (no translation)
    matrix4 VL2W;
    matrix4 L2W;
    L2W.Identity ();
    L2W.Scale    ( vector3(Scale[0], Scale[1], Scale[2]) );
    L2W.Rotate   ( radian3(Rot  [0], Rot  [1], Rot  [2]) );
    VL2W = L2W;
    L2W.Translate( vector3(Pos  [0], Pos  [1], Pos  [2]) );

    // Determine particle scale
    if( m_ScaleSprite )
    {
        ParticleScale = (Scale[0] + Scale[1] + Scale[2]) / 3.0f;
    }

    // Make sure we have particles allocated
    AllocateParticles();

    // Set the particle positions for the time to be rendered
    SetParticlePositions( T );

    // Determine blend mode
    s32 DrawBlendMode = 0;
    switch( m_CombineMode )
    {
    case COMBINEMODE_ADDITIVE:
    case COMBINEMODE_GLOW_ADD:
        DrawBlendMode = DRAW_BLEND_ADD;
        break;
    case COMBINEMODE_SUBTRACTIVE:
    case COMBINEMODE_GLOW_SUB:
        DrawBlendMode = DRAW_BLEND_SUB;
        break;
    }

    // Clear L2W
    draw_ClearL2W();

    // draw flags
    u32 DrawFlags   = DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_CULL_NONE | DRAW_NO_ZWRITE | DRAW_UV_CLAMP | DrawBlendMode;
    if( !m_ZRead )  { DrawFlags |= DRAW_NO_ZBUFFER; }

    // Start drawing
    if( m_Oriented )        { draw_Begin( DRAW_TRIANGLES, DrawFlags ); }
    else                    { draw_Begin( DRAW_SPRITES,   DrawFlags ); }

    if ( m_BitmapName.IsEmpty() )
        m_BitmapName = "fx_default.xbmp";
    
    g_pTextureMgr->ActivateBitmap( m_BitmapName );

    // Setup UVs
    vector2 UV0(0.0f,0.0f);
    vector2 UV1(1.0f,1.0f);

    vector3 ViewZ;
    vector3 Center;
    vector3 Dir;
    vector3 CrossDir;
    vector3 Start;
    vector3 End;

    // Oriented setup
    if( m_Oriented )
    {
        // Get view Z vector
        ViewZ = eng_GetView()->GetViewZ();
    }

    // Loop through particles rendering
    s32 i    = 0; //(m_iLastEmitted+1) % m_nAllocatedParticles;
    s32 iEnd = i;
    do
    {
        // Get particle pointer
        particle* p = &m_pParticles[i];

        // Is it active?
        if( p->Active )
        {
            // Combine sprite color with emitter color for final render color
            RenderColor.R   = (u8) ( ( (u32)ElementColor.R * (u32)p->Color.R ) >> 8 );
            RenderColor.G   = (u8) ( ( (u32)ElementColor.G * (u32)p->Color.G ) >> 8 );
            RenderColor.B   = (u8) ( ( (u32)ElementColor.B * (u32)p->Color.B ) >> 8 );
            RenderColor.A   = (u8) ( ( (u32)ElementColor.A * (u32)p->Color.A ) >> 8 );

            // Render sprites
            if( m_Oriented )
            {
                // Get Vectors for direction of particle and cross of view vector with that
                f32 ParticleSize    = p->Scale * ParticleScale * 0.5f;

                // If in local space then the center and direction of the particle must be transformed to world space
                if( m_WorldSpace )
                {
                    Center = p->Position;
                    Dir    = p->Velocity;
                }
                else
                {
                    Center = L2W.Transform( p->Position );
                    Dir    = VL2W.Transform( p->Velocity );
                }

                // Generate endpoints of quad and width vector
                Dir.Normalize();
                CrossDir  = ViewZ.Cross( Dir );
                Dir      *= ParticleSize;
                CrossDir *= ParticleSize;
                Start     = Center + Dir;
                End       = Center - Dir;

                // Draw Quad
                draw_Color( RenderColor );

                draw_UV( 1.0f, 0.0f ); draw_Vertex( Start - CrossDir );
                draw_UV( 0.0f, 0.0f ); draw_Vertex( End   - CrossDir );
                draw_UV( 0.0f, 1.0f ); draw_Vertex( End   + CrossDir );

                draw_UV( 1.0f, 0.0f ); draw_Vertex( Start - CrossDir );
                draw_UV( 0.0f, 1.0f ); draw_Vertex( End   + CrossDir );
                draw_UV( 1.0f, 1.0f ); draw_Vertex( Start + CrossDir );
            }
            else
            {
                // Render sprite
                WH.Set( p->Scale*ParticleScale, p->Scale*ParticleScale );
                if( m_WorldSpace )
                {
                    // Draw in world space
                    draw_SpriteUV( p->Position, WH, UV0, UV1, RenderColor, p->Rotation );
                }
                else
                {
                    // Transform to world space and render
                    vector3 Position = L2W.Transform( p->Position );
                    draw_SpriteUV( Position, WH, UV0, UV1, RenderColor, p->Rotation );
                }
            }
        }

        // Next particle
        i = (i+1) % m_nAllocatedParticles;
    } while( i != iEnd );

    // End drawing
    draw_End();

    // Render element bbox
    draw_SetL2W( L2W );
    RenderBBox( T );
    draw_ClearL2W();

    // Render the velocity representation
    RenderVelocity( T, L2W );

    // Render the translation path of the object
    RenderTrajectory();

#ifdef cgalley
    // Render the world static BBox
    bbox BBox;
    if( GetWorldStaticBBox( BBox ) )
        draw_BBox( BBox, XCOLOR_GREEN );
#endif
}

//============================================================================

void element_spemitter::RenderVelocity( f32 T, const matrix4& L2W ) const
{
    if( m_ShowVelocity )
    {
        // Pre-multiply our "speed" with the min/max velocities
        vector3 Min = m_MinVelocity * m_Speed;
        vector3 Max = m_MaxVelocity * m_Speed;

        xcolor      VelocitySolid( 255, 255, 255,  32 );
        xcolor      VelocityWire ( 255, 255, 255, 128 );;

        // See if the emission point is inside of the velocity bounds
        vector3     EmitOrigin      ( 0, 0, 0  );
        bbox        VelocityBounds  ( Min, Max );

        // Do convex hull rendering
        convex_hull Hull;

        s32 v0 = Hull.AddVertex( vector3(Min.GetX(),Min.GetY(),Min.GetZ()) );
        s32 v1 = Hull.AddVertex( vector3(Min.GetX(),Max.GetY(),Min.GetZ()) );
        s32 v2 = Hull.AddVertex( vector3(Max.GetX(),Max.GetY(),Min.GetZ()) );
        s32 v3 = Hull.AddVertex( vector3(Max.GetX(),Min.GetY(),Min.GetZ()) );

        s32 v4 = Hull.AddVertex( vector3(Min.GetX(),Min.GetY(),Max.GetZ()) );
        s32 v5 = Hull.AddVertex( vector3(Min.GetX(),Max.GetY(),Max.GetZ()) );
        s32 v6 = Hull.AddVertex( vector3(Max.GetX(),Max.GetY(),Max.GetZ()) );
        s32 v7 = Hull.AddVertex( vector3(Max.GetX(),Min.GetY(),Max.GetZ()) );

        s32 v8 = Hull.AddVertex( EmitOrigin );

        Hull.AddRenderEdge( v0, v1 );
        Hull.AddRenderEdge( v1, v2 );
        Hull.AddRenderEdge( v2, v3 );
        Hull.AddRenderEdge( v3, v0 );

        Hull.AddRenderEdge( v4, v5 );
        Hull.AddRenderEdge( v5, v6 );
        Hull.AddRenderEdge( v6, v7 );
        Hull.AddRenderEdge( v7, v4 );

        Hull.AddRenderEdge( v0, v4 );
        Hull.AddRenderEdge( v1, v5 );
        Hull.AddRenderEdge( v2, v6 );
        Hull.AddRenderEdge( v3, v7 );

        Hull.AddRenderEdge( v8, v0 );
        Hull.AddRenderEdge( v8, v1 );
        Hull.AddRenderEdge( v8, v2 );
        Hull.AddRenderEdge( v8, v3 );
        Hull.AddRenderEdge( v8, v4 );
        Hull.AddRenderEdge( v8, v5 );
        Hull.AddRenderEdge( v8, v6 );
        Hull.AddRenderEdge( v8, v7 );

        Hull.ComputeHull();
        Hull.Render( L2W );
    }
}

//============================================================================
// Flag the texture as being used
void element_spemitter::FlagExportTextures( void )
{
    g_pTextureMgr->MarkAsUsed(m_BitmapName);
}

//============================================================================
// Activate the texture
void element_spemitter::ActivateTextures( void )
{
    g_pTextureMgr->ActivateBitmap(m_BitmapName);
}

} // namespace fx_core
