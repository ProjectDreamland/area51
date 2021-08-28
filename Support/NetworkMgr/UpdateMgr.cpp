//==============================================================================
//
//  UpdateMgr.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//#if !defined(mtraub)
//#define X_SUPPRESS_LOGS
//#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "UpdateMgr.hpp"
#include "GameMgr.hpp"
#include "x_log.hpp"
#include "NetworkMgr\NetworkMgr.hpp"

//==============================================================================
//  TWEAKABLES
//==============================================================================

// static s32 PLAYER_UPDATES_PER_PACKET = 16;

#define ALL_DIRTY_BITS   0xCFFFFFFF
#define STICKY_BITS      0x30000000
#define ACTION_BITS      0xC0000000

//==============================================================================
//  FUNCTIONS
//==============================================================================

update_mgr::update_mgr( void )
{
    m_ClientIndex = -1;
    ResetUpdates();
}

//==============================================================================

update_mgr::~update_mgr( void )
{
    m_ClientIndex = -1;
}

//==============================================================================

void update_mgr::Init( s32 ClientIndex )
{
    ASSERT( m_ClientIndex == -1 );
    ASSERT( IN_RANGE( 0, ClientIndex, NET_MAX_PLAYERS ) );

    m_ClientIndex = ClientIndex;

    ResetUpdates();
}

//==============================================================================

void update_mgr::Kill( void )
{
    // The killing blow...
    m_ClientIndex = -1;
}

//==============================================================================

void update_mgr::Reset( void )
{
    LOG_MESSAGE( "update_mgr::Reset", "Client:%d", m_ClientIndex );
    ResetUpdates();
}

//==============================================================================

void update_mgr::ResetUpdates( void )
{
    m_Free         = MAX_UPDATES - 2;
    m_Used         =  0;
    m_Cursor       = -1;
    m_PlayerCursor =  0;

    // Initialize all of the update records into one long doubly linked list.
    for( s32 i = 0; i < MAX_UPDATES-2; i++ )
    {
        m_Update[i].Next = i+1;
        m_Update[i].Prev = i-1;
        m_Update[i].Slot = -1;  // Marks update as free.
    }

    // Now, update[USED_LIST] should be empty.
    m_Update[USED_LIST].Prev = USED_LIST;
    m_Update[USED_LIST].Next = USED_LIST;

    // And, update[FREE_LIST] should get the long list.
    m_Update[FREE_LIST].Prev = LIST_LAST;
    m_Update[LIST_LAST].Next = FREE_LIST;
    m_Update[FREE_LIST].Next = 0;
    m_Update[    0    ].Prev = FREE_LIST;

    // All objects are "clean".
    for( s32 i = 0; i < NET_MAX_OBJECTS; i++ )
    {
        m_ObjBits[i] = 0x00000000;
    }
}

//==============================================================================

void update_mgr::SyncActivates( void )
{
    LOG_MESSAGE( "update_mgr::SyncActivates", "Client:%d", m_ClientIndex );

    for( s32 i = 0; i < NET_MAX_OBJECTS; i++ )
    {
        if( NetObjMgr.GetObjFromSlot( i ) )
            m_ObjBits[i] = netobj::ACTIVATE_BIT;
    }
}

//==============================================================================

void update_mgr::AddDirtyBits( s32 Slot, 
                               u32 DirtyBits )
{
    ASSERT( m_ClientIndex != -1 );
    ASSERT( IN_RANGE( 0, Slot, NET_MAX_OBJECTS-1 ) );
    ASSERT( (DirtyBits & STICKY_BITS) == 0 );

    // Just OR in all the dirty bits from the object.
    m_ObjBits[Slot] |= DirtyBits;

    /*
    CLOG_MESSAGE( (Slot > 31) && (DirtyBits),
                  "update_mgr::AddDirtyBits",
                  "Slot:%d - Client:%d - NewBits:%08X - FinalBits:%08X", 
                  Slot, m_ClientIndex, DirtyBits, m_ObjBits[Slot] );
    */
}

//==============================================================================

void update_mgr::PacketAck( conn_packet& Packet, xbool Arrived )
{
    ASSERT( m_ClientIndex != -1 );

    s32 I = Packet.FirstUpdate;

    while( I != -1 )
    {
        s32 Next = m_Update[I].PacketNext;
        s32 Slot = m_Update[I].Slot;

        if( !Arrived )
        {            
            // Recover lost dirty bits.  (But only dirty bits; no sticky bits.)
            m_ObjBits[Slot] |= (m_Update[I].Bits & ALL_DIRTY_BITS);

            /*
            LOG_WARNING( "update_mgr::PacketAck", 
                         "NACK - Client:%d - Slot:%d - Packet:%d - Update:%d - Bits:%08X", 
                         m_ClientIndex, Slot, Packet.Seq, I, m_Update[I].Bits );
            */
        }
        else
        {
            /*
            LOG_MESSAGE( "update_mgr::PacketAck", 
                         "ACK - Client:%d - Slot:%d - Packet:%d - Update:%d - Bits:%08X", 
                         m_ClientIndex, Slot, Packet.Seq, I, m_Update[I].Bits );
            */

            if( m_Update[I].Bits & netobj::ACTIVATE_BIT )
            {
                m_ObjBits[Slot] &= ~netobj::ACTIVATE_STICKY_BIT;
            }

            if( m_Update[I].Bits & netobj::DEACTIVATE_BIT )
            {
                m_ObjBits[Slot] &= ~netobj::DEACTIVATE_STICKY_BIT;
            }
        }

        LOG_FLUSH();

        ReleaseUpdate( I );
        I = Next;
    }

    Packet.FirstUpdate = -1;
    Packet.NUpdates    =  0;
}

//==============================================================================

s32 update_mgr::AllocUpdate( s32 Slot )
{
    s32 I, J;

    Sanity();

    ASSERT( m_ClientIndex != -1 );

    if( m_Free == 0 )
    {
        // Yes!  We have no bananas!
        LOG_ERROR( "update_mgr::AllocUpdate",
                   "Out of updates -- Client:%d - Slot:%d",
                   m_ClientIndex, Slot );
        return( -1 );
    }

    // We want the first free update.
    I = m_Update[FREE_LIST].Next;
    ASSERT( IN_RANGE( 0, I, LIST_LAST ) );
    ASSERT( m_Update[I].Slot == -1 );

    // Take it out of the free list.

    J = m_Update[I].Next;

    m_Update[ FREE_LIST ].Next = J;
    m_Update[     J     ].Prev = FREE_LIST;

    // Insert it onto the END of the used list.

    J = m_Update[USED_LIST].Prev;

    m_Update[ USED_LIST ].Prev = I;
    m_Update[     I     ].Next = USED_LIST;
    m_Update[     I     ].Prev = J;
    m_Update[     J     ].Next = I;

    m_Update[I].Slot = Slot;

    // A little housekeeping, and we are done.

    m_Free--;
    m_Used++;

    Sanity();

    /*
    LOG_MESSAGE( "update_mgr::AllocUpdate",
                 "Client:%d - Update:%d - Slot:%d - Used:%d - Free:%d",
                 m_ClientIndex, I, Slot, m_Used, m_Free );
    LOG_FLUSH();
    */

    return( I );
}

//==============================================================================

void update_mgr::ReleaseUpdate( s32 I )
{
    s32 J, K;

    /*
    LOG_MESSAGE( "update_mgr::ReleaseUpdate",
                 "Client:%d - Update:%d - Slot:%d - Used:%d - Free:%d",
                 m_ClientIndex, I, m_Update[I].Slot, m_Used, m_Free );
    LOG_FLUSH();
    */

    Sanity();

    ASSERT( m_ClientIndex != -1 );
    ASSERT( m_Used > 0 );
    ASSERT( m_Update[I].Slot != -1 );

    // Remove I from the used list.

    J = m_Update[I].Prev;
    K = m_Update[I].Next;
    
    m_Update[J].Next = K;
    m_Update[K].Prev = J;

    // Insert I at the END of the free list.

    J = m_Update[FREE_LIST].Prev;

    m_Update[ FREE_LIST ].Prev = I;
    m_Update[     I     ].Next = FREE_LIST;
    m_Update[     I     ].Prev = J;
    m_Update[     J     ].Next = I;

    // A little housekeeping, and we are done.

    m_Free++;
    m_Used--;

    m_Update[I].Slot = -1; // Marks update as free.

    Sanity();
}

//==============================================================================

void update_mgr::ProvideUpdates( conn_packet& Packet, 
                                 bitstream&   BS,
                                 s32          NUpdatesAllowed )
{
    ASSERT( m_ClientIndex != -1 );
    ASSERT( BS.Overwrite() == FALSE );

    // Clear packet of updates.
    Packet.NUpdates    =  0;
    Packet.FirstUpdate = -1;

#if !defined(X_RETAIL) || defined(X_QA)
    Packet.BitsPlayerUpdates = 0;
    Packet.BitsGhostUpdates = 0;
    Packet.BitsOtherUpdates = 0;
#endif

    // Reserve space for "number of updates" in bitstream.
    s32 HeaderCursor = BS.GetCursor();
    BS.WriteRangedS32( 0, 0, 100 );

    // Make a pass through the objects and update the State.  Only do this for
    // objects which are native to this machine.
    //
    // We have to do this here (rather than in, say, AddDirtyBits) because it  
    // is possible for an object to be destroyed between the AddDirtyBits call
    // and here.  In that case, the object's State could still be Active and
    // yet there is no object.  This will cause an ASSERT in ProvideObjUpdate.
    //
    // Also, while we are processing the objects, we can force set the
    // ACTIVATE_BIT if ACTIVATING and DEACTIVATE_BIT if DEACTIVATING.

    s32 Lo, Hi;     // Update lo and hi index values.
    s32 CLo, CHi;   // Client lo and hi index values.

    CLo  = NET_MAX_OBJECTS_ON_SERVER;
    CLo += m_ClientIndex * NET_MAX_OBJECTS_ON_CLIENT;
    CHi  = CLo           + NET_MAX_OBJECTS_ON_CLIENT;

    if( g_NetworkMgr.IsServer() )
    {
        Lo = 0;
        Hi = NET_MAX_OBJECTS;
    }
    else
    {
        Lo = CLo;
        Hi = CHi;
    }

    for( s32 i = Lo; i < Hi; i++ )
    {
        // The code in this loop is needed on the machine that is sending
        // authoritative activate/deactivate commands for each slot.  Each
        // client is authoritative in their own restricted range.  The server
        // is authoritative for all objects *except* for those on the owning
        // client.  There is an UpdateMgr on the server for each client.  In 
        // this loop, we must skip over the client's object slot range.
        if( g_NetworkMgr.IsServer() && (i == CLo) )
        {
            i = CHi-1;
            continue;
        }

        netobj* pNetObj = NetObjMgr.GetObjFromSlot(i);

        // Set the sticky bits as needed.

        if( m_ObjBits[i] &  netobj::ACTIVATE_BIT )
            m_ObjBits[i] |= netobj::ACTIVATE_STICKY_BIT;

        if( m_ObjBits[i] &  netobj::DEACTIVATE_BIT )
            m_ObjBits[i] |= netobj::DEACTIVATE_STICKY_BIT;

        // Force the action bits as needed.

        if( (m_ObjBits[i] & netobj::ACTIVATE_STICKY_BIT) &&
            (m_ObjBits[i] & ALL_DIRTY_BITS) )
        {
            m_ObjBits[i] |= netobj::ACTIVATE_BIT;
        }

        if( (m_ObjBits[i] & netobj::DEACTIVATE_STICKY_BIT) &&
            (m_ObjBits[i] & ALL_DIRTY_BITS) )
        {
            m_ObjBits[i] |= netobj::DEACTIVATE_BIT;
        }

        // If we don't have an object on this machine, then we can't satisfy an
        // activation.

        if( !pNetObj )
        {
            m_ObjBits[i] &= ~netobj::ACTIVATE_BIT;
            m_ObjBits[i] &= ~netobj::ACTIVATE_STICKY_BIT;
        }
    }

    // Write updates for locally controlled player on a client.
    if( g_NetworkMgr.IsClient() )
    {   
        s32 Slot = g_NetworkMgr.GetLocalPlayerSlot( 0 );
        if( Slot != -1 )
        {
#if !defined(X_RETAIL) || defined(X_QA)
            s32 BSStartCursor = BS.GetCursor();
#endif
            ProvideObjUpdate( Slot, Packet, BS, NUpdatesAllowed );
#if !defined(X_RETAIL) || defined(X_QA)
            Packet.BitsPlayerUpdates = BS.GetCursor() - BSStartCursor;
#endif
        }
    }

    // Write updates for all other objects.

    if( g_NetworkMgr.IsClient() )
    {
        Lo = 32;
        Hi = NET_MAX_OBJECTS;
    }

    // If this is our first time through here, then the cursor is not yet
    // initialized.  Take care of it.
    if( m_Cursor == -1 )  m_Cursor = Lo;

    // Issue updates.
    {
        s32 RoundTrip = m_Cursor;

        while( NUpdatesAllowed && (!BS.Overwrite()) )
        {
#if !defined(X_RETAIL) || defined(X_QA)
            s32 BSStartCursor = BS.GetCursor();
#endif

            if( m_ObjBits[m_Cursor] & ALL_DIRTY_BITS )
            {
                if( !ProvideObjUpdate( m_Cursor, Packet, BS, NUpdatesAllowed ) )
                    break; 
            }

#if !defined(X_RETAIL) || defined(X_QA)
            if( m_Cursor <= 31 )
            {
                if( m_Cursor == g_NetworkMgr.GetLocalPlayerSlot( 0 ) )
                {
                    if( g_NetworkMgr.IsServer() )
                    {   
                        //player info
                        Packet.BitsPlayerUpdates += BS.GetCursor() - BSStartCursor;
                    }
                }
                else
                {
                    // ghost
                    Packet.BitsGhostUpdates += BS.GetCursor() - BSStartCursor;
                }
            }
            else
            {
                //other info
                Packet.BitsOtherUpdates += BS.GetCursor() - BSStartCursor;
            }
#endif

            m_Cursor += 1;
            if( m_Cursor >= Hi )
                m_Cursor  = Lo;
            if( m_Cursor == RoundTrip )
                break;
        }
    }

    // Write updates for other player objects.
    /*
    {
        s32 Count     = PLAYER_UPDATES_PER_PACKET;
        s32 RoundTrip = m_PlayerCursor;

        while( NUpdatesAllowed && Count && (!BS.Overwrite()) )
        {
            if( m_Object[m_PlayerCursor].DirtyBits & ALL_DIRTY_BITS )
            {
                if( !ProvideObjUpdate( m_PlayerCursor, Packet, BS, NUpdatesAllowed ) )
                    break;
            }

            Count--;
            m_PlayerCursor = (m_PlayerCursor + 1) % NET_MAX_PLAYERS;
            if( m_PlayerCursor == RoundTrip )
                break;
        }
    }
    */

    // Write updates to all other objects space permitting.
    /*
    {
        s32 RoundTrip = m_Cursor;

        while( NUpdatesAllowed && (!BS.Overwrite()) )
        {
            if( m_Object[m_Cursor].DirtyBits & ALL_DIRTY_BITS )
            {
                if( !ProvideObjUpdate( m_Cursor, Packet, BS, NUpdatesAllowed ) )
                    break; 
            }

            m_Cursor += 1;
            if( m_Cursor >= NET_MAX_OBJECTS )
                m_Cursor  = NET_MAX_PLAYERS;
            if( m_Cursor == RoundTrip )
                break;
        }
    }
    */

    // Patch up header in bitstream.
    s32 EndCursor = BS.GetCursor();
    BS.SetCursor( HeaderCursor );
    BS.WriteRangedS32( Packet.NUpdates, 0, 100 );
    BS.SetCursor( EndCursor );

//  DumpState();

    /*
    LOG_MESSAGE( "update_mgr::ProvideUpdates", 
                 "Client:%d - Packet:%d - Updates:%d",
                 m_ClientIndex, Packet.Seq, Packet.NUpdates );
    LOG_FLUSH();
    */
}

//==============================================================================

xbool update_mgr::ProvideObjUpdate( s32          Slot,
                                    conn_packet& Packet,
                                    bitstream&   BS,
                                    s32&         NUpdatesAllowed )
{
    if( (m_ObjBits[Slot] & ALL_DIRTY_BITS) == 0 )
        return( TRUE );
 
    netobj* pObject = NetObjMgr.GetObjFromSlot( Slot );
    u32     Bits    = m_ObjBits[Slot] & ALL_DIRTY_BITS;

    // Record the update in the packet.
    s32 I = AllocUpdate( Slot );
    if( I == -1 )
    {
        // Can't record the update in the local records; bail in confusion.
        return( FALSE );
    }

    // Begin adding data to stream.  Be ready in case of overrun.
    s32 StartCursor = BS.GetCursor();

    if( Bits & netobj::ACTIVATE_BIT )
        ASSERT( pObject );

    if( (Bits & netobj::DEACTIVATE_BIT) && !(Bits & netobj::ACTIVATE_BIT) )
        ASSERT( !pObject );

    // To assist in debugging...
    BS.WriteMarker();

    // Write out object slot.
    BS.WriteRangedS32( Slot, 0, NET_MAX_OBJECTS-1 );

    // Activate/deactivate flags...
    BS.WriteFlag( Bits & netobj::ACTIVATE_BIT   );
    BS.WriteFlag( Bits & netobj::DEACTIVATE_BIT );

    // ----------------------------------------------- Send Type?
    // 00 = Pure update.                                       NO
    // 01 = Deactivate.                                        NO
    // 10 = Activate.                                         YES
    // 11 = Deactivate, then activate new object.             YES

    if( Bits & netobj::ACTIVATE_BIT )
    {
        ASSERT( pObject );
        BS.WriteRangedS32( pObject->net_GetType(), 
                           netobj::TYPE_START, 
                           netobj::TYPE_END );
    }

    // NOTE: May need to do extra stuff for locally controlled player.

    // Allow the object to pack its update information into the bitstream.
    if( pObject )
    {
        // Tell the object to write an update into the bitstream using the given
        // dirty bits.
        pObject->net_ProvideUpdate( BS, Bits );

        // Objects do not clear the deactivate bit.  Do it here and now.
        Bits &= ~netobj::DEACTIVATE_BIT;

        if( Bits )
        {
            LOG_WARNING( "update_mgr::ProvideObjUpdate",
                "Slot:%d - Client:%d - Type:%s - Bits:%08X - Object did not update all dirty bits.",
                Slot, m_ClientIndex, pObject->net_GetTypeName(), Bits );
        }
    }
    else
    {
        // If there is no object, then we should have a DEACTIVATE.
        // (The absence of the object causes the deactivate.)
        ASSERT(   Bits & netobj::DEACTIVATE_BIT  );
        ASSERT( !(Bits & netobj::ACTIVATE_BIT  ) );

        // If we are here, then this must be a DEACTIVATE and there is no object
        // on which to call net_ProvideUpdate.  Thus, there is no way for the
        // DEACTIVATE_BIT to get cleared.  So, we must do it here.  Besides, all
        // of the information which represents the DEACTIVATE has already been
        // posted to the bitstream.
        Bits = 0x00000000;
    }

    // To assist in debugging...
    BS.WriteMarker();

    // If we overran, then back it all out and BAIL!
    if( BS.Overwrite() )
    {
        BS.SetCursor( StartCursor );
        ReleaseUpdate( I );
        return( FALSE );
    }

    // Record this update in case it is lost.
    m_Update[I].Bits       = m_ObjBits[Slot];
    m_Update[I].Bits      &= ~Bits;
    m_Update[I].Bits      &= ~STICKY_BITS;
    m_Update[I].PacketSeq  = Packet.Seq;
    m_Update[I].PacketNext = Packet.FirstUpdate;
    Packet.FirstUpdate     = I;
    Packet.NUpdates       += 1;

    // Housekeeping for caller.
    NUpdatesAllowed -= 1;     

    // Network statistics.
//  NetObjMgr.GetNetInfo( pObject->GetType() ).AddUpdate( BS.GetCursor() - StartCursor );

    /*
    if( Slot >= 32 )
    {
        u32 B = m_Update[I].Bits;
        LOG_MESSAGE( "update_mgr::ProvideObjUpdate",
                     "Slot:%d - Client:%d - Type:%s - Packet:%d - " // No comma.
                     "DirtyBits:%08X%s%s - Bytes:%d",
                     Slot, m_ClientIndex, 
                     pObject ? pObject->net_GetTypeName() : "<unknown>",
                     Packet.Seq, B,
                     (B & netobj::ACTIVATE_BIT  ) ? "[ACTIVATE]"   : "",
                     (B & netobj::DEACTIVATE_BIT) ? "[DEACTIVATE]" : "",
                     (BS.GetCursor() - StartCursor) >> 3 );
        LOG_FLUSH();
    }
    */

    // The revised dirty bits are those that the object did not clear during its
    // update.
    u32 StickyBits  = m_ObjBits[Slot] & STICKY_BITS;
    m_ObjBits[Slot] = Bits | StickyBits;

    return( TRUE );
}

//==============================================================================

void update_mgr::AcceptUpdates( bitstream& BS )
{
    ASSERT( m_ClientIndex != -1 );

    s32 NUpdates;

    BS.ReadRangedS32( NUpdates, 0, 100 );
    ASSERT( NUpdates <= 100 );

    if( NUpdates == 0 )
        return;

    for( s32 i = 0; i < NUpdates; i++ )
    {
        s32     StartCursor = BS.GetCursor();
        s32     Slot;
        s32     Type    = -1;
        netobj* pNetObj = NULL;

        BS.ReadMarker();
        BS.ReadRangedS32( Slot, 0, NET_MAX_OBJECTS-1 );

        xbool Activate   = BS.ReadFlag();
        xbool Deactivate = BS.ReadFlag();

        if( Activate  )
        {
            BS.ReadRangedS32( Type, netobj::TYPE_START, netobj::TYPE_END );
        }

        // Fetch the current object.  This could result in a NULL pointer.
        pNetObj = NetObjMgr.GetObjFromSlot( Slot );

        /*
        if( Slot >= 32 )
        {
            LOG_MESSAGE( "update_mgr::AcceptUpdates",
                         "Slot:%d - Client:%d - Type:%d(%s) - ObjType:%d(%s) - %s%s",
                         Slot, m_ClientIndex, Type,
                         (Type != -1) ? NetObjMgr.GetTypeInfo( netobj::type(Type) ).pTypeName : "",
                         pNetObj ? pNetObj->net_GetType() : -1,
                         pNetObj ? pNetObj->net_GetTypeName() : "",
                         Deactivate ? "[DEACTIVATE]" : "",
                         Activate   ? "[ACTIVATE]"   : "" );
            LOG_FLUSH();
        }
        */

        if( Deactivate && pNetObj )
        {
            if( !Activate || (pNetObj->net_GetType() != Type) )
            {
                NetObjMgr.DestroyObject( Slot );
                pNetObj = NULL;
            }
        }

        if( Activate )
        {
            ASSERT( Type != -1 );

            if( !pNetObj )
            {
                // ACTIVATE, and there is NO object in the slot.
                // Must create the object.
                pNetObj = CreateNetObj( Slot, (netobj::type)Type );
            }
            /* If debugging, then send the type.  Then use this else clause.
            else
            {
                // ACTIVATE, and there is an object already in the slot.
                // Verify object type.
                ASSERT( pNetObj->net_GetType() == Type );
            }
            */
        }

        if( pNetObj )
        {
            ASSERT( (Type == -1) || (pNetObj->net_GetType() == Type) );
            pNetObj->net_AcceptUpdate( BS );
        }

        BS.ReadMarker();

        // This code doesn't really do anything.  It is here merely to prevent
        // the debugger from leaving this scope if the ReadMarker above fails.
        if( BS.GetCursor() <= StartCursor )
        {
            ASSERT( FALSE );
        }
    }
}

//==============================================================================

netobj* update_mgr::CreateNetObj( s32 Slot, netobj::type Type )
{
    netobj* pNetObj = NULL;

    switch( Type )
    {
    case netobj::TYPE_PLAYER:
        // HACKOMOTRON HACKOMOTRON HACKOMOTRON
        // HACKOMOTRON HACKOMOTRON HACKOMOTRON
        GameMgr.CreatePlayer( Slot );   
        pNetObj = NetObjMgr.GetObjFromSlot( Slot );
        // HACKOMOTRON HACKOMOTRON HACKOMOTRON
        // HACKOMOTRON HACKOMOTRON HACKOMOTRON
        break;

    default:
        pNetObj = NetObjMgr.CreateObject( Type );
        NetObjMgr.ReserveSlot( Slot );
        NetObjMgr.AddObject( pNetObj );
        break;
    }

    ASSERT( pNetObj );
    return( pNetObj );
}

//==============================================================================

void update_mgr::ObjectRemoved( s32 Slot )
{
    // The given object has been deleted.  Since it is gone, it no longer 
    // matters what outstanding un-acknowledged updates it may have.  Toss out 
    // all recorded updates for the object.

    s32 i, Next;
    s32 FailSafe;
    s32 Count = 0;

    i        = m_Update[USED_LIST].Next;
    FailSafe = MAX_UPDATES;
    while( (i != USED_LIST) && (FailSafe > 0) )
    {
        Next = m_Update[i].Next;
        if( m_Update[i].Slot == Slot )
        {
            m_Update[i].Bits = 0x00000000;
            Count++;
        }
        i = Next;
        FailSafe--;
    }
    ASSERT( FailSafe > 0 );

    /*
    LOG_MESSAGE( "update_mgr::ObjectRemoved", 
                 "Slot:%d - Client:%d - Updates:%d", 
                 Slot, m_ClientIndex, Count );
    */

    // Now we must flag the object for deactivation on remote machines.  Note
    // that the server cannot deactivate objects owned by clients.

    u32 DeactivateBit = netobj::DEACTIVATE_BIT;

    if( g_NetworkMgr.IsServer() )
    {
        s32 CLo, CHi;
        CLo  = NET_MAX_OBJECTS_ON_SERVER;
        CLo += m_ClientIndex * NET_MAX_OBJECTS_ON_CLIENT;
        CHi  = CLo           + NET_MAX_OBJECTS_ON_CLIENT;
        if( IN_RANGE( CLo, Slot, CHi-1 ) )
            DeactivateBit = 0x00000000;
    }

    m_ObjBits[Slot] |= DeactivateBit;
}

//==============================================================================

void update_mgr::Sanity( void )
{
    s32 i, FailSafe, Count;

    // Check total of group counters.
    ASSERT( m_Used + m_Free == MAX_UPDATES - 2 );

    // Check used list integrity.
    i        = m_Update[USED_LIST].Next;
    FailSafe = MAX_UPDATES;
    Count    = 0;
    while( (i != USED_LIST) && (FailSafe > 0) )
    {
        ASSERT( m_Update[ m_Update[i].Next ].Prev == i );
        ASSERT( m_Update[ m_Update[i].Prev ].Next == i );
        Count++;
        FailSafe--;
        i = m_Update[i].Next;
    }
    ASSERT( FailSafe > 0 );
    ASSERT( Count == m_Used );

    // Check free list integrity.
    i        = m_Update[FREE_LIST].Next;
    FailSafe = MAX_UPDATES;
    Count    = 0;
    while( (i != FREE_LIST) && (FailSafe > 0) )
    {
        ASSERT( m_Update[ m_Update[i].Next ].Prev == i );
        ASSERT( m_Update[ m_Update[i].Prev ].Next == i );
        Count++;
        FailSafe--;
        i = m_Update[i].Next;
    }
    ASSERT( FailSafe > 0 );
    ASSERT( Count == m_Free );
}

//==============================================================================

void update_mgr::DumpState( void )
{
    if( m_Used == 0 )
        return;

    s32 Updates[32];
    s32 i;

    for( i = 0; i < 32; i++ )
    {
        Updates[i] = 0;
    }

    i = m_Update[USED_LIST].Next;
    while( i != USED_LIST )
    {
        Updates[ m_Update[i].Slot ]++;
        i = m_Update[i].Next;
    }

    x_DebugMsg( "Client:%02d -- Used:%03d - Free:%03d  ", 
                m_ClientIndex, m_Used, m_Free );

    /*
    for( i = 0; i < 32; i++ )
    {
        if( (Updates[i]) || (GetObjState(i) != INACTIVE) )
        {
            x_DebugMsg( "[%02d](%02d)  ", i, Updates[i] );
        }
    }
    */

    x_DebugMsg( "\n" );
}

//==============================================================================
