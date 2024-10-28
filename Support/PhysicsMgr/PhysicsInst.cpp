//==============================================================================
//
//  PhysicsInst.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "PhysicsInst.hpp"
#include "PhysicsMgr.hpp"
#include "Entropy.hpp"
#include "Loco\LocoCharAnimPlayer.hpp"
#include "Objects\BaseProjectile.hpp"
#include "MiscUtils\SimpleUtils.hpp"

#ifdef TARGET_PS2
#include "Entropy/PS2/ps2_spad.hpp"
#endif


//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

inline
void ClampForce( vector3& Force )
{
#ifdef ENABLE_PHYSICS_DEBUG
    // Record max force
    if( Force.LengthSquared() > 0.1f )
        g_PhysicsMgr.m_Profile.m_MaxForce = x_max( g_PhysicsMgr.m_Profile.m_MaxForce, Force.Length() );

#endif

    ASSERT( Force.IsValid() );
    
    // Too big?
    f32 Max         = g_PhysicsMgr.m_Settings.m_MaxForce;
    f32 ForceMagSqr = Force.LengthSquared();
    if( ForceMagSqr > x_sqr( Max ) )
    {
        // Compute clamping scale
        f32 ForceMag = x_sqrt( ForceMagSqr );
        f32 Scale    = Max / ForceMag;
        
        // Clamp
        Force *= Scale;
    }
}

//==============================================================================
// FUNCTIONS
//==============================================================================

physics_inst::physics_inst()
{
    m_bInitialized           = FALSE;
    m_bInAwakeList           = FALSE;
    m_bInSleepingList        = FALSE;
    m_bInCollisionWakeupList = FALSE;
    m_bPopFix                = FALSE;
    m_bActorCollision        = FALSE;
    m_bWorldCollision        = TRUE;
    m_bInstCollision         = TRUE;
    m_bActiveWhenVisible     = FALSE;
    m_WorldBBox.Clear();
    m_ConstraintWeight = 1.0f;
    m_ConstraintWeightDelta = 0.0f;
    m_KeepActiveTime = 0.0f;
    m_Zone = 0xFF;
    
    PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( sizeof( physics_inst ) );
}

//==============================================================================

physics_inst::~physics_inst()
{
    PHYSICS_DEBUG_DYNAMIC_MEM_FREE( sizeof( physics_inst ) );

    // Free
    Kill();
}

//==============================================================================
// Initialization functions
//==============================================================================

void ComputeSpacing( f32 Height, f32 Radius, s32 nSpheres, f32& Offset, f32& Spacing )
{
    // Simple case
    if( nSpheres == 1 )
    {
        Offset  = 0.0f;
        Spacing = 0.0f;
    }
    
    // Compute total height used by spheres
    f32 SpheresHeight = Radius * 2.0f * nSpheres;
    f32 Delta = Height - SpheresHeight;
        
    // Stretch?
    if( Delta >= 0 )
    {
        Delta /= (f32)(nSpheres+1);
        Offset  = ( -Height * 0.5f ) + Delta + Radius;
        Spacing = ( Radius * 2.0f ) + Delta;
    }
    else
    {
        // Shrink
        f32 Top = Radius - ( Height * 0.5f );
        f32 Bot = -Top;
        Offset = Top;
        Spacing = ( Bot - Top ) / (nSpheres-1);
    }
}

//==============================================================================

xbool physics_inst::Init( xbool bPopFix, f32 ConstraintBlendTime )
{
    s32 i;
    
    // Free current memory
    Kill();
    
    // Lookup geometry
    const geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return FALSE;
  
    // No rigid bodies?
    if( pGeom->m_nRigidBodies == 0 )
        return FALSE;
        
    // Setup constraint blending?
    ASSERT( ConstraintBlendTime >= 0.0f );
    if( ConstraintBlendTime > 0.0f )
    {
        // Blend in constraints
        m_ConstraintWeight      = 0.0f;
        m_ConstraintWeightDelta = 1.0f / ConstraintBlendTime;
    }
    else
    {
        // Constraints are fully active
        m_ConstraintWeight      = 1.0f;
        m_ConstraintWeightDelta = 0.0f;
    }
            
    // Allocate collision group
    s32 CollisionGroup = g_PhysicsMgr.GetNextCollisionGroup();
   
    // Compute pre-allocation counts
    s32 nRigidBodies     = pGeom->m_nRigidBodies;
    s32 nCollisionShapes = pGeom->m_nRigidBodies;
    s32 nConstraints     = ( nRigidBodies - 1 ) * 4;
    
    // Pre-allocate physics components
    m_RigidBodies.SetCount( 0 );
    m_RigidBodies.SetGrowAmount( 1 );
    m_RigidBodies.SetCapacity( nRigidBodies );
    m_CollisionShapes.SetCount( 0 );
    m_CollisionShapes.SetGrowAmount( 1 );
    m_CollisionShapes.SetCapacity( nCollisionShapes );
    m_BodyBodyContraints.SetCount( 0 );
    if( nConstraints )
    {
        m_BodyBodyContraints.SetGrowAmount( 4 );
        m_BodyBodyContraints.SetCapacity( nConstraints );
    }
    m_BodyWorldContraints.SetCount( 0 );
    m_BodyWorldContraints.SetGrowAmount( 4 );
            
    // Pre-allocate pop fix matrices?
    m_bPopFix = bPopFix;
    if( m_bPopFix )
    {
        // Allocate pop fix matrices
        m_PopFixMatrices.SetGrowAmount( 1 );
        m_PopFixMatrices.SetCount( pGeom->m_nBones );
        
        PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( sizeof( matrix4 ) * m_PopFixMatrices.GetCapacity() );
        
        // Clear pop bind matrices
        for( i = 0; i < pGeom->m_nBones; i++ )
            m_PopFixMatrices[ i ].Identity();
    }
        
    // Build physics components
    for( i = 0; i < nRigidBodies; i++ )
    {   
        // Lookup geometry rigid body info
        const geom::rigid_body& GeomRigidBody = pGeom->m_pRigidBodies[i];
              
        // Compute size
        vector3 Size( GeomRigidBody.Width, GeomRigidBody.Height, GeomRigidBody.Length );
        
        // Compute radius
        f32     Radius = GeomRigidBody.Radius;
        
        // Compute bind L2W
        matrix4 BodyBindL2W;
        BodyBindL2W.Setup( vector3( 1.0f, 1.0f, 1.0f ), 
                       GeomRigidBody.BodyBindRotation, 
                       GeomRigidBody.BodyBindPosition );

        // Compute world pivot info
        matrix4 PivotBindL2W;
        vector3 PivotWorldPos = GeomRigidBody.PivotBindPosition;
        PivotBindL2W.Setup( vector3( 1.0f, 1.0f, 1.0f ), 
                            GeomRigidBody.PivotBindRotation, 
                            GeomRigidBody.PivotBindPosition );

        // Create rigid body
        rigid_body& RigidBody = m_RigidBodies.Append();
        
        // Init rigid body
        RigidBody.SetCollisionInfo( CollisionGroup, i, GeomRigidBody.CollisionMask );
        RigidBody.SetPrevL2W( BodyBindL2W );
        RigidBody.SetL2W( BodyBindL2W );
        RigidBody.ZeroLinearVelocity();
        RigidBody.ZeroAngularVelocity();
        RigidBody.SetLinearDamping( 0.1f );
        RigidBody.SetAngularDamping( i == 0 ? 0.2f : 100000.0f );   // Let root bone spin!
        RigidBody.SetElasticity( 0.1f );
        RigidBody.SetDynamicFriction( 0.8f );
        RigidBody.SetStaticFriction( 0.9f );
        RigidBody.ClearForces();
        RigidBody.SetWorldCollision( ( GeomRigidBody.Flags & geom::rigid_body::FLAG_WORLD_COLLISION ) != 0 );
        
        // Create collision shape
        collision_shape& Collision = m_CollisionShapes.Append();
        switch( GeomRigidBody.Type )
        {
            default:
                ASSERTS( 0, "Geometry rigid body data is corrupt!" );
                
            case geom::rigid_body::TYPE_SPHERE:
            {
                // Setup simple sphere
                Collision.SetType( collision_shape::TYPE_SPHERE );
                Collision.SetSphereCapacity( 1 );
                Collision.AddSphere( vector3( 0,0,0 ) );
            }            
            break;
            
            case geom::rigid_body::TYPE_CYLINDER:
            {
                f32     Height      = Size.GetY();
                s32     nSpheres    = x_max( 1, (s32)( 0.5f + ( Height / ( Radius * 2.0f ) ) ) );
                
                vector3 Offset( 0, 0, 0 );
                vector3 Delta ( 0, 0, 0 );
                ComputeSpacing( Height, Radius, nSpheres, Offset.GetY(), Delta.GetY() );

                // Set shape type
                if( nSpheres == 1 )
                    Collision.SetType( collision_shape::TYPE_SPHERE );
                else                    
                    Collision.SetType( collision_shape::TYPE_CAPSULE );
                
                // Create spheres and flag as capsule
                Collision.SetSphereCapacity( nSpheres );
                for( s32 j = 0; j< nSpheres; j++ )
                {
                    Collision.AddSphere( Offset );
                    Offset += Delta;
                }                    
            }                
            break;
            
            case geom::rigid_body::TYPE_BOX:
            {
                // Compute radius and other axis
                Radius = x_min( Size.GetX(), x_min( Size.GetY(), Size.GetZ() ) );
                s32 Axis0 = 0;
                s32 Axis1 = 0;
                if( Radius == Size.GetX() )
                {
                    Axis0 = 1;
                    Axis1 = 2;
                }
                else if( Radius == Size.GetY() )
                {
                    Axis0 = 0;
                    Axis1 = 2;
                }
                else
                {
                    Axis0 = 0;
                    Axis1 = 1;
                }
                Radius *= 0.5f;
                
                f32 Size0 = Size[ Axis0 ];
                f32 Size1 = Size[ Axis1 ];
                
                s32 nSpheres0 = x_max( 1, (s32)( 0.5f + ( Size0 / ( Radius * 2.0f ) ) ) );
                s32 nSpheres1 = x_max( 1, (s32)( 0.5f + ( Size1 / ( Radius * 2.0f ) ) ) );
                
                vector3 Offset0(0,0,0), Delta0(0,0,0);
                vector3 Offset1(0,0,0), Delta1(0,0,0);
                
                ComputeSpacing( Size0, Radius, nSpheres0, Offset0[ Axis0 ], Delta0[ Axis0 ] );
                ComputeSpacing( Size1, Radius, nSpheres1, Offset1[ Axis1 ], Delta1[ Axis1 ] );
                
                
                // Set shape type
                if( ( nSpheres0 == 1 ) && ( nSpheres1 == 1 ) )
                    Collision.SetType( collision_shape::TYPE_SPHERE );
                else if( ( nSpheres0 == 1 ) || ( nSpheres1 == 1 ) )                    
                    Collision.SetType( collision_shape::TYPE_CAPSULE );
                else
                    Collision.SetType( collision_shape::TYPE_BOX );
                
                // Create spheres
                Collision.SetSphereCapacity( nSpheres0 * nSpheres1 );
                for( s32 c0 = 0; c0 < nSpheres0; c0++ )
                {
                    vector3 Offset = Offset0 + Offset1;
                    for( s32 c1 = 0; c1 < nSpheres1; c1++ )
                    {
                        Collision.AddSphere( Offset );
                        Offset += Delta1;
                    }                        
                    
                    Offset0 += Delta0;
                }                    
            }
            break;
        }
        
        // Set shape and mass properties (let root bone rotate easier)
        ASSERT( Collision.GetNSpheres() );  // Make sure some spheres were created!
        Collision.SetRadius( Radius );
        RigidBody.SetCollisionShape( &Collision, GeomRigidBody.Mass, ( i == 0 ) ? 50.0f : 100.0f );
        
        // Connect to parent?
        if( GeomRigidBody.iParentBody != -1 )
        {
            // Clear twist angle
            f32 TwistAngle = 0.0f;
        
            // Get parent body
            ASSERT( GeomRigidBody.iParentBody < i );
            rigid_body& ParentRigidBody = m_RigidBodies[ (s32)GeomRigidBody.iParentBody ];
            
            // Create hinge?
            if( 1 )
            {            
                // Create hinge?
                for( s32 j = 0; j < 3; j++ )
                {
                    // Lookup rotation DOFs
                    const geom::rigid_body::dof& GeomDOF  = GeomRigidBody.DOF[ j + geom::rigid_body::dof::DOF_RX ];
                    const geom::rigid_body::dof& PrevDOF  = GeomRigidBody.DOF[((j-1+3) % 3) + geom::rigid_body::dof::DOF_RX];
                    const geom::rigid_body::dof& NextDOF  = GeomRigidBody.DOF[((j+1)   % 3) + geom::rigid_body::dof::DOF_RX];
                    
                    // Create hinge on this axis?
                    if(     ( GeomDOF.Flags & geom::rigid_body::dof::FLAG_ACTIVE )
                        &&  ( GeomDOF.Flags & geom::rigid_body::dof::FLAG_LIMITED ) )
                    {
                        // Setup axis, direction, and angle sign
                        s32 Axis = j;
                        
                        // Setup pivot local rotation axis
                        vector3 PivotLocalAxis( 0, 0, 0 );
                        PivotLocalAxis[ Axis ] = 1.0f;
                    
                        // Setup pivot world rotation axis
                        vector3 PivotWorldAxis = PivotBindL2W.RotateVector( PivotLocalAxis );
                        PivotWorldAxis.Normalize();
                    
                        // Compute width of hinge
                        f32 W = Collision.ComputeLocalBBox().GetSize()[ Axis ];
                        
                        // Increase width of hinge constraints to make more stable on small rigid bodies
                        W *= 4.0f;
                    
                        // Compute hinge points in world space
                        vector3 HingeOffset = PivotWorldAxis * 0.5f * W;
                        vector3 Hinge0      = PivotWorldPos - HingeOffset;
                        vector3 Hinge1      = PivotWorldPos + HingeOffset;
                        
                        // Compute sloppy twist amount
                        f32 TwistMaxDist = 0.0f;
                        if( ( PrevDOF.Flags & geom::rigid_body::dof::FLAG_LIMITED ) == 0 )
                            TwistAngle = x_max( TwistAngle, PrevDOF.Max - PrevDOF.Min );
                        if( ( NextDOF.Flags & geom::rigid_body::dof::FLAG_LIMITED ) == 0 )
                            TwistAngle = x_max( TwistAngle, NextDOF.Max - NextDOF.Min );
                        if( TwistAngle != 0 )
                        {
                            // Convert to max distance constraint
                            vector3 P0( 0.0f, 0.0f, W*0.5f );
                            vector3 P1( 0.0f, 0.0f, W*0.5f );
                            P1.RotateY( DEG_TO_RAD( TwistAngle * 0.5f ) );
                            TwistMaxDist = (P0 - P1).Length();
                        }
                   
                        // Add hinge0 constraint
                        constraint& Constraint0 = m_BodyBodyContraints.Append();
                        Constraint0.Init( &RigidBody,                   // pBody0
                                          &ParentRigidBody,             // pBody1
                                          Hinge0,                       // WorldPos0
                                          TwistMaxDist,                 // MaxDist
                                          constraint::FLAG_BLEND_IN,    // Flags
                                          XCOLOR_BLUE );                // DebugColor
                                                                
                        // Add hinge1 constraint
                        constraint& Constraint1 = m_BodyBodyContraints.Append();
                        Constraint1.Init( &RigidBody,                   // pBody0
                                          &ParentRigidBody,             // pBody1
                                          Hinge1,                       // WorldPos0
                                          TwistMaxDist,                 // MaxDist
                                          constraint::FLAG_BLEND_IN,    // Flags
                                          XCOLOR_RED );                 // DebugColor
                                          
                        // Limit angle?
                        if( GeomDOF.Flags & geom::rigid_body::dof::FLAG_LIMITED )
                        {
                            // Lookup limits
                            f32 AngleMin = GeomDOF.Min;
                            f32 AngleMax = GeomDOF.Max;
                            
                            //TEMP!
                            //AngleMin = AngleMax;  // MAX POSE TEST
                            //AngleMax = AngleMin;  // MIN POSE TEST
                            
                            // Compute min, max, and mid quaternion rotations
                            quaternion QMinRot, QMaxRot, QMidRot;
                            QMinRot.Setup( PivotWorldAxis, DEG_TO_RAD( AngleMin ) );
                            QMaxRot.Setup( PivotWorldAxis, DEG_TO_RAD( AngleMax ) );
                            QMidRot = BlendSlow( QMaxRot, QMinRot, 0.5f );

                            // Compute min, max, and mid matrix rotations
                            matrix4 MinRot, MaxRot, MidRot;
                            MinRot.Setup( QMinRot );
                            MaxRot.Setup( QMaxRot );
                            MidRot.Setup( QMidRot );
                            
                            // Compute min/max limit world space positions (rotating around pivot position)
                            // Reading from right->left:
                            //    WorldMinTM = PivotBindPos * MinRot * InvPivotBindPos * BodyBindL2W
                            // Optimizes to this:
                            
                            // Put into world space, then make relative to pivot pos
                            matrix4 InvPivotBindPos_BodyBindL2W = BodyBindL2W;
                            InvPivotBindPos_BodyBindL2W.Translate( -PivotWorldPos );
                            
                            // Apply rotation around pivot
                            matrix4 WorldMinTM = MinRot * InvPivotBindPos_BodyBindL2W;
                            matrix4 WorldMaxTM = MaxRot * InvPivotBindPos_BodyBindL2W;
                            matrix4 WorldMidTM = MidRot * InvPivotBindPos_BodyBindL2W;
                            
                            // Put back into world space
                            WorldMinTM.Translate( PivotWorldPos );
                            WorldMaxTM.Translate( PivotWorldPos );
                            WorldMidTM.Translate( PivotWorldPos );

                            // Get points and convert to a max distance constraint
                            vector3 WorldMin  = WorldMinTM.GetTranslation();
                            vector3 WorldMax  = WorldMaxTM.GetTranslation();
                            vector3 WorldMid  = WorldMidTM.GetTranslation();
                            f32     MinDist   = ( WorldMin - WorldMid ).Length();
                            f32     MaxDist   = ( WorldMax - WorldMid ).Length();
                            f32     LimitDist = ( MinDist + MaxDist ) * 0.5f;

                            // Compute inverse mid pos ready for setting up constraint
                            matrix4 InvWorldMidTM = m4_InvertRT( WorldMidTM );

                            // Add limit constraint
                            constraint& ConstraintLim = m_BodyBodyContraints.Append();
                            ConstraintLim.Init( &RigidBody,                             // pBody0
                                                &ParentRigidBody,                       // pBody1
                                                InvWorldMidTM * WorldMid,               // BodyPos0
                                                ParentRigidBody.GetW2L() * WorldMid,    // BodyPos1
                                                LimitDist,                              // MaxDist
                                                constraint::FLAG_BLEND_IN,              // Flags
                                                XCOLOR_PURPLE );                        // DebugColor
                        }
                    }                    
                }
            }

            // Always create a pivot constraint to keep hinge extra strong
            {
                // Create constraint
                constraint& Constraint = m_BodyBodyContraints.Append();

                // Init constraint
                Constraint.Init( &RigidBody,        // pBody0
                                 &ParentRigidBody,  // pBody1
                                 PivotWorldPos,     // WorldPos
                                 0.0f,              // MaxDist
                                 0,                 // Flags
                                 XCOLOR_YELLOW );   // DebugColor
                                 
                // Keep ptr to pivot constraint for rendering
                RigidBody.SetPivotConstraint( &Constraint );                                 
            }  
        }
    }

    // Add to physics manager
    g_PhysicsMgr.AddInstance( this );
    
    // Add to physics manager active list
    g_PhysicsMgr.WakeupInstance( this );
    
    // Success!
    return TRUE;
}

//==============================================================================

xbool physics_inst::Init( const char* pGeomName, xbool bPopFix, f32 ConstraintBlendTime )
{
    m_SkinInst.SetUpSkinGeom( pGeomName );
    Init( bPopFix, ConstraintBlendTime );
    return m_bInitialized;
}

//==============================================================================

xbool physics_inst::Init( const skin_inst& SkinInst, xbool bPopFix, f32 ConstraintBlendTime )
{
    m_SkinInst = SkinInst;
    Init( bPopFix, ConstraintBlendTime );
    return m_bInitialized;
}

//==============================================================================

void physics_inst::Kill( void )
{
    PHYSICS_DEBUG_DYNAMIC_MEM_FREE( sizeof( matrix4 ) * m_PopFixMatrices.GetCapacity() );

    // Remove from physics manager list
    g_PhysicsMgr.RemoveInstance( this );

    // Free memory
    m_RigidBodies.Clear();
    m_CollisionShapes.Clear();
    m_BodyBodyContraints.Clear();
    m_BodyWorldContraints.Clear();
    m_PopFixMatrices.Clear();
}

//==============================================================================
// Render functions
//==============================================================================

void physics_inst::Render( u32 Flags, xcolor Ambient )
{
    // Get geometry
    const geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return;
        
    // Get geometry bone matrices
    u64 LODMask;
    s32 nActiveBones;
    const matrix4* pMatrices = GetBoneL2Ws( LODMask, nActiveBones );
    if( !pMatrices )
        return;

    // Render skin
    m_SkinInst.Render( &m_RigidBodies[0].GetL2W(),
                       pMatrices,
                       nActiveBones,
                       Flags,
                       LODMask,
                       Ambient );

    // Activate all bodies?
    if( m_bActiveWhenVisible )
        Activate();
}

//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

void physics_inst::DebugRender( void )
{
    s32 i;

    // Get geometry
    const geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return;

    // Render geometry rigid bodies
    for( i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup geometry rigid body
        const geom::rigid_body& GeomRigidBody = pGeom->m_pRigidBodies[i];
        f32     Width  = GeomRigidBody.Width;
        f32     Height = GeomRigidBody.Height;
        f32     Length = GeomRigidBody.Length;
        f32     Radius = GeomRigidBody.Radius;
        //vector3 Pivot  = GeomRigidBody.Pivot;

        // Draw
        draw_SetL2W( m_RigidBodies[i].GetL2W() );
        //draw_Sphere( Pivot, 5.0f, XCOLOR_RED );
        switch( GeomRigidBody.Type )
        {
        case geom::rigid_body::TYPE_BOX:
            {
                bbox BBox;
                BBox.Min.GetX() = -Width  * 0.5f;
                BBox.Min.GetY() = -Height * 0.5f;
                BBox.Min.GetZ() = -Length * 0.5f;
                BBox.Max.GetX() =  Width  * 0.5f;
                BBox.Max.GetY() =  Height * 0.5f;
                BBox.Max.GetZ() =  Length * 0.5f;
                draw_BBox( BBox, XCOLOR_YELLOW );
            }
            break;

        case geom::rigid_body::TYPE_SPHERE:
            {
                draw_Sphere( vector3( 0.0f, 0.0f, 0.0f ), Radius, XCOLOR_YELLOW );
            }
            break;

        case geom::rigid_body::TYPE_CYLINDER:
            {
#ifdef TARGET_PC            
                draw_Cylinder( vector3( 0, 0, 0 ),
                               Radius, 
                               Height, 
                               10, 
                               XCOLOR_YELLOW, 
                               TRUE, 
                               vector3( 0, 1, 0 ) );
#endif                               
            }
            break;
        }
    }

    draw_ClearL2W();
}

//==============================================================================

void physics_inst::RenderCollision( void )
{
    // Loop through all rigid bodies
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Render rigid body info
        m_RigidBodies[i].DebugRender();
    }
}

#endif  //#ifdef ENABLE_PHYSICS_DEBUG


//==============================================================================
// Position functions
//==============================================================================

vector3 physics_inst::GetPosition( void )const
{
    // Take average position of all bodies
    vector3 Pos( 0.0f, 0.0f, 0.0f );
    if( m_RigidBodies.GetCount() )
    {
        // Accumulate all positions
        for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
            Pos += m_RigidBodies[i].GetPosition();
            
        // Take average            
        Pos /= (f32)m_RigidBodies.GetCount();            
    }
    
    return Pos;
}

//==============================================================================
// Active functions
//==============================================================================

f32 physics_inst::GetSpeedSqr( void ) const
{
    f32 SpeedSqr = 0.0f;
    
    // Loop through all bodies and add speed
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // Take linear and angular into account
        SpeedSqr += Body.GetLinearVelocity().LengthSquared();
        SpeedSqr += Body.GetAngularVelocity().LengthSquared();
    }

    return SpeedSqr;
}

//==============================================================================

xbool physics_inst::HasActiveEnergy( void ) const
{
    // Loop through all bodies and freeze
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // If body has active energy, then so does instance
        if( Body.HasActiveEnergy() )
            return TRUE;
    }
    
    // No active bodies were found
    return FALSE;
}

//==============================================================================

void physics_inst::Deactivate( void )
{
#ifdef X_EDITOR
    // Incase geometry is missing
    if( !m_bInitialized )
        return;
#endif

    ASSERTS( m_SkinInst.GetGeom(), "Ragdoll geometry is missing - make sure all resources are compiled!" );
    ASSERTS( !( m_SkinInst.GetGeom() && !m_bInitialized ), "Geometry is present, but ragdoll not initialized?! - Grab SteveB" );
    
    // Loop through all bodies and deactivate
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // Deactivate
        Body.Deactivate();    
    }
    
    // Remove from physics manager active list
    g_PhysicsMgr.PutToSleepInstance( this );
}

//==============================================================================

void physics_inst::Activate( void )
{
#ifdef X_EDITOR
    // Incase geometry is missing
    if( !m_bInitialized )
        return;
#endif

    ASSERTS( m_SkinInst.GetGeom(), "Ragdoll geometry is missing - make sure all resources are compiled!" );
    ASSERTS( !( m_SkinInst.GetGeom() && !m_bInitialized ), "Geometry is present, but ragdoll not initialized?! - Grab SteveB" );
    ASSERT( m_bInitialized );

    // Loop through all bodies and activate
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // Activate
        Body.Activate();
    }
    
    // Add to physics manager active list
    g_PhysicsMgr.WakeupInstance( this );
}

//==============================================================================
// Matrix functions
//==============================================================================

void physics_inst::DirtyMatrices( void )
{
    // Flag matrices need rebuilding
    m_MatrixCache.SetDirty( TRUE );
}

//==============================================================================

const matrix4* physics_inst::GetBoneL2Ws( u64& LODMask, s32& nActiveBones )
{
    s32 i;
    
    // Must have some rigid bodies!
    s32 nRigidBodies  = m_RigidBodies.GetCount();
    if( !nRigidBodies )
        return NULL;

    // Get geometry
    const geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return NULL;

    PHYSICS_DEBUG_START_TIMER( g_PhysicsMgr.m_Profile.m_Render );

    // Geometry rigid body count must match physics rigid body count!
    ASSERT( nRigidBodies == pGeom->m_nRigidBodies );

    // Compute render info
    LODMask      = m_SkinInst.GetLODMask( m_RigidBodies[0].GetL2W() );
    nActiveBones = m_SkinInst.GetNActiveBones( LODMask );
    ASSERT( nActiveBones > 0 );
    ASSERT( nActiveBones <= pGeom->m_nBones );

    // Allocate matrices
    matrix4* pBoneMatrices = m_MatrixCache.GetMatrices( nActiveBones );
    if( !pBoneMatrices )
        return NULL;

    // Already valid?
    if( m_MatrixCache.IsValid( nActiveBones ) )
        return pBoneMatrices;

    // Allocate rigid body matrices
#ifdef TARGET_PS2    
    ASSERT( ( nRigidBodies * sizeof( matrix4 ) ) <= (u32)SPAD.GetUsableSize() );
    matrix4* pBodyMatrices = (matrix4*)SPAD.GetUsableStartAddr();
#else
    ASSERT( nRigidBodies <= 32 );
    static matrix4 pBodyMatrices[32];
#endif

    // Start with physics rigid body L2W's
    for( i = 0; i < nRigidBodies; i++ )
        pBodyMatrices[i] = m_RigidBodies[i].GetL2W();

    // Iterate over pivot constraints and spread the error correction across connected bodies
    // (this smooths out the impulse jitter that bodies do before they go to sleep)
    for( s32 Iters = g_PhysicsMgr.m_Settings.m_nRenderIterations; Iters > 0; Iters-- )
    {    
        // Alternate directions to spread results more evenly
        s32 iStart, iEnd, iDir;
        if( Iters & 1 )
        {
            // Forwards (skip end body)
            iStart = 0;
            iEnd   = nRigidBodies-1;
            iDir   = 1;
        }
        else
        {
            // Backwards (skip start body)
            iStart = nRigidBodies-1;
            iEnd   = 0;
            iDir   = -1;
        }
        
        // Loop through rigid bodies
        for( i = iStart; i != iEnd; i+= iDir )
        {    
            // Lookup rigid body info
            matrix4&    BodyL2W   = pBodyMatrices[i];
            rigid_body& RigidBody = m_RigidBodies[i];
        
            // Is this body connected to a parent body?
            constraint* pPivotConstraint = RigidBody.GetPivotConstraint();
            if( pPivotConstraint )
            {
                // Make sure this is a pivot constraint
                ASSERT( pPivotConstraint->GetMaxDist() == 0.0f );

                // Lookup parent rigid body index
                s32 iParentBody = pGeom->m_pRigidBodies[i].iParentBody;
                ASSERT( iParentBody != -1 );
                ASSERT( iParentBody != i );
                matrix4& ParentBodyL2W = pBodyMatrices[iParentBody];

                // Compute world pivot position
                ASSERT( pPivotConstraint->GetRigidBody( 0 ) == &RigidBody );
                vector3 WorldPivot = BodyL2W * pPivotConstraint->GetBodyPos( 0 );

                // Compute parent world pivot position            
                ASSERT( pPivotConstraint->GetRigidBody( 1 ) == &m_RigidBodies[iParentBody] );
                vector3 ParentWorldPivot = ParentBodyL2W * pPivotConstraint->GetBodyPos( 1 );

                // Compute correction and apply half to each body - spread that jitter love!
                vector3 Delta = 0.5f * ( ParentWorldPivot - WorldPivot );
                BodyL2W.Translate( Delta );
                ParentBodyL2W.Translate( -Delta );
            }
        }
    }
            
    // Bake in inverse bind ready for skinning
    for( i = 0; i < nRigidBodies; i++ )
    {
        // Compute inverse bind matrix for rigid body
        const geom::rigid_body& Body = pGeom->m_pRigidBodies[i];
        matrix4 BodyBind( vector3( 1.0f, 1.0f, 1.0f ), Body.BodyBindRotation, Body.BodyBindPosition );
        matrix4 InvBodyBind = m4_InvertRT( BodyBind );
                 
        // Compute final skin L2W
        pBodyMatrices[i] = pBodyMatrices[i] * InvBodyBind;
    }

    // Finally, compute the bone render matrices. Use pop fix?
    if( m_bPopFix )
    {
        // Loop through all bones
        for( i = 0; i < nActiveBones; i++ )
        {
            // Lookup index of rigid body that bone is attached to
            s32 iRigidBody = pGeom->m_pBone[i].iRigidBody;
            ASSERT( iRigidBody >= 0 );
            ASSERT( iRigidBody < nRigidBodies );

            // Use rigid body skin matrix with pop fix applied
            pBoneMatrices[i] = pBodyMatrices[ iRigidBody ] * m_PopFixMatrices[i];
        }
    }
    else
    {
        // Loop through all bones
        for( i = 0; i < nActiveBones; i++ )
        {
            // Lookup index of rigid body that bone is attached to
            s32 iRigidBody = pGeom->m_pBone[i].iRigidBody;
            ASSERT( iRigidBody >= 0 );
            ASSERT( iRigidBody < nRigidBodies );
            
            // Just copy rigid body skin matrix
            pBoneMatrices[i] = pBodyMatrices[ iRigidBody ];
        }
    }

    // Flag matrices as valid
    m_MatrixCache.SetDirty( FALSE );

    PHYSICS_DEBUG_STOP_TIMER( g_PhysicsMgr.m_Profile.m_Render );

    return pBoneMatrices;
}

//==============================================================================

vector3 physics_inst::GetBoneWorldPosition( s32 iBone )
{
    // Look world transform
    matrix4 L2W = GetBoneWorldTransform( iBone );
    
    // Return world position
    return L2W.GetTranslation();
}

//==============================================================================

matrix4 physics_inst::GetBoneWorldTransform( s32 iBone )
{
    // Default 
    matrix4 L2W;
    
    // Lookup geom
    geom* pGeom = m_SkinInst.GetSkinGeom();
    if( !pGeom )
    {
        L2W.Identity();
        return L2W;    
    }
    
    // Lookup render matrices
    u64 LODMask;
    s32 nActiveBones;
    const matrix4* pMatrices = GetBoneL2Ws( LODMask, nActiveBones );

    // If no matrices we are screwed
    if( !pMatrices )
    {
        L2W.Identity();
        return L2W;    
    }
    
    // If bone is not visible, attach to root bone        
    if( ( iBone >= nActiveBones ) || ( iBone >= pGeom->m_nBones ) )
        iBone = 0;

    // Lookup transform with bind baked in
    L2W = pMatrices[iBone];

    // Counter act inverse bind position (rotation is always zero)
    ASSERT( iBone < pGeom->m_nBones );
    L2W.PreTranslate( pGeom->m_pBone[iBone].BindPosition );
    
    return L2W;
}

//==============================================================================

void physics_inst::SetMatrices( loco_char_anim_player& AnimPlayer, const vector3& Vel )
{
    // No bodies setup?
    if( !m_RigidBodies.GetCount() )
        return;
    
    // Get geometry
    const geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return;
    
    // No anim playing?
    loco_motion_controller& CurrAnim = AnimPlayer.GetCurrAnim();
    if( CurrAnim.GetAnimIndex() == -1 )
        return;

    // Lookup anim info
    const anim_info& AnimInfo = CurrAnim.GetAnimInfo();
    f32 DeltaTime = ( CurrAnim.GetRate() / 30.0f ) * (f32)AnimInfo.GetFPS();
    f32 LastFrame = (f32)( AnimInfo.GetNFrames() - 2 );
    f32 CurrFrame = CurrAnim.GetFrame();
    f32 NextFrame = CurrFrame + DeltaTime;
    
    // At end of anim?
    if ( CurrFrame >= LastFrame )
    {
        // Rewind a frame
        CurrFrame = LastFrame - DeltaTime;
        NextFrame = LastFrame;
        
        // Range check
        if( CurrFrame < 0 )
            CurrFrame = 0.0f;
    }
    else
    {
        // Range check
        if( NextFrame > LastFrame )
            NextFrame = LastFrame;
    }
    
    // Compute matrices for current frame and setup position
    AnimPlayer.SetNActiveBones( pGeom->m_nBones );
    AnimPlayer.SetCurrAnimFrame( CurrFrame );
    SetMatrices( AnimPlayer.GetBoneL2Ws(), pGeom->m_nBones, FALSE );
    
    // Compute matrices for next frame and inherit vels
    AnimPlayer.SetCurrAnimFrame( NextFrame );
    SetMatrices( AnimPlayer.GetBoneL2Ws(), pGeom->m_nBones, TRUE );
    
    // Add final velocity
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];
        
        // Add to the velocity
        Body.GetLinearVelocity() += Vel * 30.0f;
    }
}

//==============================================================================

void physics_inst::SetMatrices( const matrix4* pMatrices, s32 nBones, xbool bInheritVel )
{
    s32 i, j, k;
    
    (void)nBones;
    ASSERT( nBones > 0 );

    // Must have matrices!    
    if( !pMatrices )
        return;
    
    // No bodies setup?
    if(!m_RigidBodies.GetCount() )
        return;

    // Get geometry
    const geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return;

    // Lookup root bone world position (middle of character)
    // NOTE: Bind pos is added because inverse bind pos is baked into matrix
    vector3 WorldPos = pGeom->m_pBone[0].BindPosition + pMatrices[0].GetTranslation();
    
    // Move in the air a bit in-case the npc is sitting on the floor
    WorldPos.GetY() += 40.0f;
    
    // Setup rigid bodies from animation matrices
    ASSERT( pGeom->m_nRigidBodies == m_RigidBodies.GetCount() );
    for( i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup rigid bodies
              rigid_body&       Body     = m_RigidBodies[i];
        const geom::rigid_body& GeomBody = pGeom->m_pRigidBodies[i];

        // Clear body movement
        Body.ClearForces();
        Body.ZeroLinearVelocity();
        Body.ZeroAngularVelocity();

        // Compute rigid body bind and inverse bind
        matrix4 BodyBind;
        BodyBind.Setup( vector3( 1.0f, 1.0f, 1.0f ),
                        GeomBody.BodyBindRotation,
                        GeomBody.BodyBindPosition ); 

        // Make sure bone is valid
        ASSERT( GeomBody.iBone >= 0 );
        ASSERT( GeomBody.iBone < pGeom->m_nBones );
        ASSERT( GeomBody.iBone < nBones );

        // Compute rigid body L2Ws
        matrix4 CurrBodyL2W = pMatrices[ GeomBody.iBone ] * BodyBind;
        
        // Inherit motion too?
        if( bInheritVel )
        {
            // Compute velocity (don't need to take into account rotation - this looks good enough)
            const matrix4& PrevBodyL2W = Body.GetL2W();
            vector3 Motion = CurrBodyL2W.GetTranslation() - PrevBodyL2W.GetTranslation();
            Body.SetLinearVelocity( Motion * 30.0f );
        }
        
        // Set new rigid body L2W
        Body.SetPrevL2W( CurrBodyL2W );
        Body.SetL2W( CurrBodyL2W );
        
        // Setup collision shapes
        for( j = 0; j < m_CollisionShapes.GetCount(); j++ )
        {
            // Lookup collision spheres shape
            collision_shape& Shape = m_CollisionShapes[j];
            
            // Setup spheres
            for( k = 0; k < Shape.GetNSpheres(); k++ )
            {
                // Lookup sphere
                collision_shape::sphere& Sphere = Shape.GetSphere( k );
                
                // Setup coll free pos (hopefully!) to be in middle of character
                Sphere.m_CollFreePos = WorldPos;
                
                // Setup start and end positions
                Sphere.m_PrevPos =
                Sphere.m_CurrPos = CurrBodyL2W * Sphere.m_Offset;
            }
        }
    }
    
    // Compute pop fix bind matrices?
    if( m_bPopFix )
    {
        // Create pop fix matrix for every geometry bone...
        for( i = 0; i < nBones; i++ )
        {
            // Lookup bone
            const geom::bone& Bone = pGeom->m_pBone[i];
            
            // Lookup rigid body
            ASSERT( Bone.iRigidBody >= 0 );
            ASSERT( Bone.iRigidBody < nBones );
            const geom::rigid_body& Body = pGeom->m_pRigidBodies[ Bone.iRigidBody ];

            // Compute inverse bind matrix for rigid body
            matrix4 BodyBind( vector3( 1.0f, 1.0f, 1.0f ), Body.BodyBindRotation, Body.BodyBindPosition );
            matrix4 InvBodyBind = m4_InvertRT( BodyBind );
        
            // Compute BoneL2W that will be computed from rigid body transform without pop fix up
            matrix4 BoneL2W = m_RigidBodies[ (s32)Bone.iRigidBody ].GetL2W() * InvBodyBind;
            
            // Compute correction matrix that will fix the pop
            m_PopFixMatrices[ i ] = m4_InvertRT( BoneL2W ) * pMatrices[ i ];
        }
    }
}

//==============================================================================
// Blast/force functions
//==============================================================================

void physics_inst::ApplyBlast( const vector3& Pos, f32 Radius, f32 Amount )
{
    ASSERT( Radius > 0.01f );
    ASSERT( Pos.IsValid() );
    
    // Compute radius squared
    f32 RadiusSqr = x_sqr( Radius );
    
    // Loop through all rigid bodies
    xbool bActivate = FALSE;
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // Compute distance of body from blast
        vector3 Delta   = Body.GetPosition() - Pos;
        f32     DistSqr = Delta.LengthSquared();
    
        // Within force radius and not right on top of rigid body?
        if( ( DistSqr < RadiusSqr ) && ( DistSqr > 0.001f ) )
        {
            // Compute dist and inverse dist
            f32 Dist    = 0.0f;
            f32 InvDist = 1.0f;
            if( DistSqr > 0.0001f )
            {
                Dist    = x_sqrt( DistSqr );
                InvDist = 1.0f / Dist;
            }
            
            // Compute direction
            vector3 Dir = Delta * InvDist;
            
            // Compute force
            vector3 Force = Amount * Dir * ( ( Radius - Dist ) / Radius); 

            // Keep force valid                
            ClampForce( Force );
                
            // Apply force
            Body.ApplyWorldImpulse( Force );
            bActivate = TRUE;
        }            
    }
    
    // Make sure instance is in physics managers active list
    if( bActivate )
        g_PhysicsMgr.WakeupInstance( this );
}

//==============================================================================

void physics_inst::ApplyBlast( const vector3& Pos, const vector3& Dir, f32 Radius, f32 Amount )
{
    ASSERT( Pos.IsValid() );
    ASSERT( Dir.IsValid() );
    ASSERT( Dir.LengthSquared() <= x_sqr( 1.1f ) );
    ASSERT( x_isvalid( Radius ) );
    ASSERT( x_isvalid( Amount ) );
    ASSERT( Radius > 0.01f );
    ASSERT( Amount >= 0.0f );

    // Compute radius squared
    f32 RadiusSqr = x_sqr( Radius );

    // Loop through all rigid bodies
    xbool bActivate = FALSE;
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // Compute distance of body from blast
        vector3 Delta   = Body.GetPosition() - Pos;
        f32     DistSqr = Delta.LengthSquared();

        // Within radius?
        if( DistSqr < RadiusSqr )
        {
            // Compute dist
            f32 Dist = 1.0f;
            if( DistSqr > 0.0001f )
                Dist = x_sqrt( DistSqr );

            // Compute force
            vector3 Force = Amount * Dir * ( ( Radius - Dist ) / Radius); 

            // Keep force valid                
            ClampForce( Force );

            // Apply force
            Body.ApplyWorldImpulse( Force );
            bActivate = TRUE;
        }            
    }

    // Make sure instance is in physics managers active list
    if( bActivate )
        g_PhysicsMgr.WakeupInstance( this );
}

//==============================================================================

void physics_inst::ApplyVectorForce( const vector3& Dir, f32 Amount )
{
    ASSERT( Dir.IsValid() );
    ASSERT( x_isvalid( Amount ) );
    ASSERT( Amount >= 0.0f );

    // Compute force
    ASSERT( Dir.IsValid() );
    vector3 Force = Dir * Amount;
    
    // Keep force valid                
    ClampForce( Force );
    
    // Loop through all rigid bodies
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];
        
        // Apply force
        Body.ApplyWorldImpulse( Force );
    }
    
    // Make sure instance is in physics managers active list
    g_PhysicsMgr.WakeupInstance( this );
}

//==============================================================================
// Collision functions
//==============================================================================

void physics_inst::OnColCheck( guid OwnerObject )
{
    g_CollisionMgr.StartApply( OwnerObject );
    
    // Loop over all collision shapes
    for( s32 i = 0; i < m_CollisionShapes.GetCount(); i++ )
    {
        // Lookup collision
        const collision_shape& Shape = m_CollisionShapes[i];
        
        // Lookup rigid body ID so corpse can use collision info
        ASSERT( Shape.GetOwner() );
        s32 ID = Shape.GetOwner()->GetCollisionID();
        
        // Loop over all collision spheres
        for( s32 j = 0; j < Shape.GetNSpheres(); j++ )
        {
            // Lookup sphere
            const collision_shape::sphere& Sphere = Shape.GetSphere(j);
            
            // Apply
            g_CollisionMgr.ApplySphere( Sphere.m_CurrPos, Shape.m_Radius, object::MAT_TYPE_FLESH, ID );
        }
    }
    
    g_CollisionMgr.EndApply();
}

//==============================================================================

void physics_inst::ComputeWorldBBox( void )
{
    // Clear bbox
    m_WorldBBox.Clear();

    // Loop through all bodies and accumulate bounding boxes
    for( s32 i = 0; i < m_RigidBodies.GetCount(); i++ )
    {
        // Lookup body
        rigid_body& Body = m_RigidBodies[i];

        // Update collision shape of body
        collision_shape* pColl = Body.GetCollisionShape();
        if( pColl )
        {
            // Collide with world?
            if( Body.m_Flags & rigid_body::FLAG_WORLD_COLLISION )
            {
                // Prepare for sweep test
                pColl->SetL2W( Body.GetPrevL2W(), Body.GetL2W() );
            }                
            else                
            {
                // Set directly
                pColl->SetL2W( Body.GetL2W() );
            }                
        }
        
        // Compute bbox of body
        Body.ComputeWorldBBox();
        
        // Accumulate world bbox of rigid body            
        m_WorldBBox += Body.GetWorldBBox();
    }
}

//==============================================================================
// Rigid body functions
//==============================================================================

const char* physics_inst::GetRigidBodyName( s32 iRigidBody ) const
{
    // Lookup geometry
    const skin_geom* pGeom = m_SkinInst.GetSkinGeom();
    if( !pGeom )
        return "NULL";
        
    // Invalid rigid body?
    ASSERT( m_RigidBodies.GetCount() == pGeom->m_nRigidBodies );
    if( ( iRigidBody < 0 ) || ( iRigidBody >= pGeom->m_nRigidBodies ) )
        return "NULL";

    // Found!
    return pGeom->GetRigidBodyName( iRigidBody );
}

//==============================================================================
// Constraint functions
//==============================================================================

s32 physics_inst::AddBodyWorldConstraint( s32            iRigidBody, 
                                          const vector3& WorldPos,
                                          f32            MaxDist )
{
    // Invalid rigid body or not loaded yet?
    if( ( iRigidBody < 0 ) || ( iRigidBody >= m_RigidBodies.GetCount() ) )
        return -1;

    // Create a new constraint
    s32 Index = m_BodyWorldContraints.GetCount();
    constraint& Constraint = m_BodyWorldContraints.Append();
            
    // Init constraint
    Constraint.Init( &m_RigidBodies[ iRigidBody ],  // pBody0
                     &g_WorldBody,                  // pBody1
                     WorldPos,                      // WorldPos
                     MaxDist,                       // MaxDist
                     0,                             // Flags
                     XCOLOR_PURPLE );               // DebugColor
                     
    return Index;                     
}

//==============================================================================
// Logic functions            
//==============================================================================

void physics_inst::Advance( f32 DeltaTime )
{
    // Update constraint blending
    m_ConstraintWeight += m_ConstraintWeightDelta * DeltaTime;
    m_ConstraintWeight = x_clamp( m_ConstraintWeight, 0.0f, 1.0f );
    
    // Keep active?
    if( m_KeepActiveTime > 0.0f )
    {
        // Update timer
        m_KeepActiveTime -= DeltaTime;

        // Keep all bodies active
        Activate();
    }        
}

//==============================================================================
