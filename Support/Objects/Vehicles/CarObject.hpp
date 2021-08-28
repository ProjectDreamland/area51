//==============================================================================
//
//  CarObject.hpp
//
//==============================================================================

#ifndef CAR_OBJECT_HPP
#define CAR_OBJECT_HPP

//==============================================================================
// INCLUDES
//==============================================================================
#include "VehicleObject.hpp"
#include "AudioMgr\AudioMgr.hpp"

//==============================================================================
// CLASSES
//==============================================================================
#define WHEEL_HISTORY_LENGTH    128

// Class that represents a wheel
class wheel
{
// Friends
friend class car_object;

// Data
protected:

    s32     m_iBone;           // Bone index
    vector3 m_Offset;          // Offset relative to car
    radian  m_Steer;           // Steer yaw
    vector3 m_Velocity;
    vector3 m_Position;
    vector3 m_OldPosition;
    guid    m_ObjectGuid;
    radian  m_Spin;             // Current rotation of wheel
    radian  m_SpinSpeed;        // Current rate of rotation
    xbool   m_bPassive;
    f32     m_LastShockLength;
    f32     m_CurrentShockLength;
    xbool   m_bHasTraction;
    f32     m_DebrisTimer;

    struct history
    {
        f32 ShockLength;
        f32 ShockSpeed;
        xbool bHasTraction;
    };

    s32     m_nLogicLoops;
    history m_History[WHEEL_HISTORY_LENGTH];

// Functions
public:
            wheel();
            void Init           ( s32 iBone, 
                                  const matrix4& VehicleL2W, 
                                  const vector3& WheelOffset, 
                                  guid ObjectGuid,
                                  xbool bPassive);

            void Reset          ( const matrix4& VehicleL2W );

            void Advance        ( f32 DeltaTime, const matrix4& VehicleL2W, f32 Accel, f32 Brake );
            void FinishAdvance  ( f32 DeltaTime, const matrix4& VehicleL2W );

            void ApplyInput         ( f32 DeltaTime, f32 Accel, f32 Brake );
            void EnforceCollision   ( void );
            void ApplyTraction      ( f32 DeltaTime, const matrix4& VehicleL2W );

};

//==============================================================================

// Class that represents a car
class car_object : public vehicle_object
{
// Real time type information
public:
    CREATE_RTTI( car_object, vehicle_object, object )

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    virtual const object_desc&  GetObjectType   ( void );

// Structures
private:


// Data
protected:

            // Wheel vars
            wheel       m_Wheels[4];   // FL, FR, RL, RR
            matrix4     m_BackupL2W;
            radian      m_OldStabilizationTheta;
            radian      m_StabilizationSpeed;

            voice_id    m_AudioIdle;
            voice_id    m_AudioRev;
            f32         m_RevTurbo;

            vector3     m_BubbleOffset[8];
            f32         m_BubbleRadius[8];
            s32         m_nBubbles;

            xbool       m_bDisplayShockInfo;

// Functions
public:
            // Constructor/Destructor
            car_object();
    virtual ~car_object();

protected:
            // Initialization
    virtual void  VehicleInit   ( void );
    virtual void  VehicleKill   ( void );
    virtual void  VehicleReset  ( void );
    virtual void  VehicleUpdate ( f32 DeltaTime );
    virtual void  VehicleRender ( void );

            void AdvanceSteering( f32 DeltaTime );
            void EnforceConstraints ( void );
            void UpdateAirStabilization ( f32 DeltaTime );
            void RenderShockHistory( void );
            void UpdateAudio( f32 DeltaTime );

            // Computes forces
            void ComputeL2W         ( void );
};


//==============================================================================

#endif  // #ifndef CAROBJECT_HPP
