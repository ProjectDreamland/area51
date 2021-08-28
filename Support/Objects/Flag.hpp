//==============================================================================
//
//  Flag.hpp
//
//==============================================================================

#ifndef FLAG_HPP
#define FLAG_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "NetworkMgr\NetObj.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "Cloth\Cloth.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"


//==============================================================================
//  TYPES
//==============================================================================

#define NUM_FLAG_EFFECTS 4

enum flag_effect 
{
    FLAG_TIMEOUT,
    FLAG_CAPTURE,
    FLAG_TELEPORT,
    FLAG_SPAWN,
};

// These store the possible alignments.
enum Alignments
{
    FRIENDLY_NONE  = 0x00000000,
    FRIENDLY_ALPHA = 0x00000001,
    FRIENDLY_OMEGA = 0x00000002,
    FRIENDLY_ALL   = 0xFFFFFFFF,
};

// This is just to make life easier in keeping track of what to 
// render as for the player, and in loading the properties.
enum RenderAs
{
    FRIEND_TO_NONE        = 0,
    FRIEND_TO_ENEMY_ALPHA = 1,
    FRIEND_TO_TEAM_ALPHA  = 2,
    FRIEND_TO_ALL         = 3,
    FRIEND_TO_ENEMY_OMEGA = 4,
    FRIEND_TO_TEAM_OMEGA  = 5,
};

class flag : public netobj
{
public:

    CREATE_RTTI( flag, netobj, object )

                    flag                                ( void );
                   ~flag                                ( void );

    void            SetTeamBits                         (       u32      TeamBits );
    void            SetTimeOut                          (       f32      TimeOutAge,
                                                                f32      CurrentAge );
    void            AttachToPlayer                      (       s32      PlayerIndex );
    void            SetPosition                         ( const vector3& Position,
                                                                s32      Zone1,
                                                                s32      Zone2,
                                                                xbool    bSetDirty = TRUE );

    void            StartFall                           ( void );
    void            StopFall                            ( void );

    void            SetYaw                              (       radian   Yaw );
    void            ClearTimeOut                        ( void );
    void            AddIgnore                           (       s32 PlayerIndex );

    virtual         bbox            GetLocalBBox        ( void ) const;
    virtual const   object_desc&    GetTypeDesc         ( void ) const;
    static  const   object_desc&    GetObjectType       ( void );
    virtual         s32             GetMaterial         ( void ) const;

    void            OnEnumProp                          ( prop_enum&  rPropList  );
    xbool           OnProperty                          ( prop_query& rPropQuery );

                    s32             GetVTexture         ( void );

    virtual         void            OnRender            ( void );
#ifdef TARGET_XBOX    
    virtual         void            OnRenderCloth       ( void );
#endif    
    virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
    virtual         void            OnPain              ( const pain& Pain );    
    
                    void            OnRenderTransparent ( void );

                    void            PlayEffect          ( flag_effect Effect );
                    xbool           IsAttached          ( void ) { return m_Attached; }
                    s32             GetAttachedTo       ( void ) { return m_Player;   }

    //------------------------------------------------------------------------------
#ifndef X_EDITOR
    //------------------------------------------------------------------------------

    virtual         void            net_Logic           ( f32 DeltaTime );

    //rtual         void            net_Activate        ( void );
    //rtual         void            net_Deactivate      ( void );

    virtual         void            net_AcceptUpdate    ( const bitstream& BitStream );
    virtual         void            net_ProvideUpdate   (       bitstream& BitStream, 
        u32&       DirtyBits );

    //------------------------------------------------------------------------------
#endif // X_EDITOR
    //------------------------------------------------------------------------------

protected:
    void            InitGeometry        (       u32      TeamBits );

    xbool           m_GeomReady;

    radian          m_Yaw;
    f32             m_Age;          // ## Get from net_proj. ##

    xbool           m_TimeOut;      // Should the flag time out?
    f32             m_TimeOutAge;   // Age to return/vanish.

    xbool           m_Attached;
    s32             m_Player;
    s32             m_Bone;
    s32             m_Holes;

    u32             m_IgnoreBits;
    f32             m_IgnoreTimer;

    // Flag effects.
    xbool           m_StartEffects  [ NUM_FLAG_EFFECTS ]; 
    fx_handle       m_FlagFX        [ NUM_FLAG_EFFECTS ];
    xbool           m_EffectActive  [ NUM_FLAG_EFFECTS ];

    vector3         m_VanishPos;

    xbool           m_bRendered;
    f32             m_IconOpacity;
    f32             m_BaseAlpha;

    f32             m_bIsSettled;
    f32             m_Velocity;

    enum
    {
        DIRTY_POSITION          = 0x00000001,
        DIRTY_TIMEOUT           = 0x00000002,
        DIRTY_ALL               = 0x00000003, // Leaving effects out of sum on purpose.
        DIRTY_TIMEOUT_IMMINENT  = 0x00000004,
        DIRTY_CAPTURE           = 0x00000008,
        DIRTY_TELEPORT          = 0x00000010,
    }; 

    //------------------------------------------------------------------------------
    //  Cloth support.
    //------------------------------------------------------------------------------
public:

            u32             GetRenderMode       ( void );
            virtual         void            OnProjectileImpact  ( const object&  Projectile,
    const   vector3& Velocity,
            u32      CollPrimKey, 
    const   vector3& CollPoint,
            xbool    PunchDamageHole   = TRUE,      // TRUE to punch out a hole
            f32      ManualImpactForce = -1.0f );   // <= 0 : normal behaviour for bullets
            virtual         void            OnColCheck          ( void );
            virtual         void            OnMove              ( const vector3& NewPos );
            virtual         void            OnTransform         ( const matrix4& L2W    );
            void            InitCloth           ( void );

            cloth           m_Cloth;
            f32             m_ActiveTimer;  // If >0, cloth is updated.   
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 flag::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // FLAG_HPP
//==============================================================================
