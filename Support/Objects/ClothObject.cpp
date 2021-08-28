
#include "ClothObject.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "Objects\BaseProjectile.hpp"
#include "NetworkMgr\NetObj.hpp"
#include "Objects\Actor\Actor.hpp"


//=============================================================================
// DEFINES
//=============================================================================

#define CLOTH_OBJECT_ACTIVE_TIME    5.0f        // Active time after rendering

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

static struct cloth_object_desc : public object_desc
{
    //-------------------------------------------------------------------------

    cloth_object_desc( void ) : object_desc( 
        object::TYPE_CLOTH_OBJECT, 
        "Cloth Object", 
        "PROPS",

            object::ATTR_NEEDS_LOGIC_TIME    |
            object::ATTR_COLLIDABLE          | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS   | 
            object::ATTR_BLOCKS_RAGDOLL      | 
            object::ATTR_BLOCKS_CHARACTER_LOS| 
            object::ATTR_BLOCKS_PLAYER_LOS   | 
            object::ATTR_BLOCKS_SMALL_DEBRIS | 
            object::ATTR_DAMAGEABLE          |
            object::ATTR_RENDERABLE          | 
            object::ATTR_SPACIAL_ENTRY,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new cloth_object ;
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_ClothObject_Desc;


//=============================================================================

const object_desc&  cloth_object::GetTypeDesc( void ) const
{
    return s_ClothObject_Desc;
}

//=============================================================================

const object_desc&  cloth_object::GetObjectType( void )
{
    return s_ClothObject_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

cloth_object::cloth_object( void )
{
#ifdef X_EDITOR    
    m_bDrawDebug = FALSE;
#endif
}

//=============================================================================

cloth_object::~cloth_object( void )
{
}

//=============================================================================

void cloth_object::OnImport( text_in& TextIn )
{
    (void)TextIn;
}

//=============================================================================

void cloth_object::OnInit( void )
{
    // Call base class
    object::OnInit() ;

    // Reset cloth
    m_Cloth.SetObjectGuid( GetGuid() );
    m_Cloth.Reset() ;
    m_ActiveTimer = 0.0f ;
}

//=============================================================================

void cloth_object::OnKill( void )
{
    m_Cloth.Kill();

    // Call base class
    object::OnKill() ;
}

//=============================================================================

bbox cloth_object::GetLocalBBox( void ) const 
{ 
    return m_Cloth.GetLocalBBox() ;
}

//=============================================================================

void cloth_object::OnAdvanceLogic( f32 DeltaTime )
{
    // Is cloth active?
    m_ActiveTimer -= DeltaTime ;
    if (m_ActiveTimer <= 0)
    {
        // Do not update
        m_ActiveTimer = 0 ;
        return ;
    }

    // Selects all objects whose bbox intersects the cloth object
    g_ObjMgr.SelectBBox( object::ATTR_LIVING, GetBBox(), object::TYPE_ALL_TYPES) ;
    slot_id SlotID = g_ObjMgr.StartLoop();
    while(SlotID != SLOT_NULL)
    {
        // Lookup object
        object* pObject = g_ObjMgr.GetObjectBySlot(SlotID) ;
        ASSERT(pObject) ;

        // Only collide with actors
        if (!pObject->IsKindOf( actor::GetRTTI() ))
            continue ;

        // Get actor
        actor& Actor = actor::GetSafeType( *pObject );   

        // Get loco
        loco* pLoco = Actor.GetLocoPointer() ;
        if (!pLoco)
            continue ;

        // Get physics
        character_physics& Physics = pLoco->m_Physics ;

        // Compute capped collision cylinder
        vector3 Bottom = Physics.GetPosition() ;
        vector3 Top    = Bottom ;
        Top.GetY() += Physics.GetColHeight() ;
        f32     Radius = Physics.GetColRadius() * 2.0f ;

        // Collide with the cloth
        m_Cloth.ApplyCappedCylinderColl(Bottom, Top, Radius) ;

        // Check next object
        SlotID = g_ObjMgr.GetNextResult(SlotID) ;
    }
    g_ObjMgr.EndLoop();

    // Update the cloth simulation
    m_Cloth.Advance(DeltaTime) ;

    // Force transform to update
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
}

//=============================================================================

void cloth_object::OnRender( void )
{
    CONTEXT( "cloth_object::OnRender" );

    // Trigger cloth to be active
    m_ActiveTimer = CLOTH_OBJECT_ACTIVE_TIME;

    // Must have geometry
    if (m_Cloth.GetRigidInst().GetGeom() == NULL)
        return ;

    // Compute render flags
    u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
    Flags |= GetRenderMode();

#ifdef TARGET_XBOX

    // Just render the rigid geometry now - the cloth part will get rendered later so fog works
    m_Cloth.RenderRigidGeometry( Flags );
    
#else

    // Render rigid geometry and cloth geometry
    m_Cloth.RenderRigidGeometry( Flags );
    m_Cloth.RenderClothGeometry();
    
#endif


#ifdef X_EDITOR

    // Draw debug info
    if( ( m_bDrawDebug ) || ( GetAttrBits() & ATTR_EDITOR_SELECTED ) )
    {
        m_Cloth.RenderSkeleton() ;
    }        

#endif // X_EDITOR
}

//=============================================================================

#ifdef TARGET_XBOX    

void cloth_object::OnRenderCloth( void )
{
    m_Cloth.RenderClothGeometry();
}

#endif    

//=============================================================================

void cloth_object::OnPain ( const pain& Pain )
{
    // Prepare pain (just use settings from coke can since the values are good)
    Pain.ComputeDamageAndForce( "COKE_CAN", GetGuid(), GetBBox().GetCenter() );

    // Let cloth handle it
    m_Cloth.OnPain( Pain );
}

//=============================================================================

void cloth_object::OnColCheck ( void )
{
    CONTEXT("cloth_object::OnColCheck");    

    // Collide with geometry
    rigid_inst& RigidInst  = m_Cloth.GetRigidInst();
    RigidGeom_ApplyCollision( GetGuid(), 
                              GetBBox(),
                              m_Cloth.GetRenderMask(),
                              &GetL2W(),
                              RigidInst.GetRigidGeom() );

#ifdef X_EDITOR
    // Allow cloth to be selected in the editor?
    if( g_CollisionMgr.IsEditorSelectRay() )
    {
        // Let the cloth do it's thing...
        m_Cloth.OnColCheck( GetGuid(), GetMaterial() );
        return;
    }
#endif // X_EDITOR

    // Get moving object
    guid    MovingGuid = g_CollisionMgr.GetMovingObjGuid() ;
    object* pObject    = g_ObjMgr.GetObjectByGuid(MovingGuid) ;
    
    // Collide with bullets, projectiles, or melee?
    if (        ( pObject ) 
            &&  (       ( pObject->IsKindOf( base_projectile::GetRTTI() ) )     // Normal projectiles
                    ||  ( pObject->IsKindOf( net_proj::GetRTTI() ) )            // Net projectiles
                    ||  ( pObject->IsKindOf( actor::GetRTTI() ) ) ) )           // For melee
    {
        // Let the cloth do it's thing...
        m_Cloth.OnColCheck( GetGuid(), GetMaterial() ) ;
    }
}

//=============================================================================

void cloth_object::OnPolyCacheGather( void )
{
    RigidGeom_GatherToPolyCache( GetGuid(), 
                                 GetBBox(), 
                                 m_Cloth.GetRenderMask(),
                                 &GetL2W(), 
                                 m_Cloth.GetRigidInst().GetRigidGeom() );
}

//=============================================================================

xbool cloth_object::GetColDetails( s32 Key, detail_tri& Tri )
{
    if( Key == -1 )
        return( FALSE );

    rigid_inst& Inst       = m_Cloth.GetRigidInst();
    rigid_geom* pRigidGeom = Inst.GetRigidGeom();
    if( !pRigidGeom )
        return( FALSE );

    if( !pRigidGeom->m_Collision.nHighClusters )
        return( FALSE );

    return RigidGeom_GetColDetails( pRigidGeom,
                                    &GetL2W(),
                                    Inst.GetColorTable(),
                                    Key,
                                    Tri );
}

//=============================================================================

#ifndef X_RETAIL
void cloth_object::OnColRender( xbool bRenderHigh )
{
    m_Cloth.RenderSkeleton() ;

    RigidGeom_RenderCollision( &GetL2W(),
                               m_Cloth.GetRigidInst().GetRigidGeom(),
                               bRenderHigh,
                               m_Cloth.GetRenderMask() );

    draw_BBox( GetBBox() );
}
#endif // X_RETAIL

//=============================================================================

void cloth_object::OnProjectileImpact( const object&    Projectile,
                                       const vector3&   Velocity,
                                             u32        CollPrimKey, 
                                       const vector3&   CollPoint,
                                             xbool      PunchDamageHole,       
                                             f32        ManualImpactForce )                                                                                                   
{
    m_Cloth.OnProjectileImpact( Projectile, Velocity, CollPrimKey, CollPoint, PunchDamageHole, ManualImpactForce );
}

//=============================================================================
/*
void cloth_object::OnColNotify( object& Object )
{
}
*/

//=============================================================================

void cloth_object::SetActive( xbool bActive )
{
    // Activate?
    if( bActive )
        m_ActiveTimer = CLOTH_OBJECT_ACTIVE_TIME;
    else
        m_ActiveTimer = 0.0f;                
}

//=============================================================================

void cloth_object::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove(NewPos) ;

    // Update cloth
    m_Cloth.SetL2W(GetL2W()) ;
}

//=============================================================================

void cloth_object::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform(L2W) ;

    // Update cloth
    m_Cloth.SetL2W(L2W) ;
}

//=============================================================================

void cloth_object::OnEnumProp( prop_enum& List )
{
    // Call base object
    object::OnEnumProp( List );

    // Cloth object properties
    List.PropEnumHeader ( "ClothObject", "Properties of cloth object", 0 );
    
#ifdef X_EDITOR    
    List.PropEnumBool   ( "ClothObject\\DrawDebug", "Display debug info", PROP_TYPE_DONT_SAVE );
#endif

    // Enumerate cloth
    m_Cloth.OnEnumProp( List );
}

//=============================================================================

xbool cloth_object::OnProperty( prop_query& I )
{
    // Call base class
    if( object::OnProperty( I ) )
    {
        return TRUE ;
    }
    
#ifdef X_EDITOR    
    // Cloth object properties
    if ( I.VarBool( "ClothObject\\DrawDebug", m_bDrawDebug ) )
    {
        return TRUE;
    }
#endif

    // Check cloth properties
    if ( m_Cloth.OnProperty( I ) )
    {
        // Was cloth just initialized from geometry?
        if( ( I.IsRead() == FALSE ) && ( I.IsVar( "RenderInst\\File" ) ) )
        {
            // Force bounds to be recomputed
            SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );

            // Make sure polycache is updated to exclude cloth mesh            
            g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
        }
    
        return TRUE;
    }

    return( FALSE );
}

//=============================================================================

