//==============================================================================
//
//  JumpPad.hpp
//
//==============================================================================

#ifndef JUMP_PAD_HPP
#define JUMP_PAD_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct jump_pad_desc;

class jump_pad : public object
{
public:

    CREATE_RTTI( jump_pad, object, object )

                                jump_pad                ( void );
                               ~jump_pad                ( void );
                                     
virtual         bbox            GetLocalBBox            ( void ) const;
virtual const   object_desc&    GetTypeDesc             ( void ) const;
static  const   object_desc&    GetObjectType           ( void );
virtual         s32             GetMaterial             ( void ) const;
                void            OnEnumProp              ( prop_enum&  rPropList  );
                xbool           OnProperty              ( prop_query& rPropQuery );
virtual         void            OnMove                  ( const vector3& NewPos   );      
virtual         void            OnMoveRel               ( const vector3& DeltaPos );    
virtual         void            OnTransform             ( const matrix4& L2W      );
virtual         void            OnAdvanceLogic          ( f32 DeltaTime );
virtual         void            OnRender                ( void );
virtual         void            OnRenderTransparent     ( void );

                void            PlayJump                ( void );

protected:

                void            ComputeTrigger          ( void );
                void            ComputeVelocity         ( void );

                vector3         m_TriggerSize;
                radian          m_Pitch;
                radian          m_Yaw;
                f32             m_Speed;
                f32             m_ArcTime;      // For editor only.
                f32             m_AirControl;   // 0 - 100
                xbool           m_Instantaneous;
                xbool           m_ReboostOnly;
                xbool           m_BoostOnly;

                fx_handle       m_FXHandle;
                s32             m_Audio;

                bbox            m_WorldTrigger;
                vector3         m_Velocity;

friend struct jump_pad_desc;
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 jump_pad::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // JUMP_PAD_HPP
//==============================================================================
