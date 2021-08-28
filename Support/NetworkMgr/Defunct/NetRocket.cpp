//==============================================================================
//
//  NetRocket.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetRocket.hpp"
#include "Inevitable.hpp"
#include "Network\NetBitStream.hpp"

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

void net_rocket::Activate( void )
{
    m_DirtyBits = ACTIVATE_BIT;

#if 0
    if( !m_Active )
    {
        CreateGameObj();
    }
#endif

    m_Active = TRUE;

    LOG_MESSAGE( "net_rocket::Activate", 
                 "Slot:%d - Position:%0.2f,%0.2f,%0.2f - Velocity:%0.2f,%0.2f,%0.2f", 
                 m_Slot,
                 m_Position.X, m_Position.Y, m_Position.Z,
                 m_Velocity.X, m_Velocity.Y, m_Velocity.Z );
}

//==============================================================================

void net_rocket::Deactivate( void )
{
    if( m_Active )
    {
        DestroyGameObj();
    }

    m_Active    = FALSE;
    m_DirtyBits = DEACTIVATE_BIT;

    LOG_MESSAGE( "net_rocket::Deactivate", "Slot:%d", m_Slot );
}

//==============================================================================

//extern void* inev_CreateRocketBridge( net_rocket* pPickup );

void net_rocket::CreateGameObj( void )
{
    if( m_pObject == NULL )
    {
    //  m_pObject = inev_CreateRocketBridge( this );
    }
}

//==============================================================================

//extern void inev_MarkRocketForDestructionBridge( net_rocket* pPickup );

void net_rocket::DestroyGameObj( void )
{
//  ASSERT( m_pObject );

//  inev_MarkRocketForDestructionBridge( this );

    m_pObject = NULL;
}

//==============================================================================

void net_rocket::OnAcceptUpdate( const bitstream& BS )
{
    if( BS.ReadFlag() )
    {
        BS.ReadVector( m_Position );
        Deactivate();
    }

    if( BS.ReadFlag() )
    {
        BS.ReadVector( m_Position );
        BS.ReadVector( m_Velocity );
        BS.ReadF32   ( m_FuseTime );
        Activate();
    }
}

//==============================================================================

void net_rocket::OnProvideUpdate(       bitstream& BS, 
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
        BS.WriteVector( m_Position );
    }

    if( BS.WriteFlag( DirtyBits & ACTIVATE_BIT ) )
    {
        DirtyBits &= ~ACTIVATE_BIT;
        BS.WriteVector( m_Position );
        BS.WriteVector( m_Velocity );
        BS.WriteF32   ( m_FuseTime );
    }
}

//==============================================================================

void net_rocket::Logic( void )
{
    ASSERT( m_Active );

    // ??

    // The game side version of rocket may take a few frames to be created.
    // So, if we don't have a pointer to it, try to create it...
    if( m_pObject == NULL )
        CreateGameObj();

    if( m_pObject )
    {
        if( inev_IsServer() )
        {
        }
        
        if( inev_IsClient() )
        {
        }
    }
}

//==============================================================================

void net_rocket::DebugRender( void ) const
{
}

//==============================================================================
