//=========================================================================
//
//  LocoCharAnimPlayer.hpp
//
//=========================================================================
#ifndef __LOCO_CHAR_ANIM_PLAYER_HPP__
#define __LOCO_CHAR_ANIM_PLAYER_HPP__


//=========================================================================
// INCLUDES
//=========================================================================
#include "Animation\SMemMatrixCache.hpp"
#include "LocoMotionController.hpp"
#include "Animation\BasePlayer.hpp"

//=========================================================================
// DEFINES
//=========================================================================
#define LOCO_MAX_CHAR_ANIM_PLAYER_TRACKS     9


//=========================================================================
// FORWARD DELCLARATIONS
//=========================================================================
class loco ;
class loco_ik_solver;


//=========================================================================
// LOCOMOTION CHARACTER ANIMATION PLAYER CLASS
//=========================================================================
class loco_char_anim_player : public base_player
{

//=====================================================================
// PUBLIC FUNCTIONS
//=====================================================================
public:

                            // Constructor/destructor
                            loco_char_anim_player ( void ) ;
virtual                     ~loco_char_anim_player( void ) ;

        // Animation functions

        // Tells animation player about loco owner
        void                SetLoco( loco* pLoco ) ;

        // Tells the animation system which package of animations to use.
virtual void                SetAnimGroup            ( const anim_group::handle& hGroup ) ;

        // Bone LOD control - tells the player how many bones to actually compute when mixing
        void                SetNActiveBones         ( s32 nBones ) ;

        // Sets the new animation and blends out the old animation if BlendTime is non zero
        // NOTE: If the player is currently blending and bInterruptBlend is FALSE, then
        //       the new animation will be started as soon as blending has finished
        void                SetAnim( const anim_group::handle& hAnimGroup, 
                                           s32                  iAnim,
                                           f32                  BlendTime = DEFAULT_BLEND_TIME, 
                                           f32                  Rate      = 1.0f,
                                           u32                  Flags     = 0 ) ;

        void                SetAnim( const anim_group::handle& hAnimGroup, 
                                     const char*                pAnim, 
                                           f32                  BlendTime = DEFAULT_BLEND_TIME,
                                           f32                  Rate      = 1.0f,
                                           u32                  Flags     = 0 ) ;
    
        void                SetCurrAnimFrame    ( f32 Frame ) ;         // Sets current animation frame
        void                SetCurrAnimCycle    ( s32 Cycle ) ;         // Sets current animation cycle

        // Gives access to animation tracks for reading individual yaws etc
              loco_motion_controller& GetCurrAnim     ( void ) ;
              loco_motion_controller& GetBlendAnim    ( void ) ;
        const loco_motion_controller& GetCurrAnim     ( void ) const ;
        const loco_motion_controller& GetBlendAnim    ( void ) const ;

        // Yaw functions
        radian              GetFacingYaw            ( void ) const ;        // Facing yaw (blended)
        radian              GetCurrAnimYaw          ( void ) const ;        // Current animation yaw
        void                SetCurrAnimYaw          ( radian Yaw ) ;        // Set current animation yaw
        radian              GetBlendAnimYaw         ( void ) const ;        // Blend animation yaw
        void                SetBlendAnimYaw         ( radian Yaw ) ;        // Set blend animation yaw
        void                ApplyCurrAnimDeltaYaw   ( radian DeltaYaw ) ;   // CurrAnimYaw  += DeltaYaw
        void                ApplyBlendAnimDeltaYaw  ( radian DeltaYaw ) ;   // BlendAnimYaw += DeltaYaw

        // Logic functions - advances the animation and returns the change in position that occurred
        void                Advance             ( f32 nSeconds, vector3& DeltaPos, radian& DeltaYaw ) ;
        void                Advance             ( f32 nSeconds, vector3& DeltaPos ) ;

        // Position/speed functions
        const vector3&      GetPosition         ( void ) const;         // Returns world position
        void                SetPosition         ( const vector3& Pos ) ;// Sets world position
        f32                 GetMovementSpeed    ( void ) ;

        // Prop functions

        // Returns the current L2W for a prop.  If the prop is not present
        // the return value will be FALSE and the matrix undetermined
        xbool               GetPropL2W          ( const char* pPropName, matrix4& L2W ) ;

        // Animation playback query functions
        xbool               IsBlending          ( void ) const;  // TRUE if blending between animations
        f32                 GetBlendAmount      ( void ) const;  // 0 = All blend track, 1 = All current track
        xbool               IsAtEnd             ( void ) const;  // TRUE if anim has played or looped
        f32                 GetFrame            ( void ) const;  // Current frame of current track
        s32                 GetCycle            ( void ) const;  // Cycle count of current track
        f32                 GetPrevFrame        ( void ) const;  // Previous frame of current track
        s32                 GetPrevCycle        ( void ) const;  // Previous cylce count of current track
        s32                 GetNFrames          ( void ) const;  // # of frames in current animation
        s32                 GetAnimIndex        ( void ) const;  // Index of current animation
        s32                 GetAnimTypeIndex    ( void ) const;  // Index passed to "SetAnim" (may differ from actual playing index!)
        const anim_info&    GetAnimInfo         ( void ) const;  // Info for current animation
        xbool               IsPlaying           ( const char* pAnimName, s32 iTrack=0 ) const; // TRUE if anim is playing on track

        // Animation group query functions
        const anim_group*   GetAnimGroup        ( void ) const;  // Anim group assigned to player (this may return NULL, so check it!)
        anim_group::handle  GetAnimGroupHandle  ( void ) ;  // Anim group handle assigned to player
        xbool               HasAnimGroup        ( void ) const;  // TRUE if anim group has been assigned
        s32                 GetNAnims           ( void ) const;  // # of anims in assigned anim group
        s32                 GetAnimIndex        ( const char* pAnimName ) const;               // Index of anim or -1 if not found

        // Bone query functions
        s32                 GetNBones           ( void ) const;                  // # of bones in assigned anim group
        s32                 GetNActiveBones     ( void ) const;                  // # of bones to compute (LOD)
        s32                 GetBoneIndex        ( const char* pBoneName ) const; // Index of bone, or -1 if not found
        const anim_bone&    GetBone             ( s32 iBone ) const;             // Info for bone
        const matrix4*      GetBoneL2Ws         ( void ) ;                      // All bone L2W matrices
        const matrix4&      GetBoneL2W          ( s32 iBone ) ;                 // Bone L2W matrix
              vector3       GetBonePosition     ( s32 iBone ) ;                 // Bone position
        const vector3&      GetBoneBindPosition ( s32 iBone ) const;             // Bone bind position


        // Render functions
#if !defined( CONFIG_RETAIL )
        void                RenderSkeleton      ( xbool LabelBones=FALSE ) ;
#endif // !defined( CONFIG_RETAIL )

        // Event functions
        virtual s32                 GetNEvents          ( void ) ;          // # of event in current animation
        virtual const anim_event&   GetEvent            ( s32 iEvent ) ;    // Animation event
        virtual xbool               IsEventActive       ( s32 iEvent ) ;    // TRUE if event should be fired
        xbool                       IsEventTypeActive   ( s32 Type ) ;      // TRUE if any event of type is active
        virtual vector3             GetEventPosition    ( s32 iEvent );     // World position of event.
        virtual radian3             GetEventRotation    ( s32 iEvent );     // World rotation of event.
        virtual vector3             GetEventPosition    ( const anim_event& Event );
        virtual radian3             GetEventRotation    ( const anim_event& Event );

        void                        GetEventPositionAndRotation( s32 iEvent, vector3& Position, radian3& Rotation );

        // Track functions
        void                    ClearTracks         ( void ) ;                                              // Remove tracks
        void                    ClearTrack          ( s32 iTrack ) ;                                        // Remove track
        loco_anim_controller*   GetTrack            ( s32 iTrack ) ;                                        // Assigned track
        void                    SetTrack            ( s32 iTrack, loco_anim_controller* pTrackController ) ;// Assign track
        void                    SetTracksAnimGroup  ( const anim_group::handle& hAnimGroup ) ;             // Assigns anim group 

        // IK functions
        void                    SetIKSolver         ( loco_ik_solver* pIKSolver );
        
        //
        // The YawDelta is divided by the length of the animation and is combined
        // with the animation's delta and returned in the Advance( paDeltaYaw ).  
        // 
        void                    SetYawDelta         ( const radian& YawDelta );

        // Returns the combined bbox of all the current playing animations
        bbox                    ComputeBBox         ( void );

        // Forces matrices to be re-computed the next time any bone query function is called
        void                    DirtyCachedL2Ws     ( void );

//=====================================================================
// PRIVATE FUNCTIONS
//=====================================================================

private:

        // Cached matrices functions
        void                GetInterpKeys       ( const matrix4& L2W, anim_key* pKey ) ;
        const matrix4&      GetCachedL2W        ( s32 iBone ) ;
        const matrix4*      GetCachedL2Ws       ( void ) ;
        void                UpdateCachedL2Ws    ( void ) ;

//=====================================================================
// PUBLIC DATA
//=====================================================================
public:
        // Useful bone indices
        s16                     m_iNeckBone;            // Neck bone index
        s16                     m_iHeadBone;            // Head bone index
        s16                     m_iLEyeBone;            // Index of left eye
        s16                     m_iREyeBone;            // Index of right eye
        s16                     m_iWeaponBone[2];       // Index of right/left hand weapon bone
        s16                     m_iGrenadeBone;         // Index of grenade bone
        s16                     m_iFlagBone;            // Index of mp avatar flag bone
                                                    
        // Usefull offsets
        vector3                 m_MidEyeOffset ;        // Offset between eyes from head

        // Useful matrices/positions                              
        matrix4                 m_HeadL2W ;             // Head local to world matrix
        vector3                 m_MidEyePosition ;      // Position between eyes        
        vector3                 m_AimAtOffset;          // Offset from pos for a good spot to aim at


//=====================================================================
// PRIVATE DATA
//=====================================================================
private:

        // Animation vars
        anim_group::handle      m_hAnimGroup;           // Group of anims we are using 
        s32                     m_nActiveBones ;        // Number of bones to compute (used for LODs)
        vector3                 m_WorldPos;             // World position

        // Track controller vars
        loco_motion_controller  m_AnimCurrTrack;        // Current animation controller track
        loco_motion_controller  m_AnimBlendTrack ;      // Blend animation controller track
        f32                     m_AnimBlendFrame ;      // Blend frame
        f32                     m_AnimBlendLength ;     // Blend length
        loco_anim_controller*   m_Track[LOCO_MAX_CHAR_ANIM_PLAYER_TRACKS];   // List of tracks
        
        // IK
        loco_ik_solver*         m_pIKSolver;            // User IK solver (or NULL if none)

        // Animation request vars (used to wait for blending to finish)
        xbool                   m_bRequest ;            // If TRUE, animation request is pending
        anim_group::handle      m_hRequestAnimGroup ;   // Anim group of request anim
        s32                     m_iRequestAnim ;        // Anim index of request anim
        f32                     m_RequestBlendTime ;    // Blend time of request anim
        f32                     m_RequestRate ;         // Playback rate scaler
        u32                     m_RequestFlags ;        // Flags of anim request
 
        // Locomotion vars
        loco*                   m_pLoco ;               // Pointer to owner locomotion

        // Cached matrices vars
        smem_matrix_cache       m_CachedL2Ws;           // Cached matrices

        // Chain anim vars
        f32                     m_ChainCycles;          // Cycles to play before playing chain anim
        radian                  m_YawDelta;
        f32                     m_YawStartFrame;
        f32                     m_YawEndFrame;

};

//=========================================================================
// INLINES
//=========================================================================

inline
const vector3& loco_char_anim_player::GetPosition( void ) const
{
    return m_WorldPos;
}

//=========================================================================

// Tells animation player about loco owner
inline
void loco_char_anim_player::SetLoco( loco* pLoco )
{
    m_pLoco = pLoco ;
}

//=========================================================================

inline
void loco_char_anim_player::SetAnim( const anim_group::handle& hAnimGroup, const char* pAnim, f32 BlendTime, f32 Rate, u32 Flags )
{
    SetAnim( hAnimGroup, GetAnimIndex(pAnim), BlendTime, Rate, Flags ) ;
}

//=========================================================================

inline
loco_motion_controller& loco_char_anim_player::GetCurrAnim( void )
{
    return m_AnimCurrTrack ;
}

//=========================================================================

inline
loco_motion_controller& loco_char_anim_player::GetBlendAnim( void )
{
    return m_AnimBlendTrack ;
}

//=========================================================================

inline
const loco_motion_controller& loco_char_anim_player::GetCurrAnim( void ) const
{
    return m_AnimCurrTrack ;
}

//=========================================================================

inline
const loco_motion_controller& loco_char_anim_player::GetBlendAnim( void ) const
{
    return m_AnimBlendTrack ;
}

//=========================================================================

inline
radian loco_char_anim_player::GetCurrAnimYaw  ( void ) const
{
    return m_AnimCurrTrack.GetYaw() ;
}

//=========================================================================

inline
radian loco_char_anim_player::GetBlendAnimYaw  ( void ) const
{
    return m_AnimBlendTrack.GetYaw() ;
}

//=========================================================================

inline
void loco_char_anim_player::Advance( f32 nSeconds, vector3& DeltaPos )
{
    radian DeltaYaw ;
    Advance(nSeconds, DeltaPos, DeltaYaw) ;
}

//=========================================================================

// Returns TRUE if anims are being blended
inline
xbool loco_char_anim_player::IsBlending( void ) const
{
    return (m_AnimBlendLength > 0) ;
}

// Returns blend amount (0 = all blend, 1=all current)
inline
f32 loco_char_anim_player::GetBlendAmount( void ) const
{
    if (m_AnimBlendLength > 0)
    {
        return m_AnimBlendFrame / m_AnimBlendLength ;
    }
    else
        return 1.0f ;
}

//=========================================================================

inline
xbool loco_char_anim_player::IsAtEnd( void ) const
{
    // Keep logic happy if waiting for a request
    if (m_bRequest)
        return FALSE ;

    return m_AnimCurrTrack.IsAtEnd();
}

//=========================================================================

inline
f32 loco_char_anim_player::GetFrame( void ) const
{
    return m_AnimCurrTrack.GetFrame();
}

//=========================================================================

inline
s32 loco_char_anim_player::GetCycle( void ) const
{
    return m_AnimCurrTrack.GetCycle();
}

//=========================================================================

inline
f32 loco_char_anim_player::GetPrevFrame( void ) const
{
    return m_AnimCurrTrack.GetPrevFrame();
}

//=========================================================================

inline
s32 loco_char_anim_player::GetPrevCycle( void ) const
{
    return m_AnimCurrTrack.GetPrevCycle();
}

//=========================================================================

inline
s32 loco_char_anim_player::GetNFrames( void ) const
{
    return m_AnimCurrTrack.GetNFrames();
}

//=========================================================================

inline
s32 loco_char_anim_player::GetAnimIndex( void ) const
{
    // Keep the logic happy if waiting for a request
    if (m_bRequest)
        return m_iRequestAnim ;

    return m_AnimCurrTrack.GetAnimIndex();
}

//=========================================================================

inline
s32 loco_char_anim_player::GetAnimTypeIndex( void ) const 
{
    // Keep the logic happy if waiting for a request
    if (m_bRequest)
        return m_iRequestAnim ;

    return m_AnimCurrTrack.GetAnimTypeIndex();
}

//=========================================================================

inline
const anim_info& loco_char_anim_player::GetAnimInfo( void ) const 
{
    return m_AnimCurrTrack.GetAnimInfo();
}

//=========================================================================

inline
xbool loco_char_anim_player::IsPlaying( const char* pAnimName, s32 iTrack ) const
{
    ASSERT( (iTrack>=0) && (iTrack<LOCO_MAX_CHAR_ANIM_PLAYER_TRACKS) && (m_Track[iTrack]) );
    return ((loco_anim_controller*)m_Track[iTrack])->IsPlaying( pAnimName );
}


//=========================================================================

#if defined( X_RETAIL )
inline
const anim_group* loco_char_anim_player::GetAnimGroup( void ) const
{
    anim_group* pGroup = (anim_group*)m_hAnimGroup.GetPointer();
    ASSERT( pGroup );
    return pGroup;
}
#endif

//=========================================================================

inline
anim_group::handle loco_char_anim_player::GetAnimGroupHandle( void )
{
    return m_hAnimGroup;
}

//=========================================================================

inline
xbool loco_char_anim_player::HasAnimGroup( void ) const
{
    return( m_hAnimGroup.GetPointer() != NULL );
}

//=========================================================================

inline
s32 loco_char_anim_player::GetNAnims( void ) const 
{
    const anim_group* pAnimGroup = GetAnimGroup();
    return pAnimGroup ? pAnimGroup->GetNAnims() : 0;
}

//=========================================================================

inline
s32 loco_char_anim_player::GetAnimIndex( const char* pAnimName ) const
{
    s32 Index = GetAnimGroup()->GetAnimIndex( pAnimName );
    /*
    #ifdef X_DEBUG
    if (Index == -1)
    x_DebugMsg("WARNING: Animation %s not found\n", pAnimName) ;
    #endif
    */

    return Index ;
}

//=========================================================================

inline
s32 loco_char_anim_player::GetNBones( void ) const 
{
    const anim_group* pAnimGroup = GetAnimGroup();
    return pAnimGroup ? pAnimGroup->GetNBones() : 0;
}

//=========================================================================

inline
s32 loco_char_anim_player::GetNActiveBones( void ) const
{
    return m_nActiveBones ;
}

//=========================================================================

inline
s32 loco_char_anim_player::GetBoneIndex( const char* pBoneName ) const 
{
    return GetAnimGroup()->GetBoneIndex( pBoneName );
}

//=========================================================================

inline
const anim_bone& loco_char_anim_player::GetBone( s32 iBone ) const 
{
    return GetAnimGroup()->GetBone(iBone);
}

//=========================================================================

inline
const matrix4* loco_char_anim_player::GetBoneL2Ws( void )
{
    return GetCachedL2Ws();
}

//=========================================================================
// IK functions
//=========================================================================

inline
void loco_char_anim_player::SetIKSolver( loco_ik_solver* pIKSolver )
{
    m_pIKSolver = pIKSolver;
}

//=========================================================================


//=========================================================================
#endif // END __LOCO_CHAR_ANIM_PLAYER_HPP__
//=========================================================================











































