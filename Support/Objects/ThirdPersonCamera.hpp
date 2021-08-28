#ifndef THIRD_PERSON_CAMERA_HPP
#define THIRD_PERSON_CAMERA_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_math.hpp"
#include "object.hpp"
#include "e_View.hpp"
#include "ZoneMgr\ZoneMgr.hpp"

//=========================================================================
// THIRD_PERSON_CAMERA
//=========================================================================
class third_person_camera : public object
{
public:
    virtual const object_desc& GetTypeDesc              ( void ) const;
    static  const object_desc& GetObjectType            ( void );
    virtual bbox            GetLocalBBox                ( void ) const { return bbox( vector3( 0.0f, 0.0f, 0.0f ), vector3( 1.0f, 1.0f, 1.0f ) ); }
    virtual s32             GetMaterial                 ( void ) const { return 0;}
                            third_person_camera         ( void );
            //void            Setup                       ( const vector3& OrbitPoint, radian Pitch, radian Yaw, f32 Distance );
            void            Setup                       ( const vector3& InitialOrbitPoint, 
                                                          const vector3& IdealAimDirection, 
                                                          f32            StartDist, 
                                                          f32            EndDist,
                                                          object*        pOrbitObject );
            void            ComputeView                 ( view& View ) const;

            void            MoveTowards                 ( const vector3& DesiredPosition );
            void            MoveTowards                 ( radian Pitch, radian Yaw, f32 Distance );
            void            RotateYaw                   ( radian DeltaYaw );
            void            MoveTowardsPitch            ( radian NewPitch );
            void            SetOrbitPoint               ( const vector3& DesiredOrbitPoint );

            const vector3&  GetOrbitPoint               ( void ) const { return m_OrbitPoint; }
            f32             GetDistance                 ( void ) const { return m_Distance; }
    virtual void            OnAdvanceLogic              ( f32 DeltaTime );      
            xbool           HaveClearView               ( void ) const;

protected:
            xbool           CheckForObstructions        ( const vector3& Dir, f32 DistToCheck, f32& MaxDistFound );

private:
    vector3         m_OrbitPoint;
    vector3         m_DesiredOrbitPoint;
    vector3         m_OrbitPointVelocity;

    f32             m_Distance;
    f32             m_DesiredDistance;
    f32             m_DistanceVelocity;
    f32             m_DistanceAcceleration;

    radian          m_Pitch;
    radian          m_DesiredPitch;
    radian          m_PitchVelocity;

    radian          m_Yaw;
    radian          m_DesiredYaw;
    radian          m_YawVelocity;

    guid            m_HostPlayerGuid;

    vector3         m_CameraRodEnds[2];
    f32             m_CameraRodLength;
    s32             m_iDesiredRodEnd;
    vector3         m_CameraPos;

    zone_mgr::tracker   m_ZoneTracker;
}; 


//=========================================================================
// END
//=========================================================================

#endif //THIRD_PERSON_CAMERA_HPP
