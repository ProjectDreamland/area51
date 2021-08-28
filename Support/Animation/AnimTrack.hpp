//=========================================================================
//
//  ANIMTRACK.HPP
//
//=========================================================================
#ifndef ANIMTRACK_HPP
#define ANIMTRACK_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BasePlayer.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

//=========================================================================
// CLASS ANIM_TRACK
//=========================================================================

#define DEFAULT_BLEND_TIME  (0.125f)

//=========================================================================

class track_controller
{

public:
                    track_controller          ( void );
    virtual        ~track_controller          ( void );

    // Sets location of animation data package
    virtual void    SetAnimGroup        ( const anim_group::handle& hGroup ) = 0;

    // Clears the animation to a safe unused state
    virtual void    Clear               ( void ) = 0;

    // Advances the current track by logic time
    virtual void    Advance             ( f32 nSeconds ) = 0;

    // Controls the influence this anim has during the mixing process
    virtual void    SetWeight           ( f32 ParametricWeight ) = 0;
    virtual f32     GetWeight           ( void ) = 0;

    // Returns the raw keyframe data
    virtual void    GetInterpKeys       ( anim_key* pKey ) = 0;
    virtual void    GetInterpKey        ( s32 iBone, anim_key& Key  ) = 0;

    // Mixes the anims keyframes into the dest keyframes
    virtual void    MixKeys             ( anim_key* pDestKey ) = 0; 

    // Removes yaw from root node of turn animations
    virtual void    SetRemoveTurnYaw    ( xbool bRemove ) { (void)bRemove ; }
};

//=========================================================================

class anim_track_controller : public track_controller
{
public:
    enum mix_mode
    {
        MIX_BLENDED,
        MIX_ADDITIVE,
    };

public:
                        anim_track_controller          ( void );
    virtual            ~anim_track_controller         ( void );

    // Sets location of animation data package
    void                SetAnimGroup        ( const anim_group::handle& AnimGroup );

    // Clears the animation to a safe unused state
    void                Clear               ( void );

    // Gets keys for requested frame
    void                GetInterpKeys       ( f32 Frame, anim_key* pKey ) ;

    // Sets a new animation and initializes the blend buffer
    void                SetAnim             ( s32 iAnim,             f32 BlendTime = DEFAULT_BLEND_TIME , xbool ResetFrameCount = FALSE );
    void                SetAnim             ( const char* pAnimName, f32 BlendTime = DEFAULT_BLEND_TIME );

    // Advances the current animation
    void                Advance             ( f32 nSeconds );

    // Overrides the current cursor time in the anim
    void                SetFrame            ( f32 Frame );
    void                SetCycle            ( s32 Cycle );
    f32                 GetFrame            ( void );
    s32                 GetCycle            ( void );
    f32                 GetFrameParametric  ( void );
    void                SetFrameParametric  ( f32 Frame );


    // Controls the influence this anim has during the mixing process
    void                SetWeight           ( f32 ParametricWeight );
    f32                 GetWeight           ( void );

    // Returns the raw keyframe data
    void                GetInterpKeys       ( anim_key* pKey );
    void                GetInterpKey        ( s32 iBone, anim_key& Key  );

    // Mixes the anims keyframes into the dest keyframes
    void                SetMixMode          ( mix_mode Mode );

    void                MixKeys             ( anim_key* pDestKey ); 

    void                SetAdditveRefFrame  ( s32 iRefFrame );
    s32                 GetAdditiveRefFrame ( void );


    // Controls the playback rate of the anim, 1.0 = original speed
    void                SetRate             ( f32 PlaybackRateScale );
    f32                 GetRate             ( void );

    // Returns the frame and cycle before the previous call to Advance
    f32                 GetPrevFrame        ( void );
    s32                 GetPrevCycle        ( void );

    // Returns true if pegged at end or has looped past first cycle
    xbool               IsAtEnd             ( void );

    // Returns true if the current animation equals AnimName
    xbool               IsPlaying           ( const char* pAnimName );

    // Returns true if the current animations is a turn
    xbool               IsPlayingTurnAnim   ( void ) ;
    
    // Returns true if the current animations is a transition
    xbool               IsPlayingTransitionAnim   ( void ) ;

    // Returns info about animation package
    s32                 GetNAnims           ( void );
    const anim_group&   GetAnimGroup        ( void );
    s32                 GetAnimIndex        ( const char* pAnimName );

    // Returns info on animation currently being played
    s32                 GetAnimIndex        ( void );
    const char*         GetAnimName         ( void );
    s32                 GetNFrames          ( void );
    const anim_info&    GetAnimInfo         ( void );

    // Returns vector the root node traversed from start to end of anim
    vector3             GetTotalTranslation ( void );

    // Returns information about bones in skeleton
    s32                 GetNBones           ( void );
    const anim_bone&    GetBone             ( s32 iBone );
    s32                 GetBoneIndex        ( const char* pBoneName );

    // Returns information on events in animation
    s32                 GetNEvents          ( void );
    const anim_event&   GetEvent            ( s32 iEvent );
    xbool               IsEventActive       ( s32 iEvent );
    xbool               IsEventTypeActive   ( s32 Type );
    inline void         SetManualYaw( xbool byaw ) { m_bManualYaw = byaw; }
    void                SetOverrideRootBlend( xbool bOverrideRootBlend );
    void                SetRemoveTurnYaw    ( xbool bRemove ) ;

protected:
    
    void                BlendedMixKeys      ( anim_key* pDestKey ); 
    void                AdditiveMixKeys     ( anim_key* pDestKey );

    
private:

    anim_group::handle  m_hAnimGroup;       // Group of anims we are using

    s32                 m_iAnim;            // Index of current anim
    s32                 m_nFrames;          // nFrames in animation
    f32                 m_Frame;            // Current modulated frame
    s32                 m_Cycle;            // Current Cycle, 0,1,2,3
    f32                 m_Weight;           // influence at mixing time
    f32                 m_PrevFrame;        // frame before Advance()
    s32                 m_PrevCycle;        // cycle before Advance()
    f32                 m_Rate;             // playback rate in frames per second

    anim_key*           m_pBlendKey;        // Stores keys we are blending from
    f32                 m_BlendLength;      // Stores total time we are blending
    f32                 m_BlendFrame;       // Stores point in blending we are at
    xbool               m_bManualYaw;
	xbool				m_bPreviousManualYaw; 
    xbool               m_bOverrideRootBlend;
    xbool               m_bRemoveTurnYaw ;      // Removes yaw from turn animations
    s32                 m_iRefFrame;        // Reference frame used for additive blending

    mix_mode            m_MixMode;
};

//=========================================================================
#endif // END ANIMTRACK_HPP
//=========================================================================
