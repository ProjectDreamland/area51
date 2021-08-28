//=========================================================================
// EXPLOSIVE BULLET PROJECTILE
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "ProjectileExplosiveBullett.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "objects\ParticleEmiter.hpp"
#include "gamelib\StatsMgr.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "Decals\DecalMgr.hpp"


//=========================================================================
// OBJECT DESC.
//=========================================================================
static struct explosive_bullet_projectile_desc : public object_desc
{
    explosive_bullet_projectile_desc( void ) : object_desc( 
            object::TYPE_EXPLOSIVE_BULLET_PROJECTILE, 
            "Explosive Bullet", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_NO_RUNTIME_SAVE,
            FLAGS_IS_DYNAMIC 
            /*| FLAGS_GENERIC_EDITOR_CREATE*/ )
            {}

    virtual object* Create          ( void )
    {
        return new explosive_bullet_projectile;
    }

} s_Explosive_Bullet_Projectile_Desc;

//=============================================================================================

const object_desc&  explosive_bullet_projectile::GetTypeDesc     ( void ) const
{
    return s_Explosive_Bullet_Projectile_Desc;
}


//=============================================================================================
const object_desc&  explosive_bullet_projectile::GetObjectType   ( void )
{
    return s_Explosive_Bullet_Projectile_Desc;
}

//=============================================================================================

explosive_bullet_projectile::explosive_bullet_projectile()
{
	m_MaxAliveTime  = 0.3f;
    m_ExplosionSize = 25.0f;
    m_bThroughGlass = TRUE;
}

//=============================================================================================

explosive_bullet_projectile::~explosive_bullet_projectile()
{
}

//=============================================================================

#ifndef X_RETAIL
void explosive_bullet_projectile::OnDebugRender ( void )
{
    draw_ClearL2W();
    draw_Sphere( GetPosition(), m_PainRadius, XCOLOR_RED );        
}
#endif // X_RETAIL

//=============================================================================
/*
void explosive_bullet_projectile::BroadcastPain( void )
{
    //create pain area
    bbox Pain( GetPosition(), m_PainRadius );
    g_ObjMgr.SelectBBox( ATTR_DAMAGEABLE , Pain ,object::TYPE_ALL_TYPES );    
    xarray<slot_id> ObjectSlots;
    ObjectSlots.SetGrowAmount( 10 );

    vector3 ColX;
    vector3 ColY;
    vector3 ColZ;

    GetL2W().GetColumns( ColX, ColY, ColZ );

    slot_id SlotID = g_ObjMgr.StartLoop();
    while( SlotID != SLOT_NULL )
    {
        ObjectSlots.Append() = SlotID;
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }
    g_ObjMgr.EndLoop();

    
    for( s32 i = 0; i < ObjectSlots.GetCount(); i++ )
    {
        slot_id& SlotID = ObjectSlots[i];
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

        f32 DamageMax = m_PainAmount;
        f32 ForceMax = m_PainAmount;

        //for living objects (player, npc's) we will add in cover protection
        if (pObject->GetAttrBits() & ATTR_LIVING )
        {
                //calc LOS, since we don't want as much force or damage if they are behind cover
//                vector3 ObjPos = pObject->GetBBox().GetCenter();
            vector3 ColPos = GetPosition();
            ColPos.GetY() = GetBBox().Max.GetY();

            f32 nCoverAmount = 0;

            // Commented out because the HasLOS ends up calling the collision
            // manager which does an ObjectMgr.SelectBBox.  There is already
            // a SelectBBox in progress at the top of this function and so
            // an ASSERT kicks off.

            //damage reduction of 1/3 per cover 
            DamageMax = DamageMax * (1.0f-(nCoverAmount/2.0f));
            //force reduction of 1/4 per cover (more concussion than damage)
            ForceMax = ForceMax * (1.0f-(nCoverAmount/3.0f));
        }

        pain PainEvent;
        PainEvent.Type      = (pain::type)m_PainType;
        PainEvent.Center    = GetPosition();
        PainEvent.Origin    = GetGuid();
        PainEvent.PtOfImpact= pObject->GetBBox().GetCenter();
        PainEvent.Direction = ColY;
        PainEvent.DamageR0  = DamageMax; 
        PainEvent.DamageR1  = 0; 
        PainEvent.ForceR0   = ForceMax*3;
        PainEvent.ForceR1   = 0;
        PainEvent.RadiusR0  = 0;
        PainEvent.RadiusR1  = m_PainRadius;
        PainEvent.Direction.Normalize();

        pObject->OnPain( PainEvent ); 
    }
}
*/
//=============================================================================================

void explosive_bullet_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "bullet_projectile::OnAdvanceLogic" );
    LOG_STAT( k_stats_Projectiles);
    
	//update collisions
	if ( OnProcessCollision( DeltaTime ) )
	{
		OnMove( m_CurrentPosition );
		m_MaxAliveTime -= DeltaTime;

		if ( m_MaxAliveTime < F32_MIN )
		{
			g_ObjMgr.DestroyObject( GetGuid() );
		}
	}
/*    
    if( m_BulletFlyBySfxID != -1 )
    {
        if( m_BulletSoundID )
        {
            g_AudioManager.SetPosition( m_BulletSoundID, GetPosition() );
        }
        else
        {
            slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );

            while( SlotID != SLOT_NULL )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

                // Don't play the bullet fly by sound for the bullets that he fired.
                if( pObject->GetGuid() != m_OwnerGuid )
                {
                    f32 FarClip, NearClip = 0.0f;

                    g_AudioManager.GetSampleClips( g_StringMgr.GetString( m_BulletFlyBySfxID ), NearClip, FarClip );

                    bbox Box( pObject->GetPosition(), NearClip );

                    if( Box.Intersect( GetPosition() ) )
                    {
                        if( m_BulletSoundID == 0 )
                            m_BulletSoundID = g_AudioManager.Play( g_StringMgr.GetString( m_BulletFlyBySfxID ), GetPosition(), 0 );            
                    }
                }

                SlotID = g_ObjMgr.GetNext( SlotID );
            }
        }
    }
*/
}

//=============================================================================================

void explosive_bullet_projectile::OnMove( const vector3& rNewPos )
{
	//transfom the object.
    base_projectile::OnMove( rNewPos );    
}

//=============================================================================================
/*
void explosive_bullet_projectile::Initialize( const vector3& InitPos, 
                                  const radian3& InitRot, 
                                  const vector3& InitVel,
                                        f32      DamageAmount,
                                        f32      ForceAmount,
                                        f32      Speed,
                                  const guid&    OwnerGuid ,
                                  const s32&     PainType )
{
    // Call base class
    base_projectile::Initialize(InitPos, InitRot, InitVel, DamageAmount, ForceAmount, Speed, OwnerGuid , PainType ) ;
}

//=============================================================================================

void explosive_bullet_projectile::Initialize( const vector3& InitPos , const radian3& InitRot , const vector3& InitVel , const guid& OwnerGuid )
{
	//initialize the base class
	base_projectile::Initialize( InitPos , InitRot , InitVel , OwnerGuid );
}

//=============================================================================================

void explosive_bullet_projectile::Initialize( const vector3& InitPos , const matrix4& InitMat , const vector3& InitVel , const guid& OwnerGuid )
{
	//initialize the base class
	base_projectile::Initialize( InitPos , InitMat , InitVel , OwnerGuid );
}
*/
//=============================================================================

bbox explosive_bullet_projectile::GetLocalBBox( void ) const 
{ 
    bbox BBox;
    BBox.Set( vector3( -2, -2, -2 ), vector3( 2, 2, 2) );
    return BBox;
}

//=============================================================================

void explosive_bullet_projectile::OnEnumProp( prop_enum& List )
{
    base_projectile::OnEnumProp( List );

    List.PropEnumHeader  ( "Explosive Projectile", "", 0 );
    List.PropEnumExternal( "Explosive Projectile\\Particles Resource", 
                            "Resource\0fxo\0",
                            "Particle Resource for the explosion", 0 );

    List.PropEnumFloat   ( "Explosive Projectile\\Explosion Size", "The size of the decal to scorch the ground with", 0 );
    List.PropEnumEnum    ( "Explosive Projectile\\Decal Type", "SMALL_CHAR\0MED_CHAR\0BIG_CHAR\0", "What type of decals to use.", 0 );
    List.PropEnumFloat   ( "Explosive Projectile\\Pain Radius", "The radius of the pain sphere.", 0 );
    List.PropEnumFloat   ( "Explosive Projectile\\Pain Amount", "The amount of pain to send to the objects in the pain sphere.", 0 );

}

//=============================================================================

xbool explosive_bullet_projectile::OnProperty( prop_query& PropQuery )
{
    if( base_projectile::OnProperty( PropQuery ) )
        return TRUE;

    if( PropQuery.VarFloat( "Explosive Projectile\\Explosion Size", m_ExplosionSize, 0 ) )
        return TRUE;

/*
    //TODO: Replace with new decal code
    if( PropQuery.IsVar( "Explosive Projectile\\Decal Type" ) )
    {
        if( PropQuery.IsRead() )
        {
            if( m_DecalType == decal_mgr::SML_CHAR )
                PropQuery.SetVarEnum( "SMALL_CHAR" );
            else if( m_DecalType == decal_mgr::MED_CHAR )
                PropQuery.SetVarEnum( "MED_CHAR" );
            else if( m_DecalType == decal_mgr::BIG_CHAR )
                PropQuery.SetVarEnum( "BIG_CHAR" );
            else
                ASSERT( 0 );
        }
        else
        {
            if( !x_stricmp( "SMALL_CHAR", PropQuery.GetVarEnum()) ) 
                m_DecalType = decal_mgr::SML_CHAR;
            else if( !x_stricmp( "MED_CHAR", PropQuery.GetVarEnum()) )
                m_DecalType = decal_mgr::MED_CHAR;
            else if( !x_stricmp( "BIG_CHAR", PropQuery.GetVarEnum()) )
                m_DecalType = decal_mgr::BIG_CHAR;
            else
                ASSERT( 0 );   
        }
        return TRUE;
    }
    */

    // External Particle.
    if( PropQuery.IsVar( "Explosive Projectile\\Particles Resource" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ExplosionFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ExplosionFx.SetName( pString );                

                // Load the particle effect.
                if( m_ExplosionFx.IsLoaded() == FALSE )
                    m_ExplosionFx.GetPointer();
            }
        }
        return( TRUE );
    }

    if( PropQuery.VarFloat( "Explosive Projectile\\Pain Radius", m_PainRadius ) )
        return TRUE;
    
    if( PropQuery.VarFloat( "Explosive Projectile\\Pain Amount", m_PainAmount ) )
        return TRUE;
    
    return FALSE;
}

//=============================================================================

void explosive_bullet_projectile::OnBulletHit( collision_mgr::collision& rColl, const vector3& HitPos )
{
    (void)HitPos;

    // create a char decal
    decal_package* pPackage = m_hDecalPackage.GetPointer();
    if ( pPackage )
    {
        const decal_definition* pDef = pPackage->GetDecalDef( "CharMarks", 0 );
        if ( pDef )
        {
            g_DecalMgr.CreateDecalAtPoint( *pDef,
                                           rColl.Point,
                                           rColl.Plane.Normal,
                                           vector2(m_ExplosionSize, m_ExplosionSize),
                                           pDef->RandomRoll() );
        }
    }

    // Create the paricle effect.
    particle_emitter::CreatePresetParticleAndOrient( m_ExplosionFx.GetName(), rColl.Plane.Normal, 
                                                    rColl.Point, GetZone1() );
    
    //BroadcastPain();

    // Destroy the object.
    guid Guid = GetGuid() ;

    //destroy the projectile.
    g_ObjMgr.DestroyObject( Guid ) ;
}

//=============================================================================
