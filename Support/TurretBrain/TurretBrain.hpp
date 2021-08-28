//==============================================================================
#ifndef __TURRET_BRAIN_HPP__
#define __TURRET_BRAIN_HPP__
//==============================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Animation\AnimPlayer.hpp"
#include "Objects\Actor\Actor.hpp"

//=========================================================================
// CLASS
//=========================================================================

class turret_brain
{
public:

    turret_brain();

    enum tracking_status
    {
        TRACK_NO_TARGET     = 0,    // No target
        TRACK_AIMING,               // In process of aiming, can't fire yet
        TRACK_OUTSIDE_FOF,          // Target is outside field of fire
        TRACK_LOCKED,               // Locked on target, ready to fire
    };

            void            OnEnumProp      ( prop_enum& List );
            xbool           OnProperty      ( prop_query& I );


    virtual void                SetTurretGuid   ( guid Guid );
    virtual void                SetTargetGuid   ( guid Guid );
    virtual xbool               IsTargetValid   ( void ) const;
    virtual void                SetL2W          ( const matrix4& L2W );
    virtual tracking_status     UpdateTracking  ( f32 DeltaTime );   
    virtual radian3             GetRotations    ( void ) const;    
    virtual void                SetSensorPos    ( const vector3& SensorPos );


    virtual xbool               CheckLOS        ( const vector3& Pt ) const;    // This CheckLOS is NOT buffered.
                                                                                // This version is used by the buffered
                                                                                // CheckLOS, so feel free to provide a
                                                                                // different one in a derived class

            xbool               CheckLOS        ( guid Guid );                  // This CheckLOS IS buffered. It will only check 
                                                                                // every second, if the Guid is the same as the
                                                                                // last one passed in.
    
    virtual xbool               CanSenseTarget  ( void );                       // Is target within sense radius
    virtual xbool               CanFireAtTarget ( void );                       // Is target within fire radius
    virtual xbool               IsTargetInRange ( void ) const;                 // Is target within either radii

            vector3     GetObjectAimAt          ( guid              Guid,       // Gets position to aim at for guid
                                                  actor::eOffsetPos Pos = actor::OFFSET_EYES ) const; 

            xbool           GetTargetSightYaw   ( guid Guid, radian& Yaw )const;// Gets facing direction of target object
            f32             DistToObjectAimAt   ( guid Guid ) const;            // Dist from sensor to object aim-at pt
    
protected:

    guid        m_TurretGuid;
    guid        m_TargetGuid;

    matrix4     m_W2L;

    vector3     m_SensorPos;
    vector3     m_LocalTargetDir;

    f32         m_SenseRadius;
    f32         m_FireRadius;
        
    f32         m_LOSCheckTimer;    
    slot_id     m_ObjSlotForLastLOSCheck;   // Slot of object last passed to CheckLOS

    u8          m_bLastLOSPassed:1;

};

//==============================================================================
#endif // __TURRET_BRAIN_HPP__
//==============================================================================