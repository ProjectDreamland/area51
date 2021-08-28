#ifndef __TRACKER_HPP__
#define __TRACKER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Path.hpp"
#include "ZoneMgr\ZoneMgr.hpp"


//=========================================================================
// CLASS
//=========================================================================

class tracker : public object
{
//=====================================================================
// DEFINES
//=====================================================================
public:

    // Available play types
    enum play_type
    {
        PLAY_TYPE_ONCE,         // Play path once and stop at the end
        PLAY_TYPE_LOOP,         // Play to end, then jump to start
        PLAY_TYPE_PING_PONG     // Ping-pong between start and end
    } ;

//=====================================================================
// PUBLIC BASE CLASS FUNCTIONS
//=====================================================================
public:

    CREATE_RTTI( tracker, object, object )
    
                            tracker         ( void );
    virtual bbox            GetLocalBBox    ( void ) const; 
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnMove          ( const vector3& NewPos   );        
    virtual void            OnTransform     ( const matrix4& L2W      ); 

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void            OnActivate          ( xbool bFlag ) ;

//=====================================================================
// PRIVATE BASE CLASS FUNCTIONS
//=====================================================================

protected:
    virtual void            OnInit              ( void );     
    virtual void            OnAdvanceLogic      ( f32 DeltaTime );    

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

//=====================================================================
// PUBLIC FUNCTIONS
//=====================================================================

public:
    virtual void            Update                  ( xbool bSendKeyEvents ) ;
            path*           GetPath                 ( void ) ;
            const path::key& GetCurrentKey          ( void ) const { return m_CurrentKey ;  }
            const zone_mgr::tracker GetZoneTracker  ( void ) const { return m_ZoneTracker ; }
            xbool                  IsEditorSelected ( void );
            void            SetTime                 ( f32 Time );
            f32             GetTime                 ( void );
            void            SetDirection            ( f32 Direction );
            void            SetPath                 ( guid gPath );

//=====================================================================
// DATA
//=====================================================================

protected:

    zone_mgr::tracker   m_ZoneTracker ;         // Current zone info
    guid                m_PathGuid ;            // Guid of path to track
    play_type           m_PlayType ;            // Play back type
    f32                 m_PrevTime ;            // Previous time
    f32                 m_Time ;                // Current time
    f32                 m_Speed ;               // Speed along path
    f32                 m_Direction ;           // Current direction
    path::key           m_CurrentKey ;          // Current key
    f32                 m_SizeMult;             // BBox size multiplier
};

//=========================================================================
// END
//=========================================================================
#endif
