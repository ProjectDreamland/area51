///////////////////////////////////////////////////////////////////////////////
//
//  action_change_perception.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __action_change_perception_h
#define __action_change_perception_h

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================================================================

class action_change_perception : public actions_ex_base
{
public:
                    action_change_perception                ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_CHANGE_PERCEPTION;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Change Player Perception. (blockable)"; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
    virtual         void                    OnActivate      ( xbool Flag );
   
protected:
    
    xbool               m_bIsBeginChange;

    f32                 m_GlobalTimeDialation;
    f32                 m_AudioTimeDialation;
    f32                 m_ForwardSpeedFactor;
    f32                 m_TurnRateFactor;

    f32                 m_TimeRange;
    xbool               m_bBlock;

    f32                 m_TimeDelayingSoFar;
    xbool               m_bHasBegun;
};

#endif
