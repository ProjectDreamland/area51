//=============================================================================
//  DEBRIS
//=============================================================================

#ifndef __DEBRIS_MESON_LASH_HPP__
#define __DEBRIS_MESON_LASH_HPP__

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "..\objects\Render\RigidInst.hpp"
#include "..\PainMgr\Pain.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "audiomgr\AudioMgr.hpp"
#include "..\Objects\Player.hpp"
#include "NetworkMgr\NetObj.hpp"

#define MAX_LASHES                  8
#define MAX_LASH_TARGETS            64
#define MAX_BEAMS                   10


//=============================================================================
class debris_meson_explosion : public netobj
{
public:
    CREATE_RTTI( debris_meson_explosion, netobj, object )


        //=============================================================================
                         debris_meson_explosion             ( void );
    virtual				~debris_meson_explosion             ( void );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void        OnInit              ( void );

    virtual void        Create              ( const vector3&    Pos,
                                              const vector3&    Normal,
                                              u32               Zones,
                                              guid              OwnerGuid );

    virtual void        DumbCreate          ( const vector3&    Pos,
                                              const vector3&    Normal,
                                              u32               Zones );

    virtual bbox        GetLocalBBox        ( void ) const;
    virtual s32         GetMaterial         ( void ) const { return MAT_TYPE_CONCRETE;}
    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        OnMove				( const vector3& rNewPos );
    virtual void        OnRender            ( void );
    virtual void        OnRenderTransparent ( void );

            void        AddNewTarget        ( guid Target );

            guid        GetGuidOfOwner  ( void ) { return m_GuidOfOwner; }

    //==-----------------------------------------------------------------------
    //  STRUCTURES
    //==-----------------------------------------------------------------------
protected:

    enum explosion_state
    {        
        STATE_VERIFYING_TARGETS,
        STATE_KILLING_TARGETS,
        STATE_COLLAPSING,
    };

    enum lash_state
    {
        LASH_EXTEND,
        LASH_INTERACT,
        LASH_RETRACT,
    };

    enum target_type
    {
        TARGET_ACTOR = 0,
        TARGET_NONACTOR_DAMAGEABLE,
        TARGET_NONACTOR_NONDAMAGEABLE,
        TARGET_GLOB,
        TARGET_SHIELD,  
        TARGET_THETA,
        TARGET_NETGHOST,

        TARGET_TYPE_MAX
    };

public:
    enum sounds 
    {
        SOUND_MAIN_EXPLODE,
        SOUND_MAIN_IMPLODE,

        SOUND_LASH_EXTEND,
        SOUND_LASH_EXTEND_STOP,
        SOUND_LASH_HOLD,
        SOUND_LASH_HOLD_STOP,
        SOUND_LASH_RETRACT,
        SOUND_LASH_RETRACT_STOP,

        SOUND_MAX,
    };

protected:
    struct target_behaviour
    {
        f32         ExtendMinTime;
        f32         ExtendMaxTime;
        f32         InteractTime;
        f32         RetractTime;
    };

public:
    struct lash_target
    {
        guid        gObj;
        target_type TargetType;    
        s8          ThetaBoneIndex;
        s8          NetActorLifeSeq;            // Life sequence # for use when networked
        u8          bIsValidTarget:1,           // Targets must be somewhat visible from origin
                    bStunTheta:1;
    };

protected:
    struct lash
    {
        lash()
        {
            m_pOwner            = NULL;
            m_State             = LASH_EXTEND;
            m_Scale             = 1.0f;
            m_StateTime         = 0;
            m_CurTime           = 0;
            m_StateTime         = 0;
            m_PainDirection.Set( 0.0f, 1.0f, 0.0f );
            m_TargetObject      = 0;
            m_CorpseGuid        = 0;
            m_iConstraint       = -1;
            m_VoiceID           = -1;
            m_iThetaBoneIndex   = -1;

            m_bActive           = FALSE;
            m_bThetaHarmed      = FALSE;
            m_bStunTheta        = FALSE;
            m_TargetType        = TARGET_NONACTOR_DAMAGEABLE;
            m_Color.Set(255,200,90);
        }

        ~lash()
        {
            if (m_FX.Validate())
                m_FX.KillInstance();
            if (m_VoiceID != -1)
                g_AudioMgr.Release( m_VoiceID, 0 );
        };

        xbool           HarmTarget      ( void );               // returns TRUE if the target was an actor and
                                                                // that actor died due to the pain
        xbool           AcquireCorpse   ( void );
        void            ReleaseCorpse   ( void );
        void            AdvanceLogic    ( f32 DeltaTime );        
        void            ShakeCorpse     ( f32 DeltaTime );
        
        voice_id                m_VoiceID;
        debris_meson_explosion* m_pOwner;
        lash_state              m_State;        
        vector3                 m_Origin;        
        vector3                 m_NormalAtOrigin;       // Impact normal if OriginObject = NULL, else
                                                        // it is the direction through the origin object
        vector3                 m_EndPoint;
        f32                     m_StartTime;
        f32                     m_CurTime;
        f32                     m_StateTime;
        vector3                 m_PainDirection;
        guid                    m_TargetObject;
        vector3                 m_TargetPoint;            // For a lash with no object, TargetPoint is filled out in ::Create
        target_type             m_TargetType;
        s32                     m_iThetaBoneIndex;
        fx_handle               m_FX;                     // FX for head of lash
        f32                     m_Scale;                  // Controls normals at spline verts  
        radian3                 m_RandomRot;

        xcolor                  m_Color;
        xcolor                  m_DesiredColor;
        vector4                 m_PrecisionColor;

        f32                     m_Size;
        f32                     m_DesiredSize;
        
        guid                    m_CorpseGuid;
        vector3                 m_ConstraintPosition;     // Shaken
        vector3                 m_BaseConstraintPosition; // Not shaken
        s32                     m_iConstraint;

        f32                     m_SwimScalar;

        s8                      m_NetActorLifeSeq; 

        u8                      m_bActive:1,
                                m_bThetaHarmed:1,
                                m_bStunTheta:1;
                                
    };

    struct beam
    {
        ~beam()
        {
            if (m_ImpactFX.Validate())
                m_ImpactFX.KillInstance();
            if (m_HeadFX.Validate())
                m_HeadFX.KillInstance();
        };

        vector3                 m_Dir;
        vector3                 m_EndPt;
        vector3                 m_EndNormal;
        f32                     m_RandomOffset;
        f32                     m_Timer;
        fx_handle               m_ImpactFX;
        fx_handle               m_HeadFX;
    };

    //==-----------------------------------------------------------------------
    //  FUNCTIONS
    //==-----------------------------------------------------------------------
protected:

    void    SwitchState                 ( explosion_state State );

    void    AdvanceVerificationLogic    ( f32 DeltaTime );
    void    AdvanceAttackLogic          ( f32 DeltaTime );
    void    AdvanceCollapseLogic        ( f32 DeltaTime );


    void    GatherTargets       ( u32 Attribute, const bbox& WorldBBox, object::type ObjType );
    void    SortTargets         ( void );
    void    CollapseTargetList  ( void );
    s32     UpdateLashes        ( f32 DeltaTime );
    void    RenderLashes        ( void );
    s32     CreateNewLashes     ( xbool bNetworkForced );
    void    UpdateTargetPoint   ( lash& L );

    void    UpdateBeams         ( f32 DeltaTime );
    void    RenderBeams         ( void );
    s32     CreateNewBeam       ( void );

    //==-----------------------------------------------------------------------
    //  DATA
    //==-----------------------------------------------------------------------
protected:

    guid                    m_GuidOfOwner;
    s32                     m_NetSlotOfOwner;

    rhandle<xbitmap>        m_Texture;
    rhandle<decal_package>  m_hDecalPackage;

    // Main data, timing, basic info, etc...
    f32                     m_LashSpawnTimer;
    f32                     m_TotalTime;
    f32                     m_MaxRadius;
    bbox                    m_LocalBBox;
    explosion_state         m_State;
    vector3                 m_Origin;
    vector3                 m_OriginNormal;
    f32                     m_TimeInState;

    // Targeting data
static  target_behaviour        m_TargetBehaviour[ TARGET_TYPE_MAX ];
    s32                     m_iNextToBeValidated;
    lash_target             m_LashTargets[ MAX_LASH_TARGETS ];
    s32                     m_nLashTargets;
    s32                     m_iNextTarget;

    // FX for main explosion
    voice_id                m_MainExplosionVoiceID;    
    
    fx_handle               m_CoreFancyFX;    
    fx_handle               m_MainExplosionFX;

    // Lash tendril data
    lash                    m_Lash[ MAX_LASHES ];
    s32                     m_iCurLash;

    // Secondary lash data
    beam                    m_Beam[ MAX_BEAMS ];
    f32                     m_BeamTimer;   

    u8                      m_bDestroying:1;


    //
    //  NETWORK EXTRAS  
    //
    public:

    enum
    {
        DIRTY_NEXT_TARGET_ID        = ( 1 <<  0 ),
        DIRTY_COLLAPSE              = ( 1 <<  1 ),
        DIRTY_TARGETING_INFO        = ( 1 <<  2 ),
        DIRTY_TARGET_STATUS_INFO    = ( 1 <<  3 ),      // Signals that smart expl is done harming everyone
                                                        // and the result of their pain needs to be spread to all
    };


#ifndef X_EDITOR

virtual         void    net_AcceptUpdate    ( const bitstream& BitStream );
virtual         void    net_ProvideUpdate   (       bitstream& BitStream, 
                                                    u32&       DirtyBits );

    s8                      m_NetTargetStatusCounter;   // Starts == # of targets
                                                        // Is decremented on the smart explosion each time
                                                        // lash::HarmTarget is called.  Target survivability
                                                        // is stored in the m_NetKilledMask
    u8                      m_NetKilledMask;            // Each bit determines if that target died from pain

#endif // X_EDITOR

    s32                     m_iNetCurrentTarget;    // This is where we should be according
                                                    // to the "smart" explosion

    u8                      m_bNetCollapseSignaled:1,
                            m_bActivated:1,
                            m_bNetDumbTargetStatusSignaled:1;       // Set when the smart net explosion had
                                                                    // resolved the pain survivability of all targets



};


#endif // __DEBRIS_MESON_LASH_HPP__