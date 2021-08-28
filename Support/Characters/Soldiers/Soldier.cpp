//=========================================================================
//
//  BlackOpps.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Soldier.hpp"
#include "Characters\God.hpp"
#include "Debris\debris_rigid.hpp"
#include "gamelib\StatsMgr.hpp"
#include "Objects\NewWeapon.hpp"
#include "Objects\Corpse.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Objects\AlienOrb.hpp"

//=========================================================================
// DEBUG
//=========================================================================
#ifdef DEBUG_DRAW_SOLDIER_WEAPON_AXIS
static matrix4 WeaponMatrix;
#endif

//=========================================================================
// CONSTS DEFINTIONS and STATIC FUNCTIONS
//=========================================================================

static const f32    DISPLACE_PARTICLE_EXTENT    = 20.0f;
static const f32    PLAYER_MIDDLE_HEIGHT        = 150.0f;
const f32           k_MinDistToCorpseSqr      = 400.0f * 400.0f;
//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct soldier_desc : public object_desc
{
        soldier_desc( void ) : object_desc( 
            object::TYPE_BLACK_OPPS, 
            "NPC - BlackOpps", 
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
            FLAGS_NO_ICON )               { }

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new soldier; } 

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

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

    //-------------------------------------------------------------------------

    virtual void OnEnumProp( prop_enum&   List )
    {
         object_desc::OnEnumProp( List );
    }

    //-------------------------------------------------------------------------

    virtual xbool OnProperty( prop_query& I )
    {
        if ( object_desc::OnProperty( I ) )
        {
            return TRUE;
        }

        return FALSE;
    }


} s_Soldier_Desc;

//=============================================================================

const object_desc& soldier::GetTypeDesc( void ) const
{
    return s_Soldier_Desc;
}

//=============================================================================

const object_desc& soldier::GetObjectType( void )
{
    return s_Soldier_Desc;
}

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct hazmat_desc : public object_desc
{
    hazmat_desc( void ) : object_desc( 
        object::TYPE_HAZMAT, 
        "NPC - Hazmat", 
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

        FLAGS_IS_DYNAMIC |
        FLAGS_NO_ICON )               { }

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new hazmat; } 

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

#ifdef X_EDITOR

        virtual s32  OnEditorRender( object& Object ) const
        {
            object_desc::OnEditorRender( Object );
            return -1;
        }

#endif // X_EDITOR

        //-------------------------------------------------------------------------

        virtual void OnEnumProp( prop_enum&   List )
        {
            object_desc::OnEnumProp( List );
        }

        //-------------------------------------------------------------------------

        virtual xbool OnProperty( prop_query& I )
        {
            if ( object_desc::OnProperty( I ) )
            {
                return TRUE;
            }

            return FALSE;
        }


} s_Hazmat_Desc;


//=============================================================================

const object_desc& hazmat::GetTypeDesc( void ) const
{
    return s_Hazmat_Desc;
}

//=============================================================================

const object_desc& hazmat::GetObjectType( void )
{
    return s_Hazmat_Desc;
}

hazmat::hazmat() : soldier()
{
}

//=========================================================================

hazmat::~hazmat()
{
}

//=========================================================================
// Soldier CHARACTER
//=========================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

soldier::soldier() :
    character(),
    m_Idle              ( *this, character_state::STATE_IDLE    ),
    m_Alert             ( *this, character_state::STATE_ALERT   ),
    m_Search            ( *this, character_state::STATE_SEARCH  ),
    m_Alarm             ( *this, character_state::STATE_ALARM   ),
    m_Turret            ( *this, character_state::STATE_TURRET  ),
    m_Death             ( *this, character_state::STATE_DEATH   ),
    m_Cover             ( NULL ),
    m_Attack            ( NULL ),
    m_ParticleGuid      ( 0 ),
    m_nFlashlightBoneIndex(-1),
    m_bFlashLightInited( FALSE )
{
    // Setup pointer to loco for base class to use
    m_pLoco             = &m_Loco;
    m_BlackOppsType     = x_irand( 1, 3 );
    m_VoiceID           = 0;
    m_LastBabyCry       = 0.0f;
    m_AllyCorpse      = 0;

    vector3 vBox( 100.f, 100.f, 100.f );
    m_BBox.Set( vBox , -vBox );

    //set up faction and friendly factions
    m_Faction = FACTION_BLACK_OPS;   
    m_FriendFlags |= ( FACTION_MILITARY | FACTION_WORKERS | FACTION_PLAYER_NORMAL | FACTION_GRAY);

    m_OrbGuid       = 0;
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//=========================================================================

soldier::~soldier()
{
    delete m_Attack;
    delete m_Cover;
}

//=============================================================================
//  OnInit
//
//      For now, hit locations are going to be added here.  They will get the
//      proper boneID set when the model is actually loaded.
//=============================================================================

void soldier::OnInit( void )
{
    // Call base class
    character::OnInit();

//    m_AimDegradeMultiplier = 0.7f;
}

//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

void soldier::OnEnumProp ( prop_enum&    List )
{
    // Call base class
    character::OnEnumProp(List);

/*
    // Weapon Enumeration...
    List.AddEnum(       "BlackOpps\\Weapon", 
                         GetWeaponStatesEnum(),
                        "Type of weapon used for this BlackOpps." );
*/
//b    List.AddExternal( "BlackOpps\\Flashlight Effect", "Resource\0fxo\0", "Effect file" );

}

//=============================================================================

xbool soldier::OnProperty ( prop_query& I )
{
    if ( I.IsVar( "Character\\Logical Name"))
    {
        if (!I.IsRead())
        {
            if( !x_strcmp(I.GetVarString(),"BlackOps") )
            {
                if( !m_Attack )
                    m_Attack    = new blackOp_attack_state( *this, character_state::STATE_ATTACK  );

                if( !m_Cover )
                    m_Cover     = new blackOp_cover_state( *this, character_state::STATE_COVER  );

                SetSubtype( SUBTYPE_BLACKOPS );
            }
            else if ( !x_strcmp(I.GetVarString(),"BlackOp_Leader") )
            {
                if( !m_Attack )
                    m_Attack    = new blackOp_attack_state( *this, character_state::STATE_ATTACK  );

                if( !m_Cover )
                    m_Cover     = new blackOp_cover_state( *this, character_state::STATE_COVER  );

                SetSubtype( SUBTYPE_BLACKOP_LEADER );
            }
            else if ( !x_strcmp(I.GetVarString(),"Hazmat") )
            {
                if( !m_Attack )
                    m_Attack    = new soldier_attack_state( *this, character_state::STATE_ATTACK  );

                if( !m_Cover )
                    m_Cover     = new character_cover_state( *this, character_state::STATE_COVER  );

                SetSubtype( SUBTYPE_HAZMAT );
            }
            else if ( !x_strcmp(I.GetVarString(),"Spec4") )
            {
                if( !m_Attack )
                    m_Attack    = new soldier_attack_state( *this, character_state::STATE_ATTACK  );
                
                if( !m_Cover )
                    m_Cover     = new character_cover_state( *this, character_state::STATE_COVER  );

                SetSubtype( SUBTYPE_SPEC4 );
            }
            else 
            {
                // unhandled logical name!
                ASSERT( FALSE );

                if( !m_Attack )
                    m_Attack    = new soldier_attack_state( *this, character_state::STATE_ATTACK  );

                if( !m_Cover )
                    m_Cover     = new character_cover_state( *this, character_state::STATE_COVER  );

                SetSubtype( SUBTYPE_SPEC4 );
            }
        }
    }
    return character::OnProperty(I);
}

//===========================================================================

xbool soldier::SetupShoulderLight( void )
{
    if ( !m_pLoco )
        return FALSE;

    if ( m_hAnimGroup.GetPointer() == NULL )
        return FALSE;

    const anim_info& rAnimInfo = m_hAnimGroup.GetPointer()->GetAnimInfo(0);

    // Got the anim-info.  Now get the matrix we need.
    for ( s32 i = 0; i < rAnimInfo.GetNEvents(); i++ )
    {
        anim_event& rEvent = rAnimInfo.GetEvent(i);

        // This is the hot point event in the bindpose
        if ( x_strcmp( rEvent.GetType(), "Hot Point" ) == 0 )
        {
            s32 nBoneIndex = rEvent.GetInt( anim_event::INT_IDX_BONE );

            if ( nBoneIndex == -1 )
                return FALSE;

            m_nFlashlightBoneIndex = nBoneIndex;
            m_vFlashlightBoneOffset = rEvent.GetPoint( anim_event::POINT_IDX_OFFSET );
            vector3 vTemp = rEvent.GetPoint( anim_event::POINT_IDX_ROTATION );
//            vTemp.RotateY( R_180 );
            m_FlashLightBindRot.Set( vTemp.GetX(), vTemp.GetY(), vTemp.GetZ() );
            m_bFlashLightInited = TRUE;

            return TRUE;

        }

    }

    return FALSE;
}

//=============================================================================

void soldier::OnDeath(void)
{
    character::OnDeath();

    // If an orb was transferred to us, destroy it on death
    if ( 0 != m_OrbGuid )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_OrbGuid );
        if (pObj)
        {
            if (pObj->IsKindOf( alien_orb::GetRTTI()))
            {
                alien_orb& Orb = alien_orb::GetSafeType( *pObj );

                Orb.KillOrb();
            }
        }        
    }
}

//=============================================================================

xbool soldier::CoverRetreatWhenDamaged()
{
    if( GetSubtype() == soldier::SUBTYPE_BLACKOPS ||
        GetSubtype() == soldier::SUBTYPE_BLACKOP_LEADER )
    {
        return FALSE;
    }   
    else
    {    
        return TRUE;
    }
}

//=============================================================================

void soldier::SetOrbGuid( guid OrbGuid )
{
    m_OrbGuid = OrbGuid;
}

//=============================================================================

xbool soldier::GetHasDrainableCorpse()
{
    if( GetSubtype() == soldier::SUBTYPE_BLACKOPS ||
        GetSubtype() == soldier::SUBTYPE_BLACKOP_LEADER )
    {
        return TRUE;
    }   
    else
    {    
        return FALSE;
    }
}

//=============================================================================

void soldier::BecomeLeader()
{
    // here we do everything to turn a BO into a BO leader.    
    m_LogicalName = g_StringMgr.Add("BlackOp_Leader");
    SetSubtype( SUBTYPE_BLACKOP_LEADER );
    m_Accuracy = 90;
    m_MovingTargetAccuracy = 75;
    m_CanCloak = TRUE;
}

//=============================================================================

void soldier::OnThink()     
{
    if( GetSubtype() == SUBTYPE_BLACKOPS )
    {
        object* pOurCorpse = g_ObjMgr.GetObjectByGuid( m_AllyCorpse );
        if( !pOurCorpse )
        {        
            m_AllyCorpse = 0;
            // let's search for nearby allied dead body.
            slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_CORPSE );
            while( SlotID != SLOT_NULL )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                if( pObject != NULL )
                {
                    corpse &CorpseObject = corpse::GetSafeType( *pObject );
                    if( CorpseObject.GetDrainable() )
                    {
                        // we can drain this one! How close is it, and can we reach it?
                        vector3 toCorpse = CorpseObject.GetPosition() - GetPosition();
                        if( toCorpse.LengthSquared() <= k_MinDistToCorpseSqr &&
                            CanPathTo(CorpseObject.GetPosition()) )
                        {
                            // claim it as ours.
                            m_AllyCorpse = CorpseObject.GetGuid();
                            CorpseObject.SetDrainable(FALSE);
                        }
                    }
                }
                SlotID = g_ObjMgr.GetNext( SlotID );
            }
        }
    }
    character::OnThink();
}
