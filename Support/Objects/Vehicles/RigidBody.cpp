#ifndef DISABLE_LEGACY_CODE
//==============================================================================
//
//  RigidBody.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "RigidBody.hpp"
#include "Entropy.hpp"
#include "Obj_mgr\obj_mgr.hpp"
f32 g_STAB = 1.0f;

//==============================================================================
// CLASSES
//==============================================================================

// Constructor/Destructor
rigid_body::rigid_body()
{
    // Geometry vars
    m_ObjectGuid = 0;                      // Owner object
    m_nVertices = 0;                       // # of vertices
    m_RenderOffset.Zero();                 // Rigid instance render offset
    m_LocalBBox.Clear();                   // BBox of all local verts                
    //m_NBones = 0;                          // # of bones in heirarchy

    // Definition physics
    m_bActive           = TRUE;            // TRUE if active
    m_Gravity.Set(0, -9.8f*100, 0) ;       // World space gravity
    m_InvLocalInertiaTensor.Identity();    // Local space inertia tensor
    m_Mass              = 1.0f;            // Mass of body
    m_InvMass           = 1.0f;            // (1.0f / Mass) of body
    m_LinearDamping     = 0.04f;           // Linear velocity damping
    m_AngularDamping    = 0.03f;           // Angular velcity damping
    m_Friction          = 0.1f;            // Friction (0 = none, 1 = full)
    m_Elasticity        = 0.4f;            // Bouncyness (0 = none, 1 = 100%)
    
    // Dynamic physics                      
    m_Position.Zero();                     // World position
    m_Orientation.Identity();              // World orientation
                                            
    m_LinearVelocity.Zero();               // Linear velocity
    m_AngularVelocity.Zero();              // Angular velocity
                              
    m_LinearMomentum.Zero();               // World space velocity * mass
    m_AngularMomentum.Zero();              // World space angular velocity * mass
    
    m_Force.Zero();                        // Total of forces on CM
    m_Torque.Zero();                       // Total moment on CM

    m_InvWorldInertiaTensor.Identity();    // Inverse of world space intertial tensor

    m_DeltaTime = 0;                       // Current delta time
}

//==============================================================================

rigid_body::~rigid_body()
{
}

//==============================================================================

// Adds force to body
void rigid_body::AddForce( const vector3& Position, const vector3& Force )
{
    // Linear force
    m_Force  += Force;

    // Angular force
    m_Torque += (Position - m_Position).Cross(Force);
}

//==============================================================================

// Adds impulse (immediate force) to body
void rigid_body::AddImpulse( const vector3& Position, const vector3& Force )
{
    // Linear force
    m_LinearMomentum += Force;

    // Angular force
    m_AngularMomentum += (Position - m_Position).Cross(Force);
}

//==============================================================================

void rigid_body::ProcessCollision( const vector3& P, const plane& Plane )
{
    // Get distance to plane
    f32 Dist = Plane.Distance(P);

    // Get velocity of point
    vector3 R   = P - m_Position;
    vector3 Vel = m_LinearVelocity + m_AngularVelocity.Cross(R);

    // Relative velocity
    f32 VRel = Plane.Normal.Dot(Vel);

    // Moving into plane?
    if (VRel < 0)
    {

        // Caclulate numerator of impulse
        f32     ImpulseNum = -(1.0f + m_Elasticity) * VRel;

        // Caclulate denominator of impulse
        vector3 VTemp  = ( m_InvWorldInertiaTensor * R.Cross( Plane.Normal ) ).Cross( R );
        f32     ImpulseDenom = m_InvMass * Plane.Normal.Dot( Plane.Normal ) + VTemp.Dot( Plane.Normal );

        // Find the Final Impulse
        vector3 ImpulseForce = Plane.Normal * (ImpulseNum/ImpulseDenom);

        // Apply the Impulse to the body 
        AddImpulse( P, ImpulseForce );

        // Get velocity parallel with plane
        vector3 Para, Perp;
        Plane.GetComponents(Vel, Para, Perp);

        // Apply friction force parallel to plane
        vector3 FrictionForce = -Para * m_Friction * m_Mass;
        AddForce(P, FrictionForce);

        // Add stabalizing force
        vector3 StabForce = Plane.Normal * -Dist * m_Mass * g_STAB;
        AddForce(P, StabForce);
    }
}

//==============================================================================

// Checks collision and applies forces when needed
void rigid_body::CheckCollision( void )
{
    plane Plane;
    f32   Dist;

    // Only consider collisions less than this distance to plane
    f32 BestDist = 0.1f; 

    // Get local -> world matrix
    const matrix4& L2W = GetL2W();

    // Loop through all vertices in body
    for (s32 i = 0; i < m_nVertices; i++)
    {
        // Get world space position
        vector3 P = L2W * m_Vertex[i];

        // Collision?
        if (CheckCollision(P, Plane, Dist))
        {
            // Below threshold?
            if (Dist < BestDist)
            {
                // Get velocity of point
                vector3 R   = P - m_Position;
                vector3 Vel = m_LinearVelocity + m_AngularVelocity.Cross(R);

                // Relative velocity
                f32 VRel = Plane.Normal.Dot(Vel);

                // Moving into plane?
                if (VRel < 0)
                {
                    BestDist = Dist;
                    ProcessCollision(P, Plane);
                }
            }
        }
    }
}

//==============================================================================

// Returns TRUE if there is a collision, along with the collision depth
xbool rigid_body::CheckCollision( const vector3& P, plane& Plane, f32& Depth )
{
    // Just check ground plane
    if (!m_ObjectGuid)
    {
        Plane.Setup(0,1,0, 0);
        Depth = Plane.Distance(P);
        return (Depth < 0);
    }

    // Get height from terrain...

    // Setup ray for getting terrain height
    vector3 Start = P + vector3(0,1000.0f, 0);
    vector3 End   = P;

    // Prepare for collision
    g_CollisionMgr.RaySetup(m_ObjectGuid, Start, End);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );

    // If we hit something, then fail
    if (g_CollisionMgr.m_nCollisions)
    {
        Plane = g_CollisionMgr.m_Collisions[0].Plane;
        Depth = Plane.Distance(P);
        return TRUE;
    }
    else
    {
        // No collision
        Plane.Setup(0,1,0,0);
        Depth = 0.0f;
        return FALSE;
    }
}

//==============================================================================

// Check ray collision
xbool rigid_body::CheckCollision( const vector3& S, const vector3& E, plane& Plane, f32& T )
{
    // Just check ground plane
    if (!m_ObjectGuid)
    {
        Plane.Setup(0,1,0, 0);
        return (Plane.Intersect(T, S, E));
    }

    // Get height from terrain...

    // Prepare for collision
    g_CollisionMgr.RaySetup(m_ObjectGuid, S, E);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );

    // If we hit something, then fail
    if (g_CollisionMgr.m_nCollisions)
    {
        Plane = g_CollisionMgr.m_Collisions[0].Plane;
        T     = g_CollisionMgr.m_Collisions[0].T;
        return TRUE;
    }
    else
    {
        // No collision
        Plane.Setup(0,1,0,0);
        T = 1.0f;
        return FALSE;
    }
}

//==============================================================================

// Computes forces
void rigid_body::ComputeForces( f32 DeltaTime )
{
    (void)DeltaTime;

    // Add damping
    m_Force  -= m_LinearDamping  * m_LinearVelocity;
    m_Torque -= m_AngularDamping * m_AngularVelocity;

    // Add gravity
    AddForce(m_Position, m_Gravity * m_Mass);
}

//==============================================================================

// Returns index of vertex if found, or -1 if not found
s32 rigid_body::FindVertex( const vector3& V ) const
{
    // Check all verts
    for (s32 i = 0; i < m_nVertices; i++)
    {
        // Found?
        if (m_Vertex[i] == V)
            return i;
    }

    // Not found
    return -1;
}

//==============================================================================

// Initialization
void rigid_body::Init( const rigid_geom& RigidGeom,
                       const matrix4& L2W,
                             f32      MassYScale /* =1.0f */,  
                            guid      ObjectGuid /*= 0 */)
{
    s32 i;

    // Keep owner
    m_ObjectGuid = ObjectGuid;

    //
    // Collect the unique vertices from the collision model
    // and build a bbox around them
    //
    {
        m_LocalBBox.Clear();
        for( i=0; i<RigidGeom.m_Collision.nLowVectors; i++ )
        {
            vector3 V = RigidGeom.m_Collision.pLowVector[i];

            // If vertex not found, add to list
            if( (FindVertex(V) == -1) && (m_nVertices<MAX_RIGID_BODY_VERTICES) )
            {
                m_Vertex[m_nVertices++] = V;
                m_LocalBBox += V;
            }
        }
    }
    
    // Center the local bbox
    vector3 Center = m_LocalBBox.GetCenter();
    vector3 Size   = m_LocalBBox.GetSize();
    Size *= 0.5f;

    // Shrink vertically by 50%
    Size.Y *= 0.5f;

    // Scale on Y so center of mass is near the base of the car
    //Size.Y   *= MassYScale;
    //Center.Y *= MassYScale;

    m_LocalBBox.Min = -Size+Center;
    m_LocalBBox.Max =  Size+Center;
    m_RenderOffset  = -vector3(Center.X,m_LocalBBox.Min.Y,Center.Z);

    m_Vertex[0] = vector3( m_LocalBBox.Min.X, m_LocalBBox.Min.Y, m_LocalBBox.Min.Z );
    m_Vertex[1] = vector3( m_LocalBBox.Min.X, m_LocalBBox.Min.Y, m_LocalBBox.Max.Z );
    m_Vertex[2] = vector3( m_LocalBBox.Min.X, m_LocalBBox.Max.Y, m_LocalBBox.Min.Z );
    m_Vertex[3] = vector3( m_LocalBBox.Min.X, m_LocalBBox.Max.Y, m_LocalBBox.Max.Z );
    m_Vertex[4] = vector3( m_LocalBBox.Max.X, m_LocalBBox.Min.Y, m_LocalBBox.Min.Z );
    m_Vertex[5] = vector3( m_LocalBBox.Max.X, m_LocalBBox.Min.Y, m_LocalBBox.Max.Z );
    m_Vertex[6] = vector3( m_LocalBBox.Max.X, m_LocalBBox.Max.Y, m_LocalBBox.Min.Z );
    m_Vertex[7] = vector3( m_LocalBBox.Max.X, m_LocalBBox.Max.Y, m_LocalBBox.Max.Z );
    m_nVertices = 8;

    //for (i = 0; i < m_nVertices; i++)
    //    m_Vertex[i] -= Center;


    // Set local->world
    Reset(L2W);
}

//==============================================================================

void ComputeBoxInverseInertiaTensor( const vector3& Size, f32 Mass, matrix3& I )
{
    f32     XX = Size.X * Size.X;
    f32     YY = Size.Y * Size.Y;
    f32     ZZ = Size.Z * Size.Z;
    f32     M  = Mass / 12.0f;

    I.Identity();
    I(0,0) = 1.0f / (M * (YY + ZZ));
    I(1,1) = 1.0f / (M * (XX + ZZ));
    I(2,2) = 1.0f / (M * (XX + YY));
}

void rigid_body::Reset( const matrix4& L2W )
{
    // Dynamic physics                      
    SetL2W(L2W);
                                            
    m_LinearVelocity.Zero();               // Linear velocity
    m_AngularVelocity.Zero();              // Angular velocity
                              
    m_LinearMomentum.Zero();               // World space velocity * mass
    m_AngularMomentum.Zero();              // World space angular velocity * mass
    
    m_Force.Zero();                        // Total of forces on CM
    m_Torque.Zero();                       // Total moment on CM

    m_InvWorldInertiaTensor.Identity();    // Inverse of world space intertial tensor

    // Setup inertia tensor
    vector3 Size = m_LocalBBox.GetSize();
    ComputeBoxInverseInertiaTensor(Size, m_Mass, m_InvLocalInertiaTensor);

    // TEST
    //m_AngularMomentum = vector3(100*100,100*75,100*95);
    //m_LinearMomentum  = vector3(0,0,100*10);
}

//==============================================================================

inline 
void ApplyAngularVelocity( quaternion& O, const vector3& AngVel, f32 DeltaTime )
{
    vector3 V = AngVel * DeltaTime * 0.5f;
    quaternion DeltaRot;
    DeltaRot.X = + (O.W * V.X) + (V.Y * O.Z) - (V.Z * O.Y);
    DeltaRot.Y = + (O.W * V.Y) + (V.Z * O.X) - (V.X * O.Z);
    DeltaRot.Z = + (O.W * V.Z) + (V.X * O.Y) - (V.Y * O.X);
    DeltaRot.W = - (O.X * V.X) - (V.Y * O.Y) - (V.Z * O.Z);

    O.X += DeltaRot.X;
    O.Y += DeltaRot.Y;
    O.Z += DeltaRot.Z;
    O.W += DeltaRot.W;
    O.Normalize();
}


// Integrates movement
// Currently using simple Euler integrator
void rigid_body::Integrate( f32 DeltaTime )
{
    //======================================================================
    // Linear movement
    //======================================================================


    // Compute acceleration
    m_LinearMomentum += m_Force * DeltaTime;
    
    // Compute new velocity
    m_LinearVelocity = m_LinearMomentum * m_InvMass;

    // Compute new position
    m_Position       += m_LinearVelocity * DeltaTime;


/*
// 2nd order!
F=CalcForce();
acc=F/m;
pos+=vel*t+0.5*acc*t*t;
vel+=acc*t;
*/

    //======================================================================
    // Angular movement
    //======================================================================

    // Compute new orientation
    ApplyAngularVelocity(m_Orientation, m_AngularVelocity, DeltaTime);

    // Compute new angular momentum
    m_AngularMomentum += m_Torque * DeltaTime;

    // Compute inverse world space inertia tensor
    // (NOTE: LocalInteriaTensor already takes the inverse mass into account)
    matrix3 O(m_Orientation);
    matrix3 TO = O;
    TO.Transpose();
    
    // [Orientation] [invObjectInertiaTensor] [Orientation]-1
    m_InvWorldInertiaTensor = O * m_InvLocalInertiaTensor * TO;
    //m_InvWorldInertiaTensor = TO * m_InvLocalInertiaTensor * O;

    // Compute local space angular velocity
    m_AngularVelocity = m_InvWorldInertiaTensor * m_AngularMomentum;
}

//==============================================================================

// Advances logic
void rigid_body::AdvanceSimulation( f32 DeltaTime )
{
    // Anything to do?
    if (!m_bActive)
        return;

    // Compute all forces acting on body
    ComputeForces(DeltaTime);

    // Apply collision forces
    //CheckCollision();

    // Compute new linear and angular position
    Integrate(DeltaTime);

    // Update local -> world matrix
    m_L2W.Identity();
    m_L2W.SetRotation(m_Orientation);
    m_L2W.SetTranslation(m_Position);

    // Clear force and torque for next time
    m_Force.Zero();
    m_Torque.Zero();
}

//==============================================================================

void rigid_body::Advance( f32 DeltaTime )
{
    // Accululate time
    m_DeltaTime += DeltaTime;

    // Advance in time steps
    f32 TimeStep = 1.0f / 60.0f;
    while(m_DeltaTime >= TimeStep)
    {
        AdvanceSimulation(TimeStep);
        m_DeltaTime -= TimeStep;
    }
}

//==============================================================================

// Renders geometry
void rigid_body::Render( void )
{
    s32 i;

    draw_SetL2W( GetL2W() );
    draw_BBox(m_LocalBBox, XCOLOR_WHITE );
    
    for( i=0; i<m_nVertices; i++ )
        draw_Point( m_Vertex[i], XCOLOR_WHITE );
/*
    // Render geometry
    
    // Allocate matrices
    s32      NBones    = m_NBones;
    matrix4* pMatrices = (matrix4*)smem_BufferAlloc(NBones * sizeof(matrix4));
    if (!pMatrices)
        return;
    
    // Setup matrices
    for (s32 i = 0; i < NBones; i++)
    {
        pMatrices[i] = GetL2W();
        pMatrices[i].PreTranslate(m_RenderOffset);
    }

//    m_RigidInst.Render(pMatrices, m_pColors, render::NORMAL | render::CLIPPED);

    draw_SetL2W(GetL2W());
    draw_BBox(m_LocalBBox, XCOLOR_WHITE);
    draw_ClearL2W();
*/
}


//==============================================================================
// Position functions
//==============================================================================

const vector3& rigid_body::GetPosition ( void ) const
{
    return m_Position;
}

//==============================================================================

const quaternion& rigid_body::GetOrientation ( void ) const
{
    return m_Orientation;
}

//==============================================================================

const matrix4& rigid_body::GetL2W( void ) const
{
    return m_L2W;
}

//==============================================================================

f32 rigid_body::GetMass( void ) const
{
    return m_Mass;
}

//==============================================================================

const bbox& rigid_body::GetLocalBBox( void ) const
{
    return m_LocalBBox;
}

//==============================================================================

const vector3& rigid_body::GetLinearMomentum( void ) const
{
    return m_LinearMomentum;
}

//==============================================================================

const vector3& rigid_body::GetRenderOffset( void ) const
{
    return m_RenderOffset;
}

//==============================================================================

const vector3& rigid_body::GetGravity( void ) const
{
    return m_Gravity;
}

//==============================================================================

void rigid_body::SetPosition( const vector3& Pos )
{
    m_Position = Pos;
    m_L2W.SetTranslation(Pos);
}

//==============================================================================

void rigid_body::SetOrientation( const radian3& Rot )
{
    m_Orientation.Setup(Rot);
    m_L2W.SetRotation(Rot);
}

//==============================================================================

void rigid_body::SetL2W( const matrix4& L2W )
{
    m_Position    = L2W.GetTranslation();
    m_Orientation = L2W.GetQuaternion();
    m_L2W         = L2W;
}

//==============================================================================

void rigid_body::ZeroVelocities( void )
{
    m_LinearMomentum.Zero();          // World space velocity * Mass
    m_AngularMomentum.Zero();         // World space angular velocity * mass
}

//==============================================================================

vector3 rigid_body::GetVelocity( const vector3& P ) const
{
    // Linear
    vector3 Vel = m_LinearVelocity;

    // Angular
    Vel += m_AngularVelocity.Cross(P - m_Position);

    return Vel;
}

//==============================================================================

vector3 rigid_body::GetAngularVelocity  ( const vector3& P ) const
{
    // Angular
    return m_AngularVelocity.Cross(P - m_Position);
}

//==============================================================================

vector3 rigid_body::GetVelocity( void ) const
{
    return m_LinearVelocity;
}

//==============================================================================
// Query functions
//==============================================================================

//==============================================================================

//==============================================================================
// UTIL FUNCTIONS
//==============================================================================

// Returns bounding box of rigid geom bone
bbox GetRigidGeomBoneBBox( const rigid_geom* pRigidGeom, s32 iBone )
{
    // Clear bbox
    xbool bFound = FALSE;
    bbox BBox;
    BBox.Clear();

#ifdef TARGET_PC
    // Loop through all dlists
    for (s32 i = 0; i < pRigidGeom->m_nDList; i++)
    {
        // Lookup dlist
         const rigid_geom::dlist_pc& DList = pRigidGeom->m_System.pPC[i];
        
        // Skip if not this bone
        if (DList.iBone != iBone)
            continue;
        
        // Flag bone was found
        bFound = TRUE;

        // Add all verts to bounding box for this bone
        for (s32 j = 0; j < DList.nVerts; j++)
            BBox += DList.pVert[j].Pos;
    }

#endif

#ifdef TARGET_PS2

    // Loop through all dlists
    for (s32 i = 0; i < pRigidGeom->m_nDList; i++)
    {
        // Lookup dlist
         const rigid_geom::dlist_ps2& DList = pRigidGeom->m_System.pPS2[i];
        
        // Skip if not this bone
        if (DList.iBone != iBone)
            continue;

        // Flag bone was found
        bFound = TRUE;

        // Add all verts to bounding box for this bone
        for (s32 j = 0; j < DList.nVerts; j++)
            BBox += vector3(DList.pPosition[j].X, DList.pPosition[j].Y, DList.pPosition[j].Z);
    }
#endif

    // Use origin if bone not found
    if (!bFound)
        BBox += vector3(0,0,0);

    // Done
    return BBox;
}

//==============================================================================
#endif
