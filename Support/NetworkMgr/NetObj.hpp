//==============================================================================
//
//  NetObj.hpp
//
//==============================================================================

#ifndef NETOBJ_HPP
#define NETOBJ_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\Object.hpp"

//==============================================================================
//  MACROS
//==============================================================================

#ifdef X_EDITOR
#define CREATE_NET_OBJECT( ObjDesc, NetType ) \
                g_ObjMgr.CreateObject( ObjDesc )
#else
#define CREATE_NET_OBJECT( ObjDesc, NetType ) \
                NetObjMgr.CreateAddObject( NetType )->net_GetGameGuid()
#endif

//------------------------------------------------------------------------------

#ifdef X_EDITOR
#define DESTROY_NET_OBJECT( pObject ) \
                g_ObjMgr.DestroyObject( pObject->GetGuid() )
#else
#define DESTROY_NET_OBJECT( pObject ) \
                NetObjMgr.DestroyObject( pObject->net_GetSlot() )
#endif

//==============================================================================
//  TYPES
//==============================================================================

class  netobj_mgr;
class  bitstream;
struct net_pain;

//==============================================================================

class netobj : public object
{

//------------------------------------------------------------------------------
//  Public types
//------------------------------------------------------------------------------

public:
    
    CREATE_RTTI( netobj, object, object )


    enum type
    {
        TYPE_PLAYER     =  0,
        TYPE_PICKUP,    
        TYPE_FLAG,
        TYPE_FLAG_BASE,
        TYPE_CAP_POINT,  
        TYPE_GRENADE,
        TYPE_JBEAN_GRENADE,
        TYPE_MESON_1ST,
        TYPE_MESON_2ND,
        TYPE_MESON_3RD,
        TYPE_BBG_1ST,
        TYPE_PARASITE,
        TYPE_CONTAGION,
        TYPE_TENDRIL,
        TYPE_MESON_EXPL,

        TYPE_END,
        TYPE_START      =  0,
        TYPE_NULL       = -1,
    };

//------------------------------------------------------------------------------

    enum
    {
        ACTIVATE_BIT          = 0x80000000,
        DEACTIVATE_BIT        = 0x40000000,
        ACTIVATE_STICKY_BIT   = 0x20000000,     // Not a normal dirty bit.
        DEACTIVATE_STICKY_BIT = 0x10000000,     // Not a normal dirty bit.
    };

//------------------------------------------------------------------------------

    enum mode
    {
        CONTROL_LOCAL       = 0x01,
        CONTROL_REMOTE      = 0x02,
        ON_SERVER           = 0x04,
        ON_CLIENT           = 0x08,

        LOCAL_ON_SERVER     = CONTROL_LOCAL  | ON_SERVER,
        LOCAL_ON_CLIENT     = CONTROL_LOCAL  | ON_CLIENT,
        GHOST_ON_SERVER     = CONTROL_REMOTE | ON_SERVER,
        GHOST_ON_CLIENT     = CONTROL_REMOTE | ON_CLIENT,
    };


//------------------------------------------------------------------------------
#ifndef X_EDITOR
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Public functions
//------------------------------------------------------------------------------

public:

                        netobj              ( void );
virtual                ~netobj              ( void ); 

virtual         netobj* AsNetObj            ( void ) {return((netobj*)this);};

virtual         void    net_Logic           ( f32 DeltaTime );

                u32     net_GetClearDirty   ( void );
                u32     net_GetDirtyBits    ( void ) const;
                s32     net_GetSlot         ( void ) const;
                s32     net_GetOwningClient ( void ) const;
                u32     net_GetTeamBits     ( void ) const;
                type    net_GetType         ( void ) const;
        const   char*   net_GetTypeName     ( void ) const;
virtual         s32     net_GetGameSlot     ( void ) const;
virtual         guid    net_GetGameGuid     ( void ) const;
virtual         u32     net_GetUpdateMask   ( s32 TargetClient ) const;

virtual         void    net_SetSlot         ( s32 Slot );
virtual         void    net_SetOwningClient ( s32 Client );
virtual         void    net_SetTeamBits     ( u32 TeamBits );

virtual         void    net_Activate        ( void );
virtual         void    net_Deactivate      ( void );
                
virtual         void    net_AcceptUpdate    ( const bitstream& BitStream );
virtual         void    net_ProvideUpdate   (       bitstream& BitStream, 
                                                    u32&       DirtyBits );
                
virtual         void    net_ApplyNetPain    ( net_pain& NetPain );
virtual         void    net_DebugRender     ( void ) const;

//------------------------------------------------------------------------------
//  Protected functions
//------------------------------------------------------------------------------

protected:

//------------------------------------------------------------------------------
//  Protected data
//------------------------------------------------------------------------------

protected:

                type    m_NetType;
                s32     m_NetSlot;
                s32     m_OwningClient;
                u32     m_NetDirtyBits;
                u32     m_NetModeBits;
                u32     m_NetTeamBits;

    // NOTE: When a netobj is constructed...
    //          m_NetType   = TYPE_NULL;
    //          m_NetSlot   = -1;
    //
    // The m_NetType will be set by NetObjMgr.CreateObject(), and the m_NetSlot
    // will be set by NetObjMgr.AddObject().

                friend  netobj_mgr;

//------------------------------------------------------------------------------
#endif // X_EDITOR
//------------------------------------------------------------------------------

};

//==============================================================================
//  MORE INCLUDES
//==============================================================================

#include "NetworkMgr\NetObjMgr.hpp"

//==============================================================================
#endif // NETOBJ_HPP
//==============================================================================
