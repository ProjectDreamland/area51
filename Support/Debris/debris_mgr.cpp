//=============================================================================
//  DEBRIS_MGR.CPP 
//=============================================================================
//=============================================================================
// INCLUDES
//=============================================================================

#include "Debris\debris_mgr.hpp"
#include "Objects\PropSurface.hpp"
#include "Debris\debris_rigid.hpp"
#include "Debris\debris_glass_cluster.hpp"
#include "Debris\debris_frag_explosion.hpp"
#include "Debris\debris_alien_grenade_explosion.hpp"
#include "Debris\debris_meson_lash.hpp"
#include "audiomgr\AudioMgr.hpp"
#include "..\Support\GameLib\StatsMgr.hpp"
#include "Dictionary\Global_Dictionary.hpp"

//=============================================================================
// CONSTANTS
//=============================================================================
static const f32        k_MaxDebrisVelocity     = 800;

//=============================================================================
// EXTERNAL
//=============================================================================
debris_mgr* debris_mgr::m_sThis = NULL;

//=============================================================================

debris_mgr::debris_mgr(void)
{
    ASSERT(m_sThis == NULL);
    m_sThis = this;

}

//=============================================================================

debris_mgr::~debris_mgr()
{

}

//=============================================================================

void debris_mgr::CreateDebrisFromObject(    guid thisGuid, 
                                            s32 numberOfDebrisParticles, 
                                            f32 energyLevel )
{
    LOG_STAT( k_stats_Debris );

    object* tempObject = g_ObjMgr.GetObjectByGuid( thisGuid );

    if(tempObject == NULL )
    {
        return;
    }

    bbox tempBbox = tempObject->GetBBox();

    s32 count;
    for ( count = 0; count < numberOfDebrisParticles; count++ )
    {
    
        guid    tempDebrisGuid =  g_ObjMgr.CreateObject( "Debris_Rigid" );
        object *tempDebris = g_ObjMgr.GetObjectByGuid( tempDebrisGuid );
        vector3 tempPos, tempVelocity ;
        f32 x,y,z;

        x = x_frand( tempBbox.Min.GetX() , tempBbox.Max.GetX() );
        y = x_frand( tempBbox.Min.GetY() , tempBbox.Max.GetY() );
        z = x_frand( tempBbox.Min.GetZ() , tempBbox.Max.GetZ() );
        tempPos.Set( x, y, z ); 

        x = x_frand( energyLevel* -1.0f, energyLevel );
        y = x_frand( energyLevel* -1.0f, energyLevel );
        z = x_frand( energyLevel* -1.0f, energyLevel );
        tempVelocity.Set( x, y, z );

        CapVelocity( tempVelocity );

        ((debris*)tempDebris)->Create( PRELOAD_FILE("WPN_FRG_Bindpose.rigidgeom"), tempPos, tempVelocity, 6.0f, FALSE );

    }

}

//=============================================================================

void debris_mgr::CreateDebrisFromObject(    rigid_inst& Inst,
                                            guid thisGuid, 
                                            s32 numberOfDebrisParticles, 
                                            f32 energyLevel )
{
    LOG_STAT( k_stats_Debris );

    object* tempObject = g_ObjMgr.GetObjectByGuid( thisGuid );

    if(tempObject == NULL )
    {
        return;
    }

    bbox tempBbox = tempObject->GetBBox();

    s32 count;
    for ( count = 0; count < numberOfDebrisParticles; count++ )
    {
    
        guid    tempDebrisGuid =  g_ObjMgr.CreateObject( "Debris_Rigid" );
        object *tempDebris = g_ObjMgr.GetObjectByGuid( tempDebrisGuid );
        vector3 tempPos, tempVelocity ;
        f32 x,y,z;    

        x = x_frand( tempBbox.Min.GetX() , tempBbox.Max.GetX() );
        y = x_frand( tempBbox.Min.GetY() , tempBbox.Max.GetY() );
        z = x_frand( tempBbox.Min.GetZ() , tempBbox.Max.GetZ() );
        tempPos.Set( x, y, z ); 

        x = x_frand( energyLevel* -1.0f, energyLevel );
        y = x_frand( energyLevel* -1.0f, energyLevel );
        z = x_frand( energyLevel* -1.0f, energyLevel );
        tempVelocity.Set( x, y, z );

        CapVelocity( tempVelocity );

        if ( Inst.GetGeom())
        {
            ((debris*)tempDebris)->Create( Inst, tempPos, tempVelocity, 6.0f, FALSE );
        }
        else
        {
            ((debris*)tempDebris)->Create( PRELOAD_FILE("DEB_CONC_Med.rigidgeom"), tempPos, tempVelocity, 6.0f, FALSE );
        }
    }

}

//=============================================================================

void debris_mgr::CreateGlassFromRigidGeom ( play_surface* pPlaySurface, const pain* pPain )
{
    LOG_STAT( k_stats_Debris );
    guid gObj = g_ObjMgr.CreateObject( debris_glass_cluster::GetObjectType() );
    object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
    if (NULL != pObj)
    {
        if (pObj->IsKindOf( debris_glass_cluster::GetRTTI()))
        {
            debris_glass_cluster& DGC = debris_glass_cluster::GetSafeType( *pObj );

            DGC.CreateFromRigidGeom( pPlaySurface, pPain );
        }
    }

    g_AudioMgr.PlayVolumeClipped( "GlassBreak", pPain->GetPosition(), pPlaySurface->GetZone1(), TRUE );
}

//=============================================================================

vector3 g_DebrisVelocity;
vector3 g_DebrisOffset;

//=============================================================================

void debris_mgr::CreateDebris( const vector3& InitPos, u32 Zones, const vector3& InitVelocity, const char* rigidFileName, f32 Life, xbool Bounce, u32 VMeshMask, s32 BounceSoundID )
{
    (void)BounceSoundID;
    LOG_STAT( k_stats_Debris );

    rhandle<rigid_geom> RGeom;
    RGeom.SetName( rigidFileName );

    rigid_geom* pGeom = RGeom.GetPointer();
    if( pGeom == NULL )
        return;

    for( s32 i = 0; i < pGeom->m_nVirtualMeshes; i++)
    {
        u32 Bit = 1 << i;
        if( !(VMeshMask & Bit) )
            continue;

        //  First we make a debris object
        guid Guid           = g_ObjMgr.CreateObject( debris_rigid::GetObjectType() );
        object* pObject     = g_ObjMgr.GetObjectByGuid( Guid );
        
        if( !pObject )
            return;

        if( !pObject->IsKindOf(debris_rigid::GetRTTI() ) )
            return;
    
        pObject->SetZones( Zones );
        debris_rigid& Debris = debris_rigid::GetSafeType( *pObject );
    
        vector3 TempVelocity = InitVelocity;
        CapVelocity( TempVelocity );
        Debris.SetBounceSoundID( BounceSoundID );
        Debris.Create( rigidFileName, InitPos, TempVelocity, Life, Bounce );
        Debris.SetVMeshMask( Bit );

        const radian3 Rotation( InitVelocity.GetPitch(), InitVelocity.GetYaw(), 0.0f );
        Debris.SetInitialRotation( Rotation );
    }
}

//=============================================================================

void debris_mgr::CreateDebris( const vector3& InitPos, u32 Zones, const vector3& InitVelocity, rigid_inst& Inst, f32 Life, xbool Bounce, u32 VMeshMask )
{
    LOG_STAT( k_stats_Debris );

    geom* pGeom = Inst.GetGeom();
    if( pGeom == NULL )
        return;

    for( s32 i = 0; i < pGeom->m_nVirtualMeshes; i++)
    {
        u32 Bit = 1 << i;
        if( !(VMeshMask & Bit) )
            continue;

        //  First we make a debris object
        guid    Guid     = g_ObjMgr.CreateObject( debris_rigid::GetObjectType() );
        object* pObject  = g_ObjMgr.GetObjectByGuid( Guid );
        
        if( !pObject )
            return;

        if( !pObject->IsKindOf(debris_rigid::GetRTTI() ) )
            return;

        pObject->SetZones( Zones );
        debris_rigid& Debris = debris_rigid::GetSafeType( *pObject );

        vector3 Vel = InitVelocity;
        CapVelocity( Vel );

        Debris.Create( Inst, InitPos, Vel, Life, Bounce );
        Debris.SetVMeshMask( Bit );

        const radian3 Rotation( InitVelocity.GetPitch(), InitVelocity.GetYaw(), 0.0f );
        Debris.SetInitialRotation( Rotation );
    }
}

//=============================================================================

void debris_mgr::CreateDebris( const vector3& InitPos, u32 Zones, const vector3& InitVelocity, const radian3& RandRot,       
                               rigid_inst& Inst, f32 Life, xbool Bounce, u32 VMeshMask )
{
    LOG_STAT( k_stats_Debris );

    geom* pGeom = Inst.GetGeom();
    if( pGeom == NULL )
        return;

    for( s32 i = 0; i < pGeom->m_nMeshes; i++)
    {
        u32 Bit = 1 << i;
        if( !(VMeshMask & Bit) )
            continue;

        //  First we make a debris object
        guid    Guid     = g_ObjMgr.CreateObject( debris_rigid::GetObjectType() );
        object* pObject  = g_ObjMgr.GetObjectByGuid( Guid );
        
        if( !pObject )
            return;

        if( !pObject->IsKindOf(debris_rigid::GetRTTI() ) )
            return;
    
        pObject->SetZones( Zones );
        debris_rigid& Debris = debris_rigid::GetSafeType( *pObject );
        vector3 Velocity( InitVelocity );

        Velocity.Rotate( RandRot );

        CapVelocity( Velocity );

        Debris.Create( Inst, InitPos, Velocity, Life, Bounce );
        Debris.SetVMeshMask( Bit );

        const radian3 Rotation( InitVelocity.GetPitch(), InitVelocity.GetYaw(), 0.0f );
        Debris.SetInitialRotation( Rotation );
    }
}

//=============================================================================

void debris_mgr::CreateShell ( const vector3& InitPos, u32 Zones, const radian3& InitRot, const char* rigidFileName, f32 Life, u32 VMeshMask )
{
    LOG_STAT( k_stats_Debris );

    vector3 g_BullVelocity;
    vector3 g_BullOffset;
    g_BullOffset.Set( 0.0f, -12.0f , 0.0f );
    g_BullVelocity.Set(0.0f, 200.0f, 0.0f );
    
    //  First we make a debris object
    guid Guid       = g_ObjMgr.CreateObject( debris_rigid::GetObjectType() );
    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    if( !pObject )
        return;

    if( !pObject->IsKindOf( debris_rigid::GetRTTI() ) )
        return;

    pObject->SetZones( Zones );
    debris_rigid& Debris = debris_rigid::GetSafeType(*pObject);

    //  The position we receive is the position of the bullet's creation as is the rotation

    matrix4 debrisMat;
    debrisMat.Identity  ( );
    debrisMat.Translate ( InitPos );
    debrisMat.Rotate    ( InitRot );

    Debris.SetInitialRotation( InitRot );
    vector3 tempVec = g_BullOffset;
    tempVec.Rotate( InitRot );
    vector3 tempPos = InitPos;
    tempPos += tempVec;
    vector3 initVelocity;
    initVelocity.Zero();
    initVelocity.GetX() = -200.0f;
    initVelocity.GetZ() = 200.0f;
    initVelocity += g_BullVelocity;
    radian3 tempRot = InitRot;
    {
        f32 x = x_frand(-R_5,R_5);
        f32 y = x_frand(-R_5,R_5);
        f32 z = x_frand(-R_5,R_5);
        tempRot +=  radian3( x, y, z );
    }
    initVelocity.Rotate( tempRot );

    Debris.IgnoreLiving();
    Debris.SetBounceSoundID( g_StringMgr.Add("SMP_ShellDropConcrete") );
    {
        f32 x = x_frand(-R_20,R_20);
        f32 y = x_frand(-R_20,R_20);
        f32 z = x_frand(-R_20,R_20);
        Debris.SetSpin( radian3( x, y, z ));
    }
    Debris.Create( rigidFileName, tempPos, initVelocity, Life, TRUE );
    Debris.SetVMeshMask( VMeshMask );

    const radian3 Rotation( initVelocity.GetPitch(), initVelocity.GetYaw(), 0.0f );
    Debris.SetInitialRotation( Rotation );
}

//=============================================================================

void debris_mgr::CapVelocity( vector3& Velocity )
{
    if (Velocity.LengthSquared() > (k_MaxDebrisVelocity*k_MaxDebrisVelocity))
    {
        Velocity.NormalizeAndScale( k_MaxDebrisVelocity );
    }
}

//=============================================================================

void debris_mgr::CreateSpecializedDebris ( const vector3&    InitPos, 
                                           const vector3&    InitVelocity, 
                                           object::type      Type,
                                           u32               Zones,
                                           guid              OwnerGuid )
{
    switch( Type )
    {
    case object::TYPE_DEBRIS_FRAG_EXPLOSION:
        {
            guid gObj = g_ObjMgr.CreateObject( debris_frag_explosion::GetObjectType().GetTypeName() );
            object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
            if (NULL == pObj)
                return;

            debris_frag_explosion& pExp = debris_frag_explosion::GetSafeType( *pObj );

            pExp.Create( PRELOAD_FILE("DEB_metal_bits_000.rigidgeom"), InitPos, Zones, InitVelocity, 16 );
        }        
        break;
    case object::TYPE_DEBRIS_ALIEN_GRENADE_EXPLOSION:
        {
            guid gObj = g_ObjMgr.CreateObject( debris_alien_grenade_explosion::GetObjectType().GetTypeName() );
            object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
            if (NULL == pObj)
                return;

            debris_alien_grenade_explosion& pExp = debris_alien_grenade_explosion::GetSafeType( *pObj );

            pExp.Create( PRELOAD_FILE("DEB_metal_bits_000.rigidgeom"), InitPos, Zones, InitVelocity, 32, OwnerGuid );
        }
        break;
    case object::TYPE_DEBRIS_MESON_EXPLOSION:
        {
            guid gObj = CREATE_NET_OBJECT( debris_meson_explosion::GetObjectType(), netobj::TYPE_MESON_EXPL );
            object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
            if (NULL == pObj)
                return;

            debris_meson_explosion& pExp = debris_meson_explosion::GetSafeType( *pObj );

            pExp.Create( InitPos, InitVelocity, Zones, OwnerGuid );
        }
        break;
    }
}