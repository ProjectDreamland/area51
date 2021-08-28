//==============================================================================
//
//  GameProp.hpp
//
//==============================================================================

#ifndef GAME_PROP_HPP
#define GAME_PROP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Circuit.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct game_prop_desc;

class game_prop : public object
{
public:

    enum kind
    {
        FLAG_BASE,
        CAPTURE_POINT,
    };

public:

    CREATE_RTTI( game_prop, object, object )

                                game_prop       ( void );
                               ~game_prop       ( void );
                                     
virtual         bbox            GetLocalBBox    ( void ) const;
virtual const   object_desc&    GetTypeDesc     ( void ) const;
static  const   object_desc&    GetObjectType   ( void );
virtual         s32             GetMaterial     ( void ) const;
                void            OnEnumProp      ( prop_enum&  rPropList  );
                xbool           OnProperty      ( prop_query& rPropQuery );

                kind            GetKind         ( void ) const { return( m_Kind    ); }
                circuit&        GetCircuit      ( void )       { return( m_Circuit ); }

                f32             GetRadius       ( void ) { return m_Radius;    }
                f32             GetElevation    ( void ) { return m_Elevation; }

protected:

                circuit         m_Circuit;
                kind            m_Kind;

                // For cap points...
                f32             m_Radius;
                f32             m_Elevation;

    friend game_prop_desc;
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 game_prop::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // GAME_PROP_HPP
//==============================================================================
