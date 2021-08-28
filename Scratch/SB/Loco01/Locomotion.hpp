#ifndef LOCOMOTION_HPP
#define LOCOMOTION_HPP

//=========================================================================
// INLCLUDES
//=========================================================================
#include "Animation\CharAnimPlayer.hpp"

//=========================================================================
// LOCOMOTION CLASS
//=========================================================================
class loco_aim_controler : public track_controller
{
public:
                        loco_aim_controler  ( void );
    void                Init                ( s32 iLT, s32 iLC, s32 iLB,
                                              s32 iCT, s32 iCC, s32 iCB,
                                              s32 iRT, s32 iRC, s32 iRB );
    void                SetBoneMask         ( f32 Weight, const char* pBoneName );

    void                SetBlendFactor      ( f32 wSide, f32 wUpDown, f32 BlendTime=DEFAULT_BLEND_TIME );

    virtual void        SetAnimGroup        ( const rhandle<anim_group>& hGroup );
    virtual void        Clear               ( void );
    virtual void        Advance             ( f32 nSeconds, vector3& DeltaPos, radian& DeltaYaw );
    virtual void        SetWeight           ( f32 ParametricWeight );
    virtual f32         GetWeight           ( void );
    virtual void        GetInterpKeys       ( anim_key* pKey );
    virtual void        GetInterpKey        ( s32 iBone, anim_key& Key  );
    virtual void        MixKeys             ( anim_key* pDestKey ); 
    virtual void        MixKey              ( s32 iBone, anim_key& DestKey ); 

    const anim_data&    GetAnimData         ( s32 iSide );
    const anim_group&   GetAnimGroup        ( void );

protected:

    struct bone_mask
    {
        s32 iBone;
        f32 Weight;
    };

    rhandle<anim_group> m_hAnimGroup;        // Group of anims we are using

    s32                 m_iLT, m_iLC, m_iLB; // Index of the keys that we can use
    s32                 m_iCT, m_iCC, m_iCB;
    s32                 m_iRT, m_iRC, m_iRB;

    f32                 m_wSide;             // Positive is right negative is left
    f32                 m_wUpDown;           // Positive is up negative is down
    f32                 m_Weight;            // influence at mixing time

    f32                 m_Rate;              // playback rate in frames per second
    f32                 m_BlendLength;       // Stores total time we are blending
    f32                 m_BlendFrame;        // Stores point in blending we are at
    s32                 m_nFrames;           // nFrames in animation
    anim_key*           m_pBlendKey;         // Stores keys we are blending from

    f32                 m_Frame;             // Current modulated frame
    s32                 m_Cycle;             // Current Cycle, 0,1,2,3
    f32                 m_PrevFrame;         // frame before Advance()
    s32                 m_PrevCycle;         // cycle before Advance()

    bone_mask           m_BoneMask[MAX_ANIM_TRACK_BONES];     

};

//=========================================================================
// LOCOMOTION CLASS
//=========================================================================
class locomotion
{
public:

    enum states
    {
        STATE_IDLE,         // IDLE TYPE OF STUFF
        STATE_RUN,          // RUN MOTION
        STATE_CROWCH,       // CROWCH MOTION
    };

    enum motion
    {
        MOTION_FORWARD,
        MOTION_LEFT,
        MOTION_BACK,
        MOTION_RIGHT,

        MOTION_IDLE,
        MOTION_TRANSITION,
    };

            locomotion      ( void );
    void    Initialize      ( const char* pFileName );
    void    RenderSkeleton  ( xbool LabelBones=FALSE );
    void    ComputeL2W      ( matrix4* pL2W );
    vector3 GetPosition     ( void ) { return m_Player.GetPosition(); }

            void SetMoveAt  ( vector3& Target );
            void SetLootAt  ( vector3& Target );

    virtual void  OnAdvance ( f32 nSeconds );
    virtual void  OnInit    ( void ){  }

public:

    void    ComputeAim      ( radian& H, radian& V );
    motion  ComputeMotion   ( void );

    xbool SetMotionAnim( locomotion::motion FromMotion, 
                         locomotion::motion ToMotion ) ;

    radian  ComputeMoveDir  ( void );

    // Returns that target look yaw the character needs to obtain
    radian  ComputeLookYaw  ( void ) ;

    // Returns motion, given the facing yaw
    motion  ComputeMotion   ( radian FacingYaw ) ;

public:

    loco_aim_controler      m_Controler;
    char_anim_player        m_Player;
    rhandle<anim_group>     m_AnimGroup;
    vector3                 m_MoveAt;
    vector3                 m_LookAt;
    motion                  m_Motion;
    motion                  m_ToTransition;
    radian                  m_MotionYaw ;
    radian                  m_DestYaw ;
    xbool                   m_bTransition ;

};

//=========================================================================
// END
//=========================================================================
#endif