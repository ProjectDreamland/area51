//==============================================================================
//
//  Ragdoll.hpp
//
//==============================================================================

#ifndef __RAGDOLL_HPP__
#define __RAGDOLL_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"

#include "Objects\Render\SkinInst.hpp"
#include "Animation\AnimData.hpp"
#include "Loco\LocoAnimController.hpp"
#include "CollisionMgr\CollisionMgr.hpp"

#include "Particle.hpp"
#include "GeomBone.hpp"
#include "StickBone.hpp"
#include "DistRule.hpp"

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class object ;
class loco_char_anim_player ;



//==============================================================================
// DEFINITION STRUCTURES (USED BY MAX EXPORT)
//==============================================================================

struct particle_def
{
    const char* m_pName ;
    f32         m_R, m_G, m_B ;
    f32         m_PosX, m_PosY, m_PosZ ;
    f32         m_Radius, m_Mass ;
    xbool       m_bCollision ;
} ;

//==============================================================================

struct constraint_def
{
    const char* m_pName ;    
    f32         m_R, m_G, m_B ;
    const char* m_pParticleA ;
    const char* m_pParticleB ;
    xbool       m_bEqual ;
    f32         m_EqualPercent ;
    xbool       m_bMin ;
    f32         m_MinPercent ;
    xbool       m_bMax ;
    f32         m_MaxPercent ;
    f32         m_Damp ;
    xbool       m_bBone ;
} ;

//==============================================================================

struct angle_limit_def
{
    const char* m_pName ;
    f32         m_PosX, m_PosY, m_PosZ ;
    const char* m_pDistRuleA ;
    const char* m_pDistRuleB ;
    f32         m_AxisX, m_AxisY, m_AxisZ ;
    f32         m_MinAngle, m_MaxAngle ;
} ;

//==============================================================================

struct bone_match_def
{
    const char*     m_pGeomBone ;
    const char*     m_pStickBone ;
} ;

//==============================================================================

struct ragdoll_def
{    
    xbool           m_bInitialized ;

    const char*     m_pName ;

    particle_def*   m_Particles ;
    s32             m_NParticles ;

    constraint_def* m_Constraints ;
    s32             m_NConstraints ;

    bone_match_def* m_BoneMatches ;
    s32             m_NBoneMatches ;

    // These are built at runtime
    dist_rule*      m_DistRules ;
    s32             m_NMaxDistRules ;
    s32             m_NDistRules ;
    
    geom_bone*      m_GeomBones ;
    s32             m_NMaxGeomBones ;
    s32             m_NGeomBones ;

    // Recorded during initialize for debugging
    const char*     m_pSkinGeom;
    const char*     m_pAnim;
    const char*     m_pType;
} ;

//==============================================================================
// CLASSES
//==============================================================================

// Class to track particle position
class ragdoll
{
// Defines
public:
    
    // Misc defines
    enum defines
    {
        MAX_PARTICLES = 16,
        MAX_DEBUG_PLANES      = 18,
        MAX_DIST_RULES        = 100,
        MAX_GEOM_BONES        = MAX_ANIM_BONES
    } ;

    // Available rig types
    // NOTE: You must keep this in sync with the type table at the top of ragdoll.hpp
    enum type
    {
        TYPE_GRAY,
        TYPE_GRUNT,
        TYPE_MILITARY,
        TYPE_MUTANT_TANK,
        TYPE_PLAYER,
        TYPE_CIVILIAN,
        TYPE_LEAPER,

        TYPE_COUNT
    } ;

// Structures
private:

    struct joint_side
    {
        particle*   m_pToe ;
        particle*   m_pFoot ;
        particle*   m_pKnee ;
        particle*   m_pHip ;
        particle*   m_pTorso ;
        particle*   m_pShoulder ;
        particle*   m_pElbow ;
        particle*   m_pWrist ;
        particle*   m_pFinger ;
    } ;

    struct joints
    {
        joint_side  m_Side[2] ;
        particle*   m_pNeck ;
        particle*   m_pHead ;
    } ;

    struct stick_bone_side
    {
        stick_bone  m_Foot ;
        stick_bone  m_Calf ;
        stick_bone  m_Thigh ;
        stick_bone  m_UpperArm ;
        stick_bone  m_Forearm ;
        stick_bone  m_Hand ;
    } ;

    struct stick_bones
    {
        stick_bone_side m_Side[2] ;
        stick_bone      m_Hips ;
        stick_bone      m_Torso ;
        stick_bone      m_Chest ;
        stick_bone      m_Head ;

        s32 GetCount( void )
        {
            return (sizeof(stick_bones) / sizeof(stick_bone)) ;
        }

        stick_bone& operator [] ( s32 Index )
        {
            ASSERT(Index >= 0) ;
            ASSERT(Index < GetCount()) ;

            return ((stick_bone*)this)[Index] ;
        }
    } ;

public:
    struct debug_plane
    {
        vector3 Point ;
        vector3 Normal ;
        xcolor  Color ;
    } ;

// Private data
private:
    
    // Misc
    ragdoll_def*            m_pDef ;

    // Physics components
    particle                m_Particles[MAX_PARTICLES] ;
    s32                     m_NParticles ;

    stick_bones             m_StickBones ;
    joints                  m_Joints ;

    // Rendering components
    anim_group::handle      m_hAnimGroup ;      // Animation group handle
    skin_inst               m_SkinInst ;        // Skinned instance
    f32                     m_DeltaTime ;       // Accumulated delta time
    guid                    m_ObjectGuid ;      // Owner guid (or NULL if none)

    // Audio components
    s32                     m_BodyImpactSoundID;

#ifdef X_DEBUG
    // Debug rendering
    s32                     m_NDebugPlanes ;
    debug_plane             m_DebugPlanes[MAX_DEBUG_PLANES] ;
#endif

// Public data
public:
    static enum_table<type> s_TypeList; 

// Functions
public:
         ragdoll() ;
         ~ragdoll() ;

// Functions
private:

    // Query functions
    xbool           AreParticlesConnected   ( s32 ParticleA, s32 ParticleB ) ;
    particle*       FindParticle            ( const char* pName ) ;
    s32             FindParticleIndex       ( const char* pName ) ;
    s32             FindStickBone           ( const char* pName ) ;

    s32             FindClosestStickBone    ( const char* pGeomBoneName, const vector3& C, f32& T ) ;
    s32             FindClosestGeomBone     ( const vector3& C ) ;
    
    // Plane functions
#ifdef X_DEBUG
    void            AddDebugPlane           ( const vector3& Point, const vector3& Normal, xcolor Color ) ;
#endif    
    
    void            KeepBehindPlane         ( const vector3&    Point, 
                                              const vector3&    Normal, 
                                                    particle*   pA, 
                                                    particle*   pB, 
                                                    f32         Damp = 1.0f ) ;
                                                    
    void            KeepOnPlane             ( const vector3&    Point, 
                                              const vector3&    Normal, 
                                                    particle*   pA, 
                                                    particle*   pB ) ;
    
    // Constraint functions
    void            ApplyHumanConstraints   ( void ) ;
    void            ApplyDistConstraints    ( void ) ;
    void            ApplyRagdollBoneColl    ( vector3& BoneStart, vector3& BoneEnd ) ;
    void            ApplyRagdollConstraints ( ragdoll& Ragdoll ) ;
    void            ApplyCollConstraints    ( void ) ;
    void            ApplyConstraints        ( void ) ;
    
    // Internal functions
    void            Integrate               ( f32 DeltaTime ) ;
    void            UpdateStickBones        ( void ) ;
    void            CreateMinDistRules      ( particle* pA, const char* pList[], f32 Dist, f32 Damp ) ;


// Functions
public:

    // Initialization
    xbool           Init                    ( const char*           pSkinGeomName, 
                                              const char*           pAnimName,
                                                    ragdoll::type   RagdollType, 
                                                    guid            ObjectGuid ) ;
    void            Reset                   ( void ) ;
    void            Kill                    ( void ) ;
    void            SetObjectGuid           ( guid Guid )   { m_ObjectGuid = Guid ; }

    // Applies blast force to particles
    void            ApplyBlast              ( const vector3& Pos, f32 Radius, f32 Amount ) ;
    void            ApplyBlast              ( const vector3& Pos, const vector3& Dir, f32 Radius, f32 Amount ) ;
    void            ApplyVectorForce        ( const vector3& Dir, f32 Amount );

    // Velocity functions
    void            ZeroVelocity            ( void );
    void            SetVelocity             ( const vector3& Vel );
    void            AddVelocity             ( const vector3& Vel );

    // Time functions
    void            ZeroDeltaTime           ( void ) { m_DeltaTime = 0.0f; }

    // Matrix functions
    void            ComputeMatrices         ( matrix4* pMatrices, s32 nBones ) ;
    void            ComputeBoneL2W          ( s32 iBone, matrix4& L2W ) ;
    s32             GetNGeomBones           ( void ) ;
    void            SetMatrices             ( loco_char_anim_player& AnimPlayer, const vector3& Vel ) ;
    void            SetMatrices             ( const matrix4* pMatrices, s32 nBones ) ;

#ifdef X_DEBUG
    // Render functions
    void            RenderGeometry          ( u32 Flags = render::CLIPPED ) ;
    vector3         GetGeomBonePos          ( s32 BoneIndex ) ;
    void            RenderSkeleton          ( void ) ;
    void            RenderBones             ( s32 Index = -1 ) ;
    void            RenderCollision         ( void ) ;
#endif

    // Logic functions
    void            Advance                 ( f32 DeltaTime) ;

    // Collision functions
    void            OnColCheck                  ( void ) ;
    void            OnColNotify                 ( object& Object, const collision_mgr::collision& Collision ) ;
    bbox            ComputeWorldBBox            ( void ) const ;
    void            ApplyCharacterConstraints   ( void ) ;

    // Query functions
    anim_group::handle& GetAnimGroupHandle  ( void );
    skin_inst&      GetSkinInst             ( void );
    const char*     GetAnimName             ( void ) const ;
    const char*     GetSkinGeomName         ( void ) const ;
    vector3         GetFeetPos              ( void ) const ;
    vector3         GetHipPos               ( void ) const ;
    vector3         GetCenterPos            ( void ) const ;
    xbool           IsInitialized           ( void ) const ;
    f32             GetKineticEnergy        ( void ) const ;
} ;

//==============================================================================
// INLINE FUNCTIONS
//==============================================================================

inline
anim_group::handle& ragdoll::GetAnimGroupHandle( void )
{
    return m_hAnimGroup ;
}

//==============================================================================

inline
skin_inst& ragdoll::GetSkinInst( void )
{
    return m_SkinInst;
}

//==============================================================================

inline
const char* ragdoll::GetAnimName( void ) const
{
    return m_hAnimGroup.GetName() ;
}

//==============================================================================

inline
const char* ragdoll::GetSkinGeomName( void ) const
{
    return m_SkinInst.GetSkinGeomName() ;
}

//==============================================================================
// UTIL FUNCTIONS
//==============================================================================

#ifdef X_DEBUG
void draw_Plane( const vector3& MidPt, const vector3& Normal, xcolor Color, f32 Size = 40 ) ;
#endif

void RagdollType_OnProperty ( prop_query& I, ragdoll::type& RagdollType );


#endif  // #ifndef __RAGDOLL_HPP__
