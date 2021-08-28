//=========================================================================
//
//  LocoWheelController.hpp
//
//  Automatically animates wheels from current motion
//
//=========================================================================

#ifndef __LOCO_WHEEL_CONTROLLER_HPP__
#define __LOCO_WHEEL_CONTROLLER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoAnimController.hpp"


//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class loco;


//=========================================================================
// CLASS LOCO_WHEEL_CONTROLLER
//=========================================================================
class loco_wheel_controller : public loco_anim_controller
{
// Defines
public:
        enum defines
        {
            MAX_WHEELS = 4
        };
        
// Structures
public:
        // Simple wheel tracking structure
        struct wheel
        {
            s32     m_iBone;    // Index of bone
            f32     m_Radius;   // Radius of wheel
            vector3 m_Pos;      // Position of wheel
            radian  m_DeltaRot; // Rotation speed
            radian  m_Rot;      // Current rotation
        };
        
// Construction / destruction
public:

        // Constructs a loco_wheel_controller object.
        loco_wheel_controller();

// Destroys a loco_wheel_controller object, handles cleanup and de-allocation.
virtual ~loco_wheel_controller();

// Member functions
public:

        // Initialize
        xbool               Init                ( loco*                     pLoco,
                                                  const anim_group::handle& hAnimGroup,
                                                  const char*               pTurnLoopSfx,
                                                  const char*               pTurnStopSfx,
                                                  const char*               pMoveLoopSfx,
                                                  const char*               pMoveStopSfx );
                                                  
// Sets location of animation data package
virtual void                SetAnimGroup        ( const anim_group::handle& hAnimGroup ) ;

        // Mixes the anims keyframes into the dest keyframes
virtual void                MixKeys             ( const info& Info, anim_key* pDestKey ); 

        // Advances animation and returns delta pos and delta yaw
virtual void                Advance             ( f32 DeltaTime, vector3& DeltaPos, radian& DeltaYaw );

        // Returns # of wheels
        s32                 GetWheelCount       ( void ) const { return m_nWheels ; }
        
// Member variables
protected:

        loco*       m_pLoco;                    // Owner loco
                
        wheel       m_Wheels[MAX_WHEELS];       // List of wheels
        s32         m_nWheels;                  // # of wheels
        
        const char* m_pTurnLoopSfx;             // Turn loop sound name
        const char* m_pTurnStopSfx;             // Turn stop sound name
        const char* m_pMoveLoopSfx;             // Move loop sound name
        const char* m_pMoveStopSfx;             // Move stop sound name
        
        s32         m_TurnLoopSfxId;            // Turn loop sound id
        s32         m_MoveLoopSfxId;            // Move loop sound id
};

//=========================================================================

#endif // __LOCO_WHEEL_CONTROLLER_HPP__

