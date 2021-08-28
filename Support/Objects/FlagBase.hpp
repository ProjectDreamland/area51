//==============================================================================
//
//  FlagBase.hpp
//
//==============================================================================

#ifndef FLAG_BASE_HPP
#define FLAG_BASE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "NetworkMgr\NetObj.hpp"
#include "Objects\Circuit.hpp"
#include "Objects\TeamProp.hpp"
#include "Objects\AnimSurface.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class flag_base : public netobj
{
public:

    CREATE_RTTI( flag_base, netobj, object )

                                flag_base           ( void );
                               ~flag_base           ( void );

                void            Init                (       s32      BaseCircuit,
                                                      const matrix4& L2W );
                                    
virtual         bbox            GetLocalBBox        ( void ) const;
virtual const   object_desc&    GetTypeDesc         ( void ) const;
static  const   object_desc&    GetObjectType       ( void );
virtual         s32             GetMaterial         ( void ) const;
//              void            OnEnumProp          ( prop_enum&  List  );
//              xbool           OnProperty          ( prop_query& Query );

//rtual         void            OnRender            ( void );
//rtual         void            OnAdvanceLogic      ( f32 DeltaTime );
//rtual         void            OnRenderTransparent ( void );

//------------------------------------------------------------------------------
#ifndef X_EDITOR
//------------------------------------------------------------------------------

//rtual         void            net_Activate        ( void );
//rtual         void            net_Deactivate      ( void );
                
virtual         void            net_AcceptUpdate    ( const bitstream& BitStream );
virtual         void            net_ProvideUpdate   (       bitstream& BitStream, 
                                                            u32&       DirtyBits );

//------------------------------------------------------------------------------
#endif // X_EDITOR
//------------------------------------------------------------------------------

protected:

//  s32             m_Circuit;      // Which circuit to control?

    guid            m_BaseGuid;
    xbool           m_bInitialized;

//  fx_handle       m_Top;
//  fx_handle       m_Bottom;
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 flag_base::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // FLAG_BASE_HPP
//==============================================================================
