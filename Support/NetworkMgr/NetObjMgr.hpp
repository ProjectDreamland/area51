//==============================================================================
//
//  NetObjMgr.hpp
//
//==============================================================================

#ifndef NETOBJMGR_HPP
#define NETOBJMGR_HPP

//==============================================================================
//  DEBUG MACROS
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetObj.hpp"
#include "NetLimits.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define OBJ_NET_INFO_SAMPLES    30 

//==============================================================================
//  TYPES
//==============================================================================

class  object;
struct net_pain;

//==============================================================================

class netobj_mgr
{
//------------------------------------------------------------------------------
//  Public Values and Types   
//------------------------------------------------------------------------------

public:

    struct type_info
    {
        netobj::type    Type;
        char*           pTypeName;
        s32             InstanceCount;  // Stat
        s32             MaxInstances;   // Stat

        void            Clear       ( void );
        void            Setup       ( netobj::type Type,
                                      char*        pTypeName );
    };

    struct net_info
    {
        s32             UpdateBits  [ OBJ_NET_INFO_SAMPLES ];
        s32             Updates     [ OBJ_NET_INFO_SAMPLES ];
        s32             SampleIndex;
                        
        void            Clear       ( void );
        void            GetStats    ( s32& Updates, s32& UpdateBits );
        void            AddUpdate   ( s32 NBits );
        void            NextSample  ( void );
    };

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:

                        netobj_mgr      ( void );
                       ~netobj_mgr      ( void );
                        
        void            Clear           ( void );
                        
        s32             AddObject       ( netobj* pNetObj );
        netobj*         CreateObject    ( netobj::type Type );
        netobj*         CreateAddObject ( netobj::type Type );
        void            DestroyObject   ( s32 Slot );
        netobj*         GetObjFromSlot  ( s32 Slot );
        void            ReserveSlot     ( s32 Slot );
        xbool           JustRemoved     ( s32 Slot );
        void            ClientExitGame  ( s32 Client );

        type_info&      GetTypeInfo     ( netobj::type Type );
        net_info&       GetNetInfo      ( netobj::type Type );

        void            Logic           ( f32 DeltaTime );
        void            Render          ( void );

        void            ApplyNetPain    ( net_pain& NetPain );

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------

protected:

        void            Initialize      ( void );

//------------------------------------------------------------------------------
//  Internal Data
//------------------------------------------------------------------------------

protected:

    // The Initialized flag is static to ensure that only one netobj manager is 
    // ever instantiated.

static  xbool           m_Initialized;

        s32             m_SlotCursor;
        s32             m_Reserved;

        type_info       m_TypeInfo      [ netobj::TYPE_END ];
        net_info        m_NetInfo       [ netobj::TYPE_END ];

        s16             m_JustRemoved   [ NET_MAX_OBJECTS ];
        s16             m_GameSlot      [ NET_MAX_OBJECTS ];

        #ifndef X_RETAIL
        object*         m_pObject       [ NET_MAX_OBJECTS ]; // Debugging ONLY!
        netobj*         m_pNetObj       [ NET_MAX_OBJECTS ]; // Debugging ONLY!
        #endif
};

//==============================================================================
//  GLOBAL VARIABLES
//==============================================================================

extern netobj_mgr  NetObjMgr;

//==============================================================================
#endif // NETOBJMGR_HPP
//==============================================================================
