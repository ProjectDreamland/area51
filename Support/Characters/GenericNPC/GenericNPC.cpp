//=========================================================================
//
//  GENERIC.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "GenericNPC.hpp"

#include "objects\GrenadeProjectile.hpp"
#include "objects\NewWeapon.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "MiscUtils\TrajectoryGenerator.hpp"
#include "Debris\debris_mgr.hpp"
#include "Debris\debris_rigid.hpp"
#include "gamelib\StatsMgr.hpp"

//=========================================================================
// CONSTS DEFINTIONS and STATIC FUNCTIONS
//=========================================================================

static const f32    DISPLACE_PARTICLE_EXTENT    = 20.0f;
static const f32    PLAYER_MIDDLE_HEIGHT        = 150.0f;
static const radian s_Max_Rot_Per_Second_While_Attacking    = R_0;//R_180;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct generic_desc : public object_desc
{
        generic_desc( void ) : object_desc( 
            object::TYPE_GENERIC_NPC, 
            "NPC - GENERIC", 
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
            object::ATTR_LIVING,


            FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC |
            FLAGS_NO_ICON    | 
            FLAGS_GENERIC_EDITOR_CREATE ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        genericNPC* pGENERIC = new genericNPC;
        return pGENERIC; 
    
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
    }

    //-------------------------------------------------------------------------

    virtual xbool    OnProperty( prop_query& I )
    {
        if ( object_desc::OnProperty( I ) )
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

} s_generic_Desc;

//=============================================================================

const object_desc& genericNPC::GetTypeDesc( void ) const
{
    return s_generic_Desc;
}

//=============================================================================

const object_desc& genericNPC::GetObjectType( void )
{
    return s_generic_Desc;
}

//=========================================================================
// GENERIC CHARACTER
//=========================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

genericNPC::genericNPC() :
    character(),
    m_Idle              ( *this, character_state::STATE_IDLE    ),
    m_Alert             ( *this, character_state::STATE_ALERT   ),
    m_Search            ( *this, character_state::STATE_SEARCH  ),
    m_Attack            ( *this, character_state::STATE_ATTACK  ),
    m_Cover             ( *this, character_state::STATE_COVER  ),
    m_Death             ( *this, character_state::STATE_DEATH   )
{
    // Setup pointer to loco for base class to use
    m_pLoco             = &m_Loco;

    vector3 vBox( 100.f, 100.f, 100.f );
    m_BBox.Set( vBox , -vBox );
    
    // set up the leave cover condition for us.
    m_Cover.SetLeaveCondition( character_cover_state::LEAVE_COVER_WHEN_BROKEN );
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//=========================================================================

genericNPC::~genericNPC()
{
}

//=============================================================================
//  OnInit
//
//      For now, hit locations are going to be added here.  They will get the
//      proper boneID set when the model is actually loaded.
//=============================================================================
void genericNPC::OnInit( void )
{
    // Call base class
    character::OnInit();
}

//=============================================================================

