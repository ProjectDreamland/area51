//=========================================================================
//
//  LocoAnimController.hpp
//
//=========================================================================
#ifndef __LOCO_ANIM_CONTROLLER_HPP__
#define __LOCO_ANIM_CONTROLLER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_math.hpp"
#include "Animation\AnimData.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Geom.hpp"

//=========================================================================
// DEFINES
//=========================================================================
#ifndef DEFAULT_BLEND_TIME
#define DEFAULT_BLEND_TIME  (0.125f)
#endif


//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class loco_char_anim_player ;


//=========================================================================
// CLASS LOCO ANIM CONTROLLER - Tracks playback of a single animation (no blending)
//=========================================================================

class loco_anim_controller
{
//=========================================================================
// DEFINES
//=========================================================================
public:

//=========================================================================
// STRUCTURES
//=========================================================================
public:

    // Info that is available to controller when collecting/mixing keys
    struct info
    {
        s32                     m_nActiveBones ;    // Number of bones to compute (for skeleton LODs)
        matrix4                 m_Local2World ;     // Local->world of anim player
        quaternion              m_Local2AnimSpace ; // Fixup rotation used by anim player
        quaternion              m_Local2AimSpace ;  // Fixup rotation for correct aiming
        matrix4                 m_HeadL2W ;         // Head local to world matrix
        vector3                 m_MidEyePosition ;  // World position between eyes
    } ;


public:

//=========================================================================
// PUBLIC FUNCTIONS
//=========================================================================
public:
                loco_anim_controller    ( void ) ;
virtual        ~loco_anim_controller    ( void ) ;


        // Sets location of animation data package
virtual void                SetAnimGroup        ( const anim_group::handle& hAnimGroup ) ;
virtual void                SetAnimGroup        ( const char* pAnimGroup );

        // Clears the animation to a safe unused state
virtual void                Clear               ( void ) ;

        // Anim group functions - returns info about animation package
virtual const anim_group&           GetAnimGroup        ( void ) const ;
virtual const anim_group::handle&   GetAnimGroupHandle  ( void ) const ;
        const anim_info&            GetAnimInfo         ( s32 iAnim ) const ;
        s32                         GetNAnims           ( void ) const ;
        s32                         GetAnimIndex        ( const char* pAnimName ) const ;
        s32                         GetNBones           ( void ) const ;
        const anim_bone&            GetBone             ( s32 iBone ) const ;
        s32                         GetBoneIndex        ( const char* pBoneName ) const ;

        // Animation settings functions
virtual void                SetAnim             ( const anim_group::handle& hAnimGroup, s32 iAnim, u32 Flags = 0 ) ;
virtual void                SetAnim             ( const anim_group::handle& hAnimGroup, const char* pAnimName, u32 Flags = 0 ) ;

        // Logic functions - advances animation and returns delta pos and delta yaw
virtual void                Advance             ( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw ) ;

        // Animation query functions - returns info on animation being played
        s32                 GetAnimIndex        ( void ) const;     // Index of anim currently playing
        s32                 GetAnimTypeIndex    ( void ) const;     // Index that was passed into "SetAnim" call
        u32                 GetAnimFlags        ( void ) const;     // Flags for currently playing anim
        const char*         GetAnimName         ( void ) const;     // Name of anim currently playing
        s32                 GetNFrames          ( void ) const;     // # of frames in currently playing anim
        s32                 GetLoopFrame        ( void ) const;     // Frame to loop to for current anim
        const anim_info&    GetAnimInfo         ( void ) const;     // Info for currently playing anim
        xbool               IsAtEnd             ( void ) const;     // TRUE if anim has fully played 
        xbool               IsPlaying           ( const char* pAnimName ) const; // TRUE if this anim is playing
        f32                 GetPrevFrame        ( void ) const;     // Previous frame before advance
        f32                 GetFrame            ( void ) const;     // Current frame
        void                SetFrame            ( f32 Frame );      // Sets the current frame
        void                SetTime             ( f32 Time );       // Sets the current frame based on time
        f32                 GetFrameParametric  ( void ) const;     // Current frame (0=start, 1=end)
        void                SetFrameParametric  ( f32 Frame ) ;     // Sets the current frame (0=start, 1=end)
        s32                 GetPrevCycle        ( void ) const;     // Cycle before advance
        s32                 GetCycle            ( void ) const;     // Current cycle
        void                SetCycle            ( s32 Cycle ) ;     // Sets current cycle
        s32                 GetFPS              ( void ) const;     // Playback rate (in frames per second)
        void                SetRate             ( f32 Rate ) ;      // Adjust playback rate (1=normal)
        f32                 GetRate             ( void ) const;     // Playback rate (1=normal)
        const bbox&         GetBBox             ( void ) const;     // BBox of currently playing anim
        
        void                SetLooping          ( xbool bLooping ) ;    // Sets looping flags
        xbool               IsUpperBody         ( void ) const ;        // Returns TRUE if animation is an upper body
        xbool               IsFullBody          ( void ) const ;        // Returns TRUE if animation is full body
        xbool               IsPlaying           ( void ) const;         // Returns TRUE if currently playing
        xbool               IsLooping           ( void ) const { return m_bLooping;       }

        void                SetAccumYawMotion   ( xbool bEnable );
        xbool               GetAccumYawMotion   ( void ) const;
        void                SetAccumHorizMotion ( xbool bEnable );
        xbool               GetAccumHorizMotion ( void ) const;
        void                SetAccumVertMotion  ( xbool bEnable );
        xbool               GetAccumVertMotion  ( void ) const;

        void                SetRemoveYawMotion  ( xbool bEnable );
        xbool               GetRemoveYawMotion  ( void ) const;
        void                SetRemoveHorizMotion( xbool bEnable );
        xbool               GetRemoveHorizMotion( void ) const;
        void                SetRemoveVertMotion ( xbool bEnable );
        xbool               GetRemoveVertMotion ( void ) const;

        void                SetGravity          ( xbool bEnable );
        xbool               GetGravity          ( void ) const;
        void                SetWorldCollision   ( xbool bEnable );
        xbool               GetWorldCollision   ( void ) const;

        void                SetStartedOnMainTrack   ( xbool bStarted ) { m_bStartedOnMainTrack = bStarted; }
        xbool               GetStartedOnMainTrack   ( void ) const { return m_bStartedOnMainTrack; } // TRUE if anim has been started on the main track

        xbool               IsBlendingOut           ( void ) const { return m_bIsBlendingOut; } // Blending out status
        xbool               IsBlendingIn            ( void ) const { return m_bIsBlendingIn;  } // Blending in status

        void                SetBlendingOut          ( xbool bFlag ) { m_bIsBlendingOut = bFlag; } // Trigger blending out
        void                SetBlendingIn           ( xbool bFlag ) { m_bIsBlendingIn = bFlag;  } // Trigger blending in

        xbool               IsCinemaRelativeMode    ( void ) const;
        void                SetCinemaRelativeMode   ( xbool bFlag );

        xbool               IsCoverRelativeMode     ( void ) const;
        void                SetCoverRelativeMode    ( xbool bFlag );

        // Event functions
        s32                 GetNEvents          ( void ) ;          // # of event in current animation
        const anim_event&   GetEvent            ( s32 iEvent ) ;    // Animation event
        xbool               IsEventActive       ( s32 iEvent ) ;    // TRUE if event should be fired
        xbool               IsEventTypeActive   ( s32 Type ) ;      // TRUE if any event of type is active

        // Weight functions - controls the influence during the mixing process
        void                SetWeight           ( f32 Weight ) ;    // Set weight (0=off, 1=fully)
        f32                 GetWeight           ( void ) ;          // Current weight
        
        // Key mixing functions
virtual void                GetInterpKeys       ( const info& Info, anim_key* pKey ) ;      // Grabs interpolated keys
virtual void                MixKeys             ( const info& Info, anim_key* pDestKey ) ; 
        void                AdditiveMixKeys     ( const info& Info, s32 iAnim, f32 Frame, s32 iRefFrame, anim_key* pDestKey ) ;
        
        void                MaskedMixKeys       ( const info&               Info, 
                                                        s32                 iAnim, 
                                                        f32                 Frame, 
                                                  const geom::bone_masks&   BoneMasks,
                                                        anim_key*           pDestKey );

        void                MaskedMixKeys       ( const info&               Info, 
                                                        s32                 iAnim, 
                                                        f32                 Frame, 
                                                  const geom::bone_masks&   CurrentBoneMasks,
                                                  const geom::bone_masks&   BlendBoneMasks,
                                                        f32                 BoneBlend,
                                                        anim_key*           pDestKey );


//=========================================================================
// PRIVATE DATA
//=========================================================================
protected:

        anim_group::handle  m_hAnimGroup;       // Group of anims we are using
        s32                 m_iAnim;            // Index of current anim
        s32                 m_iAnimType ;       // Index that was passed into "SetAnim" call
        u32                 m_AnimFlags;        // Associated anim flags for playback
        s16                 m_nFrames;          // # of frames in animation
        s16                 m_EndFrameOffset;   // # of frames from end of anim to trigger it's finished
        f32                 m_Frame;            // Current modulated frame
        s32                 m_Cycle;            // Current Cycle, 0,1,2,3
        f32                 m_Weight;           // Influence at mixing time
        f32                 m_PrevFrame;        // Frame before Advance()
        s32                 m_PrevCycle;        // Cycle before Advance()
        f32                 m_Rate;             // Playback rate in frames per second
        
        u32                 m_bLooping            : 1,   // TRUE if playing a looping anim
                            m_bAccumYawMotion     : 1,   // TRUE if delta Yaw motion should be extracted
                            m_bAccumHorizMotion   : 1,   // TRUE if delta XZ motion should be extracted
                            m_bAccumVertMotion    : 1,   // TRUE if delta Y motion should be extracted
                            m_bRemoveYawMotion    : 1,   // TRUE if yaw motion should be removed from motion bone
                            m_bRemoveHorizMotion  : 1,   // TRUE if horiz motion should be removed from motion bone
                            m_bRemoveVertMotion   : 1,   // TRUE if vert motion should be removed from motion bone
                            m_bGravity            : 1,   // TRUE if gravity should be applied
                            m_bWorldCollision     : 1,   // TRUE if world collision should happen
                            m_bStartedOnMainTrack : 1,   // TRUE if anim has been started on the main track
                            m_bIsBlendingOut      : 1,   // TRUE if anim is blending out
                            m_bIsBlendingIn       : 1;   // TRUE if anim if blending in

//=========================================================================
// FRIENDS
//=========================================================================
friend class loco_char_anim_player;
friend class loco;
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

inline 
void loco_anim_controller::SetAccumYawMotion( xbool bEnable )
{
    m_bAccumYawMotion = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetAccumYawMotion( void ) const
{
    return m_bAccumYawMotion;
}

//=========================================================================

inline 
void loco_anim_controller::SetAccumHorizMotion( xbool bEnable )
{
    m_bAccumHorizMotion = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetAccumHorizMotion( void ) const
{
    return m_bAccumHorizMotion;
}

//=========================================================================

inline 
void loco_anim_controller::SetAccumVertMotion( xbool bEnable )
{
    m_bAccumVertMotion = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetAccumVertMotion( void ) const
{
    return m_bAccumVertMotion;
}

//=========================================================================

inline 
void loco_anim_controller::SetRemoveYawMotion( xbool bEnable )
{
    m_bRemoveYawMotion = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetRemoveYawMotion( void ) const
{
    return m_bRemoveYawMotion;
}

//=========================================================================

inline 
void loco_anim_controller::SetRemoveHorizMotion( xbool bEnable )
{
    m_bRemoveHorizMotion = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetRemoveHorizMotion( void ) const
{
    return m_bRemoveHorizMotion;
}

//=========================================================================

inline 
void loco_anim_controller::SetRemoveVertMotion( xbool bEnable )
{
    m_bRemoveVertMotion = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetRemoveVertMotion( void ) const
{
    return m_bRemoveVertMotion;
}

//=========================================================================

inline 
void loco_anim_controller::SetGravity( xbool bEnable )
{
    m_bGravity = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetGravity( void ) const
{
    return m_bGravity;
}

//=========================================================================

inline 
void loco_anim_controller::SetWorldCollision( xbool bEnable )
{
    m_bWorldCollision = bEnable;
}

//=========================================================================

inline 
xbool loco_anim_controller::GetWorldCollision( void ) const
{
    return m_bWorldCollision;
}

//=========================================================================

inline
s32 loco_anim_controller::GetAnimIndex( void ) const
{
    return m_iAnim;
}

//=========================================================================

inline
s32 loco_anim_controller::GetAnimTypeIndex( void ) const
{
    return m_iAnimType ;
}

//=========================================================================

inline
u32 loco_anim_controller::GetAnimFlags( void ) const
{
    return m_AnimFlags;
}

//=========================================================================

inline
s32 loco_anim_controller::GetNFrames( void ) const 
{
    return m_nFrames;
}

//=========================================================================

inline
const anim_info& loco_anim_controller::GetAnimInfo( void ) const
{
    ASSERT( m_iAnim != -1 );
    const anim_info& AnimInfo = GetAnimGroup().GetAnimInfo(m_iAnim) ;
    return AnimInfo ;
}

//=========================================================================

inline
f32 loco_anim_controller::GetPrevFrame( void ) const 
{
    return m_PrevFrame;
}

//=========================================================================

inline
f32 loco_anim_controller::GetFrame( void ) const 
{
    return m_Frame;
}

//=========================================================================

inline
s32 loco_anim_controller::GetPrevCycle( void ) const 
{
    return m_PrevCycle;
}

//=========================================================================

inline
s32 loco_anim_controller::GetCycle( void ) const 
{
    return m_Cycle;
}

//=========================================================================

inline
void loco_anim_controller::SetCycle( s32 Cycle )
{
    m_Cycle = Cycle;
}

//=========================================================================

inline
void loco_anim_controller::SetRate( f32 Rate )
{
    ASSERT( m_Rate >= 0 );
    m_Rate = Rate;
}

//=========================================================================

inline
f32 loco_anim_controller::GetRate( void ) const
{
    ASSERT( m_Rate >= 0 );
    return m_Rate;
}

//=========================================================================

inline
const bbox& loco_anim_controller::GetBBox( void ) const
{
    const anim_info& AnimInfo = GetAnimInfo();
    return AnimInfo.GetBBox();
}

//=========================================================================

inline
void loco_anim_controller::SetLooping( xbool bLooping )
{
    m_bLooping = bLooping ; 
}

//=========================================================================

inline
const anim_event& loco_anim_controller::GetEvent( s32 iEvent ) 
{
    return GetAnimInfo().GetEvent( iEvent );
}

//=========================================================================

inline
xbool loco_anim_controller::IsEventActive( s32 iEvent ) 
{
    return GetAnimInfo().IsEventActive( iEvent, m_Frame, m_PrevFrame);
}

//=========================================================================

inline
xbool loco_anim_controller::IsEventTypeActive( s32 Type ) 
{
    return GetAnimInfo().IsEventTypeActive( Type, m_Frame, m_PrevFrame);
}

//=========================================================================

inline
f32 loco_anim_controller::GetWeight( void )
{
    return m_Weight;
}

//=========================================================================
#endif // END __LOCO_ANIM_CONTROLLER_HPP__
//=========================================================================
