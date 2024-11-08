//==============================================================================
//
//  Car.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "Car.hpp"
#include "Entropy.hpp"


//==============================================================================
// TEMP DATA
//==============================================================================

struct car_def
{
    f32     m_Gravity ;
    f32     m_Mass ;
    f32     m_MassYScale ;

    f32     m_LinearDamping ;
    f32     m_AngularDamping ;
    f32     m_Friction ;
    f32     m_Elasticity ;
                                        
    f32     m_ShockLength ;
    f32     m_ShockStrength ;
    f32     m_ShockDamping ;
                                        
    f32     m_WheelRadius ;
    f32     m_WheelForcePos ;
    f32     m_WheelMaxTraction ;
    f32     m_WheelMaxSideTraction ;
    f32     m_WheelMaxOffset ;

    f32     m_TorqueAccel ;
    f32     m_TorqueMax ;
    f32     m_TorqueBrake ;
    f32     m_TorqueMin ;
    f32     m_TorqueDamping ;
                                        
    f32     m_SteerMaxAngle ;
} ;

// JEEP
static car_def  g_Def = 
{
    -9.8f*100*2,     // m_Gravity
    1798.0f,        // m_Mass
    0.5f,           // m_MassYScale
                    
    0.04f,          // m_LinearDamping
    25000000,       // m_AngularDamping
    0.8f,           // m_Friction
    0.01f,          // m_Elasticity
                                    
    50.0f,          // m_ShockLength
    3000.0f,        // m_ShockStrength
    200.0f,         // m_ShockDamping
                                    
    53.0f,          // m_WheelRadius
    0.1f,           // m_WheelForcePos
    8.0f,           // m_WheelMaxTraction
    1000.0f,        // m_WheelMaxSideTraction
    -18.0f,         // m_WheelMaxOffset ;
                    
    15.0f,          // m_TorqueAccel
    10.0f,          // m_TorqueMax
    -15.0f,         // m_TorqueBrake
    -4.0f,          // m_TorqueMin
    1.0f,           // m_TorqueDamping
                                    
    35.0f,          // m_SteerMaxAngle
} ;


/*
// POLICE CAR
static car_def  g_Def = 
{
    -9.8f*100,  // m_Gravity
    1798.0f,    // m_Mass
    0.85f,      // m_MassYScale

    0.04f,      // m_LinearDamping
    25000000,   // m_AngularDamping
    0.8f,       // m_Friction
    0.01f,      // m_Elasticity
                                
    50.0f,      // m_ShockLength
    3000.0f,    // m_ShockStrength
    200.0f,     // m_ShockDamping
                                
    40.0f,      // m_WheelRadius
    0.17f,      // m_WheelForcePos
    8.0f,       // m_WheelMaxTraction
    1000.0f,    // m_WheelMaxSideTraction
    -12.0f,     // m_WheelMaxOffset ;

    15.0f,      // m_TorqueAccel
    10.0f,      // m_TorqueMax
    -15.0f,     // m_TorqueBrake
    -4.0f,      // m_TorqueMin
    1.0f,       // m_TorqueDamping
                                
    35.0f,      // m_SteerMaxAngle
} ;
*/

//==============================================================================
// WHEEL CLASS
//==============================================================================

wheel::wheel()
{
    m_iBone         = -1 ;      // Bone index
    m_Side          = 0 ;       // 1=Left, -1=Right
    m_ShockOffset.Zero() ;      // Offset relative to car
    m_ShockLength   = 0 ;       // Length of shock
    m_Length        = 0 ;       // Current length of shock
    m_Radius        = 0 ;       // Radius of wheel

    m_Steer         = 0 ;       // Steer yaw
    m_Spin          = 0 ;       // Spin roll
    m_SpinSpeed     = 0 ;       // Spinning speed
    m_L2W.Identity() ;          // Local -> world
}

//==============================================================================

void wheel::Init( s32 iBone, f32 Side, const vector3& WheelOffset, f32 ShockLength, f32 Radius )
{
    m_iBone       = iBone ;                                     // Bone index
    m_Side        = Side ;                                      // 1=Left, -1=Right
    m_ShockOffset = WheelOffset + vector3(0,ShockLength,0) ;    // Offset relative to car
    m_ShockLength = ShockLength ;                               // Length of shock
    m_Length      = ShockLength ;                               // Current length of shock
    m_Radius      = Radius  ;                                   // Radius of wheel
    m_Steer       = 0 ;                                         // Steer yaw
    m_Spin        = 0 ;                                         // Spin roll
    m_SpinSpeed   = 0 ;                                         // Spinning speed
    m_L2W.Identity() ;                                          // Local -> world
}

//==============================================================================

void wheel::Reset( void )
{
    m_Length      = m_ShockLength ;                             // Current length of shock
    m_Steer       = 0 ;                                         // Steer yaw
    m_Spin        = 0 ;                                         // Spin roll
    m_SpinSpeed   = 0 ;                                         // Spinning speed
}

//==============================================================================

void wheel::ComputeForces( rigid_body& Car, f32 DeltaTime )
{
    // If car is upside down, skip
    const matrix4& CarL2W = Car.GetL2W() ;
    f32 YDir = CarL2W(1,1) ;
    if (YDir < 0)
        return ;

    // Compute info
    f32 ShockLength = m_ShockLength ;
    f32 Mass        = Car.GetMass() ;

    // Get world anchor and wheel pos
    vector3 SpringTop = CarL2W * m_ShockOffset ;
    vector3 SpringBot = CarL2W * (m_ShockOffset - vector3(0,ShockLength+m_Radius,0)) ;
    vector3 WheelPos  = SpringBot ;

    // Cast ray to see where ground is
    f32 T      = 1 ;
    f32 Length = ShockLength + m_Radius ;
    plane Plane ;
    Car.CheckCollision(SpringTop, SpringBot, Plane, T) ;
    
    // Cap
    if (T > 1)
        T = 1 ;

    // Below ground?
    if (T < 0)
        T = 1 - T ;     // Use full length of spring + dist under the ground!

    // Collided?
    if (T < 1)
    {
        // Setup spring bot and wheel pos
        SpringBot = SpringTop + (T * (SpringBot - SpringTop)) ;
        WheelPos  = SpringTop + (g_Def.m_WheelForcePos * (SpringBot - SpringTop)) ;

        // Remove wheel radius
        T -= m_Radius / Length ;
        if (T < 0)
            T = 0 ;

        // Get current spring length
        Length *= T ;

        // Get velocity of spring
        f32 V = (m_Length - Length) / DeltaTime ;

        // Spring constants
        f32 Strength = g_Def.m_ShockStrength / m_ShockLength ;
        f32 Damp     = g_Def.m_ShockDamping  / m_ShockLength ;

        // Setup force to correct spring length
        f32 F = Strength * (ShockLength - Length) ;

        // Apply dampening force
        F += Damp * V ;

        F *= Mass ;

        // Only push up
        if (F < 0)
            F = 0 ;
        else
        {
            // Add suspension spring force
            vector3 Force = Plane.Normal * F ;
            Car.AddForce(SpringTop, Force) ;
           
            // Add force due to traction

            // Compute wheel direction
            vector3 WheelOut     = vector3(m_L2W(0,0), m_L2W(0,1), m_L2W(0,2)) * m_Side ;
		    vector3 WheelForward = WheelOut.Cross(Plane.Normal) * m_Side ;
            WheelOut.Normalize() ;
            WheelForward.Normalize() ;

            // Compute velocity of contact point
            vector3 TireVel = (WheelForward * m_SpinSpeed * m_Radius) ;
            vector3 CarVel  = Car.GetVelocity(SpringTop) ;
            vector3 Vel     = TireVel + CarVel ;

            // Compute traction and cap
            f32 Trac    = F ;
            f32 MaxTrac = Mass * g_Def.m_WheelMaxTraction ;
            if (Trac > MaxTrac)
                Trac = MaxTrac ;

            // Apply friction so tires do not slide sideways
            f32     SideTrac = Trac * 2.0f * -WheelOut.Dot(CarVel) ;
            f32     MaxSideTrac = Mass * g_Def.m_WheelMaxSideTraction ;
            if (SideTrac > MaxSideTrac)
                SideTrac = MaxSideTrac ;
            else
            if (SideTrac < -MaxSideTrac)
                SideTrac = -MaxSideTrac ;
            vector3 SideFric = SideTrac * WheelOut ;
            Car.AddForce(WheelPos, SideFric) ;

            // Split velocity into components along plane
            vector3 Para, Perp ;
            Plane.GetComponents(Vel, Para, Perp) ;

            // Add traction force
            Force = -Para * Trac ;
            Car.AddForce(WheelPos, Force) ;

              // Add force due to friction
            f32 WheelFric = Force.Dot( WheelForward ) / (m_Radius*Mass) ;
			m_SpinSpeed += WheelFric * DeltaTime ;
        }

        // Store current length for next time
        m_Length = Length ;
    }
    else
        m_Length = ShockLength ;
}

//==============================================================================

void wheel::Advance( rigid_body& Car, f32 DeltaTime, f32 Steer, f32 Accel )
{
    // Update spin speed
    if (Accel > 0)
    {
        // Accelerating
        m_SpinSpeed += Accel * R_360 * g_Def.m_TorqueAccel * DeltaTime ;
        if (m_SpinSpeed > (R_360 * g_Def.m_TorqueMax))
            m_SpinSpeed = R_360 * g_Def.m_TorqueMax ;
    }
    else
    if (Accel < 0)
    {
        // Braking
        m_SpinSpeed += Accel * R_360 * -g_Def.m_TorqueBrake * DeltaTime ;
        if (m_SpinSpeed < (R_360 * g_Def.m_TorqueMin))
            m_SpinSpeed = (R_360 * g_Def.m_TorqueMin) ;
    }
    else
    {
        // Natural friction
        if (m_SpinSpeed > 0)
        {
            m_SpinSpeed -= m_SpinSpeed * g_Def.m_TorqueDamping * DeltaTime ;
            if (m_SpinSpeed < R_1)
                m_SpinSpeed = 0 ;
        }
        else
        {
            m_SpinSpeed -= m_SpinSpeed * g_Def.m_TorqueDamping * DeltaTime ;
            if (m_SpinSpeed > -R_1)
                m_SpinSpeed = 0 ;
        }
    }

    // Update spin
    m_Spin -= m_SpinSpeed * DeltaTime ;

    // Update steer
    m_Steer = Steer * DEG_TO_RAD(g_Def.m_SteerMaxAngle) ;

    // Compute wheel pos
    vector3 WheelPos = Car.GetL2W() * (m_ShockOffset - vector3(0,MAX(m_ShockLength + g_Def.m_WheelMaxOffset, m_Length),0)) ;

    // Setup wheel rotation
    matrix4 Rot(radian3(m_Spin, m_Steer, 0)) ;

    // Setup L2W
    m_L2W = Car.GetL2W() * Rot ;
    m_L2W.SetTranslation(WheelPos) ;
}

//==============================================================================
// CAR CLASS
//==============================================================================

// Constructor/Destructor
car::car()
{
    m_Accel = 0 ;   // Acceleration from -1 to 1
    m_Steer = 0 ;   // Steer from -1 to 1
}

//==============================================================================

car::~car()
{
}

//==============================================================================

// Computes forces
void car::ComputeForces( f32 DeltaTime )
{
    // Call base class
    m_RigidBody.ComputeForces(DeltaTime) ;

    // Add spring forces from wheels
    m_Wheels[0].ComputeForces(*this, DeltaTime) ;
    m_Wheels[1].ComputeForces(*this, DeltaTime) ;
    m_Wheels[2].ComputeForces(*this, DeltaTime) ;
    m_Wheels[3].ComputeForces(*this, DeltaTime) ;
}

//==============================================================================

// Initialization
void car::Init( const char* pGeometryName, 
                       const char* pAnimName,
                       const matrix4& L2W,
                       guid  ObjectGuid /*= 0*/ )
{
    s32 i ;

    // Setup properties
    m_Gravity.Y         = g_Def.m_Gravity ;         // Gravity of body
    m_Mass              = g_Def.m_Mass ;            // Mass of body in kilograms
    m_InvMass           = 1.0f / m_Mass ;           // (1.0f / Mass) of body
    m_LinearDamping     = g_Def.m_LinearDamping ;   // Linear velocity damping
    m_AngularDamping    = g_Def.m_AngularDamping ;  // Angular velcity damping
    m_Friction          = g_Def.m_Friction ;        // Friction (0 = none, 1 = full)
    m_Elasticity        = g_Def.m_Elasticity ;      // Bouncyness (0 = none, 1 = 100%)

    // Call base class
    rigid_body::Init(pGeometryName, pAnimName, L2W, g_Def.m_MassYScale, ObjectGuid) ;

    // Lookup animation pointer
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    ASSERT(pAnimGroup) ;

    // Initialize wheels
    for (i = 0 ; i < 4 ; i++)
    {
        // Lookup bone index
        s32 iBone = -1 ;
        switch(i)
        {
            case 0: iBone = pAnimGroup->GetBoneIndex("Tire Front L") ; break ;
            case 1: iBone = pAnimGroup->GetBoneIndex("Tire Front R") ; break ;
            case 2: iBone = pAnimGroup->GetBoneIndex("Tire Rear L") ;  break ;
            case 3: iBone = pAnimGroup->GetBoneIndex("Tire Rear R") ;  break ;
        }

        // Incase object is not found
        if (iBone == -1)
            iBone = 0 ;

        // Lookup offset 
        vector3 Offset = pAnimGroup->GetBone(iBone).BindTranslation + m_RenderOffset ;

        // Initialize wheel
        m_Wheels[i].Init(iBone,                     // iBone
                         (i & 1) ? 1.0f : -1.0f,    // Side
                         Offset,                    // Offset
                         g_Def.m_ShockLength,       // ShockLength
                         g_Def.m_WheelRadius ) ;    // Radius of wheel
    }

    // Initialize car_doors
    m_NDoors = 0 ;
    for (i = 0 ; i < 2 ; i++)
    {
        // Lookup bone index
        s32 iBone = -1 ;
        switch(i)
        {
            case 0: iBone = pAnimGroup->GetBoneIndex("Door Front L") ; break ;
            case 1: iBone = pAnimGroup->GetBoneIndex("Door Front R") ; break ;
        }

        // Door not found?
        if (iBone == -1)
            continue ;

        // Get bbox of car_door
        bbox BBox = GetRigidGeomBoneBBox(m_RigidInst.GetRigidGeom(), iBone) ;

        // Take render offset into account
        BBox.Min += m_RenderOffset ;
        BBox.Max += m_RenderOffset ;

        // Compute hinge offset
        vector3 HingeOffsets[2], DoorOffset ; 

        HingeOffsets[0].X = (BBox.Min.X + BBox.Max.X) * 0.5f ;
        HingeOffsets[0].Y = BBox.Min.Y ;
        HingeOffsets[0].Z = BBox.Min.Z ;

        HingeOffsets[1].X = (BBox.Min.X + BBox.Max.X) * 0.5f ;
        HingeOffsets[1].Y = BBox.Max.Y ;
        HingeOffsets[1].Z = BBox.Min.Z ;
        
        DoorOffset.X      = (BBox.Min.X + BBox.Max.X) * 0.5f ;
        DoorOffset.Y      = (BBox.Min.Y + BBox.Max.Y) * 0.5f ; ;
        DoorOffset.Z      = BBox.Max.Z ;

        // Initialize car_door
        m_Doors[m_NDoors].Init(*this,                   // Car
                        iBone,                          // iBone
                        (m_NDoors & 1) ? 1.0f : -1.0f,  // Side
                        HingeOffsets,                   // HingeOffsets
                        DoorOffset ) ;                  // Door offsets

        // Update door count
        m_NDoors++ ;
    }
}

//==============================================================================

void car::Reset( const matrix4& L2W )
{
    s32 i ;

    // Call base class
    rigid_body::Reset(L2W) ;

    // Reset wheels
    for (i = 0 ; i < 2 ; i++)
        m_Wheels[i].Reset() ;

    // Reset car_doors
    for (i = 0 ; i < m_NDoors ; i++)
        m_Doors[i].Reset(*this) ;

    // Clear movement
    m_Accel = 0 ;   // Acceleration from -1 to 1
    m_Steer = 0 ;   // Steer from -1 to 1
}
//==============================================================================

// Advances logic
void car::AdvanceSimulation( f32 DeltaTime )
{
    s32 i ;

    // Anything to do?
    if ((!m_bActive) || (DeltaTime == 0))
        return ;

    // Call base class
    rigid_body::AdvanceSimulation(DeltaTime) ;

    // Advance front wheels
    if (m_Accel < 0)
    {
        m_Wheels[0].Advance(*this, DeltaTime, m_Steer, m_Accel) ;
        m_Wheels[1].Advance(*this, DeltaTime, m_Steer, m_Accel) ;
    }
    else
    {
        m_Wheels[0].Advance(*this, DeltaTime, m_Steer, 0) ;
        m_Wheels[1].Advance(*this, DeltaTime, m_Steer, 0) ;
    }

    // Advance rear wheels
    m_Wheels[2].Advance(*this, DeltaTime, 0, m_Accel) ;
    m_Wheels[3].Advance(*this, DeltaTime, 0, m_Accel) ;

    // Advance car_doors
    for (i = 0 ; i < m_NDoors ; i++)
        m_Doors[i].Advance(*this, DeltaTime) ;
}

//==============================================================================

// Renders geometry
void car::Render( void )
{
    s32 i ;
   
    // Allocate matrices
    s32      NBones    = m_NBones ;
    matrix4* pMatrices = (matrix4*)smem_BufferAlloc(NBones * sizeof(matrix4)) ;
    if (!pMatrices)
        return ;

    // Lookup animation pointer
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    ASSERT(pAnimGroup) ;
    
    // Setup matrices
    for (i = 0 ; i < NBones ; i++)
    {
        pMatrices[i] = GetL2W() ;
        pMatrices[i].PreTranslate(m_RenderOffset) ;
    }

    // Setup wheel matrices
    for (i = 0 ; i < 4 ; i++)
    {
        wheel& Wheel = m_Wheels[i] ;
        pMatrices[Wheel.m_iBone] = Wheel.m_L2W * pAnimGroup->GetBoneBindInvMatrix(Wheel.m_iBone) ;
    }

    // Setup door matrices
    for (i = 0 ; i < m_NDoors ; i++)
    {
        car_door& Door = m_Doors[i] ;
        pMatrices[Door.m_iBone] = Door.m_L2W * Door.m_InvBindL2W ;
    }
/*
    // Draw geometry
    m_RigidInst.Render(pMatrices, m_pColors, render::NORMAL | render::CLIPPED) ;
*/
#ifdef TARGET_PC

    // Draw debug info
    draw_ClearL2W() ;
    for (i = 0 ; i < 4 ; i++)
    {
        // Get wheel
        wheel& Wheel = m_Wheels[i] ;

        // Get pos
        vector3 Pos = Wheel.m_L2W.GetTranslation() ;

        // Draw vectors
        draw_Sphere(Pos, Wheel.m_Radius, XCOLOR_YELLOW) ;
    }

    // Draw bbox
    draw_SetL2W(GetL2W()) ;
    draw_BBox(m_LocalBBox, XCOLOR_WHITE) ;

    // Draw collision verts
    for (i = 0 ; i < m_NVertices ; i++)
        draw_Point(GetL2W() * m_Vertices[i], XCOLOR_YELLOW) ;
    
    draw_ClearL2W() ;

    // Draw car vel
    draw_Label(GetL2W().GetTranslation(), XCOLOR_RED, "Vel:%.2f %.2f %.2f",
               GetVelocity().X/60.0f,
               GetVelocity().Y/60.0f,
               GetVelocity().Z/60.0f) ;

    // Draw car_doors
    for (i = 0 ; i < m_NDoors ; i++)
    {
        car_door& Door = m_Doors[i] ;

        // Draw info
        draw_Label(Door.m_DoorPos, XCOLOR_RED, "Vel:%.2f %.2f %.2f",
                   Door.m_DoorPos.X - Door.m_DoorLastPos.X,
                   Door.m_DoorPos.Y - Door.m_DoorLastPos.Y,
                   Door.m_DoorPos.Z - Door.m_DoorLastPos.Z) ;

        // Draw car_door particles
        xcolor C = XCOLOR_GREEN ;
        if (i == 1)
            C = XCOLOR_BLUE ;                

        draw_Line(GetL2W() * Door.m_HingeOffsets[0], Door.m_DoorPos, C) ;
        draw_Line(GetL2W() * Door.m_HingeOffsets[1], Door.m_DoorPos, C) ;

        draw_Marker(GetL2W() * Door.m_HingeOffsets[0], C) ;
        draw_Marker(GetL2W() * Door.m_HingeOffsets[1], C) ;
        draw_Marker(Door.m_DoorPos, C) ;
    }
#endif
}


//==============================================================================
// Control functions
//==============================================================================

void car::SetSteer( f32 Steer )
{
    // Cap
    if (Steer < -1)
        Steer = -1 ;
    else
    if (Steer > 1)
        Steer = 1 ;

    // Keep
    m_Steer = Steer ;
}

//==============================================================================

void car::SetAccel( f32 Accel )
{
    // Cap
    if (Accel < -1)
        Accel = -1 ;
    else
    if (Accel > 1)
        Accel = 1 ;

    // Keep
    m_Accel = Accel ;
}

//==============================================================================

void car::ZeroVelocities( void )
{
    // SOMETHING FISHY!!!!
    //rigid_body::ZeroVelocities() ;
    for (s32 i = 0 ; i < 4 ; i++)
        m_Wheels[i].m_SpinSpeed = 0 ;
}

//==============================================================================
