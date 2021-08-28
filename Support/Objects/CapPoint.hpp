//==============================================================================
//
//  CapPoint.hpp
//
//==============================================================================

#ifndef CAP_POINT_HPP
#define CAP_POINT_HPP

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

class cap_point : public netobj
{
public:

    CREATE_RTTI( cap_point, object, object )

                                cap_point           ( void );
                               ~cap_point           ( void );

                void            Init                ( void );
                                    
                void            SetCircuit          ( s32 Circuit );
                void            SetL2W              ( const matrix4& L2W );

virtual         bbox            GetLocalBBox        ( void ) const;
virtual const   object_desc&    GetTypeDesc         ( void ) const;
static  const   object_desc&    GetObjectType       ( void );
virtual         s32             GetMaterial         ( void ) const;
//              void            OnEnumProp          ( prop_enum&  List  );
//              xbool           OnProperty          ( prop_query& Query );

virtual         void            OnRender            ( void );
virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
                void            OnRenderTransparent ( void );

                void            SetElevation        ( f32 Elevation ) { m_Elevation = Elevation; }
                void            SetRadius           ( f32 Radius    ) { m_Radius    = Radius;    }

                s32             GetAward            ( void );
                xbool           PlayerInfluence     ( s32 PlayerIndex );

                s32             GetCircuit          ( void );

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

    enum
    {
        DIRTY_MODE     = 0x00000001,
        DIRTY_ALL      = 0x00000001,
    }; 

    s32             m_Circuit;      // Which circuit to control?
    f32             m_AwardTimer;   // Time until next award.
    f32             m_Value;        // -1(alpha) thru +1(omega)
    s32             m_Mode;         // -1(alpha) thru +1(omega)
    s32             m_Award;        // Point to be awarded.
    vector3         m_Center;       // Center of cap point.
    f32             m_Radius;       // Radius of cap influence.
    f32             m_Elevation;    // Elevation of cap point center.
    u32             m_AlphaMask;    // Bits for Alpha players.
    u32             m_OmegaMask;    // Bits for Omega players.

    guid            m_BaseGuid;
    guid            m_BallAnim;
    guid            m_BallCore;

    xbool           m_bInitialized;

    fx_handle       m_Top;
    fx_handle       m_Bottom;
    voice_id        m_Hum;

    fx_handle       m_Arcs [32];
    xbool           m_bArcs[32];
    f32             m_Alpha;
    f32             m_IconOpacity;
    xbool           m_bRendered;
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 cap_point::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // CAP_POINT_HPP
//==============================================================================
