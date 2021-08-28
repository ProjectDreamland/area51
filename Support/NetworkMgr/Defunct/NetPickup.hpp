//==============================================================================
//
//  NetPickup.hpp
//
//==============================================================================

#ifndef NETPICKUP_HPP
#define NETPICKUP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetObj.hpp"
#include "NetObjMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================


//==============================================================================
//  TYPES
//==============================================================================

class net_pickup : public netobj
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:

//------------------------------------------------------------------------------
//  Public functions
//------------------------------------------------------------------------------

public:

virtual void        Activate        ( void );
virtual void        Deactivate      ( void );
                    
virtual void        CreateGameObj   ( void );
virtual void        DestroyGameObj  ( void );
                    
virtual void        OnAcceptUpdate  ( const bitstream& BitStream );
virtual void        OnProvideUpdate (       bitstream& BitStream, 
                                            u32&       DirtyBits, 
                                            s32        Client,
                                      const delta*     pDelta );

virtual void        Logic           ( void );

virtual void        DebugRender     ( void ) const;

        void        Setup           ( const vector3&    Position,
                                      const radian3&    Orientation,
                                      xbool             Permanent,
                                      s32               Kind,       // Ammo, health, weapon    
                                      s32               AmmoType  = 0,
                                      s32               AmmoCount = 0,
                                      void*             pScript   = NULL );

  const vector3&    GetPosition     ( void ) { return( m_Position    ); } 
  const radian3&    GetOrientation  ( void ) { return( m_Orientation ); }
        int         GetAmmoType     ( void ) { return( m_AmmoType    ); }
        xbool       GetPermanent    ( void ) { return( m_Permanent   ); }
        void*       GetScript       ( void ) { return( m_pScript     ); }
        int         GetAmmoCount    ( void ) { return( m_AmmoCount   ); }
        int         GetKind         ( void ) { return( m_Kind        ); }

//------------------------------------------------------------------------------
//  Private Types
//------------------------------------------------------------------------------

    enum dirty_bits
    {
        STATE_BIT      = 0x00000001,
//      DEACTIVATE_BIT = 0x80000000,
//      ACTIVATE_BIT   = 0x40000000,
//      DELTA_DATA_BIT = 0x20000000,
    };

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------

protected:

        vector3         m_Position;
        radian3         m_Orientation;
        xbool           m_Permanent;
        s32             m_State;
        s32             m_Kind;
        s32             m_AmmoType;
        s32             m_AmmoCount;
        void*           m_pScript;
};

//==============================================================================
#endif // NETPICKUP_HPP
//==============================================================================
