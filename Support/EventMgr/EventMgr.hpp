//==============================================================================
//
//  EventMgr.hpp
//
//==============================================================================

#ifndef EVENT_MGR_HPP
#define EVENT_MGR_HPP

#include "Animation\AnimData.hpp"
#include "Animation\CharAnimPlayer.hpp"
#include "Animation\AnimPlayer.hpp"
#include "Loco\Loco.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Animation\BasePlayer.hpp"

//==============================================================================
//  INCLUDES
//==============================================================================


//==============================================================================
//  TYPES
//==============================================================================
class event_mgr
{
public:
            event_mgr               ( void );
            ~event_mgr              ( void );
           
    void    HandleSuperEvents       ( char_anim_player&      CharAnimPlayer, object* pObj );
    void    HandleSuperEvents       ( loco_char_anim_player& CharAnimPlayer, object* pObj );
    void    HandleSuperEvents       ( simple_anim_player& SimpleAnimPlayer, object* pObj );
    void    HandleSuperEvents       ( loco_char_anim_player& CharAnimPlayer, loco_anim_controller& LocoAnimController, object* pObj );
    f32     ClosestPointToAABox     (const vector3& Point, const bbox& Box, vector3& ClosestPoint);
#if !defined(X_RETAIL)
    xbool   m_bLogAudio;
    xbool   m_bLogParticle;
#endif

protected:

    void    HandleAnimEvents        ( const anim_event& AnimEvent, object* pObj, s32 EventIndex, 
                                      base_player& BasePlayer );
        
    void    HandleAudioEvent        ( const event& Event, object* pParentObj, xbool UsePosition );
    void    HandleParticleEvent     ( const event& Event, object* pParentObj );
    void    HandleHotPointEvent     ( const event& Event, object* pParentObj );
    void    HandleGenericEvent      ( const event& Event, object* pParentObj );
    void    HandleIntensityEvent    ( const event& Event, object* pParentObj );
    void    HandleDebrisEvent       ( const event& Event, object* pParentObj );
    void    HandlePainEvent         (       event& Event, object* pParentObj );
    void    HandleSetMeshEvent      ( const event& Event, object* pParentObj );
    void    HandleSwapMeshEvent     ( const event& event, object* pParentObj );
    void    HandleFadeGeometryEvent ( const event& Event, object* pParentObj );
    void    HandleSetVirtualTextureEvent( const event& event, object* pParentObj );
    void    HandleCameraFOVEvent    ( const event& Event, object* pParentObj );
    

    matrix4 m_Tranform;    
protected:
};

extern event_mgr g_EventMgr;

//==============================================================================
#endif // EVENT_MGR_HPP
//==============================================================================
