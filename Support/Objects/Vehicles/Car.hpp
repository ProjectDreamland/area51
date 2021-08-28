//==============================================================================
//
//  Car.hpp
//
//==============================================================================

#ifndef __CAR_HPP__
#define __CAR_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "RigidBody.hpp"
#include "VehicleObject.hpp"


//==============================================================================
// CLASSES
//==============================================================================

// Class that represents a wheel
class wheel
{
// Friends
friend class car ;

// Data
protected:
    s32     m_iBone ;           // Bone index
    f32     m_Side ;            // 1=Left, -1=Right
    vector3 m_ShockOffset ;     // Offset relative to car
    f32     m_ShockLength ;     // Length of shock
    f32     m_Radius ;          // Radius of wheel

    f32     m_Length ;          // Current length of shock
    radian  m_Steer ;           // Steer yaw
    radian  m_Spin ;            // Spin roll
    radian  m_SpinSpeed ;       // Spinning speed
    matrix4 m_L2W ;             // Local -> world


// Functions
public:
            wheel() ;
            void Init           ( s32 iBone, f32 Side, const vector3& WheelOffset, f32 ShockLength, f32 Radius ) ;
            void Reset          ( void ) ;
            void ComputeForces  ( rigid_body& Car, f32 DeltaTime ) ;
            void Advance        ( rigid_body& Car, f32 DeltaTime, f32 Steer, f32 Accel ) ;

} ;


//==============================================================================

// Class that represents a car
class car : public vehicle_object
{
// Structures
private:


// Data
protected:

            // Wheel vars
            wheel       m_Wheels[4] ;   // FL, FR, RL, RR
            rigid_body  m_RigidBody;

// Functions
public:
            // Constructor/Destructor
            car() ;
    virtual ~car() ;

            // Initialization
    virtual void                Init            ( const char* pGeometryName,
                                                  const char* pAnimName, 
                                                  const matrix4& L2W,
                                                  guid  ObjectGuid = 0 ) ;
    virtual void                Reset           ( const matrix4& L2W ) ;

            // Computes forces
    virtual void                ComputeForces   ( f32 DeltaTime ) ;

            // Advances logic               
    virtual void                AdvanceSimulation( f32 DeltaTime ) ;
                                            
            // Renders geometry             
    virtual void                Render          ( void ) ;

            // Control functions
            void                SetSteer        ( f32 Steer ) ;
            void                SetAccel        ( f32 Accel ) ;
    virtual void                ZeroVelocities  ( void ) ;

                                           
} ;


//==============================================================================

#endif  // #ifndef __CAR_HPP__
