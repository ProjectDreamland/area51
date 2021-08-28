//=========================================================================
// BULLET PROJECTILE
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "ProjectileBullett.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "gamelib\StatsMgr.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "objects\Player.hpp"

//=========================================================================
// OBJECT DESC.
//=========================================================================
static struct bullet_projectile_desc : public object_desc
{
    bullet_projectile_desc( void ) : object_desc( 
            object::TYPE_BULLET_PROJECTILE, 
            "Bullet", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_NO_RUNTIME_SAVE,
            FLAGS_IS_DYNAMIC 
            /*| FLAGS_GENERIC_EDITOR_CREATE*/ )
            {}

    virtual object* Create          ( void )
    {
        return new bullet_projectile;
    }

} s_Bullet_Projectile_Desc;

//=============================================================================================

const object_desc&  bullet_projectile::GetTypeDesc     ( void ) const
{
    return s_Bullet_Projectile_Desc;
}


//=============================================================================================
const object_desc&  bullet_projectile::GetObjectType   ( void )
{
    return s_Bullet_Projectile_Desc;
}

//=============================================================================================

bullet_projectile::bullet_projectile()
{
	m_MaxAliveTime = .3f;

    m_bThroughGlass = TRUE;
}

//=============================================================================================

bullet_projectile::~bullet_projectile()
{
}

//=============================================================================================

void bullet_projectile::OnAdvanceLogic( f32 DeltaTime )
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

void bullet_projectile::OnMove( const vector3& rNewPos )
{
	//transfom the object.
    base_projectile::OnMove( rNewPos );    
}

//=============================================================================

bbox bullet_projectile::GetLocalBBox( void ) const 
{ 
    bbox BBox;
    BBox.Set( vector3( -2, -2, -2 ), vector3( 2, 2, 2) );
    return BBox;
}

//=============================================================================================

void bullet_projectile::StartFlyby( void )
{
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );

    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

        // Don't play the bullet fly by sound for the bullets that he fired.
        if( pObject->GetGuid() != m_OwnerGuid )
        {
            ((player*)pObject)->HandleBulletFlyby( *this );
        }

        // Next!
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
}


