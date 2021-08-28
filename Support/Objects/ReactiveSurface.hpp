//==============================================================================
//
//  ReactiveSurface.hpp
//
//==============================================================================
#ifndef __ReactiveSurface_HPP__
#define __ReactiveSurface_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\AnimSurface.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Objects\Event.hpp"

//==============================================================================
//  NOTES
//==============================================================================

class reactive_surface : public anim_surface
{
public:
    CREATE_RTTI( reactive_surface, anim_surface, object )

public:
    reactive_surface();
    ~reactive_surface();

    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

protected:

    void                        PlayAnim        ( s16 AnimStringIndex );
    
    enum reactive_state
    {
        IN_RANGE,
        OUT_OF_RANGE,
        ENTERING_RANGE,
        EXITING_RANGE
    };

protected:

    f32     m_ActiveDistance;
    s16     m_iEnterReaction;
    s16     m_iExitReaction;    
    s16     m_iInactiveIdle;
    s16     m_iActiveIdle;
    reactive_state m_State;
};

#endif 