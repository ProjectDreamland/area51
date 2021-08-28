//==============================================================================
//
//  NetPickup.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetPickup.hpp"
#include "x_bitstream.hpp"
#include "NetworkMgr.hpp"

#include "x_log.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  STORAGE
//==============================================================================

//==============================================================================
//  FUNCTIONS
//==============================================================================

void net_pickup::Activate( void )
{
    m_DirtyBits = ACTIVATE_BIT | STATE_BIT;

    if( !m_Active )
    {
        CreateGameObj();
    }

    m_Active = TRUE;

    LOG_MESSAGE( "net_pickup::Activate", 
                 "Slot:%d - Position:%0.2f,%0.2f,%0.2f - Details:%d,%d,%d,%d", 
                 m_Slot,
                 m_Position.X, m_Position.Y, m_Position.Z,
                 m_Permanent, m_Kind, m_AmmoType, m_AmmoCount );
}

//==============================================================================

void net_pickup::Deactivate( void )
{
    if( m_Active )
    {
        DestroyGameObj();
    }

    m_Active    = FALSE;
    m_DirtyBits = DEACTIVATE_BIT;

    LOG_MESSAGE( "net_pickup::Deactivate", "Slot:%d", m_Slot );
}

//==============================================================================

extern void* inev_CreateDumpObjectBridge( net_pickup* pPickup );

void net_pickup::CreateGameObj( void )
{
    if( m_pObject == NULL )
    {
//        m_pObject = inev_CreateDumpObjectBridge( this );
    }

    LOG_MESSAGE( "net_pickup::CreateGameObj", "ActivePickupCount: %d", NetObjMgr.CountActivePickups() );
}

//==============================================================================

extern void inev_MarkDumpObjectForDestructionBridge( net_pickup* pPickup );

void net_pickup::DestroyGameObj( void )
{
    ASSERT( m_pObject );

//    inev_MarkDumpObjectForDestructionBridge( this );

    m_pObject = NULL;
}

//==============================================================================

void net_pickup::OnAcceptUpdate( const bitstream& BS )
{
    xbool   DoActivate = FALSE;

    if( BS.ReadFlag() )
    {
        Deactivate();
    }

    if( BS.ReadFlag() )
    {
        DoActivate = TRUE;
        BS.ReadVector ( m_Position    );
        BS.ReadRadian3( m_Orientation );
        BS.ReadFlag   ( m_Permanent   );
        BS.ReadS32    ( m_Kind        );
        BS.ReadS32    ( m_AmmoType    );
        BS.ReadS32    ( m_AmmoCount   );
    }

    if( BS.ReadFlag() )
    {
        BS.ReadS32    ( m_State       );
    }

    // Do the activation if necessary
    if( DoActivate )
        Activate();
}

//==============================================================================

void net_pickup::OnProvideUpdate(       bitstream& BS, 
                                        u32&       DirtyBits, 
                                        s32        Client,
                                  const delta*     pDelta )
{
    (void)Client;
    (void)pDelta;

    // Anything to do?
    if( !DirtyBits )
        return;

    if( BS.WriteFlag( DirtyBits & DEACTIVATE_BIT ) )
    {
        DirtyBits &= ~DEACTIVATE_BIT;
    }

    if( BS.WriteFlag( DirtyBits & ACTIVATE_BIT ) )
    {
        DirtyBits &= ~ACTIVATE_BIT;
        BS.WriteVector ( m_Position    );
        BS.WriteRadian3( m_Orientation );
        BS.WriteFlag   ( m_Permanent   );
        BS.WriteS32    ( m_Kind        );
        BS.WriteS32    ( m_AmmoType    );
        BS.WriteS32    ( m_AmmoCount   );
    }

    if( BS.WriteFlag( DirtyBits & STATE_BIT ) )
    {
        DirtyBits &= ~STATE_BIT;
        BS.WriteS32    ( m_State       );
    }
}

//==============================================================================

void net_pickup::Logic( void )
{
    ASSERT( m_Active );

    // The game side version of pickups may take a few frames to be created.
    // So, if we don't have a pointer to it, try to create it...
#if 0
    if( m_pObject == NULL )
        CreateGameObj();
#endif

    if( m_pObject )
    {
        if( g_NetworkMgr.IsServer() )
        {
            // TO DO - See if permanent pickup has changed state.
            //  if( <changed state> )
            //      m_DirtyBits |= STATE_BIT;
        }
        
        if( g_NetworkMgr.IsClient() )
        {
            // TO DO - Poke local net object values into game object.
            // OPTIMIZATION - Only poke the values into game object if there
            // has been a change.  Could possibly use the client side dirty bits
            // for this since they aren't actually used on the client.
        }
    }
}

//==============================================================================

void net_pickup::DebugRender( void ) const
{
}

//==============================================================================

void net_pickup::Setup( const vector3&  Position,
                        const radian3&  Orientation,
                        xbool           Permanent,
                        s32             Kind,       // Ammo, health, weapon(?)    
                        s32             AmmoType,
                        s32             AmmoCount,
                        void*           pScript )
{
    m_Position    = Position;
    m_Orientation = Orientation;
    m_Permanent   = Permanent;
    m_Kind        = Kind;
    m_AmmoType    = AmmoType;
    m_AmmoCount   = AmmoCount;
    m_pScript     = pScript;
}

//==============================================================================
