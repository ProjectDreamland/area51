//=========================================================================
//
//  CHARANIMPLAYER.HPP
//
//=========================================================================
#ifndef CHAR_ANIM_PLAYER_HPP
#define CHAR_ANIM_PLAYER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "animtrack.hpp"
#include "BasePlayer.hpp"

//=========================================================================
// ANIM EVENT TYPES
//=========================================================================

// NOTE: - These events MUST match up with the type list in 
//         the max script "JV_B52_AnimPrim_event.ms"
enum anim_events
{
    // Define                               MaxPluginEventName
    ANIM_EVENT_NULL = 0 ,                   // DON'T NEED ONE!

    ANIM_EVENT_SFX_FOOT_HEEL,               // "SFX_Foot_Heel",
    ANIM_EVENT_SFX_FOOT_TOE,                // "SFX_Foot_Toe",
    ANIM_EVENT_SFX_ATTACK_SWOOSH,           // "SFX_AttackSwosh",

    ANIM_EVENT_PAIN_CLAW_FORWARD,           // "PAIN_ClawForward",
    ANIM_EVENT_PAIN_CLAW_LEFT_2_RIGHT,      // "PAIN_ClawLeft2Right",
    ANIM_EVENT_PAIN_CLAW_RIGHT_2_LEFT,      // "PAIN_ClawRight2Left",

    ANIM_EVENT_PAIN_HOWL,                   // "PAIN_Howl",
    
    ANIM_EVENT_PAIN_FIST_FORWARD,           // "PAIN_FistForward",
    ANIM_EVENT_PAIN_FIST_LEFT_2_RIGHT,      // "PAIN_FistLeft2Right",
    ANIM_EVENT_PAIN_FIST_RIGHT_2_LEFT,      // "PAIN_FistRight2Left",
    
    ANIM_EVENT_PAIN_FOOT_FORWARD,           // "PAIN_FootForward",
    ANIM_EVENT_PAIN_FOOT_LEFT_2_RIGHT,      // "PAIN_FootLeft2Right",
    ANIM_EVENT_PAIN_FOOT_RIGHT_2_LEFT,      // "PAIN_FootRight2Left",
    
    ANIM_EVENT_PAIN_WEAPON_FORWARD,         // "PAIN_WeaponForward",
    ANIM_EVENT_PAIN_WEAPON_LEFT_2_RIGHT,    // "PAIN_WeaponLeft2Right",
    ANIM_EVENT_PAIN_WEAPON_RIGHT_2_LEFT,    // "PAIN_WeaponRight2Left",
	
	ANIM_EVENT_GRAB_BEGIN,
	ANIM_EVENT_GRAB_ATTACH,
	ANIM_EVENT_IMPALE_BEGIN,
	ANIM_EVENT_IMPALE_ATTACH,
	ANIM_EVENT_IMPALE_DISMOUNT,
    ANIM_EVENT_SFX_ATTACKGRUNTSHORT,        // "SFX_AttackGruntShort"
    ANIM_EVENT_SFX_ATTACKGRUNTLONG,         // SFX_AttackGruntLong"
    ANIM_EVENT_SFX_HOWLRAGE,                // SFX_HowlRage"

	ANIM_EVENT_PRIMARY_FIRE,
	ANIM_EVENT_SECONDARY_FIRE,

    ANIM_EVENT_SFX_WEAPONLOOP,
    ANIM_EVENT_SFX_SWITCH_TO,
    ANIM_EVENT_SFX_SWITCH_FROM,
    ANIM_EVENT_SFX_RELOAD,
    ANIM_EVENT_SFX_ALT_RELOAD,
    
    ANIM_EVENT_GENERIC_TIMERANGE,
    
    ANIM_EVENT_GRAB_GRENADE,
    ANIM_EVENT_RELEASE_GRENADE,

	ANIM_EVENT_WPN_PRIMARY_FIRE_LR,
	ANIM_EVENT_WPN_PRIMARY_FIRE_LL,
	ANIM_EVENT_WPN_PRIMARY_FIRE_UR,
	ANIM_EVENT_WPN_PRIMARY_FIRE_UL,
	ANIM_EVENT_WPN_SECONDARY_FIRE_LR,
	ANIM_EVENT_WPN_SECONDARY_FIRE_LL,
	ANIM_EVENT_WPN_SECONDARY_FIRE_UR,
	ANIM_EVENT_WPN_SECONDARY_FIRE_UL,

    ANIM_EVENT_TOTAL
};

//=========================================================================
// CLASS CHAR_ANIM_PLAYER
//=========================================================================

#define MAX_CHAR_ANIM_PLAYER_TRACKS     8

class char_anim_player : public base_player
{

//-------------------------------------------------------------------------
public:

            char_anim_player( void );
    virtual ~char_anim_player( void );

    //
    // Tells the animation system which package of animations to use.
    //
    virtual void    SetAnimGroup        ( const anim_group::handle& hGroup );

    //
    // Sets the current Track0 animation.  The Manual bools tell the animation
    // system how much of the root position you are controlling.
    //
    void    SetAnim         ( s32 iAnim, xbool ManualVert, xbool ManualHoriz, f32 BlendTime=DEFAULT_BLEND_TIME , xbool ResetFrameCount = FALSE );
    void    SetAnimHoriz    ( s32 iAnim,                                      f32 BlendTime=DEFAULT_BLEND_TIME );
    void    SetAnimVert     ( s32 iAnim,                                      f32 BlendTime=DEFAULT_BLEND_TIME );
    void    SetAnim         ( const char* pAnimName, xbool ManualVert, xbool ManualHoriz, f32 BlendTime=DEFAULT_BLEND_TIME );
    void    SetAnimHoriz    ( const char* pAnimName,                                      f32 BlendTime=DEFAULT_BLEND_TIME );
    void    SetAnimVert     ( const char* pAnimName,                                      f32 BlendTime=DEFAULT_BLEND_TIME );

    void    SetManualYawControl ( xbool bIsManual );
    void    SetOverrideRootBlend( xbool bOverrideRootBlend );

    void    SetRemoveTurnYaw  ( xbool bRemoveTurnYaw ) ;    
    void    SetMirrorBone       ( s32 iBone )   { m_iMirrorBone = iBone; }
      
    //
    // The render offset translates the skeleton in addition to the animation.
    // This is purely a render effect.
    //
    void    SetRenderOffset   ( const vector3& RenderOffset );
    vector3 GetRenderOffset   ( void );

    //
    // The SlideDelta is divided by the length of the animation and is combined
    // with the animation's delta and returned in the Advance( DeltaPos ).  
    // The SlideDelta is always described in worldspace.
    // 
    void    SetSlideDelta    ( const vector3& SlideDelta );

    //
    // The YawDelta is divided by the length of the animation and is combined
    // with the animation's delta and returned in the Advance( paDeltaYaw ).  
    // 
    void    SetYawDelta    ( const radian& YawDelta );

    //
    // The basis matrix rotates the universe onto a different plane.
    // The animation system is still controlled by the Yaw to steer the 
    // character left and right but the notion of up and forward will have
    // been rotated.
    //
    void        SetBasisMatrix  ( const matrix4& BasisM );
    matrix4&    GetBasisMatrix  ( void );
    matrix4&    GetPreviousBasisMatrix  ( void );

    //
    // This returns the total translation of the animation in world space
    // taking into account the characters scale and orientation.  This is
    // useful to predict the position of the character at the end of the 
    // animation.
    //
    vector3 GetWorldAnimTranslation( void );

    // Gets the bind posotion of the bone
    vector3 GetBindPosition   ( s32 iBone ) ;

    //
    // Returns TRUE if at last frame of animation or in a cycle > 0
    //
    xbool   IsAtEnd         ( void );
    
    //
    // Advances the animation and returns the change in position that occurred
    //
    void    Advance         ( f32 nSeconds, vector3& DeltaPos, radian& DeltaYaw );
    void    Advance         ( f32 nSeconds, vector3& DeltaPos );

    //
    // Skips directly to a particular frame and cycle of the animation
    //
    void    SetFrame        ( f32 Frame );
    void    SetCycle        ( s32 Cycle );

    vector3 GetPosition     ( void ) const;
	radian3	GetRotation		( void ) const;
	
    radian  GetYaw          ( void );
    radian  GetPitch        ( void );
    radian  GetRoll         ( void );
    f32     GetScale        ( void );
    
    void    SetPosition                 ( const vector3& Pos );
    void    SetYaw                      ( radian Yaw );
    void    SetPitch                    ( radian Pitch );
    void    SetRoll                     ( radian Roll  );
    void    SetScale                    ( f32 Scale );
    void    SetRotationAndPosition      ( const matrix4& L2W );

    //
    // Returns the cached L2W matrix for that bone
    //
    const matrix4*  GetBoneL2Ws     ( void );
    const matrix4&  GetBoneL2W      ( s32 iBone );

    //
    // Returns the cached world space bone position
    //
    const vector3*  GetBonePositions( void );
    const vector3&  GetBonePosition ( s32 iBone );

    //
    // Returns the current L2W for a prop.  If the prop is not present
    // the return value will be FALSE and the matrix undetermined
    //
    xbool           GetPropL2W      ( const char* pPropName, matrix4& L2W );

    f32     GetFrame        ( void );
    s32     GetCycle        ( void );
    f32     GetPrevFrame    ( void );
    s32     GetPrevCycle    ( void );

    s32     GetNFrames      ( void );
    s32     GetNBones       ( void );
    s32     GetNAnims       ( void );
    f32     GetSpeed        ( void );
    s32     GetBoneIndex    ( const char* pBoneName );
    
    
    f32     GetFrameParametric  ( void );
    void    SetFrameParametric  ( f32 Frame );

    s32     GetAnimIndex    ( void );
    s32     GetAnimIndex    ( const char* pAnimName );
    xbool   IsPlaying       ( const char* pAnimName, s32 iTrack=0 );

    const anim_info&    GetAnimInfo    ( void );
    const anim_bone&    GetBone        ( s32 iBone );
    const anim_group&   GetAnimGroup   ( void );
    anim_group::handle GetAnimGroupHandle( void );
    xbool               HasAnimGroup   ( void );




    //
    // Render
    //
    void    RenderSkeleton  ( xbool LabelBones=FALSE );

    //
    // Events
    //
    virtual s32                 GetNEvents          ( void );
    virtual const anim_event&   GetEvent            ( s32 iEvent );
    virtual xbool               IsEventActive       ( s32 iEvent );
            xbool               IsEventTypeActive   ( s32 Type );
    virtual vector3             GetEventPosition    ( s32 iEvent );
    virtual radian3             GetEventRotation    ( s32 iEvent );
    virtual vector3             GetEventPosition    ( const anim_event& Event );
    virtual radian3             GetEventRotation    ( const anim_event& Event );

    //
    // Track management
    //
    void                SetTrackController  ( s32 iTrack, track_controller* pTrackController );
    track_controller*   GetTrackController  ( s32 iTrack );
    void                ClearTracks         ( void );
    void                ClearTrack          ( s32 iTrack );

//-------------------------------------------------------------------------

private:

    void                ComputeAnimToWorldM ( matrix4& M );
    void                PrepareRootKey      ( anim_key& Key );
    void                CalcBasisVectors    ( const vector3& world, vector3& i, vector3& j, vector3& k);

    void                GetInterpKeys       ( anim_key* pKey );

    void                DirtyCachedL2W      ( void );
    const matrix4&      GetCachedL2W        ( s32 iBone );
    const matrix4*      GetCachedL2Ws       ( void );
    void                ComputeBonesL2W     ( const matrix4& L2W, anim_key* pKey, matrix4* pBoneL2W );
    void                UpdateCachedL2W     ( void );

    void                DirtyCachedBonePos  ( void );
    const vector3*      GetCachedBonePoss   ( void );
    const vector3&      GetCachedBonePos    ( s32 iBone );
    void                UpdateCachedBonePos ( void );

//-------------------------------------------------------------------------

private:

    anim_group::handle  m_hAnimGroup;       // Group of anims we are using 

    xbool               m_bManualVert;      // Don't apply Y trans at render time
    xbool               m_bManualHoriz;     // Don't apply XZ trans at render time
    xbool               m_bManualYaw;       // Don't apply Yaw at render time
    xbool               m_bRemoveTurnYaw ;  // Removes yaw from root node of turn animations
    s32                 m_iMirrorBone;      // Index of bone to mirror
    
    vector3             m_WorldPos;
    radian3             m_WorldRot;
    f32                 m_WorldScale;
    radian              m_AnimHandleYaw;    // Handle of current animation

    vector3             m_RenderOffset;
    vector3             m_SlideDelta;
    radian              m_YawDelta;
    f32                 m_YawStartFrame;
    f32                 m_YawEndFrame;

    matrix4             m_BasisM;
    matrix4             m_PreviousBasisM;


    // Track controller array
    anim_track_controller   m_AnimTrack;
    track_controller*       m_Track[MAX_CHAR_ANIM_PLAYER_TRACKS];

    // Cached Current Matrices
    xbool               m_CachedL2WIsDirty;
    matrix4*            m_pCachedL2W;
    s32                 m_nCachedL2WUpdates;
    s32                 m_nCachedL2WDirties;

    // Cached Current Bone Positions
    xbool               m_CachedBonePosIsDirty;
    vector3*            m_pCachedBonePos;
    s32                 m_nCachedBonePosUpdates;
    s32                 m_nCachedBonePosDirties;
};

//=========================================================================
// INLINES
//=========================================================================

#if defined(X_RETAIL)
inline
const anim_group& char_anim_player::GetAnimGroup( void )
{
    anim_group* pG = (anim_group*)m_hAnimGroup.GetPointer();
    ASSERT( pG );
    return *pG;
}
#endif

//=========================================================================

inline
anim_group::handle char_anim_player::GetAnimGroupHandle( void )
{
    return m_hAnimGroup;
}

//=========================================================================

inline
void char_anim_player::SetRenderOffset( const vector3& RenderOffset )
{
    ASSERT( RenderOffset.IsValid() );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_RenderOffset = RenderOffset;
}

//=========================================================================

inline
vector3 char_anim_player::GetRenderOffset( void )
{
    return m_RenderOffset;
}

//=========================================================================

inline
xbool char_anim_player::IsAtEnd( void )
{
    return m_AnimTrack.IsAtEnd();
}

//=========================================================================

inline
void char_anim_player::SetFrame( f32 Frame )
{
    ASSERT( x_isvalid( Frame ) );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_AnimTrack.SetFrame( Frame );
}

//=========================================================================

inline
void char_anim_player::SetCycle( s32 Cycle )
{
    DirtyCachedL2W(); // Mark cached L2W matrices as unusable
    m_AnimTrack.SetCycle( Cycle );
}

//=========================================================================

inline
void char_anim_player::SetAnim( const char* pAnimName, xbool ManualVert, xbool ManualHoriz, f32 BlendTime )
{
    SetAnim( GetAnimIndex(pAnimName), ManualVert, ManualHoriz, BlendTime );
}

//=========================================================================

inline
void char_anim_player::Advance( f32 nSeconds, vector3&  DeltaPos )
{
    radian DummyDeltaYaw;
    Advance(nSeconds, DeltaPos, DummyDeltaYaw);
}

//=========================================================================

inline
void char_anim_player::SetPosition( const vector3& Pos )
{
    ASSERT( Pos.IsValid() );
    
    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_WorldPos = Pos;
}

//=========================================================================

inline
void char_anim_player::SetPitch( radian Pitch )
{
    ASSERT( x_isvalid( Pitch ) );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_WorldRot.Pitch = Pitch;
}

//=========================================================================

inline
void char_anim_player::SetYaw( radian Yaw )
{
    ASSERT( x_isvalid( Yaw ) );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_WorldRot.Yaw = Yaw;
}

//=========================================================================

inline
void char_anim_player::SetRoll( radian Roll )
{
    ASSERT( x_isvalid( Roll ) );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_WorldRot.Roll = Roll;
}

//=========================================================================

inline
void char_anim_player::SetScale( f32 Scale )
{
    ASSERT( x_isvalid( Scale ) );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_WorldScale = Scale;
}

//=========================================================================

inline
vector3 char_anim_player::GetPosition( void ) const
{
    return m_WorldPos;
}

//=========================================================================

inline
radian3 char_anim_player::GetRotation( void ) const
{
    return m_WorldRot;
}

//=========================================================================

inline
radian char_anim_player::GetPitch( void ) 
{
    return m_WorldRot.Pitch;
}

//=========================================================================

inline
radian char_anim_player::GetYaw( void ) 
{
    return m_WorldRot.Yaw;
}

//=========================================================================

inline
radian char_anim_player::GetRoll( void ) 
{
    return m_WorldRot.Roll;
}

//=========================================================================

inline
f32 char_anim_player::GetScale( void )
{
    return m_WorldScale;
}

//=========================================================================

inline
const vector3* char_anim_player::GetBonePositions( void )
{
    return GetCachedBonePoss();
}

//=========================================================================

inline
const vector3& char_anim_player::GetBonePosition ( s32 iBone )
{
    return GetCachedBonePos( iBone );
}

//=========================================================================

inline
f32 char_anim_player::GetFrame( void ) 
{
    return m_AnimTrack.GetFrame();
}

//=========================================================================

inline
s32 char_anim_player::GetCycle( void ) 
{
    return m_AnimTrack.GetCycle();
}

//=========================================================================

inline
f32 char_anim_player::GetPrevFrame( void ) 
{
    return m_AnimTrack.GetPrevFrame();
}

//=========================================================================

inline
s32 char_anim_player::GetPrevCycle( void ) 
{
    return m_AnimTrack.GetPrevCycle();
}

//=========================================================================

inline
s32 char_anim_player::GetNFrames( void ) 
{
    return m_AnimTrack.GetNFrames();
}

//=========================================================================

inline
s32 char_anim_player::GetAnimIndex( void ) 
{
    return m_AnimTrack.GetAnimIndex();
}

//=========================================================================

inline
s32 char_anim_player::GetNAnims( void ) 
{
    return GetAnimGroup().GetNAnims();
}

//=========================================================================

inline
s32 char_anim_player::GetNBones( void ) 
{
    return GetAnimGroup().GetNBones();
}

//=========================================================================

inline
const matrix4* char_anim_player::GetBoneL2Ws( void )
{
    return GetCachedL2Ws();
}

//=========================================================================

inline
const matrix4& char_anim_player::GetBoneL2W( s32 iBone )
{
    return GetCachedL2W(iBone);
}

//=========================================================================

inline
s32 char_anim_player::GetAnimIndex    ( const char* pAnimName ) 
{
    return GetAnimGroup().GetAnimIndex( pAnimName );
}

//=========================================================================

inline
xbool char_anim_player::IsPlaying( const char* pAnimName, s32 iTrack )
{
    ASSERT( (iTrack>=0) && (iTrack<MAX_CHAR_ANIM_PLAYER_TRACKS) && (m_Track[iTrack]) );
    return ((anim_track_controller*)m_Track[iTrack])->IsPlaying( pAnimName );
}

//=========================================================================

inline
f32 char_anim_player::GetSpeed( void ) 
{
    return GetAnimGroup().GetAnimInfo( m_AnimTrack.GetAnimIndex() ).GetSpeed();
}

//=========================================================================

inline
const anim_info& char_anim_player::GetAnimInfo( void ) 
{
    return m_AnimTrack.GetAnimInfo();
}

//=========================================================================

inline
s32 char_anim_player::GetNEvents( void ) 
{
    return m_AnimTrack.GetNEvents();
}

//=========================================================================

inline
const anim_event& char_anim_player::GetEvent( s32 iEvent ) 
{
    return m_AnimTrack.GetEvent(iEvent);
}

//=========================================================================

inline
xbool char_anim_player::IsEventActive( s32 iEvent ) 
{
    return m_AnimTrack.IsEventActive(iEvent);
}

//=========================================================================

inline
xbool char_anim_player::IsEventTypeActive( s32 Type ) 
{
    return m_AnimTrack.IsEventTypeActive(Type);
}

//=========================================================================

inline
s32 char_anim_player::GetBoneIndex( const char* pBoneName ) 
{
    return GetAnimGroup().GetBoneIndex( pBoneName );
}

//=========================================================================

inline
const anim_bone& char_anim_player::GetBone( s32 iBone ) 
{
    return GetAnimGroup().GetBone(iBone);
}

//=========================================================================

inline
f32 char_anim_player::GetFrameParametric( void )
{
    return m_AnimTrack.GetFrameParametric();
}

//=========================================================================

inline
void char_anim_player::SetFrameParametric( f32 Frame )
{
    ASSERT( x_isvalid( Frame ) );

    m_AnimTrack.SetFrameParametric( Frame );
}

//=========================================================================

inline
track_controller* char_anim_player::GetTrackController( s32 iTrack )
{
    ASSERT( (iTrack>=0) && (iTrack<MAX_CHAR_ANIM_PLAYER_TRACKS) );
    return m_Track[iTrack];
}

//=========================================================================

inline
xbool char_anim_player::HasAnimGroup( void )
{
    return( !m_hAnimGroup.IsNull() );
}

//=========================================================================

inline
void char_anim_player::SetBasisMatrix( const matrix4& BasisM )
{
    ASSERT( BasisM.IsValid() );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_BasisM = BasisM;
}

//=========================================================================

inline
matrix4& char_anim_player::GetBasisMatrix( void )
{
    return m_BasisM;
}

//=========================================================================

inline
matrix4& char_anim_player::GetPreviousBasisMatrix( void )
{
    return m_PreviousBasisM;
}

//=========================================================================

inline
vector3 char_anim_player::GetBindPosition   ( s32 iBone ) 
{
    const anim_bone& AnimBone = GetAnimGroup().GetBone(iBone) ;
    return AnimBone.BindTranslation ;
}

//=========================================================================
#endif // END CHAR_ANIM_PLAYER_HPP
//=========================================================================

