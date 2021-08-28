//=========================================================================
//
//  PlayerLoco.cpp
//
//=========================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "PlayerLoco.hpp"
#include "Objects\Actor\Actor.hpp"


//==============================================================================
// DATA
//==============================================================================

static struct player_loco_tweaks
{
    // Data
    f32 m_WalkRunMinSpeed;                
    f32 m_WalkRunMaxSpeed;                
    f32 m_WalkRunMinRate;
    f32 m_WalkRunMaxRate;

    f32 m_CrouchMinRate;
    f32 m_CrouchMaxRate;

    // Functions
    player_loco_tweaks()
    {
        m_WalkRunMinSpeed = 20.0f;
        m_WalkRunMaxSpeed = 500.0f;
        m_WalkRunMinRate  = 0.10f;
        m_WalkRunMaxRate  = 1.25f;

        m_CrouchMinRate   = 0.10f;
        m_CrouchMaxRate   = 2.00f;
    }
} g_PlayerLocoTweaks;

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================
//==============================================================================
//==============================================================================
// LOCO STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

player_loco_play_anim::player_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void player_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter() ;
}

//=========================================================================
// IDLE
//=========================================================================

player_loco_idle::player_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void player_loco_idle::OnEnter( void )
{
    loco_idle::OnEnter() ;
}

//=========================================================================
// MOVE
//=========================================================================

player_loco_move::player_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void player_loco_move::OnEnter( void )
{
    loco_move::OnEnter() ;
}

//==============================================================================
//==============================================================================
//==============================================================================
// PLAYER LOCO
//==============================================================================
//==============================================================================
//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
#pragma warning( push )
#pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

player_loco::player_loco( void ) :
loco(),
m_PlayAnim( *this ),
m_Idle    ( *this ),
m_Move    ( *this )
{
    x_memset( &m_MPAnimIndex[0], -1, sizeof( m_MPAnimIndex ) );
    m_CurrentWeaponAnims = INVEN_NULL;
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
#pragma warning( pop )
#endif

//==============================================================================

player_loco::mp_weapon player_loco::GetMPWeapon( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!
    switch( InvenWeapon )
    {
    default:
    case INVEN_NULL:                return MP_WEAPON_PST; // TO DO: Have a set of anims for no weapon?    
    case INVEN_WEAPON_SMP:          return MP_WEAPON_RFL;
    case INVEN_WEAPON_SHOTGUN:      return MP_WEAPON_RFL;
    case INVEN_WEAPON_SNIPER_RIFLE: return MP_WEAPON_RFL;
    case INVEN_WEAPON_DESERT_EAGLE: return MP_WEAPON_PST;
    case INVEN_WEAPON_MESON_CANNON: return MP_WEAPON_MSN;
    case INVEN_WEAPON_BBG:          return MP_WEAPON_RFL;
    case INVEN_WEAPON_DUAL_SMP:     return MP_WEAPON_DUAL;
    case INVEN_WEAPON_DUAL_SHT:     return MP_WEAPON_DUAL;
    case INVEN_WEAPON_MUTATION:     return MP_WEAPON_MUT;
    }
}

//==============================================================================

loco::anim_type player_loco::GetMPReloadAnimType( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    switch( InvenWeapon )
    {
    default:
        ASSERTS( 0, "Weapon not supported in mp!" );
    case INVEN_NULL:                return loco::ANIM_NULL;
    case INVEN_WEAPON_SMP:          return loco::ANIM_RELOAD_SMP;
    case INVEN_WEAPON_SHOTGUN:      return loco::ANIM_RELOAD_SHOTGUN;
    case INVEN_WEAPON_SNIPER_RIFLE: return loco::ANIM_RELOAD_SNIPER;
    case INVEN_WEAPON_DESERT_EAGLE: return loco::ANIM_RELOAD_DESERT_EAGLE;
    case INVEN_WEAPON_MESON_CANNON: return loco::ANIM_RELOAD_MSN;
    case INVEN_WEAPON_BBG:          return loco::ANIM_RELOAD_BBG;
    case INVEN_WEAPON_DUAL_SMP:     return loco::ANIM_RELOAD_DUAL_SMP;
    case INVEN_WEAPON_DUAL_SHT:     return loco::ANIM_RELOAD_DUAL_SHT;
    case INVEN_WEAPON_MUTATION:     return loco::ANIM_RELOAD;
    }
}

//==============================================================================

const char* player_loco::GetMPReloadAnimName( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    switch( InvenWeapon )
    {
    default:
    case INVEN_NULL:                return "PST_EGL_RELOAD";
    case INVEN_WEAPON_SMP:          return "RFL_SMP_RELOAD";
    case INVEN_WEAPON_SHOTGUN:      return "RFL_SHT_RELOAD";
    case INVEN_WEAPON_SNIPER_RIFLE: return "RFL_SNI_RELOAD";
    case INVEN_WEAPON_DESERT_EAGLE: return "PST_EGL_RELOAD";
    case INVEN_WEAPON_MESON_CANNON: return "PST_EGL_RELOAD"; // *Not needed so just use eagle*
    case INVEN_WEAPON_BBG:          return "PST_EGL_RELOAD"; // *Not needed so just use eagle*
    case INVEN_WEAPON_DUAL_SMP:     return "PST_EGL_RELOAD"; // *Not needed so just use eagle*
    case INVEN_WEAPON_DUAL_SHT:     return "PST_EGL_RELOAD"; // *Not needed so just use eagle*
    case INVEN_WEAPON_MUTATION:     return "PST_EGL_RELOAD"; // *Not needed so just use eagle*
    }
}

//==============================================================================

loco::anim_type player_loco::GetMPShootPrimaryAnimType( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    switch( InvenWeapon )
    {
    default:
        ASSERTS( 0, "Weapon not supported in mp!" );
    case INVEN_NULL:                return loco::ANIM_NULL;
    case INVEN_WEAPON_SMP:          return loco::ANIM_SHOOT_SMP;
    case INVEN_WEAPON_SHOTGUN:      return loco::ANIM_SHOOT_SHOTGUN;
    case INVEN_WEAPON_SNIPER_RIFLE: return loco::ANIM_SHOOT_SNIPER;
    case INVEN_WEAPON_DESERT_EAGLE: return loco::ANIM_SHOOT_DESERT_EAGLE;
    case INVEN_WEAPON_MESON_CANNON: return loco::ANIM_SHOOT_MSN;
    case INVEN_WEAPON_BBG:          return loco::ANIM_SHOOT_BBG;
    case INVEN_WEAPON_DUAL_SMP:     return loco::ANIM_SHOOT_DUAL_SMP;
    case INVEN_WEAPON_DUAL_SHT:     return loco::ANIM_SHOOT_DUAL_SHT;
    case INVEN_WEAPON_MUTATION:     return loco::ANIM_SHOOT_MUTANT;
    }
}

//==============================================================================

const char* player_loco::GetMPShootPrimaryAnimName( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    switch( InvenWeapon )
    {
    default:
        ASSERTS( 0, "Weapon not supported in mp!" );
    case INVEN_NULL:                return "PST_EGL_SHOOT";
    case INVEN_WEAPON_SMP:          return "RFL_SMP_SHOOT";
    case INVEN_WEAPON_SHOTGUN:      return "RFL_SHT_SHOOT";
    case INVEN_WEAPON_SNIPER_RIFLE: return "RFL_SNI_SHOOT";
    case INVEN_WEAPON_DESERT_EAGLE: return "PST_EGL_SHOOT";
    case INVEN_WEAPON_MESON_CANNON: return "MSN_SHOOT";
    case INVEN_WEAPON_BBG:          return "RFL_BBG_SHOOT";
    case INVEN_WEAPON_DUAL_SMP:     return "DUL_2MP_SHOOT";
    case INVEN_WEAPON_DUAL_SHT:     return "DUL_2SH_SHOOT";
    case INVEN_WEAPON_MUTATION:     return "MUT_SHOOT";
    }
}

//==============================================================================

loco::anim_type player_loco::GetMPShootSecondaryAnimType( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    switch( InvenWeapon )
    {
    default:
        ASSERTS( 0, "Weapon not supported in mp!" );
    case INVEN_NULL:                return loco::ANIM_NULL;
    case INVEN_WEAPON_SMP:          return loco::ANIM_SHOOT_SECONDARY_SMP;
    case INVEN_WEAPON_SHOTGUN:      return loco::ANIM_SHOOT_SECONDARY_SHOTGUN;
    case INVEN_WEAPON_SNIPER_RIFLE: return loco::ANIM_SHOOT_SECONDARY_SNIPER;
    case INVEN_WEAPON_DESERT_EAGLE: return loco::ANIM_SHOOT_SECONDARY_DESERT_EAGLE;
    case INVEN_WEAPON_MESON_CANNON: return loco::ANIM_SHOOT_SECONDARY_MSN;
    case INVEN_WEAPON_BBG:          return loco::ANIM_SHOOT_BBG;
    case INVEN_WEAPON_DUAL_SMP:     return loco::ANIM_SHOOT_DUAL_SMP;
    case INVEN_WEAPON_DUAL_SHT:     return loco::ANIM_SHOOT_DUAL_SHT_SECONDARY;
    case INVEN_WEAPON_MUTATION:     return loco::ANIM_SHOOT_SECONDARY_MUTANT;
    }
}

//==============================================================================

const char* player_loco::GetMPShootSecondaryAnimName( inven_item InvenWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    switch( InvenWeapon )
    {
    default:
        ASSERTS( 0, "Weapon not supported in mp!" );
    case INVEN_NULL:                return "PST_EGL_SHOOT";
    case INVEN_WEAPON_SMP:          return "RFL_SMP_SHOOT";
    case INVEN_WEAPON_SHOTGUN:      return "RFL_SHT_ALT_SHOOT";
    case INVEN_WEAPON_SNIPER_RIFLE: return "RFL_SNI_SHOOT";
    case INVEN_WEAPON_DESERT_EAGLE: return "PST_EGL_SHOOT";
    case INVEN_WEAPON_MESON_CANNON: return "MSN_SHOOT";
    case INVEN_WEAPON_BBG:          return "RFL_BBG_SHOOT";
    case INVEN_WEAPON_DUAL_SMP:     return "DUL_2MP_SHOOT";
    case INVEN_WEAPON_DUAL_SHT:     return "DUL_2SH_ALT_SHOOT";
    case INVEN_WEAPON_MUTATION:     return "MUT_SHOOT";
    }
}

//==============================================================================

const char* player_loco::GetMPWeaponName( player_loco::mp_weapon MPWeapon )
{
    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it has a set of multi-player anims!!

    switch( MPWeapon )
    {
    default:
        ASSERTS( 0, "You need to add new mp weapon type here!" );
    case MP_WEAPON_RFL:     return "RFL";   // Covers smp: sniper: shotgun, bbg
    case MP_WEAPON_PST:     return "PST";   // Covers eagle
    case MP_WEAPON_MSN:     return "MSN";   // Covers mason cannon
    case MP_WEAPON_DUAL:    return "DUL";   // Covers dual smp: dual eagle, dual sht
    case MP_WEAPON_MUT:     return "MUT";   // Covers mutant
    }
}

//==============================================================================

const char* player_loco::GetMPAnimName( player_loco::mp_anim MPAnim )
{
    switch( MPAnim )
    {
    default:
        ASSERTS( 0, "You need to add new mp anim type here!" );
    case MP_ANIM_WALK_IDLE:                     return "RUN_IDLE";
    case MP_ANIM_WALK_IDLE_TURN_LEFT:           return "RUN_IDLE_TURN_LEFT";
    case MP_ANIM_WALK_IDLE_TURN_RIGHT:          return "RUN_IDLE_TURN_RIGHT";
    //case MP_ANIM_WALK_MOVE_FRONT:               return "WALK_MOVE_FRONT";
    //case MP_ANIM_WALK_MOVE_LEFT:                return "WALK_MOVE_LEFT";
    //case MP_ANIM_WALK_MOVE_BACK:                return "WALK_MOVE_BACK";
    //case MP_ANIM_WALK_MOVE_RIGHT:               return "WALK_MOVE_RIGHT";
    case MP_ANIM_WALK_MOVE_FRONT:               return "RUN_MOVE_FRONT";
    case MP_ANIM_WALK_MOVE_LEFT:                return "RUN_MOVE_LEFT";
    case MP_ANIM_WALK_MOVE_BACK:                return "RUN_MOVE_BACK";
    case MP_ANIM_WALK_MOVE_RIGHT:               return "RUN_MOVE_RIGHT";

    case MP_ANIM_RUN_IDLE:                      return "RUN_IDLE";
    case MP_ANIM_RUN_IDLE_TURN_LEFT:            return "RUN_IDLE_TURN_LEFT";
    case MP_ANIM_RUN_IDLE_TURN_RIGHT:           return "RUN_IDLE_TURN_RIGHT";
    case MP_ANIM_RUN_MOVE_FRONT:                return "RUN_MOVE_FRONT";
    case MP_ANIM_RUN_MOVE_LEFT:                 return "RUN_MOVE_LEFT";
    case MP_ANIM_RUN_MOVE_BACK:                 return "RUN_MOVE_BACK";
    case MP_ANIM_RUN_MOVE_RIGHT:                return "RUN_MOVE_RIGHT";

    case MP_ANIM_CROUCH_IDLE:                   return "CROUCH_IDLE";
    case MP_ANIM_CROUCH_IDLE_TURN_LEFT:         return "CROUCH_IDLE_TURN_LEFT";
    case MP_ANIM_CROUCH_IDLE_TURN_RIGHT:        return "CROUCH_IDLE_TURN_RIGHT";
    case MP_ANIM_CROUCH_MOVE_FRONT:             return "CROUCH_MOVE_FRONT";
    case MP_ANIM_CROUCH_MOVE_LEFT:              return "CROUCH_MOVE_LEFT";
    case MP_ANIM_CROUCH_MOVE_BACK:              return "CROUCH_MOVE_BACK";
    case MP_ANIM_CROUCH_MOVE_RIGHT:             return "CROUCH_MOVE_RIGHT";

    case MP_ANIM_JUMP:                          return "JUMP";
    case MP_ANIM_GRENADE:                       return "GRENADE";      
    case MP_ANIM_MELEE:                         return "MELEE";        
    
    case MP_ANIM_STAND_LEAN_LEFT:               return "LEAN_LEFT_IN";
    case MP_ANIM_STAND_LEAN_RIGHT:              return "LEAN_RIGHT_IN";
    case MP_ANIM_CROUCH_LEAN_LEFT:              return "CROUCH_LEAN_LEFT_IN";
    case MP_ANIM_CROUCH_LEAN_RIGHT:             return "CROUCH_LEAN_RIGHT_IN";
    }
}

//==============================================================================

void player_loco::InitAnimIndices( void )
{
    // Setup mp animation indices
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return;

    // Setup for all weapons
    for( s32 iWeapon = 0; iWeapon < MP_WEAPON_COUNT; iWeapon++ )
    {
        // Loop over all anims
        const char* pWeapon = GetMPWeaponName( (mp_weapon)iWeapon );
        for( s32 iAnim = 0; iAnim < MP_ANIM_COUNT; iAnim++ )
        {
            // These animations are not present
            if( ( iWeapon == MP_WEAPON_MUT ) && ( iAnim == MP_ANIM_GRENADE ) )
                continue;
                
            // Search for full weapon anim
            const char* pAnim = GetMPAnimName( (mp_anim)iAnim );
            s32 AnimIndex = pAnimGroup->GetAnimIndex( xfs( "%s_%s", pWeapon, pAnim ) );

            // Must be present for ghosts!
            if( m_bGhostMode )
            {
                ASSERTS( AnimIndex != -1, xfs(" %s is missing %s_%s see Aaron", m_hAnimGroup.GetName(), pWeapon, pAnim ) ); 
            }
            
            // Add to table
            m_MPAnimIndex[ iAnim + ( iWeapon * MP_ANIM_COUNT ) ] = AnimIndex;
        }
    }
    
    // Weapons that have mp shoot anims
    static inven_item MPWeapons[] =
    {
        INVEN_WEAPON_SMP,
        INVEN_WEAPON_BBG,
        INVEN_WEAPON_SHOTGUN,
        INVEN_WEAPON_SNIPER_RIFLE,
        INVEN_WEAPON_DESERT_EAGLE,
        INVEN_WEAPON_MESON_CANNON,
        INVEN_WEAPON_DUAL_SMP,
        INVEN_WEAPON_DUAL_SHT,
        INVEN_WEAPON_MUTATION,
    };
    
    // Setup all shoot anims
    for( u32 iShoot = 0; iShoot < ( sizeof( MPWeapons ) / sizeof( MPWeapons[0] ) ); iShoot++ )
    {
        // Lookup info about weapon
        inven_item  InvenWeapon = MPWeapons[iShoot];
        
        // Lookup reload anim info
        anim_type   ReloadAnimType    = GetMPReloadAnimType( InvenWeapon );
        const char* pReloadAnimName   = GetMPReloadAnimName( InvenWeapon );
        s32         ReloadAnimIndex   = pAnimGroup->GetAnimIndex( pReloadAnimName );

        // Lookup primary shoot anim info
        anim_type   PrimaryAnimType    = GetMPShootPrimaryAnimType( InvenWeapon );
        const char* pPrimaryAnimName   = GetMPShootPrimaryAnimName( InvenWeapon );
        s32         PrimaryAnimIndex   = pAnimGroup->GetAnimIndex( pPrimaryAnimName );

        // Lookup secondary shoot anim info
        anim_type   SecondaryAnimType  = GetMPShootSecondaryAnimType( InvenWeapon );
        const char* pSecondaryAnimName = GetMPShootSecondaryAnimName( InvenWeapon );
        s32         SecondaryAnimIndex = pAnimGroup->GetAnimIndex( pSecondaryAnimName );

#ifdef X_DEBUG
        // Must be present for ghosts!
        if( m_bGhostMode )
        {
            ASSERTS( ReloadAnimIndex    != -1, xfs( "%s is missing from %s see Aaron", pReloadAnimName,    m_hAnimGroup.GetName() ) ); 
            ASSERTS( PrimaryAnimIndex   != -1, xfs( "%s is missing from %s see Aaron", pPrimaryAnimName,   m_hAnimGroup.GetName() ) ); 
            ASSERTS( SecondaryAnimIndex != -1, xfs( "%s is missing from %s see Aaron", pSecondaryAnimName, m_hAnimGroup.GetName() ) ); 
        }
#endif
        
        // Add to anim table
        m_AnimLookupTable.m_Index[ ReloadAnimType    ] = ReloadAnimIndex;
        m_AnimLookupTable.m_Index[ PrimaryAnimType   ] = PrimaryAnimIndex;
        m_AnimLookupTable.m_Index[ SecondaryAnimType ] = SecondaryAnimIndex;
    }
}

//==============================================================================

struct mapping_info
{
    const char* m_pBone;        // Bone that mapping controls
    f32         m_PivotX;       // Pivot XYZ
    f32         m_PivotY;
    f32         m_PivotZ;
};

//==============================================================================

struct constraint_info
{
    const char* m_pBone0;       // Bone0
    const char* m_pBone1;       // Bone1
    f32         m_MassRatio0;   // Mass translation ratio0
    f32         m_MassRatio1;   // Mass translation ratio0
    f32         m_Inertia0;     // Rotation ratio0
    f32         m_Inertia1;     // Rotation ratio1
    const char* m_pConBone0;    // Constraint point0
    const char* m_pConBone1;    // Constraint point1
    f32         m_MinDist;      // Min dist to keep points
    f32         m_MaxDist;      // Max dist to keep points
};

//==============================================================================

static
matrix4 ComputeBoneL2W( const anim_group*                   pAnimGroup, 
                        const anim_info&                    AnimInfo, 
                              s32                           iBone,
                        const loco_ik_solver::bone_mapping* pMappings = NULL,
                              s32                           nMappings = 0 )
{
    // Clear
    matrix4 L2W;
    L2W.Identity();
    
    // Trace up through all parents
    s32 iCurrBone = iBone;
    while( iCurrBone != -1 )
    {
        // Lookup local L2W
        matrix4  BoneL2W;
        anim_key Key;
        AnimInfo.GetRawKey( 0, iCurrBone, Key );
        Key.Setup( BoneL2W );
        
        // Concatenate by parent
        L2W = BoneL2W * L2W;
        
        // Goto parent
        iCurrBone = pAnimGroup->GetBoneParent( iCurrBone );
    }

    // Apply inverse?
    if( pMappings )
    {
        L2W = L2W * pAnimGroup->GetBoneBindInvMatrix( iBone );
    }

    // Take bone mappings into account?
    for( s32 i = 0; i < nMappings; i++ )
    {
        // Found mapping?
        if( pMappings[i].m_iBone == iBone )
        {
            L2W = L2W * pMappings[i].m_B2S;
            break;
        }
    }

    return L2W;
}

//==============================================================================

static
void SetupMapping( const anim_group*                    pAnimGroup,
                   const mapping_info&                  Info,
                         loco_ik_solver::bone_mapping&  Mapping )
{
    // Lookup bone/rigid body indices
    s32 iBone      = pAnimGroup->GetBoneIndex( Info.m_pBone, TRUE );
    ASSERT( iBone != -1 );

    // Compute bind space pos of bone
    matrix4 BodyL2W;
    BodyL2W.Identity();
    BodyL2W.SetTranslation( vector3( Info.m_PivotX, Info.m_PivotY, Info.m_PivotZ ) );
    
    // Setup mapping
    Mapping.m_iBone = iBone;
    Mapping.m_B2S   = BodyL2W;
    Mapping.m_S2B   = m4_InvertRT( BodyL2W );
}

//==============================================================================

static
void SetupConstraint( const anim_group*                     pAnimGroup,
                      const anim_info&                      AnimInfo,
                      const loco_ik_solver::bone_mapping*   pMappings,
                            s32                             nMappings,
                      const constraint_info&                Info,
                            loco_ik_solver::constraint&     Constraint )
{
    // Lookup bone indices
    s32 iBone0    = pAnimGroup->GetBoneIndex( Info.m_pBone0,    TRUE );
    s32 iBone1    = pAnimGroup->GetBoneIndex( Info.m_pBone1,    TRUE );
    s32 iConBone0 = pAnimGroup->GetBoneIndex( Info.m_pConBone0, TRUE );
    s32 iConBone1 = pAnimGroup->GetBoneIndex( Info.m_pConBone1, TRUE );
    ASSERT( iBone0    != -1 );
    ASSERT( iBone1    != -1 );
    ASSERT( iConBone0 != -1 );

    // Compute bone local->world matrices
    matrix4 L2W0 = ComputeBoneL2W( pAnimGroup, AnimInfo, iBone0, pMappings, nMappings );
    matrix4 L2W1 = ComputeBoneL2W( pAnimGroup, AnimInfo, iBone1, pMappings, nMappings );

    // Compute bone world->local matrices
    matrix4 W2L0 = m4_InvertRT( L2W0 );
    matrix4 W2L1 = m4_InvertRT( L2W1 );

    // Compute constraint position in world space
    vector3 ConPos0 = ComputeBoneL2W( pAnimGroup, 
                                      AnimInfo, 
                                      iConBone0 ).GetTranslation();
    
    // 2 constraint points specified?
    vector3 ConPos1;
    if( iConBone1 != -1 )
    {
        // Compute bone1 world pos
        ConPos1 = ComputeBoneL2W( pAnimGroup, 
                                  AnimInfo, 
                                  iConBone1 ).GetTranslation();
    }
    else
    {
        // Use same pos
        ConPos1 = ConPos0;
    }

    // Setup constraint
    Constraint.m_iBone0     = iBone0;
    Constraint.m_iBone1     = iBone1;
    Constraint.m_LocalPos0  = W2L0 * ConPos0;
    Constraint.m_LocalPos1  = W2L1 * ConPos1;
    Constraint.m_MinDist    = Info.m_MinDist;
    Constraint.m_MaxDist    = Info.m_MaxDist;
    Constraint.m_MassRatio0 = Info.m_MassRatio0;
    Constraint.m_MassRatio1 = Info.m_MassRatio1;
    Constraint.m_Inertia0   = Info.m_Inertia0;
    Constraint.m_Inertia1   = Info.m_Inertia1;
}

//==============================================================================

// List of bone mappings
static 
mapping_info s_MapInfo[] =
{
    //  pBone           PivotXYZ
    {   "L_UpperArm",   -25.0f, 147.0f, 1.5f },
    {   "L_ForeArm",    -35.0f, 111.0f, 1.6f },
    {   "Arm_L_Hand",   -37.0f, 94.0f,  0.8f  }
};

//==============================================================================

#define MASS( __x__ )    ( __x__ / 1.0f )
#define INERTIA( __x__ ) ( __x__ / 250.0f )

// List of constraints
static
constraint_info s_ConInfo[] = 
{

    // 0: Keeps fingers on weapon
    {   
        "Arm_R_Hand",       // pBone0       
        "Arm_L_Hand",       // pBone1       
        MASS( 0.0f ),       // MassRatio0
        MASS( 1.0f ),       // MassRatio1
        INERTIA( 0.0f ),    // Inertia0
        INERTIA( 1.0f ),    // Inertia1
        "Attach_L",         // pConBone0
        "NULL",             // pConBone1
        0.0f,               // MinDist
        0.0f                // MaxDist
    },

    // 1: Keep hand/forearm on wrist
    {   
        "Arm_L_Hand",       // pBone0
        "L_ForeArm",        // pBone1      
        MASS( 0.5f ),       // MassRatio0
        MASS( 0.5f ),       // MassRatio1
        INERTIA( 1.0f ),    // Inertia0
        INERTIA( 1.0f ),    // Inertia1
        "Arm_L_Hand",       // pConBone0
        "NULL",             // pConBone1
        0.0f,               // MinDist
        0.0f                // MaxDist
    },

    // 2: Keep forearm/upper arm on elbow
    {   
        "L_ForeArm",        // pBone0
        "L_UpperArm",       // pBone1      
        MASS( 0.5f ),       // MassRatio0
        MASS( 0.5f ),       // MassRatio1
        INERTIA( 1.5f ),    // Inertia0
        INERTIA( 1.0f ),    // Inertia1
        "L_ForeArm",        // pConBone0
        "NULL",             // pConBone1
        0.0f,               // MinDist
        0.0f                // MaxDist
    },

    // 3: Keep upper arm on shoulder
    {   
        "L_UpperArm",       // pBone0
        "Spine02",          // pBone1      
        MASS( 1.0f ),       // MassRatio0
        MASS( 0.0f ),       // MassRatio1
        INERTIA( 10.0f ),   // Inertia0
        INERTIA( 0.0f ),    // Inertia1
        "L_UpperArm",       // pConBone0
        "NULL",             // pConBone1
        0.0f,               // MinDist
        0.0f                // MaxDist
    },
};

//==============================================================================

void player_loco::InitIK( void )
{
    s32 i;

    // Lookup anim group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return;

    // Lookup idle anim
    s32 iIdleAnim = GetAnimIndex( loco::ANIM_WALK_IDLE );
    if( iIdleAnim == -1 )
        return;

    // Look up anim info
    const anim_info& IdleAnimInfo = pAnimGroup->GetAnimInfo( iIdleAnim );        
    
    // Setup bone mappings
    for( i = 0; i < 3; i++ )
    {
        // Setup
        SetupMapping( pAnimGroup,               // pAnimGroup
                      s_MapInfo[i],             // MappingInfo
                      m_IKBoneMappings[i] );    // BoneMapping
    }
    
    // Setup constraints
    for( i = 0; i < 4; i++ )
    {
        // Setup
        SetupConstraint( pAnimGroup,            // pAnimGroup
                         IdleAnimInfo,          // AnimInfo
                         &m_IKBoneMappings[0],  // pMappings
                         3,                     // nMappings
                         s_ConInfo[i],          // ConstraintInfo
                         m_IKConstraints[i] );  // Constraint
    }

    // Setup the IK solver
    m_IKSolver.Init( &m_IKBoneMappings[0], 3,   // pMappings,    nMappings
                     &m_IKConstraints[0],  4,   // pConstraints, nConstraints
                     4 );                       // nIterations
    
    // Finally, add to the animation player
    m_Player.SetIKSolver( &m_IKSolver );
}

//==============================================================================

void player_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid ) ;

    // Setup mp animation indices
    InitAnimIndices();

    // Setup default weapon so that move style anims are present
    //SetWeapon( INVEN_WEAPON_SMP );
    //SetWeapon( INVEN_WEAPON_SHOTGUN );
    //SetWeapon( INVEN_WEAPON_SNIPER_RIFLE );
    SetWeapon( INVEN_WEAPON_DESERT_EAGLE );
    //SetWeapon( INVEN_WEAPON_MESON_CANNON );
    //SetWeapon( INVEN_WEAPON_BBG );
    //SetWeapon( INVEN_WEAPON_DUAL_SMP );
    //SetWeapon( INVEN_WEAPON_DUAL_SHT );
    //SetWeapon( INVEN_WEAPON_MUTATION );

    // Setup move style
    SetMoveStyle( MOVE_STYLE_WALK );
}

//==============================================================================

xbool player_loco::SetWeapon( inven_item InvenWeapon )
{
    // Already using anims for this weapon?
    if( m_CurrentWeaponAnims == InvenWeapon )
        return FALSE;

    // Going into mutant?
    if( InvenWeapon == INVEN_WEAPON_MUTATION )
    {
        // Lookup owner actor
        guid   ActorGuid = m_Physics.GetGuid();
        actor* pActor    = (actor*)g_ObjMgr.GetObjectByGuid( ActorGuid );
        
        // Wait until mutant vmesh is displayed before swapping the anims
        if( ( pActor ) && ( pActor->GetAvatarMutationState() != actor::AVATAR_MUTANT ) )
            return FALSE;
    }

    // Coming from mutant?
    if( m_CurrentWeaponAnims == INVEN_WEAPON_MUTATION )
    {    
        // Lookup owner actor
        guid   ActorGuid = m_Physics.GetGuid();
        actor* pActor    = (actor*)g_ObjMgr.GetObjectByGuid( ActorGuid );
    
        // Wait until normal vmesh is displayed before swapping the anims
        if( ( pActor ) && ( pActor->GetAvatarMutationState() != actor::AVATAR_NORMAL ) )
            return FALSE;
    }
    
    // Map inventory weapon to loco weapon
    mp_weapon Weapon = GetMPWeapon( InvenWeapon );

    // Must be valid weapon
    ASSERT( Weapon >= 0 );
    ASSERT( Weapon < MP_WEAPON_COUNT );
    
    // Lookup loco and weapon anim indices
    s16* LocoAnims   = &m_AnimLookupTable.m_Index[ 0 ];
    s16* WeaponAnims = &m_MPAnimIndex[ Weapon * MP_ANIM_COUNT ];

    // If anims are not present, default to rifle anims (which should always be present)
    if( WeaponAnims[ MP_ANIM_WALK_IDLE ] == -1 )
        WeaponAnims = &m_MPAnimIndex[ MP_WEAPON_RFL * MP_ANIM_COUNT ];
    
    // Skip if no anims present
    if( WeaponAnims[ MP_ANIM_WALK_IDLE ] == -1 )
        return FALSE;
        
    // Lookup current anim state before overwriting the main loco anim indices
    loco::move_style        MoveStyle     = GetMoveStyle();
    loco_motion_controller& Cont          = m_Player.GetCurrAnim();
    f32                     Frame         = Cont.GetFrameParametric();
    loco::move_style_anim   MoveStyleAnim = GetCurrentMoveStyleAnim();
                
    // Switch over to multi-player anims
    LocoAnims[ ANIM_WALK_IDLE                    ]  = WeaponAnims[ MP_ANIM_WALK_IDLE                    ];
    LocoAnims[ ANIM_WALK_IDLE_TURN_LEFT          ]  = WeaponAnims[ MP_ANIM_WALK_IDLE_TURN_LEFT          ];
    LocoAnims[ ANIM_WALK_IDLE_TURN_RIGHT         ]  = WeaponAnims[ MP_ANIM_WALK_IDLE_TURN_RIGHT         ];
    LocoAnims[ ANIM_WALK_MOVE_FRONT              ]  = WeaponAnims[ MP_ANIM_WALK_MOVE_FRONT              ];
    LocoAnims[ ANIM_WALK_MOVE_LEFT               ]  = WeaponAnims[ MP_ANIM_WALK_MOVE_LEFT               ];
    LocoAnims[ ANIM_WALK_MOVE_BACK               ]  = WeaponAnims[ MP_ANIM_WALK_MOVE_BACK               ];
    LocoAnims[ ANIM_WALK_MOVE_RIGHT              ]  = WeaponAnims[ MP_ANIM_WALK_MOVE_RIGHT              ];
    LocoAnims[ ANIM_RUN_IDLE                     ]  = WeaponAnims[ MP_ANIM_RUN_IDLE                     ];
    LocoAnims[ ANIM_RUN_IDLE_TURN_LEFT           ]  = WeaponAnims[ MP_ANIM_RUN_IDLE_TURN_LEFT           ];
    LocoAnims[ ANIM_RUN_IDLE_TURN_RIGHT          ]  = WeaponAnims[ MP_ANIM_RUN_IDLE_TURN_RIGHT          ];
    LocoAnims[ ANIM_RUN_MOVE_FRONT               ]  = WeaponAnims[ MP_ANIM_RUN_MOVE_FRONT               ];
    LocoAnims[ ANIM_RUN_MOVE_LEFT                ]  = WeaponAnims[ MP_ANIM_RUN_MOVE_LEFT                ];
    LocoAnims[ ANIM_RUN_MOVE_BACK                ]  = WeaponAnims[ MP_ANIM_RUN_MOVE_BACK                ];
    LocoAnims[ ANIM_RUN_MOVE_RIGHT               ]  = WeaponAnims[ MP_ANIM_RUN_MOVE_RIGHT               ];
    LocoAnims[ ANIM_CROUCH_IDLE                  ]  = WeaponAnims[ MP_ANIM_CROUCH_IDLE                  ];
    LocoAnims[ ANIM_CROUCH_IDLE_TURN_LEFT        ]  = WeaponAnims[ MP_ANIM_CROUCH_IDLE_TURN_LEFT        ];
    LocoAnims[ ANIM_CROUCH_IDLE_TURN_RIGHT       ]  = WeaponAnims[ MP_ANIM_CROUCH_IDLE_TURN_RIGHT       ];
    LocoAnims[ ANIM_CROUCH_MOVE_FRONT            ]  = WeaponAnims[ MP_ANIM_CROUCH_MOVE_FRONT            ];
    LocoAnims[ ANIM_CROUCH_MOVE_LEFT             ]  = WeaponAnims[ MP_ANIM_CROUCH_MOVE_LEFT             ];
    LocoAnims[ ANIM_CROUCH_MOVE_BACK             ]  = WeaponAnims[ MP_ANIM_CROUCH_MOVE_BACK             ];
    LocoAnims[ ANIM_CROUCH_MOVE_RIGHT            ]  = WeaponAnims[ MP_ANIM_CROUCH_MOVE_RIGHT            ];
    LocoAnims[ ANIM_JUMP                         ]  = WeaponAnims[ MP_ANIM_JUMP                         ];
    LocoAnims[ ANIM_GRENADE                      ]  = WeaponAnims[ MP_ANIM_GRENADE                      ];
    LocoAnims[ ANIM_MELEE                        ]  = WeaponAnims[ MP_ANIM_MELEE                        ];
    LocoAnims[ ANIM_STAND_LEAN_LEFT              ]  = WeaponAnims[ MP_ANIM_STAND_LEAN_LEFT              ];
    LocoAnims[ ANIM_STAND_LEAN_RIGHT             ]  = WeaponAnims[ MP_ANIM_STAND_LEAN_RIGHT             ];
    LocoAnims[ ANIM_CROUCH_LEAN_LEFT             ]  = WeaponAnims[ MP_ANIM_CROUCH_LEAN_LEFT             ];
    LocoAnims[ ANIM_CROUCH_LEAN_RIGHT            ]  = WeaponAnims[ MP_ANIM_CROUCH_LEAN_RIGHT            ];

    LocoAnims[ ANIM_SHOOT                        ]  = GetAnimIndex( GetMPShootPrimaryAnimType( InvenWeapon ) );
    LocoAnims[ ANIM_SHOOT_SECONDARY              ]  = GetAnimIndex( GetMPShootSecondaryAnimType( InvenWeapon ) );
    LocoAnims[ ANIM_RELOAD                       ]  = GetAnimIndex( GetMPReloadAnimType( InvenWeapon ) );

    // Move style setup yet?
    if( MoveStyle != loco::MOVE_STYLE_NULL )
    {
        // Switch to same move style again so new anim indices are setup
        SetMoveStyle( MoveStyle );

        // If previously playing a move style anim, then restart the anim
        if( MoveStyleAnim != loco::MOVE_STYLE_ANIM_NULL )
        {
            // Blend smoothly to the new anim (interrupting any current blend)
            m_Player.SetAnim( m_MoveStyleInfo.m_hAnimGroup,                 // hAnimGroup
                              m_MoveStyleInfo.m_iAnims[ MoveStyleAnim ],    // iAnim
                              0.2f,                                         // BlendTime
                              1.0f,                                         // Rate
                              loco::ANIM_FLAG_INTERRUPT_BLEND );            // Flags
            
            // Restart on the same frame as before
            Cont.SetFrameParametric( Frame );
        }
    }
        
    // Finally, flag the current weapon anims being used
    m_CurrentWeaponAnims = InvenWeapon;
    
    // Setup aimer masks
    if(     ( m_CurrentWeaponAnims == INVEN_WEAPON_DUAL_SMP )
        ||  ( m_CurrentWeaponAnims == INVEN_WEAPON_DUAL_SHT ) )
    {
        // Aiming bone masks move the left and right arm so duals are in sync
        SetAimerBoneMasks( TRUE, 0.25f );
    }
    else
    {
        // Non-aiming bone masks do not move the left arm so that the IK doesn't
        // push the weapon through the body
        SetAimerBoneMasks( FALSE, 0.25f );
    }    
    
    // Re-initialize the IK ready to keep the hands on the weapon
    InitIK();
    
    return TRUE;
}

//==============================================================================

void player_loco::UpdateAnims( f32                  DeltaTime,
                               xbool                bIsAirborn,
                               xbool                bIsCrouching,
                               f32                  Lean )
{
    // Lookup current anim state before overwriting the main loco anim indices
    loco_motion_controller& Cont          = m_Player.GetCurrAnim();
    loco::move_style_anim   MoveStyleAnim = GetCurrentMoveStyleAnim();

    // Compute horiz movement speed
    const vector3& DeltaPos = GetDeltaPos();
    f32 Speed = x_sqr( DeltaPos.GetX() ) + x_sqr( DeltaPos.GetZ() );
    if( Speed > 0.0001f ) 
        Speed = x_sqrt( Speed );
    
    if( DeltaTime > 0.0f )            
        Speed /= DeltaTime;
    else
        Speed = 0.0f;                        

    // Airborn?
    if( bIsAirborn )
    {
        // Start the jump anim if it hasn't been already
        s32 iJumpAnim = GetAnimIndex( loco::ANIM_JUMP );
        if( m_Player.GetCurrAnim().GetAnimIndex() != iJumpAnim ) 
            PlayAnim( loco::ANIM_JUMP, 0.1f, loco::ANIM_FLAG_END_STATE_HOLD );

        // Playing the jump anim yet?
        if( m_Player.GetCurrAnim().GetAnimIndex() == iJumpAnim ) 
        {                
            // If falling down, goto the last frame of the jump anim
            if( DeltaPos.GetY() < 0.0f )
                m_Player.GetCurrAnim().SetFrameParametric( 1.0f );
        }
    }
    // Leaning and not moving?
    else if( ( Lean != 0.0f ) && ( Speed <= 1.0f ) )
    {
        // Lookup lean anim to play
        loco::anim_type AnimType = ANIM_NULL;
        if( bIsCrouching )
        {
            if( Lean > 0 )
                AnimType = ANIM_CROUCH_LEAN_LEFT;
            else                
                AnimType = ANIM_CROUCH_LEAN_RIGHT;
        }
        else
        {
            if( Lean > 0 )
                AnimType = ANIM_STAND_LEAN_LEFT;
            else                
                AnimType = ANIM_STAND_LEAN_RIGHT;
        }                
        
        // Play a lean anim?
        if( AnimType != ANIM_NULL )
        {
            // Play lean anim
            s32 iAnim = GetAnimIndex( AnimType );
            if( iAnim != -1 )
                PlayAnim( iAnim, 0.1f, loco::ANIM_FLAG_END_STATE_HOLD );
            
            // Set frame to be lean amount if playing the anim
            loco_motion_controller& Cont = m_Player.GetCurrAnim();
            if( Cont.GetAnimTypeIndex() == iAnim )
            {            
                Cont.SetRate( 0.0f );
                Cont.SetFrameParametric( x_abs( Lean ) );
            }
        }
    }        
    else
    {
        // Set to correct state
        if( Speed > 0.0f )
            SetState( loco::STATE_MOVE );
        else
            SetState( loco::STATE_IDLE );

        // Crouch?
        if( bIsCrouching )
        {
            // Switch to crouch?
            if( GetMoveStyle() != loco::MOVE_STYLE_CROUCH )
            {
                // Set new style
                SetMoveStyle( loco::MOVE_STYLE_CROUCH );
                SetBlendMoveStyle( loco::MOVE_STYLE_NULL );
                SetBlendMoveStyleAmount( 0.0f );

                // Blend to this style?
                if( MoveStyleAnim != loco::MOVE_STYLE_ANIM_NULL )
                    PlayAnim( m_MoveStyleInfo.m_hAnimGroup, m_MoveStyleInfo.m_iAnims[ MoveStyleAnim ], 0.2f );                    
            }

            // Try match the rate
            Cont.SetMatchingRate( DeltaPos, DeltaTime, g_PlayerLocoTweaks.m_CrouchMinRate, g_PlayerLocoTweaks.m_CrouchMaxRate );
        }
        else
        {
            // Switch to walk?
            if( GetMoveStyle() != loco::MOVE_STYLE_WALK )
            {
                // Set new style
                SetMoveStyle( loco::MOVE_STYLE_WALK );

                // Blend to this style?
                if( MoveStyleAnim != loco::MOVE_STYLE_ANIM_NULL )
                    PlayAnim( m_MoveStyleInfo.m_hAnimGroup, m_MoveStyleInfo.m_iAnims[ MoveStyleAnim ], 0.2f );                    
            }                

            // Scale speed from 0 -> 1
            Speed = ( Speed - g_PlayerLocoTweaks.m_WalkRunMinSpeed ) / ( g_PlayerLocoTweaks.m_WalkRunMaxSpeed - g_PlayerLocoTweaks.m_WalkRunMinSpeed );
            Speed = x_max( 0.0f, x_min( 1.0f, Speed ) );

            // Setup run mix amount
            SetBlendMoveStyle( loco::MOVE_STYLE_RUN );
            SetBlendMoveStyleAmount( Speed );

            // Try match the rate
            Cont.SetMatchingRate( DeltaPos, DeltaTime, g_PlayerLocoTweaks.m_WalkRunMinRate, g_PlayerLocoTweaks.m_WalkRunMaxRate );
        }
    }
    
    // Update IK and aimer weight
    xbool bAimer = TRUE;
    xbool bIK    = TRUE;
    
    // Turn off if playing masked reload or melee
    if( m_MaskController.IsPlaying() )    
    {
        bAimer = FALSE;
        bIK    = FALSE;
    }
    
    // Turn off if playing grenade toss
    loco_additive_controller& AddCont = m_AdditiveController[0];
    if(     ( AddCont.GetAnimTypeIndex() == GetAnimIndex( loco::ANIM_GRENADE ) )
        &&  ( AddCont.GetWeight() > 0.0f ) )
    {
        bAimer = FALSE;
        bIK    = FALSE;
    }

    // Turn off IK if holding dual smp/shotguns, or mutated
    if(     ( m_CurrentWeaponAnims == INVEN_WEAPON_DUAL_SMP )
        ||  ( m_CurrentWeaponAnims == INVEN_WEAPON_DUAL_SHT )
        ||  ( m_CurrentWeaponAnims == INVEN_WEAPON_MUTATION ) )
    {
        bIK = FALSE;
    }
        
    // Update aimer weight
    f32 AimerWeight = m_AimController.GetWeight();
    if( bAimer )
    {
        // Blend in
        AimerWeight = x_min( AimerWeight + ( 4.0f * DeltaTime ), 1.0f );
    }
    else
    {
        // Blend out
        AimerWeight = x_max( AimerWeight - ( 4.0f * DeltaTime ), 0.0f );
    }

    // Update IK weight
    f32 IKWeight = m_IKSolver.GetWeight();
    if( bIK )
    {
        // Blend in
        IKWeight = x_min( IKWeight + ( 4.0f * DeltaTime ), 1.0f );
    }
    else
    {
        // Blend out
        IKWeight = x_max( IKWeight - ( 4.0f * DeltaTime ), 0.0f );
    }

    // Finally, update aimer and IK
    m_AimController.SetWeight( AimerWeight );
    m_IKSolver.SetWeight( IKWeight );
}

//==============================================================================

#if !defined( CONFIG_RETAIL )

// Renders debug info
void player_loco::RenderInfo( xbool bRenderLoco,
                              xbool bLabelLoco,
                              xbool bRenderSkeleton,
                              xbool bLabelSkeleton )
{
    s32 i;
    
    // Call base class
    loco::RenderInfo( bRenderLoco, bLabelLoco, bRenderSkeleton, bLabelSkeleton );

    // Prepare for draw
    const matrix4* pMatrices = m_Player.GetBoneL2Ws();
    draw_ClearL2W();
    
    // Render the pivots
    for( i = 0; i < 3; i++ )
    {
        loco_ik_solver::bone_mapping& Map = m_IKBoneMappings[i];
        
        matrix4 L2W = pMatrices[ Map.m_iBone ] * Map.m_B2S;
        draw_Sphere( L2W.GetTranslation(), 2.5f, XCOLOR_WHITE );
    }      
}

#endif // !defined( CONFIG_RETAIL )

//==============================================================================
