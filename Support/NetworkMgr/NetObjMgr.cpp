//==============================================================================
//
//  NetObjMgr.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_debug.hpp"
#include "x_log.hpp"
#include "NetworkMgr.hpp"
#include "NetObjMgr.hpp"
#include "PainQueue.hpp"
#include "Obj_Mgr/Obj_Mgr.hpp"
#include "Objects/Pickup.hpp"
#include "Objects/Flag.hpp"
#include "Objects/FlagBase.hpp"
#include "Objects/CapPoint.hpp"
#include "Objects/GrenadeProjectile.hpp"
#include "Objects/ProjectileEnergy.hpp"
#include "Objects/JumpingBeanProjectile.hpp"
#include "Objects/ProjectileMesonSeeker.hpp"
#include "Objects/GravChargeProjectile.hpp"
#include "Objects/ProjectileMutantContagion.hpp"
#include "Objects/ProjectileMutantTendril.hpp"
#include "Objects/ProjectileMutantParasite2.hpp"
#include "Debris/Debris_Meson_Lash.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

netobj_mgr      NetObjMgr;
xbool           netobj_mgr::m_Initialized = FALSE;

//==============================================================================
//  TYPE INFO FUNCTIONS
//==============================================================================

void netobj_mgr::type_info::Setup( netobj::type aType,
                                   char*        apTypeName )
{
    Type            = aType;
    pTypeName       = apTypeName;

    InstanceCount   = 0;        // Stat
    MaxInstances    = 0;        // Stat
}

//==============================================================================

void netobj_mgr::type_info::Clear( void )
{
    InstanceCount   = 0;
    MaxInstances    = 0;        // Stat
}

//==============================================================================
//  NET INFO FUNCTIONS
//==============================================================================

void netobj_mgr::net_info::Clear( void )
{
    SampleIndex = 0;

    for( s32 i = 0; i < OBJ_NET_INFO_SAMPLES; i++ )
    {
        Updates   [i] = 0;
        UpdateBits[i] = 0;    
    }
}

//==============================================================================

void netobj_mgr::net_info::GetStats( s32& aUpdates, s32& aUpdateBits )
{
    aUpdates    = 0;
    aUpdateBits = 0;

    for( s32 i = 0; i < OBJ_NET_INFO_SAMPLES; i++ )
    {
        aUpdates    += Updates   [i];
        aUpdateBits += UpdateBits[i];
    }
}

//==============================================================================

void netobj_mgr::net_info::AddUpdate( s32 NBits )
{
    Updates   [ SampleIndex ] += 1;
    UpdateBits[ SampleIndex ] += NBits;
}

//==============================================================================

void netobj_mgr::net_info::NextSample( void )
{
    SampleIndex = (SampleIndex+1) % OBJ_NET_INFO_SAMPLES;

    // Clear sample counts.
    Updates   [ SampleIndex ] = 0;
    UpdateBits[ SampleIndex ] = 0;
}

//==============================================================================
//  NET OBJ MGR FUNCTIONS
//==============================================================================

netobj_mgr::netobj_mgr( void )
{
    ASSERT( !m_Initialized );
    m_Initialized = TRUE;

    //
    // Must do this with ALL net aware types.
    //
    m_TypeInfo[ netobj::TYPE_PLAYER        ].Setup( netobj::TYPE_PLAYER        , "PLAYER"     );
    m_TypeInfo[ netobj::TYPE_PICKUP        ].Setup( netobj::TYPE_PICKUP        , "PICKUP"     );
    m_TypeInfo[ netobj::TYPE_FLAG          ].Setup( netobj::TYPE_FLAG          , "FLAG"       );
    m_TypeInfo[ netobj::TYPE_FLAG_BASE     ].Setup( netobj::TYPE_FLAG_BASE     , "FLAG BASE"  );
    m_TypeInfo[ netobj::TYPE_CAP_POINT     ].Setup( netobj::TYPE_CAP_POINT     , "CAP POINT"  );
    m_TypeInfo[ netobj::TYPE_GRENADE       ].Setup( netobj::TYPE_GRENADE       , "GRENADE"    );
    m_TypeInfo[ netobj::TYPE_JBEAN_GRENADE ].Setup( netobj::TYPE_JBEAN_GRENADE , "JB GRENADE" );
    m_TypeInfo[ netobj::TYPE_MESON_1ST     ].Setup( netobj::TYPE_MESON_1ST     , "MESON 1ST"  );
    m_TypeInfo[ netobj::TYPE_MESON_2ND     ].Setup( netobj::TYPE_MESON_2ND     , "MESON 2ND"  );
    m_TypeInfo[ netobj::TYPE_CONTAGION     ].Setup( netobj::TYPE_CONTAGION     , "CONTAGION"  );
    m_TypeInfo[ netobj::TYPE_TENDRIL       ].Setup( netobj::TYPE_TENDRIL       , "TENDRIL"    );
    m_TypeInfo[ netobj::TYPE_BBG_1ST       ].Setup( netobj::TYPE_BBG_1ST       , "BBG 1ST"    );
    m_TypeInfo[ netobj::TYPE_PARASITE      ].Setup( netobj::TYPE_PARASITE      , "PARASITE"   );
    m_TypeInfo[ netobj::TYPE_MESON_EXPL    ].Setup( netobj::TYPE_MESON_EXPL    , "MESON EXPL" );

    Initialize();
}

//==============================================================================

netobj_mgr::~netobj_mgr( void )
{
}

//==============================================================================

void netobj_mgr::Clear( void )
{
    LOG_MESSAGE( "netobj_mgr::Clear", "" );
    Initialize();
}

//==============================================================================

void netobj_mgr::Initialize( void )
{
    m_SlotCursor = 32;
    m_Reserved   = -1;

    for( s32 i = netobj::TYPE_START; i < netobj::TYPE_END; i++ )
        m_TypeInfo[i].Clear();

    for( s32 i = 0; i < NET_MAX_OBJECTS; i++ )
    {
        m_GameSlot[i]    = -1;
        m_JustRemoved[i] = FALSE;
    }
}

//==============================================================================

s32 netobj_mgr::AddObject( netobj* pNetObj )
{
    ASSERT( m_Initialized );
    ASSERT( pNetObj );

    //***
    //*** Use the following code to stress the networking.
    //***
    if( FALSE )
    {
        // Rather than allocate slots in a circular pattern, always select the
        // first available slot.  This is very bad and will stress the net code.
        m_SlotCursor = -1;
    }

    s32          Sanity;
    s32          Slot   = -1;
    s32          Client = g_NetworkMgr.GetClientIndex();
    netobj::type Type   = pNetObj->net_GetType();

    if( (Type == netobj::TYPE_PLAYER) && (m_Reserved == -1) )
    {
        LOG_ERROR( "netobj_mgr::AddObject",
                   "Level contains an exported player object." );

        // ASSERTS( FALSE, "Player can't be in level load." );    
        
        // IF YOU ARE HERE, then read the following...

        // It is no longer allowable to place "player objects" in the editor.
        // Instead, you should place a "spawn point object".  I regret any 
        // inconvenience this change may cause, but it is a necessary change
        // in order to move forward with the online material.

        // As a public service, the following code should allow you to recover
        // from this error.  This is only a temporary measure.  Do not rely
        // on it.  And do not remove the ASSERT.  We need to update the levels,
        // not cripple the code.

        return( -1 );
    }

    ASSERT( IN_RANGE( netobj::TYPE_START, Type, netobj::TYPE_END-1 ) );
    ASSERT( m_TypeInfo[Type].Type == Type );

    // Select a slot for this object.
    if( m_Reserved == -1 )
    {
        s32 Lo, Hi;

        if( Client == -1 )
        {
            // Server.
            Lo = 32;
            Hi = NET_MAX_OBJECTS_ON_SERVER;
        }
        else
        {
            Lo = NET_MAX_OBJECTS_ON_SERVER + (Client * NET_MAX_OBJECTS_ON_CLIENT);
            Hi = NET_MAX_OBJECTS_ON_CLIENT + Lo;
        }        

        if( !IN_RANGE( Lo, m_SlotCursor, Hi ) )
            m_SlotCursor = Lo;

        //
        // Find an available slot.
        //

        Sanity = Hi - Lo;
        while( (Slot == -1) && (Sanity > 0) )
        {
            if( m_GameSlot[ m_SlotCursor ] == -1 )
            {
                Slot = m_SlotCursor;
                // Don't break out.  Need to increment the slot cursor below.
            }
            else
            {
                Sanity--;
            }

            m_SlotCursor += 1;
            if( m_SlotCursor >= Hi )
                m_SlotCursor  = Lo;
        }

        // If we don't have a slot here, then all slots are full.  This is a
        // problem.  This function call MUST succeed.  So, we're going to have
        // to nuke a previously existing object.  Since the slot cursor searches
        // circularly, the oldest object is probably under the slot cursor right
        // now.

        if( Slot == -1 )
        {
            Slot = m_SlotCursor;

            m_SlotCursor += 1;
            if( m_SlotCursor >= Hi )
                m_SlotCursor  = Lo;

            DestroyObject( Slot );
        }
    }
    else
    {
        Slot       = m_Reserved;
        m_Reserved = -1;
    }

    // Validate the slot.
    ASSERT( IN_RANGE( 0, Slot, NET_MAX_OBJECTS-1 ) );
    ASSERT( m_GameSlot[ Slot ] == -1 );

    // Add the object to local record keeping.
    m_GameSlot[ Slot ] = pNetObj->net_GetGameSlot();
    #ifndef X_RETAIL
    m_pObject [ Slot ] = NULL;
    m_pNetObj [ Slot ] = pNetObj;
    #endif

    // Set the assigned slot into the object.
    pNetObj->net_SetSlot( Slot );

    // Set the owning client into the object.
    if( IN_RANGE( 0, Slot, 31 ) )
    {
        // Player object.  Owned by the locally controlling client.
        pNetObj->net_SetOwningClient( g_NetworkMgr.GetClientIndex( Slot ) );
    }
    else
    {
        // Non player object.  Ownership can be derived from the slot.
        if( Slot < NET_MAX_OBJECTS_ON_SERVER )
            Client = -1;
        else
            Client = (Slot - NET_MAX_OBJECTS_ON_SERVER) / NET_MAX_OBJECTS_ON_CLIENT;

        pNetObj->net_SetOwningClient( Client );
    }

    /*
    LOG_MESSAGE( "netobj_mgr::AddObject", 
                 "Slot:%d - Type:%s(%d) - OwningClient:%d",
                 Slot, m_TypeInfo[Type].pTypeName, Type,
                 pNetObj->net_GetOwningClient() );
    */

    // Activate the object.
    pNetObj->net_Activate(); 

    return( Slot );
}

//==============================================================================

netobj* netobj_mgr::CreateObject( netobj::type Type )
{
    guid               GUID    = 0;
    object*            pObject = NULL;
    const object_desc* pDesc   = NULL;

    switch( Type )
    {
    case netobj::TYPE_PLAYER:
        // Can't do players in here at the moment.
        ASSERT( FALSE );
        break;

    case netobj::TYPE_PICKUP:
        pDesc = &pickup::GetObjectType();
        break;

    case netobj::TYPE_FLAG:
        pDesc = &flag::GetObjectType();
        break;

    case netobj::TYPE_FLAG_BASE:
        pDesc = &flag_base::GetObjectType();
        break;

    case netobj::TYPE_CAP_POINT:
        pDesc = &cap_point::GetObjectType();
        break;

    case netobj::TYPE_GRENADE:
        pDesc = &grenade_projectile::GetObjectType();
        break;

    case netobj::TYPE_JBEAN_GRENADE:
        pDesc = &jumping_bean_projectile::GetObjectType();
        break;

    case netobj::TYPE_BBG_1ST:
        pDesc = &energy_projectile::GetObjectType();
        break;

    case netobj::TYPE_MESON_1ST:
        pDesc = &grav_charge_projectile::GetObjectType();
        break;

    case netobj::TYPE_MESON_2ND:
        pDesc = &mesonseeker_projectile::GetObjectType();
        break;

    case netobj::TYPE_CONTAGION:
        pDesc = &mutant_contagion_projectile::GetObjectType();
        break;

    case netobj::TYPE_TENDRIL:
        pDesc = &mutant_tendril_projectile::GetObjectType();
        break;

    case netobj::TYPE_PARASITE:
        pDesc = &mutant_parasite_projectile::GetObjectType();
        break;

    case netobj::TYPE_MESON_EXPL:
        pDesc = &debris_meson_explosion::GetObjectType();
        break;
    }    

    GUID = g_ObjMgr.CreateObject( *pDesc );
    ASSERT( GUID );

    pObject = (object*)g_ObjMgr.GetObjectByGuid( GUID );
    ASSERT( pObject );    

    pObject->AsNetObj()->m_NetType = Type;
    return( pObject->AsNetObj() );
}

//==============================================================================

netobj* netobj_mgr::CreateAddObject( netobj::type Type )
{
    netobj* pResult = CreateObject( Type );
    AddObject( pResult );
    return( pResult );
}

//==============================================================================

void netobj_mgr::DestroyObject( s32 Slot )
{
    ASSERT( m_Initialized );
    ASSERT( m_GameSlot[ Slot ] != -1 );

    #ifndef X_RETAIL
    {
        //
        // This version has extra debugging and checking built in.  When 
        // updating this code, be sure to update the corresponding "final" code
        // below.
        //

        m_pObject[ Slot ] = g_ObjMgr.GetObjectBySlot( m_GameSlot[ Slot ] );
        ASSERT( m_pObject[ Slot ] );

        m_pNetObj[ Slot ] = m_pObject[ Slot ]->AsNetObj();
        ASSERT( m_pNetObj[ Slot ] );

        #ifdef X_ASSERT
        netobj::type Type = m_pNetObj[ Slot ]->net_GetType();
        ASSERT( IN_RANGE( netobj::TYPE_START, Type, netobj::TYPE_END-1 ) );

        /*
        LOG_MESSAGE( "netobj_mgr::DestroyObject", 
                     "Slot:%d - Type:%s(%d) - OwningClient:%d",
                     Slot, m_TypeInfo[Type].pTypeName, Type, 
                     m_pNetObj[ Slot ]->net_GetOwningClient() );
        */
        #endif // X_ASSERT

        m_pNetObj[ Slot ]->net_Deactivate();
        g_ObjMgr.DestroyObject( m_pObject[ Slot ]->GetGuid() );

        m_JustRemoved[ Slot ] = TRUE;
        m_GameSlot   [ Slot ] = -1;
        m_pObject    [ Slot ] = NULL;
        m_pNetObj    [ Slot ] = NULL;
    }
    #else
    {
        //
        // This is the "real" version.  When updating this code, be sure to
        // update the corresponding debug code above.
        //

        object* pObject = g_ObjMgr.GetObjectBySlot( m_GameSlot[ Slot ] );
        if( pObject )
        {
            netobj* pNetObj = pObject->AsNetObj();
            pNetObj->net_Deactivate();
            g_ObjMgr.DestroyObject( pObject->GetGuid() );
        }

        m_JustRemoved[ Slot ] = TRUE;
        m_GameSlot   [ Slot ] = -1;
    }
    #endif
}

//==============================================================================

void netobj_mgr::ReserveSlot( s32 Slot )
{
    ASSERT( m_Initialized );
    ASSERT( m_GameSlot[ Slot ] == -1 );
    ASSERT( m_Reserved         == -1 );

    m_Reserved = Slot;
}

//==============================================================================

netobj* netobj_mgr::GetObjFromSlot( s32 Slot )
{
    ASSERT( m_Initialized );
    ASSERT( IN_RANGE( 0, Slot, NET_MAX_OBJECTS-1 ) );

    object* pObject = g_ObjMgr.GetObjectBySlot( m_GameSlot[Slot] );

    if( pObject )
    {
        netobj* pNetObj   = pObject->AsNetObj();

        #ifndef X_RETAIL
        m_pObject[ Slot ] = pObject;
        m_pNetObj[ Slot ] = pNetObj;
        #endif

        return( pNetObj );
    }

    return( NULL );
}

//==============================================================================

netobj_mgr::type_info& netobj_mgr::GetTypeInfo( netobj::type Type )
{
    ASSERT( IN_RANGE( netobj::TYPE_START, Type, netobj::TYPE_END-1 ) );
    return( m_TypeInfo[Type] );
}

//==============================================================================

netobj_mgr::net_info& netobj_mgr::GetNetInfo( netobj::type Type )
{
    ASSERT( IN_RANGE( netobj::TYPE_START, Type, netobj::TYPE_END-1 ) );
    return( m_NetInfo[Type] );
}

//==============================================================================

void netobj_mgr::Logic( f32 DeltaTime )
{
    s32 i;

    for( i = netobj::TYPE_START; i < netobj::TYPE_END; i++ )
    {
        m_TypeInfo[i].InstanceCount = 0;
    }

    for( i = 0; i < NET_MAX_OBJECTS; i++ )
    {
        if( m_GameSlot[i] != -1 )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( m_GameSlot[i] );
            ASSERT( pObject );

            netobj* pNetObj = pObject->AsNetObj();
            ASSERT( pNetObj );

            #ifndef X_RETAIL
            m_pObject[i] = pObject;            
            m_pNetObj[i] = pNetObj;
            #endif

            pNetObj->net_Logic( DeltaTime );

            // Object is not allowed to call NetObjMgr.DestroyObject( self ).
            if( pNetObj->net_GetDirtyBits() & netobj::DEACTIVATE_BIT )
            {
                // Object flagged itself as deactivated.  Destroy it.
                DestroyObject( i );
            }
            else
            {
                netobj::type Type = pNetObj->net_GetType();
                m_TypeInfo[Type].InstanceCount++;
                m_TypeInfo[Type].MaxInstances = MAX( m_TypeInfo[Type].MaxInstances,
                                                     m_TypeInfo[Type].InstanceCount );
            }
        }
    }                               
}

//==============================================================================

void netobj_mgr::Render( void )
{
    #ifndef X_RETAIL

    for( s32 i = 0; i < NET_MAX_OBJECTS; i++ )
    {
        if( m_GameSlot[i] != -1 )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( m_GameSlot[i] );
            ASSERT( pObject );

            netobj* pNetObj = pObject->AsNetObj();
            ASSERT( pNetObj );

            m_pObject[i] = pObject;            
            m_pNetObj[i] = pNetObj;

        //  if( pNetObj->net_IsActive() )
        //  {
        //      pNetObj->net_DebugRender();
        //  }
        }
    }

    #endif
}

//==============================================================================

void netobj_mgr::ApplyNetPain( net_pain& NetPain )
{
    netobj* pNetObj = GetObjFromSlot( NetPain.Victim );
    if( pNetObj )
        pNetObj->net_ApplyNetPain( NetPain );
}

//==============================================================================

xbool netobj_mgr::JustRemoved( s32 Slot )
{
    xbool Result = m_JustRemoved[Slot];
    m_JustRemoved[Slot] = FALSE;
    return( Result );
}

//==============================================================================

void netobj_mgr::ClientExitGame( s32 Client )
{
    if( Client == -1 )
        return;

    s32 Lo = NET_MAX_OBJECTS_ON_SERVER + (NET_MAX_OBJECTS_ON_CLIENT * Client);
    s32 Hi = Lo + NET_MAX_OBJECTS_ON_CLIENT;

    for( s32 i = Lo; i < Hi; i++ )
    {
        if( m_GameSlot[ i ] != -1 )
            DestroyObject( i );            
    }
}

//==============================================================================
