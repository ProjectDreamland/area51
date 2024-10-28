//=========================================================================
//
//  Grunt.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Grunt.hpp"

#include "objects\GrenadeProjectile.hpp"
#include "objects\NewWeapon.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "MiscUtils\TrajectoryGenerator.hpp"
#include "Debris\debris_mgr.hpp"
#include "Debris\debris_rigid.hpp"
#include "gamelib\StatsMgr.hpp"

//=========================================================================
// DEBUG
//=========================================================================
#ifdef DEBUG_DRAW_GRUNT_WEAPON_AXIS
static matrix4 WeaponMatrix;
#endif

//=========================================================================
// CONSTS DEFINTIONS and STATIC FUNCTIONS
//=========================================================================

static const f32    DISPLACE_PARTICLE_EXTENT    = 20.0f;
static const f32    PLAYER_MIDDLE_HEIGHT        = 150.0f;
static const f32    k_blood_ray_distance        = 500.0f;
static const radian s_Max_Rot_Per_Second_While_Attacking    = R_0;//R_180;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct grunt_desc : public object_desc
{
        grunt_desc( void ) : object_desc( 
            object::TYPE_GRUNT, 
            "NPC - Grunt", 
            "AI",
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             | 
            object::ATTR_TRANSPARENT            |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_CHARACTER_OBJECT       |
            object::ATTR_DAMAGEABLE             |
            object::ATTR_LIVING                 |
            object::ATTR_CAST_SHADOWS,

            FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC |
            FLAGS_NO_ICON    ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        grunt* pGrunt = new grunt;
        return pGrunt; 
    
    } 

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "SkinGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

    //-------------------------------------------------------------------------
    
    virtual void      OnEnumProp        ( prop_enum&   List )
    {
        object_desc::OnEnumProp( List );
        List.PropEnumFileName( "ObjectDesc\\Armed Bone Mask File", 
                                "Armed Bone Mask info (*.msk)|*.msk|All Files (*.*)|*.*||",
                                "This may work", 0 );
         List.PropEnumFileName( "ObjectDesc\\Search Bone Mask File", 
                                "Armed Bone Mask info (*.msk)|*.msk|All Files (*.*)|*.*||",
                                "This may work", 0 );
   }

    //-------------------------------------------------------------------------

    virtual xbool    OnProperty( prop_query& I )
    {
        if ( object_desc::OnProperty( I ) )
        {
            return TRUE;
        }

        if ( I.VarFileName( "ObjectDesc\\Armed Bone Mask File", grunt::s_pArmedBoneMaskFileName, 256 ) )
        {
            return TRUE;
        }

        if ( I.VarFileName( "ObjectDesc\\Search Bone Mask File", grunt::s_pSearchBoneMaskFileName, 256 ) )
        {
            return TRUE;
        }

        return FALSE;

    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

//==========================================================================

} s_grunt_Desc;

//=============================================================================

const object_desc& grunt::GetTypeDesc( void ) const
{
    return s_grunt_Desc;
}

//=============================================================================

const object_desc& grunt::GetObjectType( void )
{
    return s_grunt_Desc;
}

//=========================================================================
// GRUNT CHARACTER
//=========================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

grunt::grunt() :
    character(),
    m_Idle              ( *this, character_state::STATE_IDLE    ),
    m_Alert             ( *this, character_state::STATE_ALERT   ),
    m_Search            ( *this, character_state::STATE_SEARCH  ),
    m_Cover             ( *this, character_state::STATE_COVER   ),
    m_Death             ( *this, character_state::STATE_DEATH   ),
    m_Attack            ( NULL )
{
    // Setup pointer to loco for base class to use
    m_pLoco                 = &m_Loco;

    m_bPlayedRage           = FALSE;

    vector3 vBox( 100.f, 100.f, 100.f );
    m_BBox.Set( vBox , -vBox );
    
    //set up faction and friendly factions
    m_Faction               = FACTION_MUTANTS_LESSER;   
    m_FriendFlags           |= ( FACTION_MUTANTS_LESSER | FACTION_PLAYER_STRAIN1 ); 
    m_MaxDistToCover        = 500.0f;
    m_bOnlyUsesCoverAhead   = TRUE;
    m_bCoverHopper          = TRUE;
    m_CombatReady           = TRUE;
    m_bCanReload            = FALSE;

}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//=========================================================================

grunt::~grunt()
{
    if( m_Attack )
    {
        delete m_Attack;
    }
}

char  grunt::s_pArmedBoneMaskFileName[ 256 ] = {0};
char  grunt::s_pSearchBoneMaskFileName[ 256 ] = {0};

//=============================================================================
//  OnInit
//
//      For now, hit locations are going to be added here.  They will get the
//      proper boneID set when the model is actually loaded.
//=============================================================================
void grunt::OnInit( void )
{
    // Call base class
    character::OnInit();
}



//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

//=========================================================================

xbool grunt::OnAnimEvent( const anim_event& Event, const vector3& WorldPos )
{
    // Call base class
    if (character::OnAnimEvent(Event, WorldPos))
        return TRUE;
    // Event not handled
    return FALSE;
}

//=============================================================================

xbool grunt::HandleSpecialImpactAnim ( const eHitType hitType )
{
    if (m_bPlayedRage)
        return FALSE;

    if (hitType == HITTYPE_LIGHT)
    {
        if ( GetLocoPointer()->GetState() != loco::STATE_PLAY_ANIM )
        {
            if (m_Health.GetHealth() < (m_MaxHealth * 0.5f) )
            {
                m_bPlayedRage = TRUE; //only do this check once

                //health is below half health
                if (x_irand(0,1))
                {
                    //50% of the time (one in a lifetime) we play a rage
                    GetLocoPointer()->PlayMaskedAnim(loco::ANIM_ADD_REACT_RAGE, loco::BONE_MASKS_TYPE_UPPER_BODY, 0.1f);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}


xbool grunt::OnProperty ( prop_query& I )
{
    if ( I.IsVar( "Character\\Logical Name"))
    {
        if (!I.IsRead())
        {
            if( !x_strcmp(I.GetVarString(),"Leaper") )
            {
                if( !m_Attack )
                    m_Attack = new leaper_attack_state( *this, character_state::STATE_ATTACK  );
                SetSubtype( SUBTYPE_LEAPER );
            }
            else if( !x_strcmp(I.GetVarString(),"Grunt_Hazmat") )
            {
                if( !m_Attack )
                    m_Attack = new grunt_attack_state( *this, character_state::STATE_ATTACK  );
                SetSubtype( SUBTYPE_GRUNT_HAZMAT );
            }
            else if( !x_strcmp(I.GetVarString(),"Grunt_Spec4") )
            {
                if( !m_Attack )
                    m_Attack = new grunt_attack_state( *this, character_state::STATE_ATTACK  );
                SetSubtype( SUBTYPE_GRUNT_SPEC4 );
            }
            else if( !x_strcmp(I.GetVarString(),"Grunt_Scientist") )
            {
                if( !m_Attack )
                    m_Attack = new grunt_attack_state( *this, character_state::STATE_ATTACK  );
                SetSubtype( SUBTYPE_GRUNT_SCIENTIST );
            }
            else
            {
                // unhandled logical name!
                ASSERT( FALSE );
                if( !m_Attack )
                    m_Attack = new grunt_attack_state( *this, character_state::STATE_ATTACK  );
                SetSubtype( SUBTYPE_GRUNT_HAZMAT );
            }
        }
    }

    return character::OnProperty(I);
}
