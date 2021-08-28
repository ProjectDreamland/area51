//==============================================================================
//
//  ConstraintPoint.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "PhysicsMgr.hpp"
#include "Constraint.hpp"
#include "Entropy.hpp"


//==============================================================================
// CONSTRAINT FUNCTIONS
//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

constraint::constraint() :
    m_BodyPos0( 0.0f, 0.0f, 0.0f ),
    m_BodyPos1( 0.0f, 0.0f, 0.0f ),
    m_MaxDist ( 0.0f ),
    m_pBody0( NULL ),
    m_pBody1( NULL )
{
    m_DebugColor = XCOLOR_RED;    // Color of constraint for debugging
    PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( sizeof( constraint ) );
}

//==============================================================================

constraint::~constraint()
{
    PHYSICS_DEBUG_DYNAMIC_MEM_FREE( sizeof( constraint ) );
}

#endif  //#ifndef ENABLE_PHYSICS_DEBUG


// Initialization with world position
void constraint::Init( rigid_body*      pBody0,
                       rigid_body*      pBody1,
                       const vector3&   WorldPos,
                       f32              MaxDist,
                       u32              Flags,
                       xcolor           DebugColor )
{
    ASSERT( pBody0 );
    ASSERT( pBody1 );

    m_pBody0     = pBody0;
    m_pBody1     = pBody1;
    m_BodyPos0   = pBody0->GetW2L() * WorldPos;
    m_BodyPos1   = pBody1->GetW2L() * WorldPos;
    m_MaxDist    = MaxDist;
    m_Flags      = Flags;

#ifdef ENABLE_PHYSICS_DEBUG
    m_DebugColor = DebugColor;
#else
    (void)DebugColor;
#endif
}

//==============================================================================

// Initialization with local position for each body
void constraint::Init( rigid_body*      pBody0,
                       rigid_body*      pBody1,
                       const vector3&   Body0Pos,
                       const vector3&   Body1Pos,
                       f32              MaxDist,
                       u32              Flags,
                       xcolor           DebugColor )
{
    ASSERT( pBody0 );
    ASSERT( pBody1 );

    m_pBody0     = pBody0;
    m_pBody1     = pBody1;
    m_BodyPos0   = Body0Pos;
    m_BodyPos1   = Body1Pos;
    m_MaxDist    = MaxDist;
    m_Flags      = Flags;

#ifdef ENABLE_PHYSICS_DEBUG
    m_DebugColor = DebugColor;
#else
    (void)DebugColor;
#endif
}

//==============================================================================

xbool constraint::PreApply( f32 DeltaTime, active_constraint& Active )
{
    // TO DO: Use prediction for max constraint?
    
    // Lookup bodies
    rigid_body* pBody0 = m_pBody0 ;
    rigid_body* pBody1 = m_pBody1 ;
    ASSERT( pBody0 );
    ASSERT( pBody1 );

#ifndef TARGET_PS2

    // Compute world space positions
    vector3 WorldPos0 = pBody0->GetL2W() * m_BodyPos0 ;
    vector3 WorldPos1 = pBody1->GetL2W() * m_BodyPos1 ;

    // Compute world delta
    vector3 Delta   = WorldPos0 - WorldPos1 ;
    f32     DistSqr = Delta.LengthSquared();

#else

    // m_WorldPos0   = pBody0->GetL2W() * m_BodyPos0 ;
    u128 WORLDPOS0;
    const matrix4& L2W0 = pBody0->GetL2W();
    asm(    "vmulaw.xyzw  acc,  COL3, vf00w
             vmaddaz.xyzw acc,  COL2, VINz
             vmadday.xyzw acc,  COL1, VINy
             vmaddx.xyzw  VOUT, COL0, VINx" :
            "=j VOUT" ( WORLDPOS0 ) :
            "j  COL0" ( L2W0.GetCol0_U128() ),
            "j  COL1" ( L2W0.GetCol1_U128() ),
            "j  COL2" ( L2W0.GetCol2_U128() ),
            "j  COL3" ( L2W0.GetCol3_U128() ),
            "j  VIN"  ( m_BodyPos0  .GetU128() ) );

    // m_WorldPos1   = pBody1->GetL2W() * m_BodyPos1 ;
    u128 WORLDPOS1;
    const matrix4& L2W1 = pBody1->GetL2W();
    asm(    "vmulaw.xyzw  acc,  COL3, vf00w
             vmaddaz.xyzw acc,  COL2, VINz
             vmadday.xyzw acc,  COL1, VINy
             vmaddx.xyzw  VOUT, COL0, VINx" :
            "=j VOUT" ( WORLDPOS1 ) :
            "j  COL0" ( L2W1.GetCol0_U128() ),
            "j  COL1" ( L2W1.GetCol1_U128() ),
            "j  COL2" ( L2W1.GetCol2_U128() ),
            "j  COL3" ( L2W1.GetCol3_U128() ),
            "j  VIN"  ( m_BodyPos1  .GetU128() ) );

    // Compute world delta and distance squared
    u128 DELTA, DOTVEC;
    f32  DistSqr;
    asm(    "vsub.xyz  VOUT, VIN0, VIN1"  : "=j VOUT" (DELTA) : "j VIN0" ( WORLDPOS0 ), "j VIN1" ( WORLDPOS1 ) );

    asm(    "vmul.xyz  DOUT, XYZW, XYZW" : "=j DOUT" (DOTVEC) : "j XYZW" (DELTA) );
    asm(    "vaddz.x   DOUT, DIN,  DINz
             vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DOTVEC)  : "j DIN" (DOTVEC) );
    asm(    "qmfc2     DOT,  DIN"        : "=r DOT"  (DistSqr) : "j DIN" (DOTVEC) );

    vector3 Delta( DELTA );
#endif

    // Constraint already satisfied?
    // NOTE: Always keep pin (zero dist) constraints active so that limbs don't jerk
    //       (fixes the punchbag dummy from jittering)
    f32 MaxDist    = m_MaxDist;
    f32 MaxDistSqr = x_sqr( MaxDist );
    if( ( m_MaxDist > 0.0f ) && ( DistSqr <= MaxDistSqr ) )
    {
        return FALSE;
    }

#ifdef TARGET_PS2
    vector3 WorldPos0( WORLDPOS0 );
    vector3 WorldPos1( WORLDPOS1 );
#endif

    // Compute mid pos and relative mid positions
    vector3 WorldMidPos = ( WorldPos0 + WorldPos1 ) * 0.5f;
    Active.m_RelMidPos0 = WorldMidPos - pBody0->GetPosition();
    Active.m_RelMidPos1 = WorldMidPos - pBody1->GetPosition();

    // Compute relative positions
    Active.m_RelPos0 = WorldPos0 - pBody0->GetPosition();
    Active.m_RelPos1 = WorldPos1 - pBody1->GetPosition();

    // Compute deviation distance between points?
    if( DistSqr > 0.0001f )
    {
        // Normalize direction between constraints
        f32 Dist = x_sqrt( DistSqr );
        Delta *= 1.0f / Dist;
        
        // Compute deviation dist from constraint limit
        if( Dist > MaxDist )
            Dist -= MaxDist;

        // Compute correction scaler
        f32 Extra            = Dist * g_PhysicsMgr.m_Settings.m_ConstraintFix / ( DeltaTime * 60.0f );
        f32 MaxConstraintFix = g_PhysicsMgr.m_Settings.m_MaxConstraintFix;
        if( Extra > MaxConstraintFix )
            Extra = MaxConstraintFix;
        else if( Extra < -MaxConstraintFix )
            Extra = -MaxConstraintFix;

        // Compute velocity correction based on deviation distance       
        Active.m_CorrectionVel = Delta * Extra;
    }
    else
    {
        // No correction needed or it can't be computed due to points being on top of each other
        Active.m_CorrectionVel.Zero();
    }
    
    // Needs solving
    return TRUE;
}

//==============================================================================

xbool constraint::Apply( active_constraint& Active )
{
    // Lookup bodies
    rigid_body* pBody0 = m_pBody0 ;
    rigid_body* pBody1 = m_pBody1 ;
    ASSERT( pBody0 );
    ASSERT( pBody1 );

#ifndef TARGET_PS2        

    // Compute velocities of each point
    vector3 Vel0 = pBody0->GetLinearVelocity() + v3_Cross( pBody0->GetAngularVelocity(), Active.m_RelPos0   );
    vector3 Vel1 = pBody1->GetLinearVelocity() + v3_Cross( pBody1->GetAngularVelocity(), Active.m_RelPos1   );

    // Compute relative velocity
    vector3 RelVel = Vel0 - Vel1 + Active.m_CorrectionVel;

#else

    // Lookup useful relative vels since we will use these quite a bit
    u128 RELPOS0 = Active.m_RelPos0.GetU128();
    u128 RELPOS1 = Active.m_RelPos1.GetU128();

    // Compute velocities of each point
    //vector3 Vel0 = pBody0->GetLinearVelocity() + v3_Cross( pBody0->GetAngularVelocity(), Active.m_RelPos0 );
    //vector3 Vel1 = pBody0->GetLinearVelocity() + v3_Cross( pBody0->GetAngularVelocity(), Active.m_RelPos1 );
    u128 VEL0;
    u128 VEL1;
    
    // VEL0 =  v3_Cross( pBody0->GetAngularVelocity(), Active.m_RelPos0 )   
    asm(    "vopmula.xyz   acc,    VEC0,   VEC1
             vopmsub.xyz   RES,    VEC1,   VEC0" :
            "=j RES"  ( VEL0 ) :
            "j  VEC0" ( pBody0->GetAngularVelocity().GetU128() ),
            "j  VEC1" ( RELPOS0 ) );

    // VEL1 = v3_Cross( pBody1->GetAngularVelocity(), m_RelPos1 )
    asm(    "vopmula.xyz   acc,    VEC0,   VEC1
             vopmsub.xyz   RES,    VEC1,   VEC0" :
            "=j RES"  ( VEL1 ) :
            "j  VEC0" ( pBody1->GetAngularVelocity().GetU128() ),
            "j  VEC1" ( RELPOS1 ) );

    // VEL0 += pBody0->GetLinearVelocity()
    asm(    "vadd.xyzw     VOUT, VEC0, VEC1" : 
            "=j VOUT" ( VEL0 ) : 
            "j VEC0"  ( pBody0->GetLinearVelocity().GetU128() ), 
            "j VEC1"  ( VEL0 ) );

    // VEL1 += pBody1->GetLinearVelocity()
    asm(    "vadd.xyzw     VOUT, VEC0, VEC1" : 
            "=j VOUT" ( VEL1 ) : 
            "j VEC0"  ( pBody1->GetLinearVelocity().GetU128() ), 
            "j VEC1"  ( VEL1 ) );

    // Compute relative velocity
    //vector3 RelVel = Vel0 - Vel1 + Active.m_CorrectionVel;
    u128 RELVEL;
    asm(    "vsub.xyz  VOUT, VIN0, VIN1"  : "=j VOUT" (RELVEL) : "j VIN0" ( VEL0 ), "j VIN1" ( VEL1 ) );
    asm(    "vadd.xyz  VOUT, VIN0, VIN1"  : "=j VOUT" (RELVEL) : "j VIN0" ( RELVEL ), "j VIN1" ( Active.m_CorrectionVel.GetU128() ) );
    vector3 RelVel( RELVEL );

#endif

    // Compute relative speed
    f32 RelSpeedSqr = RelVel.LengthSquared();
    if( RelSpeedSqr < 0.00001f )
        return FALSE;
    f32 RelSpeed = x_sqrt( RelSpeedSqr );

    // Compute impulse to satisfy constraint
    vector3 N = RelVel / RelSpeed;
    f32 Numerator   = -RelSpeed;
    f32 Denominator = pBody0->GetInvMass() + pBody1->GetInvMass();

#ifndef TARGET_PS2        

    // Add components
    vector3 DENOM0 = v3_Cross( pBody0->GetWorldInvInertia().RotateVector( v3_Cross( Active.m_RelPos0, N ) ), Active.m_RelPos0 );
    vector3 DENOM1 = v3_Cross( pBody1->GetWorldInvInertia().RotateVector( v3_Cross( Active.m_RelPos1, N ) ), Active.m_RelPos1 );
    Denominator += v3_Dot( N, DENOM0 + DENOM1 );
    
#else

    // DENOM0 = v3_Cross( pBody0->GetWorldInvInertia().RotateVector( v3_Cross( Active.m_RelPos0, N ) ), Active.m_RelPos0 ); 
    // DENOM1 = v3_Cross( pBody1->GetWorldInvInertia().RotateVector( v3_Cross( Active.m_RelPos1, N ) ), Active.m_RelPos1 );
    u128 DENOM0;
    u128 DENOM1;

    // DENOM0 = v3_Cross( m_RelPos0  , N )
    asm(    "vopmula.xyz   acc,    VEC0,   VEC1
             vopmsub.xyz   RES,    VEC1,   VEC0" :
            "=j RES"  ( DENOM0 ) :
            "j  VEC0" ( RELPOS0 ),
            "j  VEC1" ( N.GetU128() ) );

    // DENOM1 = v3_Cross( m_RelPos1  , N )
    asm(    "vopmula.xyz   acc,    VEC0,   VEC1
            vopmsub.xyz   RES,    VEC1,   VEC0" :
            "=j RES"  ( DENOM1 ) :
            "j  VEC0" ( RELPOS1 ),
            "j  VEC1" ( N.GetU128() ) );

    // DENOM0 = pBody0->GetWorldInvInertia().RotateVector( DENOM0 )
    const matrix4& WorldInvInertia0 = pBody0->GetWorldInvInertia();
    asm(    "vmulaz.xyzw acc,   COL2, VINz
             vmadday.xyzw acc,  COL1, VINy
             vmaddx.xyzw  VOUT, COL0, VINx" :
            "=j VOUT" ( DENOM0 ) :
            "j  COL0" ( WorldInvInertia0.GetCol0_U128() ),
            "j  COL1" ( WorldInvInertia0.GetCol1_U128() ),
            "j  COL2" ( WorldInvInertia0.GetCol2_U128() ),
            "j  VIN"  ( DENOM0 ) );

    // DENOM1 = pBody1->GetWorldInvInertia().RotateVector( DENOM1 )
    const matrix4& WorldInvInertia1 = pBody1->GetWorldInvInertia();
    asm(    "vmulaz.xyzw acc,   COL2, VINz
            vmadday.xyzw acc,  COL1, VINy
            vmaddx.xyzw  VOUT, COL0, VINx" :
            "=j VOUT" ( DENOM1 ) :
            "j  COL0" ( WorldInvInertia1.GetCol0_U128() ),
            "j  COL1" ( WorldInvInertia1.GetCol1_U128() ),
            "j  COL2" ( WorldInvInertia1.GetCol2_U128() ),
            "j  VIN"  ( DENOM1 ) );

    // DENOM0 = v3_Cross( DENOM0, m_RelPos0   )
    asm(    "vopmula.xyz   acc,    VEC0,   VEC1
             vopmsub.xyz   RES,    VEC1,   VEC0" :
            "=j RES"  ( DENOM0 ) :
            "j  VEC0" ( DENOM0 ),
            "j  VEC1" ( RELPOS0 ) );

    // DENOM1 = v3_Cross( DENOM1, m_RelPos1   )
    asm(    "vopmula.xyz   acc,    VEC0,   VEC1
             vopmsub.xyz   RES,    VEC1,   VEC0" :
            "=j RES"  ( DENOM1 ) :
            "j  VEC0" ( DENOM1 ),
            "j  VEC1" ( RELPOS1 ) );

    // Factor out dot product with N:
    // DENOM0 += DENOM1
    asm( "vadd.xyzw VOUT, VEC0, VEC1" : "=j VOUT" (DENOM0) : "j VEC0" (DENOM0), "j VEC1" (DENOM1) );
    
    // Perform the final computation:
    // Dot =   v3_Dot( N, v3_Cross( pBody0->GetWorldInvInertia().RotateVector( v3_Cross( m_RelPos0, N ) ), m_RelPos0   ) )
    //       + v3_Dot( N, v3_Cross( pBody1->GetWorldInvInertia().RotateVector( v3_Cross( m_RelPos1, N ) ), m_RelPos1   ) );
    u128 DOTVEC;
    f32  Dot;
    asm(    "vmul.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DOTVEC) : "j VEC0" (N.GetU128() ), "j VEC1" (DENOM0) );
    asm(    "vaddz.x   DOUT, DIN,  DINz
             vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DOTVEC) : "j DIN"  (DOTVEC) );
    asm(    "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DOTVEC) );

    Denominator += Dot;

#endif

    // Valid?    
    if ( Denominator < 0.00001f )
        return FALSE;

    // Apply impulse to active bodies only so other body is not woken up!
    vector3 NormalImpulse = Active.m_Weight * N * ( Numerator / Denominator );

    if( pBody0->IsActive() )
        pBody0->ApplyLocalImpulse(  NormalImpulse, Active.m_RelMidPos0   );

    if( pBody1->IsActive() )        
        pBody1->ApplyLocalImpulse( -NormalImpulse, Active.m_RelMidPos1   );

    return TRUE;
}

//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

// Render functions
void constraint::DebugRender ( void )
{
    // Lookup rigid bodies
    const rigid_body* pBody0 = GetRigidBody( 0 );
    const rigid_body* pBody1 = GetRigidBody( 1 );

    // Draw info
    if( pBody0 )
    {
        draw_Sphere( pBody0->GetL2W() * GetBodyPos( 0 ), 2.0f, m_DebugColor );
    }

    if( pBody1 )
    {        
        draw_Sphere( pBody1->GetL2W() * GetBodyPos( 1 ), 2.0f, m_DebugColor );
    }

    // Draw line between 
    if( pBody0 && pBody1 )
    {        
        draw_Line( pBody0->GetL2W() * GetBodyPos( 0 ), 
                   pBody1->GetL2W() * GetBodyPos( 1 ), 
                   m_DebugColor );
    }            
}

#endif    

//==============================================================================
