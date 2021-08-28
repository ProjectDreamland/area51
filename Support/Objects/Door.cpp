//=========================================================================
//
// Door.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "Door.hpp"
#include "..\Support\Zonemgr\ZoneMgr.hpp"
#include "..\Support\Objects\Portal.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "x_context.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\NetObjMgr.hpp"

//=========================================================================

#define DOOR_DATA_VERSION           100
#define PROXIMITY_GROW_AMOUNT       5
#define PROXIMITY_COLLISION         (1<<1)
#define CLOSE_PROXIMITY_COLLISION   (1<<2)
#define OVERRIDE_LOGIC              (1<<3)
#define FIRST_LOGIC_PASS            (1<<4)

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct door_desc : public object_desc
{
    door_desc( void ) : object_desc( 
        object::TYPE_DOOR, 
        "Door",
        "PROPS",

        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_COLLIDABLE         | 
        object::ATTR_BLOCKS_ALL_PROJECTILES | 
        object::ATTR_BLOCKS_ALL_ACTORS | 
        object::ATTR_BLOCKS_RAGDOLL | 
        object::ATTR_BLOCKS_CHARACTER_LOS | 
        object::ATTR_BLOCKS_PLAYER_LOS | 
        object::ATTR_BLOCKS_PAIN_LOS | 
        object::ATTR_BLOCKS_SMALL_DEBRIS | 
        object::ATTR_RENDERABLE         | 
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_TARGETS_OBJS          |
        FLAGS_BURN_VERTEX_LIGHTING  ) {}
    
    //-------------------------------------------------------------------------
    
    virtual object* Create( void ) { return new door; }
    
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

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_Door_Desc;

//=========================================================================

const object_desc& door::GetTypeDesc( void ) const
{
    return s_Door_Desc;
}

//=========================================================================

const object_desc&  door::GetObjectType   ( void )
{
    return s_Door_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

door::door( void )
{ 
    m_CurrentState      = CLOSED;
    m_InitialState      = CLOSED;
    m_TargetState       = CLOSED;
    m_RestingState      = CLOSED;
    m_PortalGuid        = NULL_GUID;
    m_PreCloseWaitTime  = 3.0f;
    m_DoorSoundID       = 0;
    m_SoundOcclusion    = 0.5f;
    m_DoorClosedSfx[0]  = 0;
    m_CurrentWaitTime   = 0.0f;
    m_Flags             = 0;
    m_Flags             |= FIRST_LOGIC_PASS;
    m_DoorCloseDelay    = 0;
    m_bUseProximityBox  = FALSE;

//    m_ProximityBBox.Max.Set( 0.0f, 0.0f, 0.0f );
//    m_ProximityBBox.Min.Set( 0.0f, 0.0f, 0.0f );

//    m_CloseProximityBBox.Max.Set( 0.0f, 0.0f, 0.0f );
//    m_CloseProximityBBox.Min.Set( 0.0f, 0.0f, 0.0f );

    m_RenderBBox.Max.Set( 0.0f, 0.0f, 0.0f );
    m_RenderBBox.Min.Set( 0.0f, 0.0f, 0.0f );

    m_ProximityBox.Min.Set( -300.0f,   0.0f, -100.0f );
    m_ProximityBox.Max.Set(  300.0f, 250.0f,  100.0f );
    
    m_BBoxScale.Set( 0.0f, 0.0f, 130.0f );

    for( s32 i = 0; i < MAX_STATE; i++ )
        m_AnimIndex[i] = -1;
    
    m_DoorBoneDebug = FALSE;

    //
    // Disable running the logic till a living object collides with us.
    //
    //SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
}

//=========================================================================

void door::OnInit( void )
{
    anim_surface::OnInit();

    //
    // Disable running the logic till a living object collides with us.
    //
    m_Flags = 0;
    m_Flags |= FIRST_LOGIC_PASS;
}

//=========================================================================

void door::SetAnimIndex( void )
{   
    s32 i = 0;
    s32 j = 0;

    if( m_hAnimGroup.IsLoaded() == FALSE )
        return;

    for( i = 0; i < m_AnimPlayer.GetAnimGroup()->GetNAnims(); i++ )
    {
        const char* pAnimName = m_AnimPlayer.GetAnimGroup()->GetAnimInfo( i ).GetName();

        for( j = 0; j < MAX_STATE; j++ )
        {
            if( m_AnimIndex[ j ] == -1 )
            {
                switch( j )
                {
                    case LOCKING:   if( x_stristr( pAnimName, "LOCKING"  ) != NULL )    m_AnimIndex[ j ] = i;
                    break;
                    case LOCKED:    if( x_stristr( pAnimName, "LOCKED"  ) != NULL )     m_AnimIndex[ j ] = i;
                    break;
                    case UNLOCKING: if( x_stristr( pAnimName, "UNLOCKING" ) != NULL )   m_AnimIndex[ j ] = i;
                    break;
                    case CLOSING:   if( x_stristr( pAnimName, "CLOSING"  ) != NULL )    m_AnimIndex[ j ] = i;
                    break;
                    case CLOSED:    if( x_stristr( pAnimName, "CLOSED"   ) != NULL )    m_AnimIndex[ j ] = i;
                    break;
                    case PRECLOSE:  if( x_stristr( pAnimName, "PRECLOSE" ) != NULL )    m_AnimIndex[ j ] = i;
                    break;
                    case OPENING:   if( x_stristr( pAnimName, "OPENING"  ) != NULL )    m_AnimIndex[ j ] = i;               
                    break;
                    case OPEN:      if( x_stristr( pAnimName, "OPENED"   ) != NULL )    m_AnimIndex[ j ] = i;
                    break;
                    default: 
                    break;
                }
            }
        }
    }
}

//=========================================================================

bbox door::GetLocalBBox( void ) const
{
    return m_RenderBBox;
}

//=========================================================================

bbox door::GetDoorBBox( void ) const
{
    if( m_bUseProximityBox )
    {
        bbox BBox = m_ProximityBox;
        
        BBox.Translate( GetPosition() );

        return BBox;
    }
    else
    {
        bbox BBox = m_RenderBBox;

        BBox.Translate( GetPosition() );

        BBox.Min -= m_BBoxScale;
        BBox.Max += m_BBoxScale;

        return BBox;
    }
}

//=========================================================================

#ifndef X_RETAIL
void door::OnDebugRender( void )
{
    anim_surface::OnDebugRender();

    const matrix4* pBone = GetBoneL2Ws() ;
    if (!pBone)
        return ;

    for( s32 i = 0; i < m_AnimPlayer.GetNBones(); i++ )
    {
        draw_Marker( pBone[i].GetTranslation() );
    }

    draw_BBox( GetBBox(), XCOLOR_WHITE );
}
#endif // X_RETAIL

//=========================================================================

void door::OnRender( void )
{
    CONTEXT( "door::OnRender" );
    anim_surface::OnRender();

#ifdef X_EDITOR
    if( mp_settings::s_Selected )
        m_Circuit.SpecialRender( GetPosition() );

    if( m_bUseProximityBox )
    {
        draw_BBox( GetDoorBBox(), XCOLOR_BLUE );
    }
#endif
}

//=========================================================================

void door::WakeUp( void )
{    
    if( !(m_Flags & OVERRIDE_LOGIC) )
    {
        // 
        // If a living object collide with the door then start opening it, other wise ignore it.
        // If the normal logic is overridden then ignore this action.
        //
        if( m_CurrentState == LOCKED )
        {
            if( !g_AudioMgr.IsValidVoiceId( m_DoorSoundID ) )
                m_DoorSoundID = g_AudioMgr.PlayVolumeClipped( m_DoorClosedSfx, GetPosition(), GetZone1(), TRUE );
        }
        else
        {
            m_DoorCloseDelay = 2;
            m_TargetState    = OPEN;
            m_Flags         |= PROXIMITY_COLLISION;

            SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );

            // If the door is in the middle of closing anim, stop and start playing the opening anim where
            // the closing anim left off.
            if( m_CurrentState == CLOSING )
            {
                f32 FrameParametric = m_AnimPlayer.GetFrameParametric();
                SwitchState( OPENING );
                m_AnimPlayer.SetFrameParametric( (1.0f - FrameParametric) );
            }
        }
    }
}

//=========================================================================

void door::OnColCheck( void )
{    
    anim_surface::OnColCheck();
}

//=========================================================================

void door::OnColNotify ( object& Object )
{
    object::OnColNotify( Object );
}

//=========================================================================

void door::SetTargetState( state TargetState )
{
    m_TargetState = TargetState;
    
    // Only triggers can make a door unlock.
    if( m_CurrentState == LOCKED )
    {   
        if( (m_TargetState == OPEN) || (m_TargetState == CLOSED) )
        {
            SwitchState( UNLOCKING );
        }
    }
    else if( (m_CurrentState == CLOSING) && (m_TargetState == OPEN) )
    {
        f32 FrameParametric = m_AnimPlayer.GetFrameParametric();
        SwitchState( OPENING );
        m_AnimPlayer.SetFrameParametric( (1.0f - FrameParametric) );
    }
    else if( (m_CurrentState == LOCKING) && (m_TargetState == OPEN) )
    {
        f32 FrameParametric = m_AnimPlayer.GetFrameParametric();
        SwitchState( UNLOCKING );
        m_AnimPlayer.SetFrameParametric( (1.0f - FrameParametric) );
    }

    SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );    
}

//=========================================================================

void door::OverRideLogic( xbool bRunLogic )
{
    if( bRunLogic )
        m_Flags &= ~OVERRIDE_LOGIC;
    else
        m_Flags |= OVERRIDE_LOGIC;
}

//=========================================================================

void door::OnAdvanceLogic ( f32 DeltaTime )
{
    CONTEXT( "door::OnAdvanceLogic" );
   
    // The first time the door tries to do it logic set the portal state so we know that every thing is loaded at
    // this point.
    if( m_Flags & FIRST_LOGIC_PASS )
    {
        m_Flags &= ~FIRST_LOGIC_PASS;
        
        // If there anyone in the bbox.
        if( CheckProximity() == FALSE )
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );

        // Set the portal state.
        if( m_PortalGuid )
        {
            if( (m_InitialState == LOCKED) || (m_InitialState == CLOSED) )
                g_ZoneMgr.TurnPortalOff( m_PortalGuid ); 
            else
                g_ZoneMgr.TurnPortalOn( m_PortalGuid ); 
        }

        return;
    }

    //
    // First check if there are any living objects in the door way, if not then let the door get to its intial
    //  state before shutting it down.
    //
    
    if( m_Flags & OVERRIDE_LOGIC )
    {
        if( m_CurrentState == m_TargetState )
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
    }
    else
    {
        if( CheckProximity() == FALSE )
        {
            if( m_CurrentState == m_RestingState )
                SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
            else
                m_TargetState = m_RestingState;
        }
    }

    UpdateState( DeltaTime );

    anim_surface::OnAdvanceLogic( DeltaTime );
}

//==============================================================================

xbool door::CheckProximity( void )
{   
    //
    // Check if all the living objects that collided with the door are still in its proximity.
    //
    
    xbool Result = FALSE;
    g_ObjMgr.SelectBBox( object::ATTR_LIVING, GetBBox() );

    slot_id SlotID = g_ObjMgr.StartLoop();
    
    if( SlotID != SLOT_NULL )
    {
        Result = TRUE;
    }
    
    g_ObjMgr.EndLoop();
    return Result;
}

//==============================================================================

void door::UpdateState ( f32 DeltaTime )
{
    //Update the active state for the door.
    CONTEXT("door::UpdateState");
    
    switch ( m_CurrentState )
    {
        case LOCKING:
        {
            if( (m_AnimPlayer.AnimDone()) || (m_AnimIndex[ m_CurrentState ] == -1) )
            {
                SwitchState( LOCKED );
            }            
        }
        break;  
        case LOCKED:
        {   
            // Force the bbox to update.
            OnMove( GetPosition() );

            // Only a trigger can set the state to unlocking.
//            if( (m_TargetState == OPEN) || (m_TargetState == CLOSED) )
//            {
//                SwitchState( UNLOCKING );
//            }
        }
        break;   
        case UNLOCKING:
        {
            if( (m_AnimPlayer.AnimDone()) || (m_AnimIndex[ m_CurrentState ] == -1) )
            {
                SwitchState( CLOSED );
            }
        }
        break;
        case CLOSING:
        {            
            // Is there an animation for closing.
            if( m_AnimIndex[ m_CurrentState ] != -1 )
            {
                // Calculate the sound propagation.
                s32 nFrames     = m_AnimPlayer.GetNFrames();
                f32 CurrFrame   = m_AnimPlayer.GetFrame();

                f32 Occlusion   = (1.0f - m_SoundOcclusion) * (1.0f - (CurrFrame/nFrames)) + m_SoundOcclusion;
                //m_SoundOcclusion * (1.0f - (CurrFrame/nFrames)) + ;
            
                g_ZoneMgr.SetPortalOcclusion( m_PortalGuid, Occlusion );
            }
            else
            {
                g_ZoneMgr.SetPortalOcclusion( m_PortalGuid, m_SoundOcclusion );
            }

            if( (m_AnimPlayer.AnimDone()) || (m_AnimIndex[ m_CurrentState ] == -1) )
            {
                // Turn the portal off here.
                SwitchState( CLOSED );
            }            

        }
        break;  
        case CLOSED:
        {
            if( m_TargetState == OPEN )
            {
                SwitchState( OPENING );
            }
            else if( m_TargetState == LOCKED )
            {
                SwitchState( LOCKING );
            }

            g_ZoneMgr.TurnPortalOff( m_PortalGuid ); 

            // Force the bbox to update.
            OnMove( GetPosition() );
        }
        break;   
        case PRECLOSE:
        {
            m_CurrentWaitTime += DeltaTime;

            if( m_PreCloseWaitTime < m_CurrentWaitTime) 
            {
                m_DoorCloseDelay--;
                if( m_DoorCloseDelay <= 0 )
                {
                    m_DoorCloseDelay = 0;
                    SwitchState( CLOSING );
                    m_CurrentWaitTime = 0.0f;
                }
            }
        }
        break; 
        case OPENING:
        {
            if( (m_AnimPlayer.AnimDone()) || (m_AnimIndex[ m_CurrentState ] == -1) )
            {
                SwitchState( OPEN );
            }            
            
            
            if( m_AnimIndex[ m_CurrentState ] != -1 && m_CurrentState == OPENING )
            {
                // Calculate the sound propagation.
                s32 nFrames     = m_AnimPlayer.GetNFrames();
                f32 CurrFrame   = m_AnimPlayer.GetFrame();

                f32 Occlusion   = ((1.0f - m_SoundOcclusion) * (CurrFrame/nFrames)) + m_SoundOcclusion;
            
                g_ZoneMgr.SetPortalOcclusion( m_PortalGuid, Occlusion );

                g_ZoneMgr.TurnPortalOn( m_PortalGuid ); 
            }
            else
            {
                g_ZoneMgr.SetPortalOcclusion( m_PortalGuid, 1.0f );

                g_ZoneMgr.TurnPortalOn( m_PortalGuid ); 
            }
        }
        break;  
        case OPEN:
        {
            if( m_TargetState == CLOSED )
            {
                SwitchState( PRECLOSE );
            }
            else if( m_TargetState == LOCKED )
            {
                SwitchState( CLOSING );
            }
        }
        break;             
        default:
        break;
    }
}

//==============================================================================

void door::SwitchState ( state State )
{
    m_CurrentState = State;
    if ( m_AnimIndex[ m_CurrentState ] >= 0 )
    {
        m_AnimPlayer.SetAnim( m_AnimIndex[ m_CurrentState ], 0);
    }
}

//==============================================================================

void door::OnEnumProp( prop_enum&  rPropList )
{
    m_Circuit.OnEnumProp( rPropList );
    anim_surface::OnEnumProp( rPropList );

    u32 PropFlags = 0;
    
    //object info
    rPropList.PropEnumString	( "Door", "Door Properties", 0 );
    
    rPropList.PropEnumGuid   ( "Door\\Portal" ,              "The portal the door controls" , PropFlags );

    rPropList.PropEnumEnum   ( "Door\\Initial State", 
                          "CLOSED\0LOCKED\0OPEN\0", 
                          "The initial state for the door ", 0 );

    rPropList.PropEnumEnum   ( "Door\\Resting State", 
                          "CLOSED\0LOCKED\0OPEN\0", 
                          "The initial state for the door ", 0 );

    rPropList.PropEnumExternal("Door\\Closed Sound",        "Sound\0soundexternal\0","When the player can't get through the door.", PROP_TYPE_MUST_ENUM );

    rPropList.PropEnumFloat  ( "Door\\Sound Occlusion",      "The amount of sound this door will let through when closed 0 -> 1( 1.0 == All of thesound going through ).", 0 );

    rPropList.PropEnumBool   ( "Door\\Run Logic",            "Is the door going to be running its default logic.", 0 );

    //rPropList.PropEnumBBox   ( "Door\\Proximity Box",        "Trigger area for the door" );
    //rPropList.PropEnumBBox   ( "Door\\Close Proximity Box",  "Trigger area for any close proximity events" );
    rPropList.PropEnumBool   ( "Door\\Use Proximity Box" ,   "Use a proximity box instead of scaling bbox.", PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumBBox   ( "Door\\Proximity Box" ,       "Proximity Box to use for collision.", m_bUseProximityBox ? 0 : PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumVector3( "Door\\Proximity Box Scale" , "Add this distance +/- to the collision box (cm).", m_bUseProximityBox ? PROP_TYPE_DONT_SHOW : 0 );
    rPropList.PropEnumFloat  ( "Door\\PreClose Time",        "The time to wait from going OPEN to CLOSING", 0 );

    rPropList.PropEnumBool   ( "Door\\Door Bone Debug",      "Renders all of the bones in the door object", 0 );
}


//=============================================================================

xbool door::OnProperty( prop_query& rPropQuery )
{    
    if( anim_surface::OnProperty( rPropQuery )  )
    {
        // Add extra data for this type
        if( rPropQuery.IsVar( "RenderInst\\Anim" ) )
        {
            if( rPropQuery.IsRead() == FALSE )
            {
                // Set the intial state.
                if( m_hAnimGroup.IsLoaded() == TRUE )
                {
                    // We don't always get an animation for the state to we can use the last state index since it leads up
                    // to the current animation.
                    s32 State = m_InitialState;
                    if(m_AnimIndex[ m_InitialState] < 0 )
                        State = m_InitialState-1;

                    if ( m_AnimIndex[ State ] >= 0 )
                    {
                        m_AnimPlayer.SetAnim( m_AnimIndex[ State ], 0);
                    
                        // If its the initial state then set the first frame if not then use the last frame of 
                        // the previous( transition ) state.
                        m_AnimPlayer.SetFrame( ( State==m_InitialState) ? 1 : m_AnimPlayer.GetNFrames() - 1 );
                    }

                    // Make sure bbox is never smaller than the animation bbox
                    // Initialize the indices for the animation
//                    m_ProximityBBox += m_Inst.GetGeom()->m_BBox;
//                    m_CloseProximityBBox += m_Inst.GetGeom()->m_BBox;

                    m_RenderBBox = m_hAnimGroup.GetPointer()->GetBBox();

                    SetAnimIndex();
                }               
            }
        }

        return TRUE;
    }

    if( m_Circuit.OnProperty( rPropQuery ) )
    {
        return TRUE;
    }
    
    // The Initial state.
    if( rPropQuery.IsVar( "Door\\Initial State" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( m_InitialState )
            {
                case CLOSED     : rPropQuery.SetVarEnum( "CLOSED" ); break;
                case LOCKED     : rPropQuery.SetVarEnum( "LOCKED" ); break;
                case OPEN       : rPropQuery.SetVarEnum( "OPEN" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "CLOSED", rPropQuery.GetVarEnum()) )
            {
                m_TargetState = m_CurrentState = m_InitialState = CLOSED;
            }
            else if( !x_stricmp( "LOCKED", rPropQuery.GetVarEnum() ) )
            {
                m_TargetState = m_CurrentState = m_InitialState = LOCKED;
            }
            else if( !x_stricmp( "OPEN", rPropQuery.GetVarEnum() ) )
            {
                m_TargetState = m_CurrentState = m_InitialState = OPEN;
            }

            if( m_PortalGuid )
            {
                if( (m_InitialState == LOCKED) || (m_InitialState == CLOSED) )
                    g_ZoneMgr.TurnPortalOff( m_PortalGuid ); 
                else
                    g_ZoneMgr.TurnPortalOn( m_PortalGuid ); 
            }

            // Set the intial state.
            if( m_hAnimGroup.IsLoaded() == TRUE )
            {
                // We don't always get an animation for the state to we can use the last state index since it leads up
                // to the current animation.
                s32 State = m_InitialState;
                if(m_AnimIndex[ m_InitialState] < 0 )
                    State = m_InitialState-1;

                if ( m_AnimIndex[ State ] >= 0 )
                {
                    m_AnimPlayer.SetAnim( m_AnimIndex[ State ], 0);
                    
                    // If its the initial state then set the first frame if not then use the last frame of 
                    // the previous( transition ) state.
                    m_AnimPlayer.SetFrame( ( State==m_InitialState) ? 1 : m_AnimPlayer.GetNFrames() - 1 );
                }
            }
        }

        return TRUE;
    }

    // The Resting state.
    if( rPropQuery.IsVar( "Door\\Resting State" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( m_RestingState )
            {
                case CLOSED     : rPropQuery.SetVarEnum( "CLOSED" ); break;
                case LOCKED     : rPropQuery.SetVarEnum( "LOCKED" ); break;
                case OPEN       : rPropQuery.SetVarEnum( "OPEN" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "CLOSED", rPropQuery.GetVarEnum()) )
            {
                m_RestingState = CLOSED;
            }
            else if( !x_stricmp( "LOCKED", rPropQuery.GetVarEnum() ) )
            {
                m_RestingState = LOCKED;
            }
            else if( !x_stricmp( "OPEN", rPropQuery.GetVarEnum() ) )
            {
                m_RestingState = OPEN;
            }
        }

        return TRUE;
    }

    // External
    if( rPropQuery.IsVar( "Door\\Closed Sound" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_DoorClosedSfx, 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = rPropQuery.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                if( String == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                    m_DoorClosedSfx[0] = 0;
                }
                else
                {
                    s32 PkgIndex = String.Find( '\\', 0 );
                    
                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );

                        m_hAudioPackage.SetName( Pkg );                

                        // Load the audio package.
                        if( m_hAudioPackage.IsLoaded() == FALSE )
                            m_hAudioPackage.GetPointer();
                    }

                    x_strncpy( m_DoorClosedSfx, String, 64 );
                }
            }
        }
        return( TRUE );
    }
    
    //portal which is controlled by the door
    if ( rPropQuery.VarGUID( "Door\\Portal" , m_PortalGuid ) )
    {
        
        object* pObj = NULL;
        if ( m_PortalGuid != NULL_GUID )
        {
            pObj = g_ObjMgr.GetObjectByGuid( m_PortalGuid );
        }

        // Cast object to a zone portal
        if( pObj )
        {            
            if( pObj->GetType() == object::TYPE_ZONE_PORTAL )
            {
                zone_portal&   Portal = zone_portal::GetSafeType( *pObj );                
                Portal.SetSoundOcclusion( m_SoundOcclusion );
            }
            else
            {
                x_DebugMsg( "Invalid portal type associated with the door\n" );
            }
        }
        return TRUE;
    }

    if( rPropQuery.VarFloat( "Door\\Sound Occlusion", m_SoundOcclusion ) )
    {
        // Make sure that the value is in bounds.
        if( m_SoundOcclusion > 1.0f )
            m_SoundOcclusion = 1.0f;
        else if( m_SoundOcclusion < 0.01f )
            m_SoundOcclusion = 0.01f;

        object* pObj = NULL;
        if ( m_PortalGuid != NULL_GUID )
        {
            pObj = g_ObjMgr.GetObjectByGuid( m_PortalGuid );
        }


        // Cast object to a zone portal
        if( pObj )
        {   
            if( pObj->GetType() == object::TYPE_ZONE_PORTAL )
            {
                zone_portal&   Portal = zone_portal::GetSafeType( *pObj );                
            
                Portal.SetSoundOcclusion( m_SoundOcclusion );
            }
            else
            {
                x_DebugMsg( "Invalid portal type associated with the door\n" );
            }
        }

        return TRUE;
    }
    

    if( rPropQuery.IsVar( "Door\\Run Logic" ) )
    {
        if( rPropQuery.IsRead() )
        {
            xbool Flag = (m_Flags & OVERRIDE_LOGIC) ? FALSE : TRUE;
                
            rPropQuery.SetVarBool( Flag );
        }
        else
        {
            xbool Flag = rPropQuery.GetVarBool();

            if( Flag )
                m_Flags &= ~OVERRIDE_LOGIC;
            else
                m_Flags |= OVERRIDE_LOGIC;
        }
        
        return TRUE;
    }
/*
    if( rPropQuery.VarBBox( "Door\\Proximity Box", m_ProximityBBox ) )
    {
        // Don't let the proximity box be smaller than the collision box.
        if( m_hAnimGroup.IsLoaded() == TRUE )
        {
            m_ProximityBBox += m_Inst.GetGeom()->m_BBox;
/*
            if( m_ProximityBBox.Max.X < m_Inst.GetGeom()->m_BBox.Max.X )
                m_ProximityBBox.Max.X = m_Inst.GetGeom()->m_BBox.Max.X;

            if( m_ProximityBBox.Max.Y < m_Inst.GetGeom()->m_BBox.Max.Y )
                m_ProximityBBox.Max.Y = m_Inst.GetGeom()->m_BBox.Max.Y;

            if( m_ProximityBBox.Max.Z < m_Inst.GetGeom()->m_BBox.Max.Z )
                m_ProximityBBox.Max.Z = m_Inst.GetGeom()->m_BBox.Max.Z;

            if( m_ProximityBBox.Min.X > m_Inst.GetGeom()->m_BBox.Min.X )
                m_ProximityBBox.Min.X = m_Inst.GetGeom()->m_BBox.Min.X;

            if( m_ProximityBBox.Min.Y > m_Inst.GetGeom()->m_BBox.Min.Y )
                m_ProximityBBox.Min.Y = m_Inst.GetGeom()->m_BBox.Min.Y;

            if( m_ProximityBBox.Min.Z > m_Inst.GetGeom()->m_BBox.Min.Z )
                m_ProximityBBox.Min.Z = m_Inst.GetGeom()->m_BBox.Min.Z;

        }

        // Force the world box to update.
        OnMove( GetPosition() );

        return TRUE;
    }

    if( rPropQuery.VarBBox( "Door\\Close Proximity Box", m_CloseProximityBBox ) )
    {
        // Don't let the proximity box be smaller than the collision box.
        if( m_hAnimGroup.IsLoaded() == TRUE )
        {            
            m_CloseProximityBBox += m_Inst.GetGeom()->m_BBox;
        }

        // Force the world box to update.
        OnMove( GetPosition() );

        return TRUE;
    }
*/
    if( rPropQuery.VarBool( "Door\\Use Proximity Box", m_bUseProximityBox ) )
    {
        OnMove( GetPosition() );
        return TRUE;
    }

    if( rPropQuery.VarBBox( "Door\\Proximity Box", m_ProximityBox ) )
    {
        OnMove( GetPosition() );
        return TRUE;
    }


    if( rPropQuery.VarVector3( "Door\\Proximity Box Scale", m_BBoxScale ) )
    {
        if( !rPropQuery.IsRead() )
        {
            m_BBoxScale.Max( 0.0f );

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    if( rPropQuery.VarFloat( "Door\\PreClose Time", m_PreCloseWaitTime ) )
    {
        return TRUE;
    }
    
    if( rPropQuery.VarBool( "Door\\Door Bone Debug", m_DoorBoneDebug ) )
        return TRUE;

    return( FALSE );
}