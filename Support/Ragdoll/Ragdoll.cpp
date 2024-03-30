//==============================================================================
//
//  Ragdoll.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "Ragdoll.hpp"
#include "Entropy.hpp"
#include "GameLib\StatsMgr.hpp"
#include "Objects\BaseProjectile.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\DeadBody.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Loco\LocoCharAnimPlayer.hpp"
#include "VerletCollision.hpp"

#include "GrayRagdoll.hpp"
#include "GruntRagdoll.hpp"
#include "HazmatRagdoll.hpp"
#include "MutantTankRagdoll.hpp"
#include "PlayerRagdoll.hpp"
#include "ScientistRagdoll.hpp"
#include "LeaperRagdoll.hpp"



//==============================================================================
// DATA
//==============================================================================

// Physics defines
static f32 RAGDOLL_GRAVITY                      = -9.8f * 100 ;
static f32 RAGDOLL_FRICTION                     = 0.25f ;
static f32 RAGDOLL_BOUNCY                       = 0.35f ;
static f32 RAGDOLL_MIN_FRICTION                 = 0.9f ;
static f32 RAGDOLL_MAX_FRICTION                 = 1.0f ;
static f32 RAGDOLL_LINEAR_DAMPEN                = 0.008f ;
static f32 RAGDOLL_ANGULAR_DAMPEN               = 0.002f ;
//static f32 RAGDOLL_JOINT_DAMPEN                 = 0.01f ;
static s32 RAGDOLL_ITER_COUNT                   = 5 ;   // Less = turn to rubber
static f32 RAGDOLL_ELBOW_SELF_INTERSECT_DIST    = 15.0f ;
static f32 RAGDOLL_WRIST_SELF_INTERSECT_DIST    = 30.0f ;
static f32 RAGDOLL_FOOT_SELF_INTERSECT_DIST     = 30.0f ;
static f32 RAGDOLL_PARTICLE_RADIUS              = 15.0f ;
static f32 RAGDOLL_TIME_STEP                    = 1.0f / 30.0f ;
static f32 RAGDOLL_COLLISION_BACKOFF            = 0.1f ;
static f32 RAGDOLL_MAX_SPEED                    = 100.0f ;   // Dist per 30th of a sec
static f32 RAGDOLL_COLL_BBOX_INFLATE            = 20.0f;
static f32 RAGDOLL_MIN_COLL_DIST                =  0.1f;
static f32 RAGDOLL_MIN_AUDIO_VELOCITY           =  0.5f;
static s32 RAGDOLL_MAX_ITERATIONS               = 1;



//==============================================================================

// *** ENUM INFO FOR PROPERTY SYSTEM - MUST MATCH "s_RagdollDefTable" TABLE BELOW!!! ***
typedef enum_pair<ragdoll::type> ragdoll_type_enum_pair;
static ragdoll_type_enum_pair s_TypeEnumPair[] = 
{
    // *** THIS TABLE MUST MATCH "s_RagdollDefTable" BELOW!!! ***
    ragdoll_type_enum_pair("GRAY",                  ragdoll::TYPE_GRAY),
    ragdoll_type_enum_pair("GRUNT",                 ragdoll::TYPE_GRUNT),
    ragdoll_type_enum_pair("MILITARY",              ragdoll::TYPE_MILITARY),
    ragdoll_type_enum_pair("MUTANT_TANK",           ragdoll::TYPE_MUTANT_TANK),
    ragdoll_type_enum_pair("PLAYER",                ragdoll::TYPE_PLAYER),
    ragdoll_type_enum_pair("CIVILIAN",              ragdoll::TYPE_CIVILIAN),
    ragdoll_type_enum_pair("LEAPER",                ragdoll::TYPE_LEAPER),

    ragdoll_type_enum_pair( k_EnumEndStringConst,   ragdoll::TYPE_COUNT) //**MUST BE LAST**//
};
enum_table<ragdoll::type>  ragdoll::s_TypeList( s_TypeEnumPair);              

// *** AVAILABLE RIG TYPES - MUST MATCH "s_RagdollDefTable" TABLE ABOVE!!! ***
ragdoll_def* s_RagdollDefTable[] =
{
    // *** THIS TABLE MUST MATCH "s_TypeEnumPair" ABOVE!!! ***
    &GrayRagdoll,       // TYPE_GRAY
    &GruntRagdoll,      // TYPE_GRUNT
    &HazmatRagdoll,     // TYPE_MILITARY
    &MutantTankRagdoll, // TYPE_MUTANT_TANK
    &PlayerRagdoll,     // TYPE_PLAYER
    &SciRagdoll,        // TYPE_CIVILIAN
    &LeaperRagdoll,     // TYPE_LEAPER
} ;

//==============================================================================

// List of upper body particles
const char* s_UpperBodyParticles[] =
{
    "Particle L Hip",
    "Particle R Hip",
    "Particle L Torso",
    "Particle R Torso",
    "Particle L Shoulder",
    "Particle R Shoulder",
    "Particle Neck",
    "Particle Head",
    "Particle L Wrist",
    "Particle R Wrist",
    "Particle L Elbow",
    "Particle R Elbow",
    NULL
} ;

// List of lower body particles
const char* s_LowerBodyParticles[] =
{
    "Particle L Torso",
    "Particle R Torso",
    "Particle L Hip",
    "Particle R Hip",
    "Particle L Knee",
    "Particle R Knee",
    "Particle L Foot",
    "Particle R Foot",
    NULL
} ;

//==============================================================================
// SCRATCH PAD DMA FUNCTIONS
//==============================================================================

#ifdef TARGET_PS2

inline
void spad_DmaTo( u32 SpadOffset, const void* pSrc, s32 NBytes )
{
    ASSERTS( ( (u32)pSrc & 63 ) == 0, "Source must be 64 byte aligned!" );
    ASSERTS( ( NBytes & 15    ) == 0, "Size must be 16 byte aligned!" );
    
    *D9_MADR = ((u32)pSrc)&0x0fffffff;
    *D9_SADR = SpadOffset;
    *D9_QWC  = NBytes/16;
    *D9_CHCR = (1<<8);
    asm __volatile__ ( "sync.l" );
}

//=========================================================================

inline
void spad_SyncDmaTo( void )
{
    while ( *D9_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }
}

//=========================================================================

inline
void spad_CopyTo( u32 SpadOffset, const void* pSrc, s32 NBytes )

{
    //x_memcpy( (void*)0x70000000 + SpadOffset, pSrc, NBytes );
    spad_DmaTo( SpadOffset, pSrc, NBytes );
    spad_SyncDmaTo();
};

#endif  //#ifdef TARGET_PS2


//==============================================================================
// CLASSES
//==============================================================================

ragdoll::ragdoll()
{
    m_pDef         = NULL ;
    m_NParticles   = 0 ;
    m_BodyImpactSoundID  = 0;

#ifdef X_DEBUG
    m_NDebugPlanes = 0 ;
#endif
}

//==============================================================================

ragdoll::~ragdoll()
{
    Kill() ;
}

//==============================================================================

xbool ragdoll::AreParticlesConnected( s32 ParticleA, s32 ParticleB )
{
    // Check all dist_rules
    for (s32 i = 0 ; i < m_pDef->m_NDistRules ; i++)
    {
        // Get dist_rule
        dist_rule& DistRule = m_pDef->m_DistRules[i] ;

        // Let min and equal be tested for...
        if ( DistRule.m_Type == dist_rule::DIST_MAX )
            continue ;

        // Lookup particles
        s32 P0 = DistRule.m_ParticleOffset[0] / sizeof(particle);
        s32 P1 = DistRule.m_ParticleOffset[1] / sizeof(particle);
        
        // Found?
        if ( ( P0 == ParticleA ) && ( P1 == ParticleB ) )
            return TRUE ;

        if ( ( P1 == ParticleA ) && ( P0 == ParticleB ) )
            return TRUE ;
    }

    return FALSE ;
}

//==============================================================================

particle* ragdoll::FindParticle( const char* pName )
{
    // Check all particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        const char* pParticleName = m_Particles[i].m_pName ;

        if (x_strcmp(pParticleName, pName) == 0)
            return &m_Particles[i] ;
    }

    // Not found
    return NULL ;
}

//==============================================================================

s32 ragdoll::FindParticleIndex( const char* pName )
{
    // Check all particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        const char* pParticleName = m_Particles[i].m_pName ;

        if (x_strcmp(pParticleName, pName) == 0)
            return i ;
    }

    // Not found
    return -1 ;
}

//==============================================================================

s32 ragdoll::FindStickBone( const char* pName )
{
    ASSERT(pName) ;

    // Check all stick bones
    for (s32 i = 0 ; i < m_StickBones.GetCount() ; i++)
    {
        // Get name
        const char* pStickBoneName = m_StickBones[i].m_pName ;
        if (!pStickBoneName)
            continue ;

        // Found?
        if (x_strcmp(pStickBoneName, pName) == 0)
            return i ;
    }

    // Not found
    return -1 ;
}

//==============================================================================

s32 ragdoll::FindClosestStickBone( const char* pGeomBoneName, const vector3& C, f32& T )
{
    s32 i ;
    T = 0 ;

    // Check to see if stick bone has been specified
    for (i = 0 ; i < m_pDef->m_NBoneMatches ; i++)
    {
        // Lookup def
        bone_match_def& Def = m_pDef->m_BoneMatches[i] ;
        
        // Found?
        if( x_stristr( Def.m_pGeomBone, pGeomBoneName ) )
        {
            // Lookup stick bone
            s32 StickBoneIndex = FindStickBone(Def.m_pStickBone) ;
            ASSERT(StickBoneIndex != -1) ;
                
            // Get stick bone
            stick_bone& StickBone = m_StickBones[StickBoneIndex] ;

            // Get ratio of point on line
            T = C.GetClosestPToLSegRatio(StickBone.m_Start, StickBone.m_End) ;

            // Return stick bone index
            return StickBoneIndex ;
        }
    }

    // Bone not specified, so auto search the best matching stick bone
    s32   BestBone = -1 ;
    f32   BestDist = F32_MAX ;
    for (i = 0 ; i < m_StickBones.GetCount() ; i++)
    {
        // Get stick bone
        stick_bone& StickBone = m_StickBones[i] ;
        if (!StickBone.m_pName)
            continue ;

        // Get ratio of point on line
        f32 Ratio = C.GetClosestPToLSegRatio(StickBone.m_Start, StickBone.m_End) ;
        
        // Get point on line
        vector3 P = StickBone.m_Start + (Ratio * (StickBone.m_End - StickBone.m_Start)) ;

        // Closest so far?
        f32 Dist = (C - P).LengthSquared() ;
        if (Dist < BestDist)
        {
            BestDist = Dist ;
            BestBone = i ;
            T        = Ratio ;
        }
    }

    ASSERT(BestBone != -1) ;

    return BestBone ;
}

//==============================================================================

s32 ragdoll::FindClosestGeomBone( const vector3& C )
{
    s32 BestBone = -1 ;
    f32 BestDist  = F32_MAX ;
    for (s32 i = 0 ; i < m_pDef->m_NGeomBones ; i++)
    {
        // Get bone
        geom_bone& Bone = m_pDef->m_GeomBones[i] ;

        // Get center of bone in world bind space
        vector3 M = Bone.m_SkinBind * Bone.m_LocalBBox.GetCenter() ;

        // Get distance from center
        f32 Dist = (C - M).LengthSquared() ;
        if (Dist < BestDist)
        {
            BestDist  = Dist ;
            BestBone = i ;
        }
    }

    return BestBone ;
}

//==============================================================================

#ifdef X_DEBUG

void ragdoll::AddDebugPlane( const vector3& Point, const vector3& Normal, xcolor Color )
{
    ASSERT(m_NDebugPlanes < MAX_DEBUG_PLANES) ;
    debug_plane& Plane = m_DebugPlanes[m_NDebugPlanes++] ;

    Plane.Point  = Point ;
    Plane.Normal = Normal ;
    Plane.Color  = Color ;
}

#endif

//==============================================================================

void ragdoll::KeepBehindPlane( const vector3&   Point, 
                               const vector3&   Normal, 
                                     particle*  pA, 
                                     particle*  pB, 
                                     f32        Damp /*= 1.0f*/ )
{
    ASSERT( pA );
    ASSERT( pB );

    // Setup plane
    plane Plane ;
    Plane.Setup(Point, Normal) ;
    
    // Get distance from plane
    f32 D = Plane.Distance(pA->m_Pos) ;
    if (D > 0)
    {
        // Setup masses
        f32 InvMass0 = pA->GetInvMass() ;
        f32 InvMass1 = pB->GetInvMass() ;
        f32 TotalInvMass = InvMass0 + InvMass1 ;
        if( TotalInvMass == 0.0f )
            return ;

        // Move particles towards plane
        D *= Damp / TotalInvMass ;
        Plane.Normal *= D;
        pA->m_Pos -= InvMass0 * Plane.Normal ;
        pB->m_Pos += InvMass1 * Plane.Normal ;
        
        #ifdef X_DEBUG
            AddDebugPlane(Point, Normal, XCOLOR_YELLOW) ;
        #endif            
    }
    else
    {
        #ifdef X_DEBUG
            AddDebugPlane(Point, Normal, XCOLOR_RED) ;
        #endif            
    }
}

//==============================================================================

void ragdoll::KeepOnPlane( const vector3&   Point, 
                           const vector3&   Normal, 
                                 particle*  pA, 
                                 particle*  pB )
{
    ASSERT( pA );
    ASSERT( pB );

    // Setup plane
    plane Plane ;
    Plane.Setup(Point, Normal) ;

    // Setup masses
    f32 InvMass0 = pA->GetInvMass() ;
    f32 InvMass1 = pB->GetInvMass() ;
    f32 TotalInvMass = InvMass0 + InvMass1 ;
    if( TotalInvMass == 0.0f )
        return;
        
    // Move particles towards plane
    f32 D = Plane.Distance(pA->m_Pos) ;
    D *= 1.0f / TotalInvMass ;
    Plane.Normal *= D;
    pA->m_Pos -= InvMass0 * Plane.Normal ;
    pB->m_Pos += InvMass1 * Plane.Normal ;

    #ifdef X_DEBUG
        AddDebugPlane(Point, Normal, XCOLOR_GREEN) ;
    #endif        
}

//==============================================================================

void ragdoll::ApplyHumanConstraints( void )
{
    CONTEXT("ragdoll::ApplyHumanConstraints") ;

    s32     i ;
    vector3 Point, Normal, Out ;
    vector3 Torso2Torso, Torso2Hip,
            Hip2Torso, Hip2Hip, Hip2Knee,
            Knee2Hip, Knee2Foot ;
    vector3 Shoulder2Torso, Shoulder2Elbow,  Shoulder2Shoulder ;

    // Do both sides of body
    for (i = 0 ; i < 2 ; i++)
    {
        // Useful info
        f32         Side     = (i == 0) ? 1.0f : -1.0f ;
        joint_side& Joints   = m_Joints.m_Side[i] ;
        joint_side& OpJoints = m_Joints.m_Side[i^1] ;

        // Keep shoulder behind of hip plane
        Torso2Torso    = OpJoints.m_pTorso->m_Pos - Joints.m_pTorso->m_Pos ;
        Torso2Hip      = Joints.m_pHip->m_Pos - Joints.m_pTorso->m_Pos ;
        Point          = (Joints.m_pShoulder->m_Pos + Joints.m_pTorso->m_Pos) * 0.5f ;
        Normal         = Torso2Torso.Cross(Torso2Hip) * Side ;
        KeepBehindPlane(Point, Normal, Joints.m_pShoulder, Joints.m_pTorso, 1.0f) ;

        // Stop knee from going too far out
        Normal = Joints.m_pHip->m_Pos - OpJoints.m_pHip->m_Pos ;
        Point  = Joints.m_pHip->m_Pos + (2.0f * Normal) ;
        KeepBehindPlane(Point, Normal, Joints.m_pKnee, Joints.m_pHip) ;

        // Keep knee behind hip plane
        Hip2Torso   = Joints.m_pTorso->m_Pos - Joints.m_pHip->m_Pos ;
        Torso2Torso = OpJoints.m_pTorso->m_Pos - Joints.m_pTorso->m_Pos ;
        Normal      = Hip2Torso.Cross(Torso2Torso) * Side ;
        Point       = (Normal * 0.01f) + ((Joints.m_pKnee->m_Pos + Joints.m_pHip->m_Pos) * 0.5f) ;
        KeepBehindPlane(Point, Normal, Joints.m_pKnee, Joints.m_pHip, 1.0f) ;

        // Keep foot behind thigh plane
        Knee2Hip    = Joints.m_pHip->m_Pos   - Joints.m_pKnee->m_Pos ;
        Knee2Foot   = Joints.m_pFoot->m_Pos  - Joints.m_pKnee->m_Pos ;
        Hip2Hip     = OpJoints.m_pHip->m_Pos - Joints.m_pHip->m_Pos ;
        Point       = (Joints.m_pFoot->m_Pos + Joints.m_pKnee->m_Pos) * 0.5f ;
        Normal      = Hip2Hip.Cross(Knee2Hip) * Side ;
        KeepBehindPlane(Point, Normal, Joints.m_pFoot, Joints.m_pKnee, 1.0f) ;

        // Compute leg plane normal
        Hip2Hip  = OpJoints.m_pHip->m_Pos - Joints.m_pHip->m_Pos ;
        Hip2Knee = Joints.m_pKnee->m_Pos  - Joints.m_pHip->m_Pos ;
        Out      = Hip2Hip.Cross(Hip2Knee) ;
        Normal   = Out.Cross(Hip2Knee) ;

        // Keep knee on leg plane
        //Normal = OpJoints.m_pTorso->m_Pos - Joints.m_pTorso->m_Pos ;
        //Point  = (Joints.m_pKnee->m_Pos + Joints.m_pHip->m_Pos) * 0.5f ;
        //KeepOnPlane(Point, Normal, Joints.m_pKnee, Joints.m_pHip) ;

        // Keep foot on leg plane without causing any twist - works finally!!!
        //Normal = OpJoints.m_pTorso->m_Pos - Joints.m_pTorso->m_Pos ;
        Point  = (Joints.m_pFoot->m_Pos + Joints.m_pKnee->m_Pos) * 0.5f ;
        KeepOnPlane(Point, Normal, Joints.m_pFoot, Joints.m_pKnee) ;


        // Correct toe?
        if (Joints.m_pToe)
        {
            // Keep toe on leg plane
            Normal = OpJoints.m_pTorso->m_Pos - Joints.m_pTorso->m_Pos ;
            Point  = (Joints.m_pToe->m_Pos + Joints.m_pFoot->m_Pos) * 0.5f ;
            KeepOnPlane(Point, Normal, Joints.m_pToe, Joints.m_pFoot) ;
        
            // Keep toe pointing forwards
            Hip2Hip     = OpJoints.m_pHip->m_Pos - Joints.m_pHip->m_Pos ;
            Knee2Foot   = Joints.m_pFoot->m_Pos  - Joints.m_pKnee->m_Pos ;
            Point       = (Joints.m_pToe->m_Pos + Joints.m_pFoot->m_Pos) * 0.5f ;
            Normal      = Hip2Hip.Cross(Knee2Foot) * Side ;
            KeepBehindPlane(Point, Normal, Joints.m_pToe, Joints.m_pFoot, 1.0f) ;
        }

        // Keep elbow behind side plane (stop it going into the head)
        Normal         = OpJoints.m_pShoulder->m_Pos - Joints.m_pShoulder->m_Pos ;
        Point          = (Joints.m_pElbow->m_Pos + Joints.m_pShoulder->m_Pos) * 0.5f ;
        KeepBehindPlane(Point, Normal, Joints.m_pElbow, Joints.m_pShoulder, 1.0f) ;

        // Keep elbow infront of torso
        Shoulder2Shoulder = OpJoints.m_pShoulder->m_Pos - Joints.m_pShoulder->m_Pos ;
        Shoulder2Torso = Joints.m_pTorso->m_Pos - Joints.m_pShoulder->m_Pos ;
        Point          = (Joints.m_pElbow->m_Pos + Joints.m_pShoulder->m_Pos) * 0.5f ;
        Normal         = Shoulder2Shoulder.Cross(Shoulder2Torso) * Side ;
        Point -= Normal * 0.0001f ;  // Keep slightly infront so geom arm rotation doesn't flip
        KeepBehindPlane(Point, Normal, Joints.m_pElbow, Joints.m_pShoulder, 1.0f) ;

        // Keep wrist behind upper arm plane
        Shoulder2Torso = Joints.m_pTorso->m_Pos - Joints.m_pShoulder->m_Pos ;
        Shoulder2Elbow = Joints.m_pElbow->m_Pos - Joints.m_pShoulder->m_Pos ;
        Point          = (Joints.m_pWrist->m_Pos + Joints.m_pElbow->m_Pos) * 0.5f ;
        Normal         = Shoulder2Torso.Cross(Shoulder2Elbow) * Side ;
        KeepBehindPlane(Point, Normal, Joints.m_pWrist, Joints.m_pElbow) ;

        // Keep wrist on arm plane - Old stiff arms method!
        //Shoulder2Torso = Joints.m_pTorso->m_Pos - Joints.m_pShoulder->m_Pos ;
        //Shoulder2Elbow = Joints.m_pElbow->m_Pos - Joints.m_pShoulder->m_Pos ;
        //Point          = (Joints.m_pWrist->m_Pos + Joints.m_pElbow->m_Pos) * 0.5f ;
        //Normal         = Shoulder2Elbow.Cross(Shoulder2Torso.Cross(Shoulder2Elbow)) ;
        //KeepOnPlane(Point, Normal, Joints.m_pWrist, Joints.m_pElbow) ;
    }
}

//==============================================================================

void ragdoll::ApplyDistConstraints( void )
{
    CONTEXT("ragdoll::ApplyDistConstraints") ;

#ifdef TARGET_PS2
    // Rules have been dma'd to scratch pad
    dist_rule* DistRules = (dist_rule*)0x70000000;
#else
    // Use statically defined rules
    dist_rule* DistRules  = m_pDef->m_DistRules ;
#endif
    
    // Apply distance constraints
    s32 NDistRules = m_pDef->m_NDistRules ;
    for (s32 i = 0 ; i < NDistRules ; i++)
        DistRules[i].Apply( m_Particles );
}

//==============================================================================

void ragdoll::ApplyRagdollBoneColl( vector3& BoneStart, vector3& BoneEnd )
{
    // Since we are testing a point against a pill, use double the radius
    // to simulate testing a sphere against a pill
    f32 Radius    = RAGDOLL_PARTICLE_RADIUS*2 ;
    f32 RadiusSqr = Radius * Radius ;

    // Loop through all particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Lookup particle
        particle& Particle = m_Particles[i] ;

        // Skip if it cannot be moved
        if (Particle.m_InvMass == 0)
            continue ;

        // Get vector and distance to line down middle of cylinder
        vector3 Delta   = Particle.m_Pos.GetClosestVToLSeg(BoneStart, BoneEnd) ;
        f32     DistSqr = Delta.LengthSquared() ;

        // Project out of capped cylinder?
        if ((DistSqr > 0.00001f) && (DistSqr < RadiusSqr))
        {
            // Keep away from cylinder center line
            f32 Dist = x_sqrt(DistSqr) ;
            ASSERT(Dist > 0) ;
            f32 PenDist = (Dist - Radius) ;
            f32 Diff    = PenDist / Dist ;

            ASSERT(Delta.IsValid()) ;
            ASSERT(x_isvalid(Dist)) ;
            ASSERT(x_isvalid(Radius)) ;

            // Scale and dampen
            Delta *= Diff ;

            // Project out of bone
            Particle.m_Pos += Delta * 0.5f ;

            // Move bone particles the opposite direction also
            BoneStart -= Delta * 0.5f ;
            BoneEnd   -= Delta * 0.5f ;

            // Friction is proportional to the penetration distance and mass
            f32 Friction = -PenDist * RAGDOLL_FRICTION / Particle.m_InvMass ;
            if (Friction < RAGDOLL_MIN_FRICTION)
                Friction = RAGDOLL_MIN_FRICTION ;
            else                
            if (Friction > RAGDOLL_MAX_FRICTION)
                Friction = RAGDOLL_MAX_FRICTION ;

            // Split vel into components
            vector3 Vel = Particle.GetVelocity() ;
    
            // Compute components into cylinder
            Delta.Normalize() ;
            vector3 Perp = Delta.Dot(Vel) * Delta ;
            vector3 Para = Vel - Perp ;

            // Apply friction parallel to the plane
            Para -= Para*Friction ;

            // Bounce
            Perp -= Perp * RAGDOLL_BOUNCY ;

            // Compute new vel
            Vel = Perp + Para ;

            // Set new velocity
            Particle.SetVelocity(Vel) ;
        }
    }
}

//==============================================================================

void ragdoll::ApplyRagdollConstraints( ragdoll& Ragdoll )
{
    CONTEXT("ragdoll::ApplyRagdollConstraints") ;

    // Check both sides of ragdoll
    const joints& Joints = Ragdoll.m_Joints ;
    for (s32 i = 0 ; i < 2 ; i++)
    {
        // Lookup side
        const joint_side& Side   = Joints.m_Side[i] ;
        const joint_side& OpSide = Joints.m_Side[i ^ 1] ;

        // Project out of bones
        ApplyRagdollBoneColl(Side.m_pFoot->m_Pos,        Side.m_pKnee->m_Pos        ) ;
        ApplyRagdollBoneColl(Side.m_pKnee->m_Pos,        Side.m_pHip->m_Pos         ) ;
        ApplyRagdollBoneColl(Side.m_pHip->m_Pos,         Side.m_pTorso->m_Pos       ) ;
        ApplyRagdollBoneColl(Side.m_pTorso->m_Pos,       Side.m_pShoulder->m_Pos    ) ;
        ApplyRagdollBoneColl(Side.m_pShoulder->m_Pos,    Joints.m_pNeck->m_Pos      ) ;
        
        ApplyRagdollBoneColl(Side.m_pTorso->m_Pos,       OpSide.m_pShoulder->m_Pos  ) ;
    }

    // Lookup sides
    const joint_side& Side   = Joints.m_Side[0] ;
    const joint_side& OpSide = Joints.m_Side[1] ;

    // Check opposite side bones
    ApplyRagdollBoneColl(Side.m_pShoulder->m_Pos, OpSide.m_pShoulder->m_Pos ) ;
    ApplyRagdollBoneColl(Side.m_pHip->m_Pos,      OpSide.m_pHip->m_Pos      ) ;

    // Check these bones
    ApplyRagdollBoneColl(Joints.m_pNeck->m_Pos, Joints.m_pHead->m_Pos       ) ;
}

//==============================================================================

bbox ragdoll::ComputeWorldBBox( void ) const
{
    // Compute world bbox of ragdoll
    bbox WorldBBox ;
    WorldBBox.Clear() ;
    for (s32 i = 0 ; i < m_NParticles ; i++)
        WorldBBox += m_Particles[i].m_Pos ;
    WorldBBox.Inflate(RAGDOLL_PARTICLE_RADIUS, RAGDOLL_PARTICLE_RADIUS, RAGDOLL_PARTICLE_RADIUS) ;

    return WorldBBox ;
}

//==============================================================================

void ragdoll::ApplyCharacterConstraints( void )
{
    CONTEXT("ragdoll::ApplyCharacterConstraints") ;

    // Compute world bbox of ragdoll
    bbox WorldBBox = ComputeWorldBBox() ;

/*
    // Selects all objects whose bbox intersects the ragdoll object
    g_ObjMgr.SelectBBox( object::ATTR_LIVING, WorldBBox, object::TYPE_ALL_TYPES) ;
    slot_id SlotID = g_ObjMgr.StartLoop();
    while(SlotID != SLOT_NULL)
    {
        // Lookup object
        object* pObject = g_ObjMgr.GetObjectBySlot(SlotID) ;
        ASSERT(pObject) ;

        // Get bbox of object
        bbox    BBox = pObject->GetBBox() ;

        // Compute capped cylinder from bbox
        vector3 Bottom = pObject->GetPosition() ;
        vector3 Top    = Bottom ;
        Top.Y = BBox.Max.Y ;
        f32     Radius   = 100.0f ;

        // Collide with the ragdoll
        ApplyCappedCylinderColl(Bottom, Top, Radius + RAGDOLL_PARTICLE_RADIUS) ;

        // Check next object
        SlotID = g_ObjMgr.GetNextResult(SlotID) ;
    }
    g_ObjMgr.EndLoop();
*/

    // Find all dead bodies
   // g_ObjMgr.SelectBBox(object::ATTR_COLLIDABLE, WorldBBox, object::TYPE_DEAD_BODY) ;
   g_ObjMgr.SelectBBox(object::ATTR_COLLIDABLE, WorldBBox, object::TYPE_NULL) ; // #TODO: !!!
    slot_id SlotID = g_ObjMgr.StartLoop();
    while(SlotID != SLOT_NULL)
    {
        // Lookup object
        dead_body* pDeadBody = (dead_body*)g_ObjMgr.GetObjectBySlot(SlotID) ;
        ASSERT(pDeadBody) ;

        // Get ragdoll pointer
        ragdoll* pRagdoll = pDeadBody->GetRagdollPointer() ;
        ASSERT(pRagdoll) ;

        // Collide with the ragdoll (but not self)
        if (this != pRagdoll)
            ApplyRagdollConstraints(*pRagdoll) ;

        // Check next object
        SlotID = g_ObjMgr.GetNextResult(SlotID) ;
    }
    g_ObjMgr.EndLoop();
}

//==============================================================================

void ragdoll::ApplyCollConstraints( void )
{
    CONTEXT("ragdoll::ApplyCollConstraints") ;

    s32 i ;

    // Setup ground plane
    plane GroundPlane ;
    GroundPlane.Setup(vector3(0,0,0), vector3(0,1,0)) ;

    // Compute world bbox taking particle velocities into account
    bbox WorldBBox ;
    WorldBBox.Clear() ;
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Get delta for particle
        vector3 Delta   = Particle.m_Pos - Particle.m_LastCollPos ;
        f32     DistSqr = Delta.LengthSquared() ;
        
        // Will this particle be considered for collision?
        if (DistSqr >= x_sqr(RAGDOLL_MIN_COLL_DIST))
        {
            // Get distance to move and create bbox expand delta
            f32     Dist = x_sqrt(DistSqr) ;
            vector3 Expand(Dist, Dist, Dist) ;

            // Add bounds of desired movement + maximum deflection distance that could occur
            WorldBBox += Particle.m_Pos + Expand ;
            WorldBBox += Particle.m_Pos - Expand ;
            WorldBBox += Particle.m_LastCollPos + Expand ;
            WorldBBox += Particle.m_LastCollPos - Expand ;
        }
        else
        {
            // Just add bounds of desired movement
            WorldBBox += Particle.m_Pos ;
            WorldBBox += Particle.m_LastCollPos ;
        }
    }

    // Inflate to take particle radius into account and for a bit of safety
    WorldBBox.Inflate(RAGDOLL_COLL_BBOX_INFLATE, RAGDOLL_COLL_BBOX_INFLATE, RAGDOLL_COLL_BBOX_INFLATE) ; 

    // Prepare verlet collision by collecting possible collision objects
    VerletCollision_CollectObjects(WorldBBox) ;

    // Apply collision constraints
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;
        if (!Particle.m_bCollision)
            continue ;

        // Particle fixed?
        if (Particle.m_InvMass == 0)
            continue ;

        // Compute start and end pts
        vector3 S = Particle.m_LastCollPos ;
        vector3 E = Particle.m_Pos ;
        vector3 Delta = E-S;
        f32     DistSq = Delta.LengthSquared();
        if( DistSq < x_sqr(RAGDOLL_MIN_COLL_DIST) )
            continue;

        // Use collision manager to find collision pt?
        if (m_ObjectGuid)
        {
            sphere_cast Cast ;

            // Collision?
            if ( VerletCollision_SphereCast(S, E, RAGDOLL_PARTICLE_RADIUS, Cast) )
            {
                // Pull back from collision a tad
                f32     T     = Cast.m_CollT ;
                f32     Dist  = x_sqrt(DistSq);
                T -= RAGDOLL_COLLISION_BACKOFF / Dist ;
                if (T < 0)
                    T = 0 ;

                // Put new start pos at collision pos
                S = S + (T * Delta) ;

                // Get penetration depth of the end point we wanted to reach
                Dist = Cast.m_CollPlane.Distance(E) - RAGDOLL_PARTICLE_RADIUS ;

                // Friction is proportional to the penetration distance and mass
                f32 Friction = -Dist * RAGDOLL_FRICTION / Particle.m_InvMass ;
                if (Friction < RAGDOLL_MIN_FRICTION)
                    Friction = RAGDOLL_MIN_FRICTION ;
                else                
                if (Friction > RAGDOLL_MAX_FRICTION)
                    Friction = RAGDOLL_MAX_FRICTION ;

                // Split vel into components
                vector3 Vel = Particle.GetVelocity() ;
                vector3 Perp, Para ;
                Cast.m_CollPlane.GetComponents(Vel, Para, Perp) ;

                // Calculate the impact speed squared.
                f32 ImpactSpeed = Perp.LengthSquared();

                // Apply friction parallel to the plane
                Para -= Para*Friction ;

                // Bounce
                Perp -= Perp * RAGDOLL_BOUNCY ;

                // Compute new vel
                Vel = Perp + Para ;

                // Project end point out of plane that was collided with
                E += Cast.m_CollPlane.Normal * (-Dist + RAGDOLL_COLLISION_BACKOFF) ;

                // Now see how close we can get to the final projected pos
                if ( VerletCollision_SphereCast(S, E, RAGDOLL_PARTICLE_RADIUS, Cast) )
                {
                    // Pull back from collision a tad
                    T     = Cast.m_CollT ;
                    Delta = E - S ;
                    Dist  = Delta.Length() ;
                    T -= RAGDOLL_COLLISION_BACKOFF / Dist ;
                    if (T < 0)
                        T = 0 ;

                    // Setup new end pt
                    E = S + (T * (E - S)) ;
                }

                // Set new velocity
                Particle.SetVelocity(Vel) ;

                // Set new position
                Particle.m_Pos = E ;

                if( (m_BodyImpactSoundID==0) && ImpactSpeed > x_sqr( RAGDOLL_MIN_AUDIO_VELOCITY ) )
                {
                    xbool bIsBody = 
                        ( &m_Particles[i] == m_Joints.m_Side[0].m_pHip ) ||
                        ( &m_Particles[i] == m_Joints.m_Side[1].m_pHip ) ||
                        ( &m_Particles[i] == m_Joints.m_Side[0].m_pShoulder ) ||
                        ( &m_Particles[i] == m_Joints.m_Side[1].m_pShoulder ) ||
                        ( &m_Particles[i] == m_Joints.m_Side[0].m_pTorso ) ||
                        ( &m_Particles[i] == m_Joints.m_Side[1].m_pTorso );

                    if( bIsBody )
                    {
                        xbool bPlaySound = (!g_AudioMgr.IsValidVoiceId( m_BodyImpactSoundID )) || (g_AudioMgr.GetCurrentPlayTime( m_BodyImpactSoundID ) < 0.5f);

                        if( bPlaySound )
                        {
                            //f32 Velocity = x_min( ImpactSpeed, RAGDOLL_MAX_AUDIO_VELOCITY ) - RAGDOLL_MIN_AUDIO_VELOCITY;
                            //f32 Range    = RAGDOLL_MAX_AUDIO_VELOCITY - RAGDOLL_MIN_AUDIO_VELOCITY;
                            //f32 Scale    = Velocity / Range;
                            //f32 Volume   = (0.75 + Scale * 0.25f) * RAGDOLL_AUDIO_VOLUME_SCALE; // volume range is [0.75..1.0]

                            //LOG_MESSAGE( "ragdoll::ApplyCollConstraints", "Vel: %f, Range: %f, Scale: %f, Volume: %f", Velocity, Range, Scale, Volume );
                            m_BodyImpactSoundID  = g_AudioMgr.Play( "TerroristA_BodyFall", Particle.m_Pos, TRUE, TRUE );
                            //g_AudioManager.SetVolume( m_BodyImpactSoundID, Volume );
                        }
                    }
                }
            }
        }
        else
        {
            // Use test ground plane for collision

            // Get penetration depth - below plane?
            f32 Dist = GroundPlane.Distance(Particle.m_Pos) - RAGDOLL_PARTICLE_RADIUS ;
            if (Dist < 0)
            {
                // Friction is proportional to the penetration distance and mass
                f32 Friction = -Dist * RAGDOLL_FRICTION / Particle.m_InvMass ;
                if (Friction < RAGDOLL_MIN_FRICTION)
                    Friction = RAGDOLL_MIN_FRICTION ;
                else                
                if (Friction > RAGDOLL_MAX_FRICTION)
                    Friction = RAGDOLL_MAX_FRICTION ;

                // Split vel into components
                vector3 Vel = Particle.GetVelocity() ;
                vector3 Perp, Para ;
                GroundPlane.GetComponents(Vel, Para, Perp) ;

                // Apply friction parallel to the plane
                Para -= Para*Friction ;

                // Bounce
                Perp -= Perp * RAGDOLL_BOUNCY ;

                // Apply friction parallel to the plane
                Vel = Perp + Para ;

                // Set new particle vel
                Particle.SetVelocity(Vel) ;

                // Project particle out of the plane
                Particle.m_Pos -= GroundPlane.Normal * Dist ;
            }
        }

        // Update last collision free pos
        Particle.m_LastCollPos = Particle.m_Pos ;
    }
}

//==============================================================================

void ragdoll::ApplyConstraints( void )
{
    CONTEXT("ragdoll::ApplyConstraints") ;

    s32 i,j ;

    // Iterate constraints
    for (j = 0 ; j < 1 ; j++)
    {
        // Collision with world
        ApplyCollConstraints() ;

#ifdef TARGET_PS2
        // DMA distance rules to scratch pad
        if( m_pDef->m_NDistRules )
            spad_CopyTo( 0, m_pDef->m_DistRules, m_pDef->m_NDistRules * sizeof(dist_rule) );
#endif

        // Model constraints
        for (i = 0 ; i < RAGDOLL_ITER_COUNT ; i++)
        {
            #ifdef X_DEBUG
                // Clear render planes
                m_NDebugPlanes = 0 ;
            #endif

            // Human
            ApplyHumanConstraints() ;

            // Distance
            ApplyDistConstraints() ;
        }
    }

    // Compute center of mass velocity
    vector3 CenterVel(0,0,0) ;
    for (i = 0 ; i < m_NParticles ; i++)
        CenterVel += m_Particles[i].GetVelocity() ;
    CenterVel *= 1.0f / (f32)m_NParticles ;

    // Apply damping
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Compute angular and linear velocity
        vector3 Vel        = Particle.GetVelocity() ;
        vector3 AngularVel = Vel - CenterVel ;
        vector3 LinearVel  = Vel - AngularVel ;
            
        // Dampen
        LinearVel  -= LinearVel  * RAGDOLL_LINEAR_DAMPEN ;
        AngularVel -= AngularVel * RAGDOLL_ANGULAR_DAMPEN ;

        // Compute new vel
        Vel = LinearVel + AngularVel ;

// SB - I have not got this working correctly yet - it causes the ragdoll to breakdance!!
/*
        // Apply joint dampaning?
        if (Particle.m_pParent)
        {
            // Dampen relative to parent particle
            vector3 ParentVel = Particle.m_pParent->GetVelocity() ;
            Vel -= ParentVel ;
            Vel -= Vel * RAGDOLL_JOINT_DAMPEN ;
            Vel += ParentVel ;
        }
*/
        // Set new vel
        Particle.SetVelocity(Vel) ;
    }
}

//==============================================================================

// Framerate dependent integration
void ragdoll::Integrate( f32 DeltaTime )
{
    // Nothing to do?
    if (DeltaTime == 0)
        return ;

    // Setup contants
    vector3 Gravity          = vector3(0,RAGDOLL_GRAVITY,0) ;
    f32     DeltaTimeSquared = DeltaTime * DeltaTime ;

    // Apply verlet integration to all particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Lookup particle
        particle* pParticle = &m_Particles[i] ;
        ASSERT(pParticle) ;

        // Compute movement
        vector3 Pos   = pParticle->m_Pos ;
        if (pParticle->m_InvMass != 0)
        {
            vector3 Vel   = pParticle->m_Pos - pParticle->m_LastPos ;
            vector3 Accel = (Gravity * DeltaTimeSquared) ;

            // Move!
            pParticle->m_Pos += Vel + Accel ;
        }

        // Update last position
        pParticle->m_LastPos = Pos ;
    }
}

//==============================================================================

void ragdoll::UpdateStickBones( void )
{
    CONTEXT("ragdoll::UpdateStickBones") ;

    vector3 AxisX, AxisY, AxisZ, Start, End ;

    // Setup both sides
    for (s32 i = 0 ; i < 2 ; i++)
    {
        f32                 Side   = (i == 0) ? 1.0f : -1.0f ;
        joint_side&         Joints = m_Joints.m_Side[i] ;
        stick_bone_side&    Bones  = m_StickBones.m_Side[i] ;
        
        // Thigh
        Start = Joints.m_pHip->m_Pos ;
        End   = Joints.m_pKnee->m_Pos ;
        AxisZ = End - Start ;
        AxisX = AxisZ.Cross(m_Joints.m_Side[1].m_pHip->m_Pos - m_Joints.m_Side[0].m_pHip->m_Pos) * Side ;
        AxisY = AxisX.Cross(AxisZ) ;
        Bones.m_Thigh.Update(AxisX, AxisY, AxisZ, Start, End) ;

        // Setup leg plane "out"
        AxisX = AxisX.Cross(AxisZ) ;

        // Calf
        if (Joints.m_pToe)
        {
            Start = Joints.m_pKnee->m_Pos ;
            End   = Joints.m_pFoot->m_Pos ;
            AxisZ = End - Start ;
            AxisY = AxisX.Cross(AxisZ) ;
            Bones.m_Calf.Update(AxisX, AxisY, AxisZ, Start, End) ;
        }
        else
        {
            Start = Joints.m_pKnee->m_Pos ;
            End   = Joints.m_pFoot->m_Pos ;
            AxisZ = End - Start ;
            AxisY = AxisX.Cross(AxisZ) ;
            Bones.m_Calf.Update(AxisX, AxisY, AxisZ, Start, End) ;
        }

        // Foot
        if (Joints.m_pToe)
        {
            Start = Joints.m_pFoot->m_Pos ;
            End   = Joints.m_pToe->m_Pos ;
            AxisZ = End - Start ;
            AxisY = AxisX.Cross(AxisZ) ;
            Bones.m_Foot.Update(AxisX, AxisY, AxisZ, Start, End) ;
        }


        // UpperArm
        Start = Joints.m_pShoulder->m_Pos ;
        End   = Joints.m_pElbow->m_Pos ;
        AxisZ = End - Start ;
        AxisX = Joints.m_pTorso->m_Pos - Joints.m_pShoulder->m_Pos ;
        AxisY = AxisZ.Cross(AxisX) ;
        AxisX = AxisZ.Cross(AxisY) ;
        Bones.m_UpperArm.Update(AxisX, AxisY, AxisZ, Start, End) ;
        
        // Forearm
        Start = Joints.m_pElbow->m_Pos ;
        End   = Joints.m_pWrist->m_Pos ;
        AxisZ = End - Start ;
        AxisY = AxisZ.Cross(AxisY) ;
        AxisX = AxisY.Cross(AxisZ) ;
        Bones.m_Forearm.Update (AxisX,  AxisY,  AxisZ,  Start, End) ;
        
        // Forearm - Old stiff arms method!
        //Start = Joints.m_pElbow->m_Pos ;
        //End   = Joints.m_pWrist->m_Pos ;
        //AxisZ = End - Start ;
        //AxisY = AxisZ.Cross(AxisX) ;
        //Bones.m_Forearm.Update(AxisX, AxisY, AxisZ, Start, End) ;

        // Hand
        if (Joints.m_pFinger)
        {
            Start = Joints.m_pWrist->m_Pos ;
            End   = Joints.m_pFinger->m_Pos ;
            AxisZ = End - Start ;
            AxisY = AxisZ.Cross(AxisX) ;
            Bones.m_Hand.Update(AxisX, AxisY, AxisZ, Start, End) ;
        }
    }

    // Hips
    Start = (m_Joints.m_Side[0].m_pTorso->m_Pos + m_Joints.m_Side[1].m_pTorso->m_Pos) * 0.5f ;
    End   = (m_Joints.m_Side[0].m_pHip->m_Pos   + m_Joints.m_Side[1].m_pHip->m_Pos)   * 0.5f ;
    AxisZ = End - Start ;
    AxisX = m_Joints.m_Side[1].m_pTorso->m_Pos - m_Joints.m_Side[0].m_pTorso->m_Pos ;
    AxisY = AxisZ.Cross(AxisX) ;
    m_StickBones.m_Hips.Update(AxisX, AxisY, AxisZ, Start, End) ;
    
    // Torso
    Start = (m_Joints.m_Side[0].m_pShoulder->m_Pos + m_Joints.m_Side[1].m_pShoulder->m_Pos) * 0.5f ;
    End   = (m_Joints.m_Side[0].m_pTorso->m_Pos    + m_Joints.m_Side[1].m_pTorso->m_Pos)    * 0.5f ;
    AxisZ = End - Start ;
    AxisX = m_Joints.m_Side[1].m_pShoulder->m_Pos - m_Joints.m_Side[0].m_pShoulder->m_Pos ;
    AxisY = AxisZ.Cross(AxisX) ;
    m_StickBones.m_Torso.Update(AxisX, AxisY, AxisZ, Start, End) ;
    
    // Chest
    Start = m_Joints.m_Side[0].m_pShoulder->m_Pos ;
    End   = m_Joints.m_Side[1].m_pShoulder->m_Pos ;
    AxisZ = End - Start ;
    AxisX = m_Joints.m_pNeck->m_Pos - ((m_Joints.m_Side[0].m_pTorso->m_Pos + m_Joints.m_Side[0].m_pTorso->m_Pos)*0.5f) ;
    AxisY = AxisZ.Cross(AxisX) ;
    AxisX = AxisZ.Cross(AxisY) ;
    m_StickBones.m_Chest.Update(AxisX, AxisY, AxisZ, Start, End) ;

    // Head
    Start = m_Joints.m_pNeck->m_Pos ;
    End   = m_Joints.m_pHead->m_Pos ;
    AxisZ = End - Start ;
    AxisX = AxisZ.Cross(m_Joints.m_Side[0].m_pShoulder->m_Pos - m_Joints.m_pNeck->m_Pos) ;
    AxisY = AxisZ.Cross(AxisX) ;
    m_StickBones.m_Head.Update(AxisX, AxisY, AxisZ, Start, End) ;
}

//==============================================================================

void ragdoll::CreateMinDistRules( particle* pA, const char* pList[], f32 Dist, f32 Damp )
{
    s32 i = 0 ;
    s32 ParticleA = FindParticleIndex(pA->m_pName) ;
    while(pList[i])
    {
        s32 ParticleB = FindParticleIndex(pList[i]) ;
        if (    (ParticleB != -1) &&
                (ParticleB != ParticleA) &&
                (!AreParticlesConnected(ParticleA, ParticleB)) )
        {
            // Create min dist dist_rule
            ASSERT(m_pDef->m_NDistRules < m_pDef->m_NMaxDistRules) ;
            dist_rule& DistRule = m_pDef->m_DistRules[m_pDef->m_NDistRules++] ;
            DistRule.Init(dist_rule::DIST_MIN, "Auto keep apart", m_Particles, ParticleA, ParticleB, Dist, FALSE, Damp) ;
        }

        i++ ;
    }
}

//==============================================================================

xbool ragdoll::Init( const char*         pSkinGeomName, 
                     const char*         pAnimName,
                           ragdoll::type RagdollType, 
                           guid          ObjectGuid )
{
    CONTEXT("ragdoll::Init") ;

    s32 i ;

    // Must have geometry and anim
    if ( (!pSkinGeomName) || (!pAnimName) )
        return FALSE;

    // Make sure type is valid
    ASSERT(RagdollType >= 0) ;
    ASSERT(RagdollType < TYPE_COUNT) ;

    // Make sure table is in sync with enums!
    ASSERT( (sizeof(s_RagdollDefTable) / sizeof(s_RagdollDefTable[0])) == TYPE_COUNT ) ;
    
    // Make sure enum and def table is in sync! ( -1 is because the enum has a end of list entry)
    ASSERT( (sizeof(s_RagdollDefTable) / sizeof(s_RagdollDefTable[0])) == ( s_TypeList.GetCount()-1 )) ;

    // Keep definition pointer
    m_pDef = s_RagdollDefTable[RagdollType] ;
    ASSERT(m_pDef) ;
    ragdoll_def& RagdollDef = *m_pDef ;

    // Setup skin geom
    m_SkinInst.SetUpSkinGeom(pSkinGeomName) ;
    skin_geom* pSkinGeom = m_SkinInst.GetSkinGeom() ;
    if (!pSkinGeom)
        return FALSE;

    // Setup anim
    m_hAnimGroup.SetName(pAnimName) ;
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return FALSE;

    // Keep guid
    m_ObjectGuid = ObjectGuid ;

    // Cleanup current
    Kill() ;

    // Lookup info
    const anim_group& AnimSkel = *pAnimGroup ;

    // Clear lists
    m_NParticles   = 0 ;

    #ifdef X_DEBUG
        m_NDebugPlanes = 0 ;
    #endif
    
    // Create particles from definitions
    for (i = 0 ; i < RagdollDef.m_NParticles ; i++)
    {
        ASSERT(m_NParticles < ragdoll::MAX_PARTICLES) ;
        
        particle_def& Def = RagdollDef.m_Particles[i] ;
        ASSERT(FindParticle(Def.m_pName) == NULL) ;
        
        particle& Particle = m_Particles[m_NParticles++] ;

        // Grab color
        xcolor Col ;
        Col.R = (u8)Def.m_R ;
        Col.G = (u8)Def.m_G ;
        Col.B = (u8)Def.m_B ;
        Col.A = 255 ;

        // Grab position
        vector3 Pos( Def.m_PosX, Def.m_PosZ, Def.m_PosY ) ;

        // Setup
        Particle.Init(Def.m_pName, Pos, Def.m_Mass, Def.m_bCollision, Col) ;
    }

    // Bake bind
    for (i = 0 ; i < m_NParticles ; i++)
        m_Particles[i].SetBind() ;

    // Lookup particle joints
    m_Joints.m_Side[0].m_pToe      = FindParticle("Particle L Toe") ;
    m_Joints.m_Side[0].m_pFoot     = FindParticle("Particle L Foot") ;
    m_Joints.m_Side[0].m_pKnee     = FindParticle("Particle L Knee") ;
    m_Joints.m_Side[0].m_pHip      = FindParticle("Particle L Hip") ;
    m_Joints.m_Side[0].m_pTorso    = FindParticle("Particle L Torso") ;
    m_Joints.m_Side[0].m_pShoulder = FindParticle("Particle L Shoulder") ;
    m_Joints.m_Side[0].m_pElbow    = FindParticle("Particle L Elbow") ;
    m_Joints.m_Side[0].m_pWrist    = FindParticle("Particle L Wrist") ;
    m_Joints.m_Side[0].m_pFinger   = FindParticle("Particle L Finger") ;
    
    m_Joints.m_Side[1].m_pToe      = FindParticle("Particle R Toe") ;
    m_Joints.m_Side[1].m_pFoot     = FindParticle("Particle R Foot") ;
    m_Joints.m_Side[1].m_pKnee     = FindParticle("Particle R Knee") ;
    m_Joints.m_Side[1].m_pHip      = FindParticle("Particle R Hip") ;
    m_Joints.m_Side[1].m_pTorso    = FindParticle("Particle R Torso") ;
    m_Joints.m_Side[1].m_pShoulder = FindParticle("Particle R Shoulder") ;
    m_Joints.m_Side[1].m_pElbow    = FindParticle("Particle R Elbow") ;
    m_Joints.m_Side[1].m_pWrist    = FindParticle("Particle R Wrist") ;
    m_Joints.m_Side[1].m_pFinger   = FindParticle("Particle R Finger") ;
    
    m_Joints.m_pNeck                = FindParticle("Particle Neck") ;
    m_Joints.m_pHead                = FindParticle("Particle Head") ;

    // Setup particle parents for correct angular dampening
    if (m_Joints.m_pHead)
        m_Joints.m_pHead->SetParent(m_Joints.m_pNeck) ;
    for (i = 0 ; i < 2 ; i++)
    {
        joint_side& Side = m_Joints.m_Side[i] ;

        if (Side.m_pToe)
            Side.m_pToe->SetParent(Side.m_pFoot) ;
        
        if (Side.m_pFoot)
            Side.m_pFoot->SetParent(Side.m_pKnee) ;

        if (Side.m_pKnee)
            Side.m_pKnee->SetParent(Side.m_pHip) ;
        
        if (Side.m_pHip)
            Side.m_pHip->SetParent(Side.m_pTorso) ;
        
        if (Side.m_pShoulder)
            Side.m_pShoulder->SetParent(Side.m_pTorso) ;
        
        if (Side.m_pFinger)
            Side.m_pFinger->SetParent(Side.m_pWrist) ;
        
        if (Side.m_pWrist)
            Side.m_pWrist->SetParent(Side.m_pElbow) ;
        
        if (Side.m_pElbow)
            Side.m_pElbow->SetParent(Side.m_pShoulder) ;
    }
    
    // Initialize stick bones
    if (m_Joints.m_Side[0].m_pToe)
        m_StickBones.m_Side[0].m_Foot.Init  ("L Foot",         XCOLOR_YELLOW) ;
    m_StickBones.m_Side[0].m_Calf.Init      ("L Calf",         XCOLOR_YELLOW) ;
    m_StickBones.m_Side[0].m_Thigh.Init     ("L Thigh",        XCOLOR_YELLOW) ;
    m_StickBones.m_Side[0].m_UpperArm.Init  ("L Upper Arm",    XCOLOR_YELLOW) ;
    m_StickBones.m_Side[0].m_Forearm.Init   ("L Forearm",      XCOLOR_YELLOW) ;
    if (m_Joints.m_Side[0].m_pFinger)
        m_StickBones.m_Side[0].m_Hand.Init  ("L Hand",         XCOLOR_YELLOW) ;
    
    if (m_Joints.m_Side[1].m_pToe)
        m_StickBones.m_Side[1].m_Foot.Init  ("R Foot",         XCOLOR_YELLOW) ;
    m_StickBones.m_Side[1].m_Calf.Init      ("R Calf",         XCOLOR_YELLOW) ;
    m_StickBones.m_Side[1].m_Thigh.Init     ("R Thigh",        XCOLOR_YELLOW) ;
    m_StickBones.m_Side[1].m_UpperArm.Init  ("R Upper Arm",    XCOLOR_YELLOW) ;
    m_StickBones.m_Side[1].m_Forearm.Init   ("R Forearm",      XCOLOR_YELLOW) ;
    if (m_Joints.m_Side[1].m_pFinger)
        m_StickBones.m_Side[1].m_Hand.Init  ("R Hand",         XCOLOR_YELLOW) ;
    
    m_StickBones.m_Hips.Init ("Hips",   XCOLOR_YELLOW) ;
    m_StickBones.m_Torso.Init("Torso",  XCOLOR_YELLOW) ;
    m_StickBones.m_Chest.Init("Chest",  XCOLOR_YELLOW) ;
    m_StickBones.m_Head.Init ("Head",   XCOLOR_YELLOW) ;

    // Initialize ragdoll definition with distance rules and geometry bones?
    if (!RagdollDef.m_bInitialized)
    {
        // Flag it's initialized
        RagdollDef.m_bInitialized = TRUE ;
        RagdollDef.m_pSkinGeom    = pSkinGeomName;
        RagdollDef.m_pAnim        = pAnimName;
        RagdollDef.m_pType        = s_TypeList.GetString( RagdollType );

        // Create dist_rules
        for (i = 0 ; i < RagdollDef.m_NConstraints ; i++)
        {
            constraint_def& Def = RagdollDef.m_Constraints[i] ;

            // Grab particles
            s32       iParticleA = FindParticleIndex(Def.m_pParticleA) ;
            s32       iParticleB = FindParticleIndex(Def.m_pParticleB) ;
            ASSERT(iParticleA != -1) ;
            ASSERT(iParticleB != -1) ;
            particle* pParticleA = &m_Particles[iParticleA] ;
            particle* pParticleB = &m_Particles[iParticleB] ;
            ASSERT(pParticleA) ;
            ASSERT(pParticleB) ;
            ASSERT(pParticleA != pParticleB) ;

            // Grab color
            xcolor Col ;
            Col.R = (u8)Def.m_R ;
            Col.G = (u8)Def.m_G ;
            Col.B = (u8)Def.m_B ;
            Col.A = 255 ;

            // Setup dist
            f32 Dist = (pParticleA->m_Pos - pParticleB->m_Pos).Length() ;

            // Create equal dist_rule?
            if (Def.m_bEqual)
            {
                // Create equal dist dist_rule
                ASSERT(Def.m_EqualPercent == 100) ;
                ASSERT(RagdollDef.m_NDistRules < RagdollDef.m_NMaxDistRules) ;
                dist_rule& DistRule = RagdollDef.m_DistRules[RagdollDef.m_NDistRules++] ;
                DistRule.Init(dist_rule::DIST_EQUAL, Def.m_pName, m_Particles, iParticleA, iParticleB, Dist*Def.m_EqualPercent / 100.0f, Def.m_bBone, Def.m_Damp, Col) ;
            }

            // Create min dist_rule?
            if (Def.m_bMin)
            {
                // Create min dist dist_rule
                ASSERT(RagdollDef.m_NDistRules < RagdollDef.m_NMaxDistRules) ;
                dist_rule& DistRule = RagdollDef.m_DistRules[RagdollDef.m_NDistRules++] ;
                DistRule.Init(dist_rule::DIST_MIN, Def.m_pName, m_Particles, iParticleA, iParticleB, Dist*Def.m_MinPercent / 100.0f, Def.m_bBone, Def.m_Damp, Col) ;
            }

            // Create min dist_rule?
            if (Def.m_bMax)
            {
                // Create max dist dist_rule
                ASSERT(RagdollDef.m_NDistRules < RagdollDef.m_NMaxDistRules) ;
                dist_rule& DistRule = RagdollDef.m_DistRules[RagdollDef.m_NDistRules++] ;
                DistRule.Init(dist_rule::DIST_MAX, Def.m_pName, m_Particles, iParticleA, iParticleB, Dist*Def.m_MaxPercent / 100.0f, Def.m_bBone, Def.m_Damp, Col) ;
            }
        }
    
        // Keep wrists/elbows from getting under the body
        CreateMinDistRules(m_Joints.m_Side[0].m_pWrist, s_UpperBodyParticles, RAGDOLL_WRIST_SELF_INTERSECT_DIST, 0.9f) ;
        CreateMinDistRules(m_Joints.m_Side[1].m_pWrist, s_UpperBodyParticles, RAGDOLL_WRIST_SELF_INTERSECT_DIST, 0.9f) ;
        CreateMinDistRules(m_Joints.m_Side[0].m_pElbow, s_UpperBodyParticles, RAGDOLL_ELBOW_SELF_INTERSECT_DIST, 0.9f) ;
        CreateMinDistRules(m_Joints.m_Side[1].m_pElbow, s_UpperBodyParticles, RAGDOLL_ELBOW_SELF_INTERSECT_DIST, 0.9f) ;
    
        // Keep feet away from lower body
        CreateMinDistRules(m_Joints.m_Side[0].m_pFoot, s_LowerBodyParticles, RAGDOLL_FOOT_SELF_INTERSECT_DIST, 0.9f) ;
        CreateMinDistRules(m_Joints.m_Side[1].m_pFoot, s_LowerBodyParticles, RAGDOLL_FOOT_SELF_INTERSECT_DIST, 0.9f) ;

        // Update stick bones ready for skin matching
        UpdateStickBones() ;

        // Create geom_bones from skin model
        ASSERT(pSkinGeom->m_nBones <= AnimSkel.GetNBones()) ;
        for (i = 0 ; i < pSkinGeom->m_nBones ; i++)
        {
            // Lookup real geometry skin bone
            const anim_bone& SkinBone = AnimSkel.GetBone(i) ;

            // Get bind matrix
            matrix4 BindMatrix = SkinBone.BindMatrixInv ;
            BindMatrix.Invert() ;

            // Setup bone world and local space bbox
            bbox& WorldBBox = pSkinGeom->m_pBone[i].BBox ;
            bbox  LocalBBox = WorldBBox ;
            LocalBBox.Transform(SkinBone.BindMatrixInv) ;

            // Create a geom_bone
            ASSERT(RagdollDef.m_NGeomBones < RagdollDef.m_NMaxGeomBones) ;
            geom_bone& GeomBone = RagdollDef.m_GeomBones[RagdollDef.m_NGeomBones++] ;
        
            // Setup default info
            f32         BindT     = 0 ;
            s32         StickBone = -1 ;

            // Get center of geom_bone in world space
            vector3 C = WorldBBox.GetCenter() ;

            // Get closest stick bone and bind position
            StickBone = FindClosestStickBone( SkinBone.Name, C, BindT ) ; 

            // Optimization:
            // Bind T can be the same for all bones - so we can directly
            // store the L2W matrix inside the stick bone
            BindT = 0.0f ;

            // Setup
            GeomBone.Init( SkinBone.Name,       // Name
                           &m_StickBones[0],    // StickBones
                           StickBone,           // StickBone
                           BindT,               // BindT
                           BindMatrix,          // SkinBind
                           LocalBBox,           // LocalBBox
                           XCOLOR_WHITE) ;      // Color
        }
    }
    else
    {
        // Update stick bones ready for rendering
        UpdateStickBones() ;

        // Make sure ragdoll geometry compatible!
        if( pSkinGeom->m_nBones != m_pDef->m_NGeomBones )
        {
            ASSERTS( pSkinGeom->m_nBones != m_pDef->m_NGeomBones, 
                     xfs( "[%s] assigned to ragdoll[%s][%s][%s]\nhas different bone count! Check .skingeom and .anim for character/deadbody.",
                     pSkinGeomName, m_pDef->m_pType, m_pDef->m_pSkinGeom, m_pDef->m_pAnim ) );

            return FALSE;
        }
        
        // Make sure ragdoll anim is compatible!
        if( pAnimGroup->GetNBones() != m_pDef->m_NGeomBones )
        {
            ASSERTS( pAnimGroup->GetNBones() == m_pDef->m_NGeomBones, 
                     xfs( "[%s] assigned to ragdoll[%s][%s][%s]\nhas different bone count! Check .skingeom and .anim for character/deadbody.",
                     pAnimName, m_pDef->m_pType, m_pDef->m_pSkinGeom, m_pDef->m_pAnim ) );

            return FALSE;
        }
    }

    // Make sure anim group and geometry is compatible!
    if ( pSkinGeom->m_nBones != pAnimGroup->GetNBones() )
    {
        ASSERTS( pSkinGeom->m_nBones == pAnimGroup->GetNBones(),
                xfs( "Bone counts for [%s] and [%s] do not match!\nCheck .skingeom and .anim for character/deadbody.", pSkinGeomName, pAnimName ) );
        return FALSE;
    }

    // Attach particles to geom bones
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;
        
        // Get geom bone
        Particle.m_GeomBoneIndex = FindClosestGeomBone(Particle.m_Pos) ;
        ASSERT(Particle.m_GeomBoneIndex != -1) ;
    }

    // Clear time
    m_DeltaTime = 0 ;

    Reset() ;

    // Success
    return TRUE;
}

//==============================================================================

void ragdoll::Reset( void )
{
    s32 i ;

    #ifdef X_DEBUG
        // Clear draw planes
        m_NDebugPlanes = 0 ;
    #endif

    // Reset particles
    for (i = 0 ; i < m_NParticles ; i++)
        m_Particles[i].Reset() ;
}   

//==============================================================================

// Applies a positional blast
void ragdoll::ApplyBlast( const vector3& Pos, f32 Radius, f32 Amount )
{
    // No blast?
    if ( ( Radius == 0 ) || ( Amount == 0 ) )
        return ;

    ASSERT(Pos.IsValid()) ;
    ASSERT(x_isvalid(Radius)) ;
    ASSERT(Radius > 0) ;
    ASSERT(x_isvalid(Amount)) ;

    // Compute the general direction
    vector3 Dir = GetCenterPos() - Pos ;
    Dir.Normalize() ;

    // Now put vel in particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Get vector from Bomb->Particle
        vector3 Delta  = Particle.m_Pos - Pos ;
        f32     Dist   = Delta.Length() ;
        if ((Dist < Radius) && (Dist > 0.000001f))
        {
            // Compute blast amount
            f32 Ratio = (Radius - Dist) / Radius ;
            f32 Blast = Ratio * Particle.m_InvMass * Amount * RAGDOLL_TIME_STEP ;
            
            // Add 20% linear vel
            ASSERT(Dir.IsValid()) ;
            ASSERT(x_isvalid(Ratio)) ;
            ASSERT(x_isvalid(Blast)) ;
            Particle.m_LastPos -= 0.2f * Dir * Blast ;
            
            // Add 80% angular vel
            Delta.NormalizeAndScale(0.8f) ;
            ASSERT(Delta.IsValid()) ;
            ASSERT(x_isvalid(Blast)) ;
            Particle.m_LastPos -= Delta * Blast ;
        }
    }
}

//==============================================================================

// Applies a directional blast
void ragdoll::ApplyBlast( const vector3& Pos, const vector3& Dir, f32 Radius, f32 Amount )
{
    // No blast?
    if ( ( Radius == 0 ) || ( Amount == 0 ) )
        return ;

    ASSERT(Pos.IsValid()) ;
    ASSERT(Dir.IsValid()) ;
    ASSERT(x_isvalid(Radius)) ;
    ASSERT(Radius > 0) ;
    ASSERT(x_isvalid(Amount)) ;

    // Now put vel in particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Get vector from Bomb->Particle
        vector3 Delta = Particle.m_Pos - Pos ;
        f32     Dist  = Delta.Length() ;
        if ((Dist < Radius) && (Dist > 0.000001f))
        {
            f32 Blast = (Radius - Dist) / Radius ;
            ASSERT(Dir.IsValid()) ;
            ASSERT(x_isvalid(Blast)) ;
            Particle.m_LastPos -= Particle.m_InvMass * (Dir * Blast * Amount * RAGDOLL_TIME_STEP) ;
        }
    }
}

//==============================================================================

// Applies an instance force
void ragdoll::ApplyVectorForce( const vector3& Dir, f32 Amount )
{
    // No blast?
    if ( Amount == 0 )
        return ;

    ASSERT(Dir.IsValid()) ;
    ASSERT(x_isvalid(Amount)) ;

    // Now put vel in particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;
        Particle.m_LastPos -= Particle.m_InvMass * Dir * Amount * RAGDOLL_TIME_STEP ;
    }
}

//==============================================================================

void ragdoll::ZeroVelocity( void )
{
    // Now put vel in particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Clear velocity
        Particle.m_LastPos = Particle.m_Pos ;
    }
}

//==============================================================================

void ragdoll::SetVelocity( const vector3& Vel )
{
    // Take time step into account
    vector3 RagdollVel = Vel * RAGDOLL_TIME_STEP ;

    // Now put vel in particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Clear velocity
        Particle.m_LastPos = Particle.m_Pos - RagdollVel ;
    }
}

//==============================================================================

void ragdoll::AddVelocity( const vector3& Vel )
{
    // Take time step into account
    vector3 RagdollVel = Vel * RAGDOLL_TIME_STEP ;

    // Now put vel in particles
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Add to velocity
        Particle.m_LastPos -= RagdollVel ;
    }
}

//==============================================================================

void ragdoll::Kill( void )
{
}

//==============================================================================

void ragdoll::ComputeMatrices( matrix4* pMatrices, s32 nBones )
{
    CONTEXT("ragdoll::ComputeMatrices") ;

    // Lookup geom and stick bones
    ASSERT(m_pDef) ;
    geom_bone*  GeomBones  = m_pDef->m_GeomBones ;
    stick_bone* StickBones = &m_StickBones[0] ;

    // Loop through all geom_bones
    ASSERTS( nBones <= m_pDef->m_NGeomBones,
             xfs( "Ragdoll[%s][%s][%s] hasn't enough bones for request.\nCheck .skingeom and .anim for character/deadbody.",
                  m_pDef->m_pType, m_pDef->m_pSkinGeom, m_pDef->m_pAnim ) );
    for (s32 i = 0 ; i < nBones ; i++)
        GeomBones[i].GetSkinL2W(StickBones, pMatrices[i]) ;
}

//==============================================================================

void ragdoll::ComputeBoneL2W( s32 iBone, matrix4& L2W )
{
    ASSERT( (iBone>=0) && (iBone<m_pDef->m_NGeomBones) );

    ASSERT( m_pDef );
    geom_bone*  GeomBones  = m_pDef->m_GeomBones;
    stick_bone* StickBones = &m_StickBones[0];

    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    ASSERT( pAnimGroup );
    const anim_bone&  AnimBone   = pAnimGroup->GetBone(iBone);
    GeomBones[iBone].GetSkinL2W( StickBones, L2W );
    L2W.PreTranslate( AnimBone.BindTranslation );
}

//==============================================================================

s32 ragdoll::GetNGeomBones( void )
{
    ASSERT(m_pDef) ;
    return m_pDef->m_NGeomBones ;
}

//==============================================================================

// Sets matrices from animation to setup particle positions and give them velocity
void ragdoll::SetMatrices( loco_char_anim_player& AnimPlayer, const vector3& Vel )
{
    s32 i ;

    ASSERT(Vel.IsValid()) ;

    // No anim playing?
    loco_motion_controller& CurrAnim = AnimPlayer.GetCurrAnim() ;
    if ( CurrAnim.GetAnimIndex() == -1)
        return ;

    // Lookup anim info
    const anim_info& AnimInfo = CurrAnim.GetAnimInfo();
    f32 CurrFrame = CurrAnim.GetFrame() ;
    f32 NextFrame = CurrAnim.GetFrame() ;
    f32 FPS       = (f32)AnimInfo.GetFPS();

    // Move to previous or next frame in animation?
    if ( CurrAnim.IsAtEnd() )
        NextFrame -= RAGDOLL_TIME_STEP * FPS ;
    else
        NextFrame += RAGDOLL_TIME_STEP * FPS ;

    // Cap
    if ( NextFrame < 0 )
        NextFrame = 0 ;
    else
    if ( NextFrame >= CurrAnim.GetNFrames() )
        NextFrame = (f32)(CurrAnim.GetNFrames()-1) ;

    // Lookup feet position
    vector3 FeetPos = AnimPlayer.GetPosition() ;

    // Need to track root node position since we'll use it for velocity
    vector3 CurrRootPos(0,0,0) ;
    vector3 NextRootPos(0,0,0) ;

    // Lookup root pos
    if ( NextFrame > CurrFrame )
        CurrRootPos = AnimPlayer.GetBoneL2W(0).GetTranslation() ;
    else
        NextRootPos = AnimPlayer.GetBoneL2W(0).GetTranslation() ;

    // Setup position of all particles
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;

        // Setup current position in world space
        vector3 WorldPos = AnimPlayer.GetBoneL2W(Particle.m_GeomBoneIndex) * Particle.m_BindPos ; 
        ASSERT(WorldPos.IsValid()) ;
        
        // Start collision in center of npc, off the ground to stop bodies poking through stuff
        // eg. cube farm or the floor
        Particle.m_LastCollPos = WorldPos ;
        Particle.m_LastCollPos.GetX() = FeetPos.GetX() ;
        Particle.m_LastCollPos.GetZ() = FeetPos.GetZ() ;
        Particle.m_LastCollPos.GetY() += RAGDOLL_PARTICLE_RADIUS*2.0f ;

        // Playing forwards?
        if ( NextFrame > CurrFrame )
        {
            // Setup last position
            Particle.m_LastPos = WorldPos ;
        }
        else
        {
            // Playing backwards
            Particle.m_Pos = WorldPos ;
        }
    }

    // Goto next frame in the animation
    AnimPlayer.SetCurrAnimFrame( NextFrame ) ;

    // Get root pos
    if ( NextFrame > CurrFrame )
        NextRootPos = AnimPlayer.GetBoneL2W(0).GetTranslation() ;
    else
        CurrRootPos = AnimPlayer.GetBoneL2W(0).GetTranslation() ;

    // Compute root bone velocity
    vector3 TotalVel = ( NextRootPos - CurrRootPos ) + Vel ;

    // Now setup new particle positions to inherit velocity from the animation
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;
        
        // Setup current position in world space
        vector3 WorldPos = AnimPlayer.GetBoneL2W(Particle.m_GeomBoneIndex) * Particle.m_BindPos ; 
        ASSERT(WorldPos.IsValid()) ;

        // Playing forwards?
        if ( NextFrame > CurrFrame )
        {
            // Store as new pos
            Particle.m_Pos = WorldPos ;
        }
        else
        {
            // Setup last position
            Particle.m_LastPos = WorldPos ;
        }

        // Take velocity and root bone velocity into account
        Particle.m_LastPos -= TotalVel ;
    }

    // Update stick bones
    UpdateStickBones() ;

    // Apply constraints
    ApplyConstraints() ;
}

//==============================================================================

void ragdoll::SetMatrices( const matrix4* pMatrices, s32 nBones )
{
    s32 i ;
    (void)nBones;

    // Setup position of all particles
    for (i = 0 ; i < m_NParticles ; i++)
    {
        // Get particle
        particle& Particle = m_Particles[i] ;
        
        // Setup current position in world space
        ASSERT(Particle.m_GeomBoneIndex >= 0) ;
        ASSERT(Particle.m_GeomBoneIndex < nBones) ;
        vector3 WorldPos = pMatrices[Particle.m_GeomBoneIndex] * Particle.m_BindPos ; 

        // Move away from floor a tad
        WorldPos.GetY() += 10 ;

        // Store
        ASSERT(WorldPos.IsValid()) ;
        Particle.m_Pos = Particle.m_LastPos = Particle.m_LastCollPos = WorldPos ;
    }

    // Update stick bones
    UpdateStickBones() ;

    // Apply constraints
    ApplyConstraints() ;
}

//==============================================================================

#ifdef X_DEBUG

void ragdoll::RenderGeometry(u32 Flags /*= render::CLIPPED*/)
{
    // Any geometry?
    if (!m_SkinInst.GetSkinGeom())
        return ;

    // Lookup bone count
    ASSERT(m_pDef) ;
    s32 NBones = m_pDef->m_NGeomBones ;
    ASSERT(NBones) ;

    // Allocate matrices
    matrix4* pMatrices = (matrix4*)smem_BufferAlloc(NBones * sizeof(matrix4)) ;
    if (!pMatrices)
        return ;

    // Compute render matrices
    ComputeMatrices(pMatrices, NBones) ;

    // Create L2W
    matrix4 L2W ;
    L2W.Identity() ;

    // Render
    m_SkinInst.Render(&L2W, pMatrices, NBones, Flags, m_SkinInst.GetLODMask(L2W)) ;
}

//==============================================================================

vector3 ragdoll::GetGeomBonePos ( s32 BoneIndex )
{
    matrix4 L2W ;
    
    m_pDef->m_GeomBones[BoneIndex].GetSkinL2W(&m_StickBones[0], L2W) ;
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    ASSERT(pAnimGroup) ;

    vector3 Pos = L2W * pAnimGroup->GetBone(BoneIndex).BindTranslation ;
    return Pos ;
}

//==============================================================================

void ragdoll::RenderSkeleton( void )
{
    s32 i ; 

    // Lookup distance rules
    dist_rule*  DistRules  = m_pDef->m_DistRules ;
    s32         NDistRules = m_pDef->m_NDistRules ;

    // Render all particles
    for (i = 0 ; i < m_NParticles ; i++)
        m_Particles[i].Render(RAGDOLL_PARTICLE_RADIUS) ;

    // Render all dist_rules
    for (i = 0 ; i < NDistRules ; i++)
        DistRules[i].Render(m_Particles) ;

    // Render debug planes
    for (i = 0 ; i < m_NDebugPlanes ; i++)
        draw_Plane(m_DebugPlanes[i].Point, m_DebugPlanes[i].Normal, m_DebugPlanes[i].Color, 3.0f) ;

    // Render stick bones
    for (i = 0 ; i < m_StickBones.GetCount() ; i++)
        m_StickBones[i].Render() ;
/*
    // Render geom bones
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    ASSERT(pAnimGroup) ;
    for(i = 0 ; i < m_pDef->m_NGeomBones ; i++)
    {
        vector3 BP = GetGeomBonePos( i );
        vector3 PP = BP;

        s32 Parent = pAnimGroup->GetBone(i).iParent ;
        if( Parent != -1 )
            PP = GetGeomBonePos( Parent );

        draw_Line( BP, PP, XCOLOR_GREEN );
        draw_Marker( BP, XCOLOR_RED );

        draw_Label( BP, XCOLOR_WHITE, pAnimGroup->GetBone(i).Name );
    }
*/
}

//==============================================================================

void ragdoll::RenderBones( s32 Index )
{
    s32 i ; 

    // Lookup geom and stick bones
    geom_bone*  GeomBones  = m_pDef->m_GeomBones ;
    s32         NGeomBones = m_pDef->m_NGeomBones ;
    stick_bone* StickBones = &m_StickBones[0] ;

    // Just render a particular bone?
    if ( (Index != -1) && (NGeomBones) )
        Index = Index % NGeomBones ;

    // Render all geom_bones
    for (i = 0 ; i < NGeomBones ; i++)
    {
        if ((Index == -1) || (i == Index))
            GeomBones[i].Render(StickBones) ;
    }
}

//==============================================================================

void ragdoll::RenderCollision( void )
{
    // Renders collision (useful for debugging)
    VerletCollision_Render() ;
}

#endif  //#ifdef X_DEBUG

//==============================================================================

void ragdoll::Advance( f32 DeltaTime )
{
    // #TODO: !!!
   // LOG_STAT(k_stats_Ragdoll);

    CONTEXT("ragdoll::Advance") ;

    vector3 Center(0,0,0);

    // Use world collision?
    if (m_ObjectGuid)
    {
        s32 i;

        // Compute center of body
        for(i = 0 ; i < m_NParticles ; i++)
            Center += m_Particles[i].m_Pos;
        Center /= (f32)m_NParticles;

        // Cap velocities
        for (i = 0 ; i < m_NParticles ; i++)
        {
            particle& Particle = m_Particles[i] ;
            
            // If particle is too far away from center pull it back in
            vector3 CenterToParticle = Particle.m_Pos - Center;
            if( CenterToParticle.LengthSquared() > x_sqr(800.0f) )
            {
                CenterToParticle.NormalizeAndScale(800.0f);
                Particle.m_Pos = Center + CenterToParticle;
                Particle.m_LastPos = Particle.m_Pos;
                Particle.m_LastCollPos = Particle.m_Pos;
            }

            // Get velocity and cap if too big
            vector3 Vel      = Particle.GetVelocity() ;
            f32     SpeedSqr = Vel.LengthSquared() ;
            if (SpeedSqr > x_sqr(RAGDOLL_MAX_SPEED))
            {
                Vel.NormalizeAndScale(RAGDOLL_MAX_SPEED) ;
                Particle.SetVelocity(Vel) ;
            }
        }
    }

    // Add to total time
    m_DeltaTime += DeltaTime ;

    // Cap iterations
    s32 nMaxIterations = RAGDOLL_MAX_ITERATIONS ;

    // Apply delta time pool
    while((nMaxIterations--) && (m_DeltaTime >= RAGDOLL_TIME_STEP))
    {
        // Integrate
        Integrate(RAGDOLL_TIME_STEP) ;

        // Iterate to solve constraints
        ApplyConstraints() ;

        // Next
        m_DeltaTime -= RAGDOLL_TIME_STEP ;
    }

    // Get ready for rendering
    UpdateStickBones() ;
}

//==============================================================================
// Collision functions
//==============================================================================


//==============================================================================

void ragdoll::OnColCheck( void )
{
    // Apply all joint spheres
    g_CollisionMgr.StartApply( m_ObjectGuid ) ;
    for (s32 i = 0 ; i < m_NParticles ; i++)
        g_CollisionMgr.ApplySphere(m_Particles[i].m_Pos, RAGDOLL_PARTICLE_RADIUS, object::MAT_TYPE_FLESH, i) ;
    g_CollisionMgr.EndApply();    
}

//==============================================================================

void ragdoll::OnColNotify( object& Object, const collision_mgr::collision& Collision )
{
    // Did this come from a projectile?
    if (Object.IsKindOf( base_projectile::GetRTTI() ) )
    {
        // Lookup projectile
        base_projectile& Bullet = base_projectile::GetSafeType(Object) ;

        // Lookup particle that got hit
        ASSERT(Collision.PrimitiveKey >= 0) ;
        ASSERT(Collision.PrimitiveKey < m_NParticles) ;
        particle& Particle = m_Particles[Collision.PrimitiveKey] ;

        // Add to the velocity
        Particle.m_LastPos -= Bullet.GetVelocity() ;
    }
}

//==============================================================================
// Query functions
//==============================================================================

vector3 ragdoll::GetFeetPos( void ) const
{
    vector3 P = (m_Joints.m_Side[0].m_pFoot->m_Pos + m_Joints.m_Side[1].m_pFoot->m_Pos) * 0.5f ;
    P.GetY() -= RAGDOLL_PARTICLE_RADIUS ;
    return P ;
}

//==============================================================================

vector3 ragdoll::GetHipPos( void ) const
{
    return (m_Joints.m_Side[0].m_pTorso->m_Pos + m_Joints.m_Side[1].m_pTorso->m_Pos) * 0.5f  ;
}

//==============================================================================

vector3 ragdoll::GetCenterPos( void ) const
{
    vector3 C(0,0,0) ;
    for (s32 i = 0 ; i < m_NParticles ; i++)
        C += m_Particles[i].m_Pos ;
    C /= (f32)m_NParticles ;
    return C ;
}

//==============================================================================

xbool ragdoll::IsInitialized( void ) const
{
    return (m_hAnimGroup.GetPointer()) && (m_SkinInst.GetSkinGeom()) ;
}

//==============================================================================

f32 ragdoll::GetKineticEnergy( void ) const
{
    f32 E = 0 ;
    for (s32 i = 0 ; i < m_NParticles ; i++)
    {
        E += 0.5f * m_Particles[i].m_Mass * m_Particles[i].GetVelocity().LengthSquared() ;
    }

    return E ;
}


//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

#ifdef X_DEBUG

// Draws a plane
void draw_Plane( const vector3& MidPt, const vector3& Normal, xcolor Color, f32 Size /*= 40 */)
{
    vector3 Temp( Normal );
    Temp.Normalize() ;

    vector3 Out = vector3(0,1,0).Cross(Temp) ;
    Out.Normalize() ;
    vector3 Side  = Out.Cross(Temp) ;
    Side.Normalize() ;

    vector3 Corner = MidPt - (Side*Size) - (Out*Size) ;
    vector3 Edge1  = 2*Side*Size ;
    vector3 Edge2  = 2*Out*Size ;
    draw_Grid(Corner, Edge1, Edge2, Color, 4) ;

    draw_Begin(DRAW_LINES, DRAW_NO_ZBUFFER) ;
    draw_Color(Color) ;
    draw_Vertex(MidPt) ;
    draw_Vertex(MidPt + (10 * Temp)) ;
    draw_End() ;
}

#endif

//==============================================================================

void RagdollType_OnProperty ( prop_query& I, ragdoll::type& RagdollType )
{
    // Updating UI?
    if( I.IsRead() )
    {
        if ( ragdoll::s_TypeList.DoesValueExist(RagdollType) )
        {
            I.SetVarEnum( ragdoll::s_TypeList.GetString(RagdollType) );
        }
        else
        {
            I.SetVarEnum( "INVALID" );
        } 
    }
    else
    {
        // Reading from UI/File
        const char* pValue = I.GetVarEnum();

        // Found?
        if( ragdoll::s_TypeList.GetValue( pValue, RagdollType ) )
        {
            return;
        }
        else if ( !x_strcmp(pValue,"HAZMAT") )      // Old version support
        {
            RagdollType = ragdoll::TYPE_MILITARY;
        }
        else if ( !x_strcmp(pValue,"SCIENTIST") )   // Old version support
        {
            RagdollType = ragdoll::TYPE_CIVILIAN;
        }
        else if ( !x_strcmp(pValue,"HUMAN") )       // Old version support
        {
            RagdollType = ragdoll::TYPE_CIVILIAN;
        }
    }
}

//==============================================================================

