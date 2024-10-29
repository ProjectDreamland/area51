//==============================================================================
///
// bomb_object.cpp
//
// Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
// Bomb object
//
//==============================================================================


//=============================================================================
// INCLUDES
//=============================================================================

#include "bomb_object.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "audiomgr\audiomgr.hpp"

#include "Render\Render.hpp"
#include "Debris\Debris_mgr.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "objects\player.hpp"

//=============================================================================
// DEFINES
//=============================================================================
#define ON_ADVANCE_LOGIC_SECOND 1.0f 
#define LEFT_DIGIT      0
#define MIDDLE_DIGIT    1
#define RIGHT_DIGIT     2
#define SCREEN_COVER    3
#define SCREEN_BLANK    4
#define SCREEN_COLON    5
#define SCREEN_HAND     6

bomb_object::BombState_Pair bomb_object::s_BombStatePairs[] = 
{
        BombState_Pair("Bomb Active",                                  bomb_object::BOMB_ACTIVE),
        BombState_Pair("Bomb Not Active",                              bomb_object::BOMB_NOT_ACTIVE),
        BombState_Pair("Bomb Disarmed",                                bomb_object::BOMB_DISARMED),     
        BombState_Pair("Bomb Exploding",                               bomb_object::BOMB_EXPOLDING),
        BombState_Pair("Bomb Detonated",                               bomb_object::BOMB_DETONATED),
        BombState_Pair("Bomb Triggered",                               bomb_object::BOMB_TRIGGERED),
        BombState_Pair( k_EnumEndStringConst,                          bomb_object::BOMB_END),  //MUST BE LAST
};

bomb_object::BombState_Table bomb_object::s_BombStateTable( s_BombStatePairs );


//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

static struct bomb_object_desc : public object_desc
{
    //-------------------------------------------------------------------------

    bomb_object_desc( void ) : object_desc( 
        object::TYPE_BOMB, 
        "Bomb Object",
        "PROPS",
        object::ATTR_RENDERABLE             | 
        object::ATTR_COLLIDABLE             | 
        object::ATTR_BLOCKS_ALL_PROJECTILES | 
        object::ATTR_BLOCKS_ALL_ACTORS      | 
        object::ATTR_BLOCKS_RAGDOLL         | 
        object::ATTR_BLOCKS_SMALL_DEBRIS    | 
        object::ATTR_DAMAGEABLE             |
        object::ATTR_SPACIAL_ENTRY          |
        object::ATTR_NEEDS_LOGIC_TIME,

        FLAGS_IS_DYNAMIC | FLAGS_TARGETS_OBJS | FLAGS_GENERIC_EDITOR_CREATE |
        FLAGS_NO_ICON|
        FLAGS_BURN_VERTEX_LIGHTING 
        ) 
        {}
    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new bomb_object;
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_bomb_object_Desc;

//=============================================================================

const object_desc&  bomb_object::GetTypeDesc( void ) const
{
    return s_bomb_object_Desc;
}

//=============================================================================

const object_desc&  bomb_object::GetObjectType( void )
{
    return  s_bomb_object_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

bomb_object::bomb_object(void) : destructible_obj()
{
    SetHUDEventState(HUD_BOMB_NOT_ACTIVE);
    m_EventBombState = BOMB_INVALID;


    m_EventTimeStamp    = 0;
    m_DeactivateTime    = 1.0f;  //in seconds
    m_EventTimerVector  = m_DeactivateTime / 12.0f;
    m_EventUnit         = 0;
    m_HUDBombTimer      = 0.0f;
    m_BombTimer         = 0.0f;
    m_DetonateTimeMinutes = 1.0f;
    m_DetonateTimeSeconds = 0.0f;
    x_memset(m_iMesh, -1, sizeof(m_iMesh)) ;  // Set all mesh indices to -1
    m_BombExplodState = false;
    m_DisarmSound         = NULL_GUID;
    m_CountDownSound = NULL_GUID;
    m_BombAboutToExplodeSound = NULL_GUID;

    m_BombTimerMinutes = (u32)((f32)m_DetonateTimeMinutes);           
    m_BombTimerSeconds = (u32)((f32)m_DetonateTimeSeconds);
    TurnOnBlankSreenMesh();

    m_ScreenShakeTime   = 0.0f;
    m_ScreenShakeAmount = 0.0f;
    m_ScreenShakeSpeed  = 0.0f;
    m_FeedBackIntensity = 0.0f;
    m_FeedBackDuration  = 0.0f;
    m_DisarmBlink = 0.0f;
    m_BlinkDisarmFlag = FALSE;
    
    m_UseScreenShake    = FALSE;
}

//=============================================================================

bomb_object::~bomb_object(void)
{

}

//=============================================================================

bbox bomb_object::GetLocalBBox( void ) const 
{
   return destructible_obj::GetLocalBBox();
}

//=============================================================================

s32 bomb_object::FindCorrectMesh(const char* pName )
{
    // Lookup geometry
    const geom* pGeom = m_Inst.GetGeom() ;
 
    // No geoetry?
    if (!pGeom)
        return -1 ;

    // Search for a mesh name
    for (s32 i = 0 ; i < pGeom->m_nMeshes ; i++)
    {
        if (x_stricmp(pGeom->GetMeshName( i ), pName) == 0)
            return i;
    }

    return -1;
}

//=============================================================================

void bomb_object::OnRender( void )
{
    CONTEXT( "bomb_object::OnRender" );

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        if ( pRigidGeom->m_nBones > 1 )
        {
            x_throw( xfs( "bomb_object can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
        }
        else
        {
            DisplayBombCount(m_BombTimerMinutes, m_BombTimerSeconds);
            m_Inst.Render( &GetL2W(), Flags | GetRenderMode(), m_MeshMask);
        }
    }
    else
    {
#ifdef X_EDITOR        
        draw_BBox( GetBBox() );
#endif
    }
}

//=============================================================================

xbool bomb_object::RenderDigit( s32 iMesh, u32 Digit )
{

    // DS: 5/21/04 - this object won't work quite right anymore due to changes
    // in the way we do uv animations. Since it's not used currently, I'll just
    // comment this section out, and we can revisit later if necessary
    (void)iMesh;
    (void)Digit;
    return FALSE;

    /*
    // Mesh not found?
    if (iMesh == -1)
        return FALSE;

    // No geometry?
    geom* pGeom = m_Inst.GetGeom() ;
    if (!pGeom)
        return FALSE;

    // Get sub mesh
    s32 iSubMesh = pGeom->m_pMesh[iMesh].iSubMesh;
    
    // Lookup material on this submesh
    s32 iMaterial = pGeom->m_pSubMesh[iSubMesh].iMaterial;

    // Lookup uv anim
    render::tex_anim      Anim;
    render::hgeom_inst    hInst = m_Inst.GetInst() ;
    render::htexanim_inst hTexInst = render::GetTexAnimData(hInst);
    if (render::GetUVAnim( hTexInst, iMesh, iMaterial, Anim ) )
    {
        Anim.Type   = render::tex_anim::FIXED;
        Anim.iStart = Digit;
        render::SetUVAnim( hTexInst,  iSubMesh, iMaterial , Anim );
    }

    // Success
    return TRUE;
    */
}

//=============================================================================

void bomb_object::DisplayBombCount( u32 Minutes, u32 Seconds)
{
    // Render all digits
    RenderDigit( m_iMesh[LEFT_DIGIT],   Minutes      );
    RenderDigit( m_iMesh[MIDDLE_DIGIT], Seconds / 10 );
    RenderDigit( m_iMesh[RIGHT_DIGIT],  Seconds % 10 );
}

//=============================================================================

void bomb_object::OnActivate ( xbool Flag )
{
    // Call base class
    object::OnActivate(Flag);
    
    // Trigger bomb?
    if(Flag)
    {
        SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
    }
    else
    {
        SetAttrBits( GetAttrBits() | ~object::ATTR_NEEDS_LOGIC_TIME );
        SetBombEventState(BOMB_NOT_ACTIVE);  
    }
}

//=============================================================================

void bomb_object::OnAdvanceLogic( f32 DeltaTime )
{
    // Update states
    UpdateHUDBombStates    ( GetHUDEventState(), DeltaTime);
    UpdateBombStates       ( GetBombEventState(), DeltaTime );

    // Call base class
    destructible_obj::OnAdvanceLogic( DeltaTime );


    //see if bomb got shot and blew itself up
    if((destructible_obj::GetHealth()) <= 0.0f)
    {
        //bomb has destroyed itself
        SetBombEventState(BOMB_DETONATED);
    }
    // Perform state logic
    s32 State = GetBombEventState();
    switch(State)
    {
        case BOMB_DETONATED:
            //Bomb has been detonated take it off the render list
            SetAttrBits( GetAttrBits() & ~object::ATTR_RENDERABLE);
        break;
        case BOMB_DISARMED:
            //lets keep getting onAdvance while bomb blinks          
            if(!m_BlinkDisarmFlag)
            {
                if(!GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME)
                    SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
            }
 
        break;
        case BOMB_NOT_ACTIVE:
        break;

    default:
        //make sure destruct object doesn't take out attr_needs_logic_time bit
       if(!GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME)
           SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
    }
} 

//=============================================================================

void bomb_object::UpdateHUDBombStates( s32 HUDEventState, f32 DeltaTime)
{
    // Update timers
    m_HUDBombTimer +=DeltaTime;

    // Do HUD states
    switch( HUDEventState )
    {
        case HUD_BOMB_NOT_ACTIVE:
            m_EventUnit = 0;
 
            m_EventTimerVector = m_DeactivateTime / 12.0f;
            m_EventTimeStamp   = m_HUDBombTimer;

            if( m_DisarmSound != 0 )
            {
                g_AudioMgr.Release( m_DisarmSound, 0.0f );     
                m_DisarmSound = 0;
            }
        break;

        case HUD_BOMB_DISARMING:
            if( (m_HUDBombTimer - m_EventTimeStamp) > m_EventTimerVector)
            {
                m_EventTimeStamp = m_HUDBombTimer;
        
                // Start Looping Disarming Sound
                if( m_EventUnit == 0 )
                    m_DisarmSound = g_AudioMgr.Play( "Bomb_Disarming_Loop", GetPosition(), TRUE, TRUE );     

                m_EventUnit++;

                if( m_EventUnit > 12 )
                {
                    if( m_DisarmSound != 0 )
                    {
                        g_AudioMgr.Release( m_DisarmSound, 0.0f );     
                        m_DisarmSound = 0;
                    }

                    m_EventUnit = 0;
                    if(GetBombEventState() == BOMB_EXPOLDING)
                        SetHUDEventState(HUD_BOMB_DETONATED);
                    else
                    {
                        g_AudioMgr.Play( "Bomb_Defused", GetPosition(), TRUE, TRUE );     
                        SetHUDEventState(HUD_BOMB_DISARMED);
                        SetBombEventState(BOMB_DISARMED);
                        m_DisarmBlink = 0.0f;
                        m_BlinkDisarmFlag = FALSE;
                        TurnOnBlankSreenMesh();

                    }

                }
            }
        break;

        case HUD_BOMB_DISARMED:
            m_HUDBombTimer = 0.0f;
            //-- YAY
        break;

        case HUD_BOMB_DETONATED:
            m_HUDBombTimer = 0.0f;
            //-- OH NO!
        break;

        default:
        break;
    }
}  

//=============================================================================

void bomb_object::UpdateBombStates( s32 BombEventState, f32 DeltaTime )
{
    // Do Bomb states
    switch( BombEventState )
    {
    case    BOMB_NOT_ACTIVE: 
       object::OnActivate(FALSE);
    break;

    case    BOMB_ACTIVE: 
        TurnOnDigitMeshes();
    break;
    case    BOMB_COUNTING_DOWN:

        // Display countdown
        if(DeltaTime + m_BombTimer >= ON_ADVANCE_LOGIC_SECOND) 
        {
            m_BombTimer =0.0f;

            if(m_BombTimerSeconds > 0)
            {
                m_CountDownSound = g_AudioMgr.Play( "Bomb_CountDownTick", GetPosition(), TRUE, TRUE );
                m_BombTimerSeconds--;
            }
            else if(m_BombTimerMinutes > 0)
            {
                m_CountDownSound = g_AudioMgr.Play( "Bomb_CountDownTick", GetPosition(), TRUE, TRUE );     
                m_BombTimerMinutes--;
                m_BombTimerSeconds = 59;
            }      
        }

        m_BombTimer += DeltaTime;
        
        if(m_BombTimerMinutes == 0 && m_BombTimerSeconds == 0)
        {
            //if(!g_AudioManager.IsValidVoice(m_BombAboutToExplodeSound))
                m_BombAboutToExplodeSound = g_AudioMgr.Play( "Bomb_AboutToExplode", GetPosition(), TRUE, TRUE );     

            //turn off meshes to prepare for explosion
            SetBombEventState(BOMB_EXPOLDING);
            m_BombExplodState = true;

            //turn of countdown meshes
            //to display have a nice day
            TurnOnAboutToExpoldeMesh();
            m_BombTimer = 0.0f;
        }
    break;
    case    BOMB_DISARMED:
        if(DeltaTime + m_DisarmBlink >= (ON_ADVANCE_LOGIC_SECOND / 2)) 
        {
            TurnOnDigitMeshes();
            //turn off onadvanced logic routine
            object::OnActivate(FALSE);
            SetHUDEventState( HUD_BOMB_DISARMED );
            m_BlinkDisarmFlag = TRUE;
        }
        m_DisarmBlink += DeltaTime;

    break;
    case    BOMB_EXPOLDING:
      if(DeltaTime + m_BombTimer >= ON_ADVANCE_LOGIC_SECOND * 2) 
        {
            destructible_obj::m_Health = 0.0f; //explode

            if( m_UseScreenShake )
            {
                //--- 
                player* pPlayerObj = SMP_UTIL_GetActivePlayer();
                if ( pPlayerObj )
                {
                    pPlayerObj->ShakeView(m_ScreenShakeTime, m_ScreenShakeAmount, m_ScreenShakeSpeed );

                    //force feedback
                    input_Feedback(m_FeedBackDuration, m_FeedBackIntensity, pPlayerObj->GetActivePlayerPad() );
                }
            }

            SetBombEventState(BOMB_DETONATED);
            m_BombExplodState = false;
        } 
        m_BombTimer += DeltaTime;
    break;
    case    BOMB_DETONATED:
        SetHUDEventState( HUD_BOMB_DETONATED );
    break;
    case    BOMB_TRIGGERED:
        //convert float value to minutes and seconds
        m_BombTimerMinutes = (u32)((f32)m_DetonateTimeMinutes);           
        m_BombTimerSeconds = (u32)((f32)m_DetonateTimeSeconds);
        TurnOnDigitMeshes();
 
        m_BombTimer = 0.0f;
        SetBombEventState(BOMB_COUNTING_DOWN);
    break;
    }
}

//=============================================================================

void bomb_object::SetHUDEventState( s32 state )
{
    m_EventHUDState = state;
}

//=============================================================================

void bomb_object::SetBombEventState( s32 state )
{
    m_EventBombState = state; 
}

//=============================================================================

s32 bomb_object::GetHUDEventState( void )
{
    return( m_EventHUDState );
}

//=============================================================================

s32 bomb_object::GetBombEventState( void )
{
    return( m_EventBombState );
}

//=============================================================================

s32 bomb_object::GetBombClockSegments( void )
{
    return ( m_EventUnit );
}

//=============================================================================

void bomb_object::InitMeshIndices( void)
{
    // Set up bomb meshes for anim control
    m_iMesh[LEFT_DIGIT]   = FindCorrectMesh("num_lft");
    m_iMesh[MIDDLE_DIGIT] = FindCorrectMesh("num_mid");
    m_iMesh[RIGHT_DIGIT]  = FindCorrectMesh("num_rt");
    m_iMesh[SCREEN_COVER] = FindCorrectMesh("screen_HAND");
    m_iMesh[SCREEN_BLANK] = FindCorrectMesh("screen_BLANK");
    m_iMesh[SCREEN_COLON] = FindCorrectMesh("num_col");
}

//=============================================================================

void bomb_object::OnEnumProp( prop_enum& List )
{
    // Call base class
    destructible_obj::OnEnumProp( List );

    List.PropEnumString ( "Bomb","Bomb Properties", 0 );
    List.PropEnumEnum   ( "Bomb\\State", s_BombStateTable.BuildString(), "Bomb States", PROP_TYPE_EXPOSE);
    List.PropEnumFloat  ( "Bomb\\Deactivate Time",     "Time in Milliseconds needed to deactivate bomb.", 0 );
    List.PropEnumFloat  ( "Bomb\\Detonation Min",      "Time untill the bomb will explode Minutes",PROP_TYPE_EXPOSE );
    List.PropEnumFloat  ( "Bomb\\Detonation Sec",      "Time untill the bomb will explode Seconds",PROP_TYPE_EXPOSE );

    List.PropEnumHeader ( "Bomb\\Screen Shake",                      "Defines a screen shake when the bomb has detonated", 0 );
    List.PropEnumFloat  ( "Bomb\\Screen Shake\\Time",                "Defines amount of time the screen shakes", 0 );
    List.PropEnumFloat  ( "Bomb\\Screen Shake\\Amount",              "Defines how strong the screen shakes.", 0 );
    List.PropEnumFloat  ( "Bomb\\Screen Shake\\Speed",               "Defines how quick the shake tries to fix itself.", 0 );
    List.PropEnumFloat  ( "Bomb\\Screen Shake\\Feedback Intensity",  "Force feedback intensity.", 0 );
    List.PropEnumFloat  ( "Bomb\\Screen Shake\\Feedback Duration",   "Force feedback duration.", 0 );
    List.PropEnumBool   ( "Bomb\\Screen Shake\\Use",   "Defines if we use the screen shake for this object or not.",PROP_TYPE_EXPOSE );
}

//=============================================================================

xbool bomb_object::OnProperty( prop_query& I )
{
 
    if(destructible_obj::OnProperty(I))
    {
        if (I.IsRead() == FALSE)
        {
            InitMeshIndices();
            TurnOnBlankSreenMesh();
        }
        return TRUE;
    }
    
    // HACK: fix this later!
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );

        if ( I.IsVar  ( "Bomb\\State" ))
    {

        if( I.IsRead() )
        {
            if (m_EventBombState != BOMB_INVALID )
                I.SetVarEnum(s_BombStateTable.GetStringFromIndex(m_EventBombState) );
            else
                I.SetVarEnum( "" );
        }
        else
        {
            if (x_strlen(I.GetVarEnum()) > 0)
            {
                m_EventBombState = s_BombStateTable.GetIndex( I.GetVarEnum());

                switch(m_EventBombState)
                {
                case BOMB_ACTIVE:
                   OnActivate(TRUE);
                break;
                case BOMB_NOT_ACTIVE:
                   SetBombEventState(BOMB_NOT_ACTIVE);  
                break;
                case BOMB_DISARMED:
                    SetBombEventState(BOMB_DISARMED);
                break;
                case BOMB_EXPOLDING:
                    OnActivate(TRUE);
                    SetBombEventState(BOMB_EXPOLDING);
                    m_BombExplodState = true;
                    m_BombTimerMinutes = 0;
                    m_BombTimerSeconds = 0;

                    //turn of countdown meshes
                    //to display have a nice day
                    TurnOnAboutToExpoldeMesh();
                    m_BombTimer = 0.0f;
                break;
                case BOMB_DETONATED:
                    //Bomb has been detonated take it off the render list
                    SetBombEventState(BOMB_DETONATED);
                break;
                case BOMB_TRIGGERED:
                    OnActivate(TRUE);
                    SetBombEventState(BOMB_TRIGGERED);
                break;
                }
            }
        }
        return TRUE;
    }   

    if( I.VarFloat( "Bomb\\Deactivate Time", m_DeactivateTime, 1.0f, 12.0f ) )
        return TRUE;

    if( I.VarFloat( "Bomb\\Detonation Min", m_DetonateTimeMinutes, 0.0f, 9.0f) )
        return TRUE;
   
    if( I.VarFloat( "Bomb\\Detonation Sec", m_DetonateTimeSeconds, 0.0f, 59.0f) )
        return TRUE;

    if( I.VarFloat( "Bomb\\Screen Shake\\Time", m_ScreenShakeTime, 0.0f, 10.0f ) )
        return TRUE;

    if( I.VarFloat( "Bomb\\Screen Shake\\Amount", m_ScreenShakeAmount, 0.0f, 10.0f ) )
        return TRUE;
    
    if( I.VarFloat( "Bomb\\Screen Shake\\Speed", m_ScreenShakeSpeed, 0.0f, 10.0f ) )
        return TRUE;

    if( I.VarFloat( "Bomb\\Screen Shake\\Feedback Intensity", m_FeedBackIntensity, 0.0f, 10.0f ) )
        return TRUE;

    if( I.VarFloat( "Bomb\\Screen Shake\\Feedback Duration", m_FeedBackDuration, 0.0f, 10.0f ) )
        return TRUE;

    if( I.VarBool ( "Bomb\\Screen Shake\\Use", m_UseScreenShake ) )
        return TRUE;
    
    return( FALSE );
}


void bomb_object::TurnOnDigitMeshes(void)
{
    m_MeshMask = 0xFFFFFFFFFFFFFFFF;
}
void bomb_object::TurnOnBlankSreenMesh(void)
{
        //Set screen to default to blank
    m_MeshMask = 0xFFFFFFFFFFFFFFFF;
    m_MeshMask &= ~(1 << m_iMesh[LEFT_DIGIT  ]);
    m_MeshMask &= ~(1 << m_iMesh[MIDDLE_DIGIT]);
    m_MeshMask &= ~(1 << m_iMesh[RIGHT_DIGIT ]);
    m_MeshMask &= ~(1 << m_iMesh[RIGHT_DIGIT ]);
    m_MeshMask &= ~(1 << m_iMesh[SCREEN_COVER]);
    m_MeshMask &= ~(1 << m_iMesh[SCREEN_COLON]);
    m_MeshMask |=  (1 << m_iMesh[SCREEN_BLANK]);
}
void bomb_object::TurnOnAboutToExpoldeMesh(void)
{
    m_MeshMask = 0xFFFFFFFFFFFFFFFF;
    m_MeshMask &= ~(1 << m_iMesh[LEFT_DIGIT  ]);
    m_MeshMask &= ~(1 << m_iMesh[MIDDLE_DIGIT]);
    m_MeshMask &= ~(1 << m_iMesh[RIGHT_DIGIT ]);
    m_MeshMask &= ~(1 << m_iMesh[RIGHT_DIGIT ]);
    m_MeshMask &= ~(1 << m_iMesh[SCREEN_BLANK]);
    m_MeshMask &= ~(1 << m_iMesh[SCREEN_COLON]);
    m_MeshMask |=  (1 << m_iMesh[SCREEN_COVER]);
}

//=============================================================================





