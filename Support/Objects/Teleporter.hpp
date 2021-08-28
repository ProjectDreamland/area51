//==============================================================================
//
//  Teleporter.hpp
//
//==============================================================================

#ifndef TELEPORTER_HPP
#define TELEPORTER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct teleporter_desc;

class teleporter : public object
{
public:

    CREATE_RTTI( teleporter, object, object )

                                teleporter          ( void );
                               ~teleporter          ( void );
                                     
virtual         bbox            GetLocalBBox        ( void ) const;
virtual const   object_desc&    GetTypeDesc         ( void ) const;
static  const   object_desc&    GetObjectType       ( void );
virtual         s32             GetMaterial         ( void ) const;
                void            OnEnumProp          ( prop_enum&  rPropList  );
                xbool           OnProperty          ( prop_query& rPropQuery );
virtual         void            OnMove              ( const vector3& NewPos   );      
virtual         void            OnMoveRel           ( const vector3& DeltaPos );    
virtual         void            OnTransform         ( const matrix4& L2W      );
virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
virtual         void            OnRender            ( void );
virtual         void            OnRenderTransparent ( void );

                void            Receive             (       s32      PlayerIndex, 
                                                      const vector3& RelativePos, 
                                                            radian   RelativeYaw );

                void            PlayTeleportOut     ( void );
                void            PlayTeleportIn      ( void );

protected:

                void            ComputeTrigger      ( void );

                vector3         m_TriggerSize;
                guid            m_Target[8];
                xbool           m_Relative;
                xbool           m_BoostOut;
                f32             m_BoostPitch;   // Range  -90.. +90
                f32             m_BoostYaw;     // Range -360..+360
                f32             m_BoostSpeed;

                fx_handle       m_FXHandle;
                s32             m_Audio;

                bbox            m_WorldTrigger;
                u32             m_IgnoreBits;

friend struct teleporter_desc;
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 teleporter::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // TELEPORTER_HPP
//==============================================================================
