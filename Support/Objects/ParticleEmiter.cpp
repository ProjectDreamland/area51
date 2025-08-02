//==============================================================================
//
//  particle_emitter.cpp
//
//==============================================================================
//==============================================================================
//  INCLUDES
//==============================================================================

#include "objects\ParticleEmiter.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "GameLib\StatsMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Render\Render.hpp"
#include "Objects\player.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

//#define DISABLE_PARTICLES         //Disable particle effects load and init
//#define DEBUG_CHECK_GUID_TABLES   //Checks all newly created particle guids against past particle guids..
//#define PROFILE_PARTICLES         //Flag for profiling particle effects...

#define PARTICLE_EMITTER_DATA_VERSION   100

static const f32    k_logic_timeout         = 5.0f;                     // time in seconds before the particle stops updating if not seen

//anonymous linking variables for particle subsystems..
extern s32 fx_Sprite;
extern s32 fx_LinearKeyCtrl; 
extern s32 fx_SPEmitter;

//debug varaibles only for editor
#ifdef X_EDITOR
static vector3 P0;
static vector3 P1;
#endif // X_EDITOR

//=========================================================================
// Bitmap callback routines for the fx_mgr.
//=========================================================================

xbool FxLoadBitmap( const char* pBitmapName, xhandle& Handle )
{   
    rhandle<xbitmap> hBitmap;
    hBitmap.SetName( pBitmapName );
    if ( hBitmap.GetPointer() == NULL )
    {
        Handle.Handle = HNULL;
        return FALSE;
    }

    Handle.Handle = reinterpret_cast<xhandle&>(hBitmap);
    return TRUE;
}

//=========================================================================

void FxUnloadBitmap( xhandle Handle )
{
    // TODO: If the resource system gets cleaned up, here is where you
    // would decrement the ref counter or unload the data.
    (void)Handle;
}

//=========================================================================

const xbitmap* FxResolveBitmap( xhandle Handle )
{
    rhandle<xbitmap> hBitmap = reinterpret_cast< rhandle<xbitmap>& >(Handle.Handle);
    return hBitmap.GetPointer();
}

//=========================================================================
//FXO loader.. plugs the FX_Runtime library into our load code here..

static u32 s_UnitFXID = 0;

static struct fx_rsc_loader : public rsc_loader
{
    fx_rsc_loader( void ) : rsc_loader( "FX file", ".fxo" ) {};

    void* PreLoad( X_FILE* pFP )
    {
        return pFP;
    }

    void* Resolve( void* pData )
    {
        MEMORY_OWNER( "fx_rsc_loader::Resolve()" );
        X_FILE* pFP = (X_FILE*)pData;

        // make sure the fx manager goes through our resource system
        FXMgr.SetBitmapFns( FxLoadBitmap, FxUnloadBitmap, FxResolveBitmap );

        // Create a unique name
        char* pEffectName;
        {
            MEMORY_OWNER( "EFFECT NAME" );
            pEffectName = new char[ 16 ];
        }
        ASSERT( pEffectName );
        x_sprintf( pEffectName, "%d", s_UnitFXID++ );

        xbool bSuccess = FXMgr.LoadEffect( pEffectName, pFP );
        (void) bSuccess;
        ASSERT( bSuccess );

        return pEffectName;
    }

    void Unload( void* pData )
    {
        FXMgr.UnloadEffect( (char*)pData );
        delete [](char*)pData;
    }

} s_FXLoader;


//=========================================================================

//Preset particle fxo files - MUST BE IN SYNC with the preset enums
const char* particle_emitter::s_PresetPathTable[] =
{
    PRELOAD_FILE( "Bullet_Impact_Hard_000.fxo"      ),  // particle_emitter::HARD_SPARK
    PRELOAD_FILE( "Bullet_Impact_Soft_000.fxo"      ),  // particle_emitter::SOFT_SPARK
    PRELOAD_FILE( "Forcefield_impact.fxo"           ),  // particle_emitter::ENERGY_SPARK

    PRELOAD_FILE( "BloodSpray_001.fxo"              ),  // particle_emitter::DIRT_PUFF
        
    PRELOAD_FILE( "Explosion_GrenadeFrag_001.fxo"   ),  // particle_emitter::GRENADE_EXPLOSION
    
    PRELOAD_FILE( "WPN_JBG_trail_000.fxo"           ),  // particle_emitter::JUMPING_BEAN_TRAIL
    PRELOAD_FILE( "MSN_explosion.fxo"               ),  // particle_emitter::JUMPING_BEAN_EXPLOSION

    PRELOAD_FILE( "Meson_grenade_fx_000.fxo"        ),  // particle_emitter::GRAV_GRENADE_TRAIL
    PRELOAD_FILE( "Meson_grenade_fx_001.fxo"        ),  // particle_emitter::GRAV_GRENADE_EXPLOSION
    
    PRELOAD_FILE( "uber_meson_grenade.fxo"          ),  // particle_emitter::MSN_SECONDARY_TRAIL
    PRELOAD_FILE( "uber_meson_grenade_explode.fxo"  ),  // particle_emitter::MSN_SECONDARY_EXPLOSION
    PRELOAD_FILE( "MSN_explosion.fxo"               ),  // particle_emitter::MSN_PROJ_EXPLOSION
    
    PRELOAD_FILE( "IMPACT_Flesh_Pop_000.fxo"        ),  // particle_emitter::IMPACT_FLESH_POP
    PRELOAD_FILE( "IMPACT_Flesh_Cloud_000.fxo"      ),  // particle_emitter::IMPACT_FLESH_CLOUD
    PRELOAD_FILE( "IMPACT_Flesh_Hit_000.fxo"        ),  // particle_emitter::IMPACT_FLESH_HIT
    
    PRELOAD_FILE( "Dust_PunchBag.fxo"               ),  // particle_emitter::IMPACT_FABRIC_HIT
};

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct particle_emitter_desc : public object_desc
{
    particle_emitter_desc( void ) : object_desc( 
        object::TYPE_PARTICLE, 
        "Particle Object", 
        "EFFECTS",

            object::ATTR_RENDERABLE         |
            object::ATTR_TRANSPARENT        |
            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC ) {}

    //-------------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new particle_emitter;
    }

    //-------------------------------------------------------------------------

    virtual void OnEnumProp( prop_enum& List )
    {
        object_desc::OnEnumProp( List );
        List.PropEnumHeader( "ParticleDesc", "", 0 );
        List.PropEnumBool   ( "ParticleDesc\\DebugBBox",     "This tell whether we render the particles bbox", 0 );
    }
    
    //-------------------------------------------------------------------------

    virtual xbool OnProperty( prop_query&  I )
    {
        if( object_desc::OnProperty( I ) )
            return TRUE;
        
        if( I.VarBool( "ParticleDesc\\DebugBBox", s_RenderDebugParticle ) )
            return TRUE;
        
        return FALSE;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        if( Object.IsKindOf( particle_emitter::GetRTTI() ) )
        {
            particle_emitter& Emitter = particle_emitter::GetSafeType( Object );      
            if (Emitter.m_Type == particle_emitter::GENERIC_DYNAMIC_PARTICLE || 
                Emitter.m_Type == particle_emitter::UNINITIALIZED_PARTICLE)
            {
                return EDITOR_ICON_PARTICLE_EMITTER;
            }
        }  
        return -1;
    }

#endif // X_EDITOR

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    xbool s_RenderDebugParticle;

} s_Particle_Emitter_Desc;

//=========================================================================

const object_desc&  particle_emitter::GetTypeDesc     ( void ) const
{
    return s_Particle_Emitter_Desc;
}

//=========================================================================

const object_desc&  particle_emitter::GetObjectType   ( void )
{
    return s_Particle_Emitter_Desc;
}

//==============================================================================

particle_emitter::logic_pair particle_emitter::s_LogicPairTable[] = 
{
        logic_pair("ONCE",                   particle_emitter::PLAY_ONCE),
        logic_pair("REPEATED",               particle_emitter::PLAY_REPEATED),
        logic_pair("FOREVER",                particle_emitter::PLAY_FOREVER),
        logic_pair("PERSISTANT",             particle_emitter::PLAY_PERSISTANT),
        
        logic_pair( k_EnumEndStringConst,    particle_emitter::PLAY_END),  //MUST BE LAST
};

particle_emitter::endroutine_pair particle_emitter::s_EndRoutinePairTable[] = 
{
        endroutine_pair("DESTROY",            particle_emitter::END_DESTROY),
        endroutine_pair("DEACTIVATE",         particle_emitter::END_DEACTIVATE),

        endroutine_pair( k_EnumEndStringConst,    particle_emitter::END_END),  //MUST BE LAST
};

particle_emitter::visibility_pair particle_emitter::s_VisibilityPairTable[] = 
{
        visibility_pair("ALL_FORMS",      particle_emitter::VIS_ALL),
        visibility_pair("HUMAN_FORM",     particle_emitter::VIS_HUMAN),
        visibility_pair("MUTANT_FORM",    particle_emitter::VIS_MUTANT),

        visibility_pair( k_EnumEndStringConst,    particle_emitter::VIS_END),  //MUST BE LAST
};

particle_emitter::logic_table particle_emitter::s_LogicEnumTable( s_LogicPairTable );
particle_emitter::endroutine_table particle_emitter::s_EndRoutineEnumTable( s_EndRoutinePairTable );
particle_emitter::visibility_table particle_emitter::s_VisibilityEnumTable( s_VisibilityPairTable );

//==============================================================================

particle_emitter::particle_emitter(void) : 
    m_Type(UNINITIALIZED_PARTICLE),
    m_LogicType(INVALID_EMISSION_TYPE), 
    m_EndRoutine(END_DESTROY),
    m_Visibility(VIS_ALL),
    m_Color( 255, 255, 255, 255 ),
    m_PlayedOnce(FALSE),
    m_RandomWaitMin(0),
    m_RandomWaitMax(0),
    m_RandomWait(0),
    m_RepeatTimes(0),
    m_Scale(1.0f),
    
    m_OnActivate(TRUE),
    m_ParticleBBox(vector3(0.0f,0.0f,0.0f), 40.0f),
    m_bDestroyed(FALSE)
{
    m_TimeSinceLastRender = 0.0f;
    m_EmiterArea = 5000.0f;
}

//==============================================================================

particle_emitter::~particle_emitter(void)
{
    UnloadCurrentEffect();
}

//=========================================================================

void particle_emitter::OnInit( void )
{
    object::OnInit();

    m_Type          = UNINITIALIZED_PARTICLE;
    m_LogicType     = INVALID_EMISSION_TYPE;

    //NOTE :: these variables are used to resolve anonymous registration problems within the
    //FXManager..
    
    fx_Sprite        =0;
    fx_LinearKeyCtrl =0; 
    fx_SPEmitter     =0;
}

//==============================================================================

bbox particle_emitter::GetLocalBBox( void ) const 
{ 
    return m_ParticleBBox;
}
 
//=========================================================================

void particle_emitter::OnRender( void )
{

}
//=========================================================================

void particle_emitter::OnRenderTransparent( void )
{   
    CONTEXT("particle_emitter::OnRenderTransparent");

    //check the validity of the handle...
    if ( m_FxHandle.IsValid() == TRUE )
    {
        if (m_FxHandle.m_Fx.IsInstanced() && !m_OnActivate)
        {
            //this is an inactive instanced based particle, don't render
            return;
        }

        //logic to control the updates of particles..
        m_TimeSinceLastRender=0;

        switch(m_Visibility)
        {
        case VIS_ALL:   break;
        case VIS_HUMAN:
            if (g_RenderContext.m_bIsMutated)
                return;
            break;
        case VIS_MUTANT:
            if (!g_RenderContext.m_bIsMutated)
                return;
            break;
        default: ASSERT(FALSE); break;
        }

        // most particles are additive, so simply multiplying them by the
        // fog color is not good enough. If we tweak their alpha values,
        // they'll just fade with distance so the fog will overtake them.
        // This isn't quite right either, but might be "good enough".
        xcolor FogColor = render::GetFogValue( GetBBox().GetCenter(), g_RenderContext.LocalPlayerIndex );
        xcolor FxColor  = m_Color;
        FxColor.A = (u8)((f32)FxColor.A * ( (f32)(255 - FogColor.A) / 255.0f ) );
        m_FxHandle.m_Fx.SetColor( FxColor );
        
        m_FxHandle.m_Fx.Render();
    }
}

//=========================================================================

#ifndef X_RETAIL
void particle_emitter::OnDebugRender( void )
{   
    //draw the particle bonding box only if were not a preset effect...
    if ( m_Type == GENERIC_DYNAMIC_PARTICLE || m_Type == UNINITIALIZED_PARTICLE )
    {
        if( GetAttrBits() & ATTR_EDITOR_SELECTED )
        {
            draw_BBox( GetBBox(), xcolor(255,255,255) );
        }
    
        //direction of -z for particle..
        vector3 Endpoint = (GetL2W()*vector3(0,0,-500)) + vector3(0, 20, 0);
        draw_Line( GetPosition() + vector3(0, 20, 0), Endpoint, xcolor(255,0,0));

        //bbox Bounds = m_FxHandle.m_Fx.GetBounds();
        //draw_BBox( Bounds );
    }

    //bullet normal line
 //   draw_Line( P0, P1,xcolor(255,0,0) );
}
#endif // X_RETAIL

//=========================================================================

void particle_emitter::OnMove( const vector3& NewPos   )
{
    CONTEXT("particle_emitter::OnMove");

    object::OnMove( NewPos );

    if ( m_FxHandle.IsValid()== TRUE )
    {   
        m_FxHandle.m_Fx.SetTranslation( NewPos );
    }
};

//=========================================================================

void particle_emitter::OnTransform ( const matrix4& L2W )
{
    if ( m_FxHandle.IsValid()== TRUE )
    {   
        m_FxHandle.m_Fx.SetScale      ( vector3(m_Scale,m_Scale,m_Scale) );
        m_FxHandle.m_Fx.SetRotation   (  L2W.GetRotation() );
        m_FxHandle.m_Fx.SetTranslation(  L2W.GetTranslation() );
    }
    ComputeLocalBBox( L2W );

    object::OnTransform( L2W );
}

//=========================================================================
void particle_emitter::OnAdvanceLogic( f32 DeltaTime )
{  
    m_TimeSinceLastRender += DeltaTime;

    LOG_STAT(k_stats_ParticleSystem);

    CONTEXT( "particle_emitter::OnAdvanceLogic" );

    if( m_bDestroyed )
    {
        // Has the remaining particle effects died.
        if( m_FxHandle.m_Fx.IsFinished() )
        {
            g_ObjMgr.DestroyObject( GetGuid() );
        }
        else
        {
            m_FxHandle.m_Fx.AdvanceLogic(DeltaTime);
        }
        return;
    }

    if ( m_Type == INVALID_PARTICLE )
    {
        DestroyParticle();
        return;
    }

    if ( m_FxHandle.IsValid()== FALSE )
        return;

    if( m_LogicType != PLAY_ONCE && m_LogicType != PLAY_PERSISTANT )
    {
        // Get the players bbox and inflate ( range unknown right now ) 
        slot_id PlayerSlot = g_ObjMgr.GetFirst( object::TYPE_PLAYER ) ;
        xbool Intersected = FALSE;

        while ( PlayerSlot != SLOT_NULL )
        {
            player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( PlayerSlot ) ;
            if ( pPlayer && pPlayer->IsActivePlayer() )
            {
                bbox playerBBox = pPlayer->GetBBox();
                playerBBox.Inflate(m_EmiterArea,m_EmiterArea,m_EmiterArea);

                if( GetBBox().Intersect(playerBBox) )
                    Intersected = TRUE;
            }
            PlayerSlot = g_ObjMgr.GetNext( PlayerSlot ) ;
        }

        // If non intersected then return.
        if( !Intersected )
            return;
    }

    if (!m_FxHandle.m_Fx.IsInstanced())
    {
        m_FxHandle.m_Fx.SetSuspended(!m_OnActivate);
    }

    if ( !m_OnActivate )
    {
        // if a particle is suspended, we need to give it delta time so that
        // the existing particles can peter out, but we don't want to
        // restart it.
        m_FxHandle.m_Fx.AdvanceLogic(DeltaTime);
    }
    else
    {
        switch(m_LogicType)
        {
        case PLAY_ONCE:
            PlayOnceLogic(DeltaTime);
            break;
        case PLAY_REPEATED:
            PlayRepeatedLogic(DeltaTime);
            break;
        case PLAY_FOREVER:
            PlayForeverLogic(DeltaTime);
            break;
        case PLAY_PERSISTANT:
            PlayPresistantLogic(DeltaTime);
            break;
        default:
            m_FxHandle.m_Fx.AdvanceLogic(DeltaTime);
            break;
        }
    }

    // Update the bbox
    {
        bbox Bounds = m_FxHandle.m_Fx.GetBounds();
 
        //check if the box is malformed...
        if ( Bounds.Min.GetX() >= Bounds.Max.GetX() )
        {
            m_ParticleBBox = bbox(vector3(0.0f,0.0f,0.0f), 40.0f);
        }
        else
        {   
            m_ParticleBBox = SMP_UTIL_GetUntransformedBBox( GetL2W(), Bounds );
            m_ParticleBBox += bbox(vector3(0.0f,0.0f,0.0f), 40.0f);
        }

        // Make sure we upadate the bbox
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
        UpdateTransform();
    }

    // the particle update can change the bbox
    //ComputeLocalBBox( GetL2W() );
}

//=============================================================================

void particle_emitter::ComputeLocalBBox( const matrix4& L2W )
{
    if ( m_FxHandle.IsValid()== FALSE )
    {
        m_ParticleBBox = bbox(vector3(0.0f,0.0f,0.0f), 40.0f);
        return;
    }

    bbox Bounds = m_FxHandle.m_Fx.GetBounds();
 
    //check if the box is malformed...
    if ( Bounds.Min.GetX() >= Bounds.Max.GetX() )
    {
        m_ParticleBBox = bbox(vector3(0.0f,0.0f,0.0f), 40.0f);
    }
    else
    {   
        m_ParticleBBox = SMP_UTIL_GetUntransformedBBox( L2W, Bounds );
        m_ParticleBBox += bbox(vector3(0.0f,0.0f,0.0f), 40.0f);
    }

    // Make sure we update bbox
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
}

//=============================================================================

void particle_emitter::OnColCheck( void )
{
#ifdef X_EDITOR
    if (m_Type == GENERIC_DYNAMIC_PARTICLE || m_Type == UNINITIALIZED_PARTICLE || m_Type == INVALID_PARTICLE )
    {
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplyAABBox( GetBBox() );
        g_CollisionMgr.EndApply();
    }
#endif // X_EDITOR
}

//==============================================================================

void particle_emitter::OnEnumProp( prop_enum&  rPropList )
{
    object::OnEnumProp( rPropList );

    //object info
    rPropList.PropEnumString ( "Particle Emitter", 
        "Particle Emitter information.", PROP_TYPE_HEADER );

    rPropList.PropEnumExternal ( "Particle Emitter\\Audio Package", "Resource\0audiopkg","The audio package associated with this particle object.", 0 );
        
    rPropList.PropEnumBool( "Particle Emitter\\OnActivate" , "On Activate flag.", 0 );

    rPropList.PropEnumFloat( "Particle Emitter\\Scale" , 
        "Uniform Scale of the particle effect.", 0 );

    rPropList.PropEnumFloat( "Particle Emitter\\RenderArea",
        "Distance from Player to emiter that the emiter will start rendering.",0 );
    
    rPropList.PropEnumFloat( "Particle Emitter\\Max random delay" , 
        "Maximum time between possible runs of the particle effect (seconds).", 0 );

    rPropList.PropEnumFloat( "Particle Emitter\\Min random delay" , 
        "Minimum time between possible runs of the particle effect (seconds).", 0 );

    rPropList.PropEnumInt( "Particle Emitter\\Repeat Times" , 
        "How many times to play the particle, in repeated play logic.", 0 );
    
    rPropList.PropEnumInt( "Particle Emitter\\Particle Type" , 
        "", PROP_TYPE_DONT_SHOW|PROP_TYPE_DONT_SAVE );

    rPropList.PropEnumEnum( "Particle Emitter\\Particle Logic" , s_LogicEnumTable.BuildString(), "Particle logic.", PROP_TYPE_MUST_ENUM );
  
    rPropList.PropEnumExternal( "Particle Emitter\\FxFile" ,  "Resource\0fxo", "File of particle type" , PROP_TYPE_MUST_ENUM);

    rPropList.PropEnumColor ( "Particle Emitter\\Color",     "Color multiplier applied to the FX.\nAllows you to tint effects or make them darker, more transparent etc", 0 );

    if ( m_FxHandle.IsValid() == TRUE )
    {
        rPropList.PropEnumBool( "Particle Emitter\\Instanced" , "On Activate flag.", PROP_TYPE_DONT_SAVE |PROP_TYPE_DONT_EXPORT| PROP_TYPE_READ_ONLY );
    }

    rPropList.PropEnumEnum( "Particle Emitter\\Visibility" ,  s_VisibilityEnumTable.BuildString(), "What player types see this kind of particle?", 0 );
    if ((m_LogicType == PLAY_ONCE) || (m_LogicType == PLAY_REPEATED))
    {
        rPropList.PropEnumEnum( "Particle Emitter\\End Routine" , s_EndRoutineEnumTable.BuildString(), "How does this particle end?", 0 );
    }
}

//=============================================================================

xbool particle_emitter::OnProperty( prop_query& rPropQuery )
{
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );
    
    if( object::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }
 
    xbool IsRead = FALSE;

    if( rPropQuery.IsVar( "Particle Emitter\\Instanced" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_FxHandle.IsValid() && m_FxHandle.m_Fx.IsInstanced() )
            {
                rPropQuery.SetVarBool(TRUE);
            }
            else
            {
                rPropQuery.SetVarBool(FALSE);
            }
        }
        return TRUE;
    }


    // External
    if( rPropQuery.IsVar( "Particle Emitter\\Audio Package" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );
    }
        
    if ( rPropQuery.VarBool( "Particle Emitter\\OnActivate"  ,        m_OnActivate  ))
    { 
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Particle Emitter\\Max random delay"  , m_RandomWaitMax ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Particle Emitter\\Min random delay" , m_RandomWaitMin ) )
    {
        return TRUE;
    }
        
    if ( rPropQuery.VarFloat( "Particle Emitter\\Scale" , m_Scale ) )
    {
        if( !rPropQuery.IsRead() )
        {
            ResizeParticleEffect();
        }
        return TRUE;
    }

    if( rPropQuery.VarFloat ("Particle Emitter\\RenderArea", m_EmiterArea ) )
    {
        return TRUE;
    }
      
    if ( rPropQuery.VarInt(  "Particle Emitter\\Particle Type" , *((s32*)&m_Type) ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarInt(  "Particle Emitter\\Repeat Times" , m_RepeatTimes ) )
    {
        return TRUE;
    }

    if ( SMP_UTIL_IsEnumVar<particle_logic_type,particle_logic_type>(rPropQuery, "Particle Emitter\\Particle Logic", m_LogicType, s_LogicEnumTable ) )
    {
        return TRUE;
    }
    
    if ( SMP_UTIL_IsEnumVar<particle_endroutine_type,particle_endroutine_type>(rPropQuery, "Particle Emitter\\End Routine", m_EndRoutine, s_EndRoutineEnumTable ) )
    {
        return TRUE;
    }

    if ( SMP_UTIL_IsEnumVar<particle_visibility_type,particle_visibility_type>(rPropQuery, "Particle Emitter\\Visibility", m_Visibility, s_VisibilityEnumTable ) )
    {
        return TRUE;
    }

#ifdef X_EDITOR
    //for editor build external handling
   
    if (SMP_UTIL_IsExternalVar(rPropQuery, "Particle Emitter\\FxFile", m_FxSourceFile, IsRead))
    {
        if (!IsRead)
        {
            if (!m_FxSourceFile.IsEmpty())
                InitGenricParticle( m_FxSourceFile.Get() );
        }

        return TRUE;
    }
#endif // X_EDITOR

#if defined(TARGET_PS2) || defined(TARGET_XBOX)
    //on the PS2 we do not need to keep around the resource string, just use it to init ourselves..

    med_string TmpStr;

    if (SMP_UTIL_IsExternalVar(rPropQuery, "Particle Emitter\\FxFile", TmpStr, IsRead))
    {
        if (!IsRead)
        {
            if (!TmpStr.IsEmpty())
                InitGenricParticle( TmpStr.Get() );
        }

        return TRUE;
    }
#endif

    if( rPropQuery.VarColor( "Particle Emitter\\Color", m_Color ) )
        return TRUE;

    return( FALSE );
}

//=============================================================================

void particle_emitter::SetScale( f32 NewScale )
{
    // Don't set this to zero.
    // A zero scale can cause a zero matrix to be build and if a rotation
    // is extracted from that, you will end up with a gimbal lock situation
    // and have a QNAN special as a roll value.
    NewScale = MAX(0.001f,NewScale);
    m_Scale = NewScale;
    ResizeParticleEffect();
}

//=============================================================================
void particle_emitter::ResizeParticleEffect ( void )
{ 
    const matrix4 L2W = GetL2W();
 
    matrix4 M(vector3(m_Scale,m_Scale,m_Scale), L2W.GetRotation(), L2W.GetTranslation());

    OnTransform( M );
}

//=============================================================================

void particle_emitter::InitGenricParticle( const char*  rParticleFxd )
{
    UnloadCurrentEffect();
    
#ifdef DISABLE_PARTICLES
    return;
#endif

    ASSERT( rParticleFxd );

    if (rParticleFxd[0] == '\0')
    {
        m_Type = INVALID_PARTICLE;
        return;
    }
    
   //make sure to unload the current effect... 
    // CJ: Why was this done twice? UnloadCurrentEffect();

    //try and load the particle type using the rParticleFxd name
    m_FxHandle.Init( rParticleFxd );
    
    if (m_FxHandle.IsValid())
    {
        m_Type = GENERIC_DYNAMIC_PARTICLE;
        m_FxHandle.m_Fx.SetScale        ( vector3(m_Scale,m_Scale,m_Scale) );
        m_FxHandle.m_Fx.SetRotation     ( GetL2W().GetRotation() );
        m_FxHandle.m_Fx.SetTranslation  ( GetPosition() );
    }
    else
    {
        m_Type = INVALID_PARTICLE;
    }
}

//=============================================================================

void particle_emitter::InitPresetParticle( particle_type ParticleType )
{
#ifdef DISABLE_PARTICLES
    m_Type = INVALID_PARTICLE;
    return;
#endif

    //INVALID_PARTICLE particles will immediately be destroyed in the 
    //OnAdvanceLogic call...
    
    //using the type of particle get the fx handle..
    
    //only preset particles can be init by this function
    if (m_Type == GENERIC_DYNAMIC_PARTICLE)
    { 
        m_Type = GENERIC_DYNAMIC_PARTICLE;
        return;
    }

    //invalid types....
    if ( ParticleType == INVALID_PARTICLE )
    { 
        m_Type = INVALID_PARTICLE;
        return;
    }
    
    //not invalid but special case for editor...
    if ( ParticleType == UNINITIALIZED_PARTICLE )
    {
        m_Type = UNINITIALIZED_PARTICLE;
        return;
    }

    //make sure to unload the current effect... 
    UnloadCurrentEffect();
    
    // Use a preset particle?
    if( ( ParticleType >= 0 ) && ( ParticleType < PRESET_COUNT ) )
    {
        ASSERTS( ( sizeof( s_PresetPathTable ) / sizeof( s_PresetPathTable[ 0 ] ) ) == PRESET_COUNT, 
                "s_PresetPathTable is out of sync with enums!!!" );
        
        // Try init
        m_FxHandle.Init( s_PresetPathTable[ ParticleType ] );
        if (m_FxHandle.IsValid())
        {
            m_Type = ParticleType;
                 
            m_FxHandle.m_Fx.SetScale        ( vector3(m_Scale,m_Scale,m_Scale) );
            m_FxHandle.m_Fx.SetRotation     ( GetL2W().GetRotation() );
            m_FxHandle.m_Fx.SetTranslation  ( GetPosition() );
        }
        else
        {
            m_Type = INVALID_PARTICLE;
        }
    }
    else
    {
        m_Type = INVALID_PARTICLE;
    }
}

//=============================================================================

void particle_emitter::UnloadCurrentEffect( void )
{
    //will kill the effect

    m_FxHandle.Unload();
}

//=============================================================================

void particle_emitter::DestroyParticle( void )
{
    m_bDestroyed = TRUE;
    if ( m_FxHandle.IsValid() && !m_FxHandle.m_Fx.IsInstanced() )
        m_FxHandle.m_Fx.SetSuspended( TRUE );    
}

//=============================================================================

xbool particle_emitter::IsDestroyed( void )
{
    return m_bDestroyed;
}

//=============================================================================

void particle_emitter::StopParticle( void )
{
    if (m_EndRoutine == END_DESTROY)
        DestroyParticle();
    else
        OnActivate(FALSE);
}

//=============================================================================

void particle_emitter::PlayOnceLogic( f32 DeltaTime )
{
    //Plays the particle effect once and stops when the effect is done...

    if (m_PlayedOnce == TRUE)
    {
        StopParticle();
        return;
    }

    m_FxHandle.m_Fx.AdvanceLogic( DeltaTime );

    if (m_FxHandle.m_Fx.IsFinished() == TRUE)
        m_PlayedOnce = TRUE;
}

//=============================================================================

void particle_emitter::PlayRepeatedLogic( f32 DeltaTime )
{
    //plays the effect N number of times with some randomness thrown inbetween runs...

    m_RandomWait -= DeltaTime;

    if (m_RandomWait > 0)
        return;

    if (m_RepeatTimes <= 0)
    {
        StopParticle();
        return;
    }

    m_FxHandle.m_Fx.AdvanceLogic( DeltaTime );
    
    if (m_FxHandle.m_Fx.IsFinished() == TRUE)
    {
        m_RepeatTimes--;
        m_RandomWait = x_frand( m_RandomWaitMin, m_RandomWaitMax );
        m_FxHandle.m_Fx.Restart();
    }
}

//=============================================================================

void particle_emitter::PlayForeverLogic( f32 DeltaTime )
{
    //plays the effect forever with some randomness thrown inbetween runs...
    
    m_RandomWait -= DeltaTime;

    if (m_RandomWait > 0)
        return;
    
    m_FxHandle.m_Fx.AdvanceLogic( DeltaTime );
    
    if (m_FxHandle.m_Fx.IsFinished() == TRUE)
    {
        m_RandomWait = x_frand( m_RandomWaitMin, m_RandomWaitMax );
        m_FxHandle.m_Fx.Restart();
    }
}

//=============================================================================

void particle_emitter::PlayPresistantLogic ( f32 DeltaTime )
{
    //plays the effect once and then waits for restart or until the object is destroyed...
    
    if (m_PlayedOnce == TRUE)
        return;
    
    m_FxHandle.m_Fx.AdvanceLogic( DeltaTime );
    
    if (m_FxHandle.m_Fx.IsFinished() == TRUE)
        m_PlayedOnce = TRUE;
}

//=============================================================================

void particle_emitter::Restart( void )
{
    if ( m_FxHandle.IsValid()== TRUE )
    {
        m_FxHandle.m_Fx.Restart();
        m_TimeSinceLastRender = 0;
        SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
        m_PlayedOnce = FALSE;
    }
}

//=============================================================================

void particle_emitter::SetParticleType( particle_type ParticleType )
{
    m_Type          = ParticleType;

    InitPresetParticle( m_Type );
}

//=============================================================================

void particle_emitter::SetParticleType( const char* pFXName )
{
    InitGenricParticle( pFXName );
}

//=============================================================================

void particle_emitter::SetLogicType( particle_logic_type LogicType )
{
    m_LogicType = LogicType;

    //PLAY_PERSISTANT types start in non-playing state


    if (LogicType == PLAY_PERSISTANT)
    {
        m_PlayedOnce = TRUE;
    }
} 
    
//=============================================================================
// Interface functions for creating particle effects at runtime..

guid  particle_emitter::CreateParticleEmitter   ( particle_type ParticleType, particle_logic_type LogicType )
{
    guid Guid = g_ObjMgr.CreateObject( particle_emitter::GetObjectType() );
    particle_emitter& Particle = particle_emitter::GetSafeType( *((particle_emitter*)g_ObjMgr.GetObjectByGuid( Guid )) );
    
    Particle.SetParticleType( ParticleType );
    Particle.SetLogicType   ( LogicType   );

    return Guid;
}

//=============================================================================

guid  particle_emitter::CreateParticleEmitter   ( const char* pFXName, particle_logic_type LogicType )
{
    guid Guid = g_ObjMgr.CreateObject( particle_emitter::GetObjectType() );
    particle_emitter& Particle = particle_emitter::GetSafeType( *((particle_emitter*)g_ObjMgr.GetObjectByGuid( Guid )) );
    
    Particle.SetParticleType( pFXName   );
    Particle.SetLogicType   ( LogicType );

    return Guid;
}

//=============================================================================

guid    particle_emitter::CreatePresetParticle    ( particle_type ParticleType, u16 Zone1 /* = 0  */)
{
    ASSERT( ParticleType != GENERIC_DYNAMIC_PARTICLE ); //generic dynamic particles need to use CreateGenericParticle
   
    guid ParticleGuid = CreateParticleEmitter( ParticleType, PLAY_ONCE);

    object_ptr<object> ParticleObj ( ParticleGuid );

    if (ParticleObj.IsValid())
    { 
        ParticleObj.m_pObject->SetZone1( Zone1 );
    }

    return ParticleGuid;
}

//=============================================================================

guid particle_emitter::CreateGenericParticle ( const char* rParticlePath, u16 Zone1 /* = 0  */)
{
    guid Guid = g_ObjMgr.CreateObject( particle_emitter::GetObjectType() );
    particle_emitter& Emit = particle_emitter::GetSafeType( *((particle_emitter*)g_ObjMgr.GetObjectByGuid( Guid )) );
    
    Emit.SetParticleType( GENERIC_DYNAMIC_PARTICLE );
    Emit.SetLogicType   ( PLAY_ONCE   );
    
    Emit.InitGenricParticle( rParticlePath );
    Emit.SetZone1( Zone1 );

    return Guid;
}

//===========================================================================

void particle_emitter::InitParticleFromName( const char* pName )
{
    InitGenricParticle( pName );
    m_LogicType = PLAY_FOREVER;
}
//=============================================================================

void particle_emitter::CreateProjectileCollisionEffect( const collision_mgr::collision& rCollision, guid OwnerGuid )
{

#ifdef DISABLE_PARTICLES
    return;
#endif

#ifdef X_EDITOR

    //DEBUG line based upon the normal of the collision
    
    P0 = rCollision.Point;
    P1 = P0 + 100*rCollision.Plane.Normal;

#endif // X_EDITOR

    //Handle the particle effects of the collsion...

    //NOTE : use the negative normal becuase by convention the particle effect shoots out from -z which, 
    //so in oritentatiing the particle effect to the positive normal would cause the particle effects to shoot into
    //the walls...

    object_ptr<object> CollisionObj(rCollision.ObjectHitGuid);

    u16 CollisionZone1 = CollisionObj.IsValid() ? CollisionObj.m_pObject->GetZone1() : 0;
    
    ProcessCollisionSfx( rCollision.Flags, rCollision.Point, CollisionZone1, OwnerGuid );
     
    switch ( rCollision.Flags )
    {
        //hard surfaces/////////////////////////////////////////////
    case MAT_TYPE_ROCK:
    case MAT_TYPE_CONCRETE:
    case MAT_TYPE_SOLID_METAL:
    case MAT_TYPE_HOLLOW_METAL:    
    case MAT_TYPE_METAL_GRATE:
        {
            // Create the Spark Particle Effect 
            particle_emitter::CreatePresetParticleAndOrient( 
                    particle_emitter::HARD_SPARK, 
                    -rCollision.Plane.Normal, 
                    rCollision.Point,
                    CollisionZone1);
        }

        break;
        
        //soft surfaces/////////////////////////////////////////////
    case MAT_TYPE_PLASTIC:
    case MAT_TYPE_WATER:
    case MAT_TYPE_WOOD:
    case MAT_TYPE_ICE:
    case MAT_TYPE_EARTH:
        {
           // Create the Spark Particle Effect 
            particle_emitter::CreatePresetParticleAndOrient( 
                    particle_emitter::SOFT_SPARK, 
                    -rCollision.Plane.Normal, 
                    rCollision.Point,
                    CollisionZone1);
        }
        break;
        
        //flesh surfaces////////////////////////////////////////////
    case MAT_TYPE_LEATHER:
    case MAT_TYPE_EXOSKELETON:
    case MAT_TYPE_FLESH:
    case MAT_TYPE_BLOB:
        {
            // Particle effects should be played by the object reciving the
            // OnPain call...
        }
        break;
        
        //glass surfaces////////////////////////////////////////////
    case MAT_TYPE_BULLET_PROOF_GLASS:
    case MAT_TYPE_GLASS:
        {
            // Create the Spark Particle Effect 
            particle_emitter::CreatePresetParticleAndOrient( 
                    particle_emitter::HARD_SPARK, 
                    -rCollision.Plane.Normal, 
                    rCollision.Point,
                    CollisionZone1);
        }
        break;

        //alien force fields/////////////////////////////////////////
    case MAT_TYPE_ENERGY_FIELD:
        {
            // Create the force-field spark effect
            particle_emitter::CreatePresetParticleAndOrient(
                    particle_emitter::ENERGY_SPARK,
                    -rCollision.Plane.Normal,
                    rCollision.Point,
                    CollisionZone1);
        }
        break;

        //undefined surfaces/////////////////////////////////////////
    case MAT_TYPE_FIRE:
    case MAT_TYPE_GHOST:
    case MAT_TYPE_FABRIC:
    case MAT_TYPE_CERAMIC:
    case MAT_TYPE_WIRE_FENCE:
    default:
        //no-op
        break;
    }
}

//=============================================================================

void particle_emitter::ProcessCollisionSfx ( s32 MaterialType, const vector3& Position, u16 ZoneId, guid OwnerGuid )
{
   const char* pName = NULL;
    switch( MaterialType )
    {
        case MAT_TYPE_EARTH:              pName = "BulletImpactConcrete";           break;
        case MAT_TYPE_ROCK:               pName = "BulletImpactRock";               break;
        case MAT_TYPE_CONCRETE:           pName = "BulletImpactConcrete";           break;
        case MAT_TYPE_SOLID_METAL:        pName = "BulletImpactMetal";              break;
        case MAT_TYPE_HOLLOW_METAL:       pName = "BulletImpactHollowMetal";        break;
        case MAT_TYPE_METAL_GRATE:        pName = "BulletImpactMetalGrate";         break;
        case MAT_TYPE_PLASTIC:            pName = "BulletImpactPlastic";            break;
        case MAT_TYPE_WATER:              pName = "BulletImpactWater";              break;
        case MAT_TYPE_WOOD:               pName = "BulletImpactWood";               break;
        case MAT_TYPE_ENERGY_FIELD:       pName = "BulletImpactEnergyField";        break;
        case MAT_TYPE_BULLET_PROOF_GLASS: pName = "BulletImpactBulletProofGlass";   break;
        case MAT_TYPE_ICE:                pName = "BulletImpactIce";                break;
        case MAT_TYPE_LEATHER:            pName = "BulletImpactFlesh";              break;
        case MAT_TYPE_EXOSKELETON:        pName = "BulletImpactFlesh";              break;
        case MAT_TYPE_FLESH:              pName = "BulletImpactFlesh";              break;
        case MAT_TYPE_BLOB:               pName = "BulletImpactFlesh";              break;
        case MAT_TYPE_FABRIC:             pName = "BulletImpactFabric";             break;
        case MAT_TYPE_CERAMIC:            pName = "BulletImpactCeramic";            break;
        case MAT_TYPE_GLASS:              pName = "BulletImpactGlass";              break;
        case MAT_TYPE_RUBBER:             pName = "BulletImpactRubber";             break;
    }

    if( pName )
    {
        voice_id VoiceID = g_AudioMgr.Play( pName, Position, ZoneId, TRUE );
        g_AudioManager.NewAudioAlert( VoiceID, audio_manager::BULLET_IMPACTS, Position, ZoneId, OwnerGuid );
    }
}

//=============================================================================

void particle_emitter::CreateOnPainEffect( const pain&          Pain,            
                                                 f32            DisplaceAmount,
                                                 particle_type  ParticleType,
                                                 xcolor         Color )
{
#ifdef DISABLE_PARTICLES
    return;
#endif
    
    // SB:
    // TO DO - Could query Pain to find out what material was hit so this function
    //         could be generic - currently it's only used by actors so it is assumed
    //         that the material is flesh.
    
    // Dynamically choose particle type based on pain?
    if( ParticleType == UNINITIALIZED_PARTICLE )
    {
        switch( Pain.GetHitType() )
        {
            case 0:
                ParticleType = IMPACT_FLESH_CLOUD;
                break;

            case 1:
            case 2:
                ParticleType = IMPACT_FLESH_POP;
                break;

            default:
                ParticleType = IMPACT_FLESH_HIT;
                break;            
        }
    }

    // Create particle
    guid ParticleGuid = CreatePresetParticle( ParticleType );
    ASSERT( ParticleGuid != NULL_GUID );
    
    // Lookup particle object
    particle_emitter* pParticle = (particle_emitter*)g_ObjMgr.GetObjectByGuid( ParticleGuid );
    ASSERT( pParticle );

    // Set color
    pParticle->SetColor( Color );
            
    // Align         
    SMP_UTIL_PointObjectAt( pParticle, 
                            Pain.GetDirection(), 
                            Pain.GetImpactPoint() + ( DisplaceAmount * Pain.GetDirection() ), 
                            0.0f );
     
    return;
}

//=============================================================================

guid particle_emitter::CreatePresetParticleAndOrient (
                                                      particle_type     ParticleType, 
                                                      const vector3&    rDir, 
                                                      const vector3&    rPos, 
                                                      u16               Zone1 /* = 0  */)
{
    guid ParticleGuid = CreatePresetParticle( ParticleType );

    ASSERT( ParticleGuid != NULL_GUID );
    
    object* pObject = g_ObjMgr.GetObjectByGuid( ParticleGuid );

    SMP_UTIL_PointObjectAt( pObject, rDir, rPos, 0.0f );

    pObject->SetZone1( Zone1 );
    
    return ParticleGuid;
}

//=============================================================================

guid particle_emitter::CreatePresetParticleAndOrient ( const char* rParticlePath, const vector3& rDir, 
                                                      const vector3& rPos, u16 Zone1 )
{
    // Create a particle object using the particle name and set its position and rotation.
    guid ParticleGuid = g_ObjMgr.CreateObject( particle_emitter::GetObjectType() );
    object* pObject = g_ObjMgr.GetObjectByGuid( ParticleGuid );
    particle_emitter& Emit = particle_emitter::GetSafeType( *pObject );

    Emit.SetParticleType( GENERIC_DYNAMIC_PARTICLE );
    Emit.SetLogicType   ( PLAY_ONCE   );
    
    if( x_stristr( rParticlePath, ".fxo" ) == NULL )
        Emit.InitGenricParticle( xfs( "%s.fxo", rParticlePath) );
    else
        Emit.InitGenricParticle( rParticlePath );

    SMP_UTIL_PointObjectAt( pObject, rDir, rPos, 0.0f );
    Emit.SetZone1( Zone1 );

    return ParticleGuid;
}

//=============================================================================

guid particle_emitter::CreatePersistantParticle ( particle_type ParticleType, u16 Zone1 /* = 0  */ )
{
    ASSERT( ParticleType != GENERIC_DYNAMIC_PARTICLE ); //generic dynamic particles need to use CreateGenericParticle
 
    guid ParticleGuid = CreateParticleEmitter( ParticleType, PLAY_PERSISTANT );

    ASSERT( ParticleGuid );

    object_ptr<object> ParticleObj(ParticleGuid);

    if (ParticleObj.IsValid())
    {
        ParticleObj.m_pObject->SetZone1( Zone1 );
    }

    return ParticleGuid;
}

//=============================================================================

guid particle_emitter::CreatePersistantParticle ( const char* pFXName, u16 Zone1 /* = 0  */ )
{
    guid ParticleGuid = CreateParticleEmitter( pFXName, PLAY_PERSISTANT );

    ASSERT( ParticleGuid );

    object_ptr<object> ParticleObj(ParticleGuid);

    if (ParticleObj.IsValid())
    {
        ParticleObj.m_pObject->SetZone1( Zone1 );
    }

    return ParticleGuid;
}

//=============================================================================

void particle_emitter::OnActivate ( xbool Flag )
{
    if ( m_FxHandle.IsValid() && !m_FxHandle.m_Fx.IsInstanced() )
        m_FxHandle.m_Fx.SetSuspended(!Flag);
    m_OnActivate = Flag;
    m_PlayedOnce = FALSE;
}
















