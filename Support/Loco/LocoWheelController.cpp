//=========================================================================
//
//  LocoWheelController.hpp
//
//  Automatically animates wheels from current motion
//
//=========================================================================

#include "LocoWheelController.hpp"
#include "Loco.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

loco_wheel_controller::loco_wheel_controller() :
        loco_anim_controller(),     // Base class
        
        m_pLoco         ( NULL  ),      // Owner loco
        
        m_nWheels       ( 0     ),      // # of wheels
        
        m_pTurnLoopSfx  ( NULL  ),      // Turning sound name
        m_pTurnStopSfx  ( NULL  ),      // Turn stop sound name
        m_pMoveLoopSfx  ( NULL  ),      // Moving forwards/backwards sound name
        m_pMoveStopSfx  ( NULL  ),      // Move stop sound name

        m_TurnLoopSfxId ( 0     ),      // Turn loop sound id
        m_MoveLoopSfxId ( 0     )       // Move loop sound id
{
}

//=========================================================================

loco_wheel_controller::~loco_wheel_controller()
{
}

//=========================================================================

// Initialize
xbool loco_wheel_controller::Init( loco*                     pLoco,
                                   const anim_group::handle& hAnimGroup,
                                   const char*               pTurnLoopSfx,
                                   const char*               pTurnStopSfx,
                                   const char*               pMoveLoopSfx,
                                   const char*               pMoveStopSfx )
{
    // Must be valid
    ASSERT( pLoco );
    ASSERT( pTurnLoopSfx );
    ASSERT( pTurnStopSfx );
    ASSERT( pMoveLoopSfx );
    ASSERT( pMoveStopSfx );
    
    // Keep values
    m_pLoco        = pLoco;
    m_pTurnLoopSfx = pTurnLoopSfx;
    m_pTurnStopSfx = pTurnStopSfx;
    m_pMoveLoopSfx = pMoveLoopSfx;
    m_pMoveStopSfx = pMoveStopSfx;
    
    // Setup anim group
    SetAnimGroup( hAnimGroup );
    
    // Success if wheels were found
    return ( m_nWheels > 0 );
}

//=========================================================================

// Sets location of animation data package
void loco_wheel_controller::SetAnimGroup( const anim_group::handle& hAnimGroup )
{
    // Call base class
    loco_anim_controller::SetAnimGroup( hAnimGroup );

    // Clear wheel count
    m_nWheels = 0;
    m_Weight  = 0.0f;
    
    // Lookup anim group
    const anim_group* pAnimGroup = hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return;
    
    // Wheel bone list    
    static const char* s_WheelBones[MAX_WHEELS] =
    {
        "Wheel_Left_Back",
        "Wheel_Left_Front",
        "Wheel_Right_Back",
        "Wheel_Right_Front",
    };
    
    // Lookup wheels
    m_nWheels = 0;
    for( s32 i = 0; i < MAX_WHEELS; i++ )
    {
        // Wheel found?
        s32 iBone = pAnimGroup->GetBoneIndex( s_WheelBones[i], TRUE );
        if( iBone != -1 )
        {
            // Make sure parent is root bone
            ASSERT( pAnimGroup->GetBoneParent( iBone ) == 0 );
            
            // Add to list
            wheel& Wheel = m_Wheels[m_nWheels++];
            Wheel.m_iBone  = iBone;
            Wheel.m_Radius = 18.0f; // HARD CODED FOR NOW!!!
            Wheel.m_Pos.Zero();
            Wheel.m_DeltaRot = 0;
            Wheel.m_Rot = 0;
        }
    }
    
    // Turn on?
    if( m_nWheels )
        m_Weight = 1.0f;
}

//=========================================================================

void loco_wheel_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    s32 i;
    
    // Should only be active track if wheels are present!
    ASSERT( m_nWheels );

    // Compute root bone L2W
    matrix4 RootL2W( Info.m_Local2AnimSpace * pDestKey[0].Rotation );
    RootL2W.SetTranslation( pDestKey[0].Translation );
    RootL2W = Info.m_Local2World * RootL2W;

    // Compute forward direction of root bone
    vector3 ForwardDir = RootL2W.RotateVector( vector3( 0.0f, 0.0f, 1.0f ) );
    
    // Update all wheels
    for( i = 0; i < m_nWheels; i++ )
    {
        // Lookup wheel
        wheel& Wheel = m_Wheels[i];

        // Skip the wheel if it's not being displayed in the geometry
        // (on some levels the anims have the wheel bones, but the geometry doesn't to save memory)
        if( Wheel.m_iBone >= Info.m_nActiveBones )
            continue;        
        
        // Lookup key
        ASSERT( Wheel.m_iBone >= 0 ) ;
        ASSERT( Wheel.m_iBone < Info.m_nActiveBones );
        anim_key& WheelKey = pDestKey[Wheel.m_iBone];
        
        // Compute wheel L2W
        matrix4 WheelL2W( WheelKey.Rotation );
        WheelL2W.SetTranslation( WheelKey.Translation );
        WheelL2W = RootL2W * WheelL2W;
        
        // Compute position
        vector3 Pos = WheelL2W.GetTranslation();

        // Any movement?
        vector3 Delta = Pos - Wheel.m_Pos;
        if( Delta.LengthSquared() > 0.0f )
        {
            // Update wheel position
            Wheel.m_Pos = Pos;
        
            // Compute forward motion of wheel
            f32 ForwardDelta = v3_Dot( Delta, ForwardDir );
            
            // Compute rotation speed in radians
            Wheel.m_DeltaRot = ForwardDelta / Wheel.m_Radius;
            
            // Update rotation
            Wheel.m_Rot += Wheel.m_DeltaRot;
        }
                
        // Finally override wheel rotation to key
        WheelKey.Rotation.Setup( vector3( 1.0f, 0.0f, 0.0f ), Wheel.m_Rot ); 
    }
}

//=========================================================================

// Start and stops a looping sound
static 
void UpdateLoopingSound(       xbool    bPlay, 
                         const char*    pLoopSfxName, 
                         const char*    pStopSfxName, 
                         const vector3& Position,
                               s32      ZoneID,
                               s32&     VoiceId )
{
    // Play?
    if( bPlay )
    {
        // Start looping sound?
        if( ( !VoiceId ) && ( pLoopSfxName ) )
        {
            VoiceId = g_AudioMgr.PlayVolumeClipped( pLoopSfxName, Position, ZoneID, TRUE );
        }

        // Update looping sound position
        if( VoiceId )
            g_AudioMgr.SetPosition( VoiceId, Position, ZoneID );
    }
    else
    {
        // Stop looping sound?
        if( VoiceId )
        {
            // Stop the looping sound
            g_AudioMgr.Release( VoiceId, 0.25f );
            VoiceId = 0;

            // Play stop sound
            if( pStopSfxName )
                g_AudioMgr.PlayVolumeClipped( pStopSfxName, Position, ZoneID, TRUE );
        }
    }
}

//=========================================================================

void loco_wheel_controller::Advance( f32 DeltaTime, vector3& DeltaPos, radian& DeltaYaw )
{
    (void)DeltaTime;
    
    // Clear deltas
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;

    // Should only be active track if wheels are present!
    ASSERT( m_nWheels );
    
    // Turn on
    m_Weight = 1.0f;

    // Compute moving and turning status
    xbool  bVehicleTurning = FALSE;
    xbool  bVehicleMoving  = FALSE;
    radian DeltaRot = m_Wheels[0].m_DeltaRot;
    for( s32 i = 0; i < m_nWheels; i++ )
    {
        // Lookup wheel
        wheel& Wheel = m_Wheels[i];

        // If any wheels are moving in a different direction to one another then vehicle is turning
        bVehicleTurning |= ( x_sign( DeltaRot ) != x_sign( Wheel.m_DeltaRot ) );
        
        // Moving?
        bVehicleMoving |= ( x_abs( Wheel.m_DeltaRot ) >= R_1 );
        
        // Clear delta for next time
        Wheel.m_DeltaRot = 0;
    }    
    
    // If wheels didn't go past move threshold, then they are not turning
    if( !bVehicleMoving )
        bVehicleTurning = FALSE;
    
    // Wheelchair can only be turning of moving
    if( bVehicleTurning )
        bVehicleMoving = FALSE;
    
    // Lookup world position of wheel chair
    ASSERT( m_pLoco );
    vector3 Position = m_pLoco->GetPosition();
    
    // Update turning sound
    s32 ZoneID = -1;
    // rmb error
    UpdateLoopingSound( bVehicleTurning, m_pTurnLoopSfx, m_pTurnStopSfx, Position, ZoneID, m_TurnLoopSfxId );
    
    // Update moving sound
    // rmb error
    UpdateLoopingSound( bVehicleMoving, m_pMoveLoopSfx, m_pMoveStopSfx, Position, ZoneID, m_MoveLoopSfxId );
}

//=========================================================================
