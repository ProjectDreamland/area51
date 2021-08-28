//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "RawMesh.hpp"


#include "hkdynamics2/world/hkWorldConstructionInfo.h"
#include "hkdynamics2/world/hkWorld.h"

#include "hkdemoframework2/hkDemoFramework.h"
#include "hkutilities2/hkHavok2Common.h"
#include "hkdemoframework2/hkDefaultGame.h"
#include "hkcollide2/shape/hkStorageMeshShape.h"
#include "hkcollide2/shape/hkMoppBvTreeShape.h"
#include "hkcollide2/algorithms/mopp/utility/hkMoppUtility.h"
#include "hkgeometry2/hkSphere.h"
#include "hkcollide2/shape/hkMultiSphereShape.h"
#include "hkcollide2/shape/hkBvShape.h"


//==============================================================================
//  STORAGE
//==============================================================================

view                View;
hkWorld*            s_pWorld        = NULL;
hkListShape*        s_pDynamicShape = NULL;
hkListShape*        s_pComplexShape = NULL;
hkMoppBvTreeShape*  s_pTerrainShape = NULL;

xarray<hkRigidBody*>    s_MyRigidBodyList;
xarray<hkRigidBody*>    s_WorldGeom;

// Key for the ahvok evaluation
extern const char*		  HK_KEYCODE  = "330630-PC-KPPGCAODINV8";
extern const unsigned int HK_KEYVALUE = 0x0;	

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================

void InitHavokWorld( void )
{
    //
    // Init the (x_files like) for Havok.
    //
    hkBaseSystem::init();

    //
    // Initialize the world
    //
    hkWorldConstructionInfo Info;

    Info.setBroadPhaseWorldSize( 150.0f );
    Info.setupSolverInfo ( HK_SOLVER_TYPE_4ITERS_MEDIUM );
    Info.m_gravity              = hkVector4( 0,-9.8f,0);
    Info.m_collisionTolerance   = 0.1f;
    s_pWorld = new hkWorld( Info );
    ASSERT(s_pWorld);

    //
    // Register ALL agents (though some may not be necessary)
    //
    hkAgentRegisterUtil::registerAllAgents(s_pWorld->getCollisionDispatcher());
}

//==============================================================================

void CreateDefaultWorld( void )
{
	// Create the shape that will be used to create all dynamic objects.
	{
		hkArray<hkShape*> shapeArray;
		
		hkBoxShape* boxShape1 = new hkBoxShape(hkVector4(0.3f, 0.1f, 0.1f));
		hkBoxShape* boxShape2 = new hkBoxShape(hkVector4(0.1f, 0.3f, 0.1f));
		hkBoxShape* boxShape3 = new hkBoxShape(hkVector4(0.1f, 0.1f, 0.3f));

		shapeArray.pushBack(boxShape1);
		shapeArray.pushBack(boxShape2);
		shapeArray.pushBack(boxShape3);

		s_pDynamicShape = new hkListShape(shapeArray.begin(), shapeArray.getSize());
        ASSERT( s_pDynamicShape );

        // Since s_pDynamicShape now owns all these shapes, reduce their reference counts
        boxShape1->removeReference();
        boxShape2->removeReference();
        boxShape3->removeReference();
    }

    // Use this shape to define a more complex shape with hkTransformShapes
    {
		hkArray<hkShape*> shapeArray;

        hkTransform t;
		t.setIdentity();

		hkTransformShape* transShape1 = new hkTransformShape(s_pDynamicShape);	
		t.setTranslation(hkVector4( 0.15f, 0.0f, -0.15f));
		transShape1->setTransform(t);

		hkTransformShape* transShape2 = new hkTransformShape(s_pDynamicShape);
		t.setTranslation(hkVector4(-0.15f, 0.0f, -0.15f));
		transShape2->setTransform(t);

		hkTransformShape* transShape3 = new hkTransformShape(s_pDynamicShape);
		t.setTranslation(hkVector4( 0.15f, 0.0f,  0.15f));
		transShape3->setTransform(t);

		hkTransformShape* transShape4 = new hkTransformShape(s_pDynamicShape);
		t.setTranslation(hkVector4(-0.15f, 0.0f,  0.15f));
		transShape4->setTransform(t);

		shapeArray.pushBack(transShape1);
		shapeArray.pushBack(transShape2);
		shapeArray.pushBack(transShape3);
		shapeArray.pushBack(transShape4);

		s_pComplexShape = new hkListShape(shapeArray.begin(), shapeArray.getSize());
        ASSERT( s_pComplexShape );

        // Since m_complexShape now owns all these shapes, reduce their reference counts
        transShape1->removeReference();
        transShape2->removeReference();
        transShape3->removeReference();
        transShape4->removeReference();
    }
    
    int edgeSize = 8;
    float baseScale = 1.0f;
    float scale = edgeSize * baseScale * 0.5f;

    // Create a shape for static terrain cells
    {
        hkStorageMeshShape* meshStorage = new hkStorageMeshShape();
        meshStorage->setRadius(0.1f);
    
        // Create tessellated mesh bucket
        {
            int halfSize = (int)(edgeSize * 0.5f);
            int vertCount = (edgeSize + 1) * (edgeSize + 1);
            int triCount = halfSize * halfSize * 8;
            float baseIncline = 0.4f;
            float sideHeight = 1.0f;
        
            meshStorage->reserveMemory(vertCount, triCount); 
        
            // Add vertices
            {
                for (int x = -halfSize; x <= halfSize; x++)
                {
                    for (int z = -halfSize; z <= halfSize; z++)
                    {
                        float y = (z + halfSize) * baseIncline;
                    
                        if (x == -halfSize || x == halfSize || z == -halfSize || z == halfSize)
                        {
                            y = 2.0f * halfSize * baseIncline + sideHeight;
                        }
                    
                        hkVector4 vertex = hkVector4(x * baseScale, y, z * baseScale);

                        meshStorage->addVertex(vertex);
                    }
                }
            }
        
            // Add triangles
            {
                for (int x = 0; x < edgeSize; x++)
                {
                    for (int z = 0; z < edgeSize; z++)
                    {
                        int base = z * (edgeSize + 1) + x;
                    
                        meshStorage->addTriangle(base + 0, base + 1, base + edgeSize + 1);
                        meshStorage->addTriangle(base + 1, base + edgeSize + 2, base + edgeSize + 1);
                    }
                }
            }
        }
    
        // Wrap mesh in a MOPP (good for speed with high triangle count meshes)
        hkMoppFitToleranceRequirements req;
        hkMoppCode* moppCode = hkMoppUtility::buildCode(meshStorage, req);
        s_pTerrainShape = new hkMoppBvTreeShape(meshStorage, moppCode);
        meshStorage->removeReference();
        moppCode->removeReference();
    }

    // Create the terrain from multiple cells
    {
        hkRigidBodyConstructionInfo rInfo;
        rInfo.m_motionType = HK_FIXED_MOTION;
        rInfo.m_shape = s_pTerrainShape;
        rInfo.m_restitution = 0.3f;
    
        for (int x = -2; x <= 2; x += 2)
        {
            for (int z = -2; z <= 2; z += 2)
            {
        		rInfo.m_position.set(x * scale, 0.0f, z * scale);
                hkRigidBody* terrainRigidBody = new hkRigidBody(rInfo);

                s_pWorld->addEntity(terrainRigidBody);
                terrainRigidBody->removeReference();

                s_WorldGeom.Append( terrainRigidBody );
            }
        }
    }

	// To illustrate using the referenced shapes, create multiple rigidbodys from them.
    {
		hkRigidBodyConstructionInfo rInfo;

		rInfo.m_shape = s_pDynamicShape;
		rInfo.m_motionType = HK_BOX_INERTIA_MOTION;

        hkReal mass = 50.0f;
		hkMassProperties massProperties;
		hkInertiaTensorComputer::computeShapeVolumeMassProperties(s_pDynamicShape, mass, massProperties);

		rInfo.m_inertiaTensor = massProperties.m_inertiaTensor;
		rInfo.m_centerOfMass = massProperties.m_centerOfMass;
		rInfo.m_mass = massProperties.m_mass;			
		 
        int x;
        for (x = -1; x <= 1; x += 2)
        {
            for (int z = -1; z <= 1; z += 2)
            {
        		rInfo.m_position.set(x * scale, 5.0f, z * scale);
                hkRigidBody* dynamicRigidBody = new hkRigidBody(rInfo);

                s_pWorld->addEntity(dynamicRigidBody);
                dynamicRigidBody->removeReference();

                s_MyRigidBodyList.Append( dynamicRigidBody );
            }
        }

        rInfo.m_shape = s_pComplexShape;

        mass = 200.0f;
		hkInertiaTensorComputer::computeShapeVolumeMassProperties(s_pComplexShape, mass, massProperties);

		rInfo.m_inertiaTensor = massProperties.m_inertiaTensor;
		rInfo.m_centerOfMass = massProperties.m_centerOfMass;
		rInfo.m_mass = massProperties.m_mass;			
		 
        for (x = -1; x <= 1; x += 2)
        {
            for (int z = -1; z <= 1; z += 2)
            {
        		rInfo.m_position.set(x * scale, 10.0f, z * scale);
                hkRigidBody* dynamicRigidBody = new hkRigidBody(rInfo);

                s_pWorld->addEntity(dynamicRigidBody);
                dynamicRigidBody->removeReference();

                s_MyRigidBodyList.Append( dynamicRigidBody );
            }
        }
	}
}

//==============================================================================

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(1315.44f,1390.30f,-935.648f) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    //
    // Initialize the physics world
    //
    InitHavokWorld();
    CreateDefaultWorld();
}

//=========================================================================

void Shutdown( void )
{
}

//=========================================================================

xbool HandleInput( void )
{
    while( input_UpdateState() )
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 16.125f/8;
        f32    R = 0.005f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_KBD_W       ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_S       ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_A       ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_D       ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_R       ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_F       ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );       
        Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
        Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );

    }

    return( TRUE );
}

//==============================================================================

void RenderShape( const hkShape* pShape, const hkTransform& L2W )
{
    //	hkTransform transform;
    //	transform.setIdentity();
    //
    int a = sizeof(hkRigidBody);
	switch( pShape->getPrimaryType() )
	{
//        case HK_SHAPE_SPHERE:               a = 2;break;
//        case HK_SHAPE_MULTI_SPHERE:         a = 2;break;
        case HK_SHAPE_PLANE:                a = 2;break;
        case HK_SHAPE_LINE_SEGMENT:         a = 2;break;
//        case HK_SHAPE_TRANSFORM:            a = 2;break;
//        case HK_SHAPE_BV:                   a = 2;break;
//        case HK_SHAPE_BV_TREE:              a = 2;break;
//        case HK_SHAPE_LIST:                 a = 2;break;
//        case HK_SHAPE_COLLECTION:           a = 2;break;
//        case HK_SHAPE_CONNECTED_TRIANGLE:   a = 2;break;
//        case HK_SHAPE_TRIANGLE:             a = 2;break;
        case HK_SHAPE_CONVEX_VERTICES:      a = 2;break;
//		case HK_SHAPE_BOX:                  a = 2;break;
        default:                            a = 2;break;


		case HK_SHAPE_TRANSFORM:
		{
			const hkTransformShape* ts = static_cast<const hkTransformShape*>( pShape );

			hkTransform T; 
            T.setMul( L2W, ts->getTransform() );
			RenderShape( ts->getChildShape(), T );

			break;
		}

		case HK_SHAPE_BOX:
		{
			const hkBoxShape* boxShape = static_cast<const hkBoxShape*>(pShape);

            hkVector4 V4 = boxShape->getHalfExtent();
            bbox      BBox( -vector3(V4(0), V4(1), V4(2)), vector3( V4(0), V4(1), V4(2)) );

            // Convert to xfiles friendly
            matrix4 L2WS;
            L2WS = *(matrix4*)&L2W;
            L2WS(3,3)=1;                // Havok puts crap in the w component of the matrix

            // Convert the world from meters to centemeters
            L2WS.Scale( vector3( 100, 100, 100 ) );

            // Render the bbox
            draw_SetL2W( L2WS );
            draw_BBox( BBox );
			break;
		}        

		case HK_SHAPE_LIST:
		case HK_SHAPE_COLLECTION:
		{
			const hkShapeCollection* shapeCollection = static_cast<const hkShapeCollection*>(pShape);
			hkShapeCollection::AllocBuffer HK_ALIGNED_VARIABLE( buffer, 16 );

			for( int i = 0; i < shapeCollection->getNumChildShapes(); ++i )
			{
				hkShapeCollection::hkShapeKey key = shapeCollection->getKeyAt(i);
				const hkShape* child = shapeCollection->getChildShape(key, buffer);
				if (child != HK_NULL)
				{
					RenderShape( child, L2W );
				}
			}
			break;
		}

		case HK_SHAPE_BV_TREE:
		{
			const hkBvTreeShape* bvShape = static_cast<const hkBvTreeShape*>( pShape );
			RenderShape( bvShape->getShapeCollection(), L2W );
			break;
		}

		case HK_SHAPE_CONNECTED_TRIANGLE:
		case HK_SHAPE_TRIANGLE:
		{
			const hkTriangleShape* triangleShape = static_cast<const hkTriangleShape*>(pShape);

			hkVector4 transformedVertices[3];
			transformedVertices[0].setTransformedPos( L2W, triangleShape->getVertex(0) );
			transformedVertices[1].setTransformedPos( L2W, triangleShape->getVertex(1) );
			transformedVertices[2].setTransformedPos( L2W, triangleShape->getVertex(2) );

            // Convert the world from meters to centemeters as well as to xfiles happy format
            vector3 V[3];
            for( s32 i=0;i<3;i++)
            {
                V[i].Set( transformedVertices[i](0)*100, transformedVertices[i](1)*100, transformedVertices[i](2)*100 );
            }

            // Compute the color of the triangle
            static s32    C = 0;
            plane Plane( V[0], V[1], V[2] );            
            Plane.Normal = Plane.Normal * vector3( 127, 127, 127 );
            Plane.Normal = Plane.Normal + vector3( 127, 217, 127 );
            xcolor Color( (u8)Plane.Normal.X, (u8)Plane.Normal.X, (u8)Plane.Normal.Z, 255);

            // Finally render the stupid thing
            draw_NGon( V, 3, Color, FALSE );

			break;
		}

		case HK_SHAPE_SPHERE:
		{
			const hkSphereShape* sphereShape = static_cast<const hkSphereShape*>(pShape);

            draw_Sphere( *(vector3*)&L2W.getTranslation(), sphereShape->getRadius() );

			break;
		}

		case HK_SHAPE_MULTI_SPHERE:
		{
			const hkMultiSphereShape* s = static_cast<const hkMultiSphereShape*>(pShape);
			const hkVector4* v = s->getSpheres();
			for(int i = 0; i < s->getNumSpheres(); ++i)
			{
				draw_Sphere( *(vector3*)&v[i], v[i](3) );
			}

			break;
		}

		case HK_SHAPE_BV:
		{
			const hkBvShape* bvShape = static_cast<const hkBvShape*>(pShape);
			RenderShape( bvShape->getBoundingVolumeShape(), L2W );
			break;
		}

        /*
		case HK_SHAPE_SPHERE:
		{
			const hkSphereShape* sphereShape = static_cast<const hkSphereShape*>(shape);
			hkVector4 zeroVector;
			zeroVector.setZero4();
			hkSphere sphere( zeroVector, sphereShape->getRadius());

			hkDisplaySphere* displaySphere = new hkDisplaySphere(sphere, m_environment.m_sphereThetaRes, m_environment.m_spherePhiRes);

			displaySphere->getTransform() = transform;
			displayGeometries.pushBack(displaySphere);

			break;
		}
		case HK_SHAPE_MULTI_SPHERE:
		{
			const hkMultiSphereShape* s = static_cast<const hkMultiSphereShape*>(shape);
			const hkVector4* v = s->getSpheres();
			for(int i = 0; i < s->getNumSpheres(); ++i)
			{
				hkSphere sphere( hkVector4::getZero(), v[i](3) );
				hkDisplaySphere* displaySphere = new hkDisplaySphere(sphere, m_environment.m_sphereThetaRes, m_environment.m_spherePhiRes);

				displaySphere->getTransform().setIdentity();
				displaySphere->getTransform().setTranslation(v[i]);
				displayGeometries.pushBack(displaySphere);
			}

			break;
		}
		case HK_SHAPE_PLANE:
		{
			// TODO
			break;
		}
		case HK_SHAPE_LINE_SEGMENT:
		{
			// TODO
			break;
		}
		case HK_SHAPE_BOX:
		{
			const hkBoxShape* boxShape = static_cast<const hkBoxShape*>(shape);
			hkDisplayBox* displayBox = new hkDisplayBox(boxShape->getHalfExtent());
			displayBox->getTransform() = transform;
			displayGeometries.pushBack(displayBox);
			break;
		}
        

		case HK_SHAPE_TRANSFORM:
		{
			const hkTransformShape* ts = static_cast<const hkTransformShape*>( shape );
			hkTransform T; T.setMul( transform, ts->getTransform() );

			buildShapeDisplay( ts->getChildShape(), T, displayGeometries);

			break;
		}
		case HK_SHAPE_BV:
		{
			const hkBvShape* bvShape = static_cast<const hkBvShape*>(shape);
			buildShapeDisplay( bvShape->getBoundingVolumeShape(), transform, displayGeometries);
			break;
		}
		case HK_SHAPE_LIST:
		case HK_SHAPE_COLLECTION:
		{
			const hkShapeCollection* shapeCollection = static_cast<const hkShapeCollection*>(shape);
			hkShapeCollection::AllocBuffer HK_ALIGNED_VARIABLE( buffer, 16 );
			for (int i = 0; i < shapeCollection->getNumChildShapes(); ++i)
			{
				hkShapeCollection::hkShapeKey key = shapeCollection->getKeyAt(i);
				const hkShape* child = shapeCollection->getChildShape(key, buffer);
				if (child != HK_NULL)
				{
					buildShapeDisplay(child, transform, displayGeometries);
				}
			}
			break;
		}

		case HK_SHAPE_CONVEX_VERTICES:
		{
			const hkConvexVerticesShape* convexVerticesShape = static_cast<const hkConvexVerticesShape*>(shape);

			hkArray<hkSphere> vertices;
			convexVerticesShape->getCollisionSpheres(vertices);

				// Convert these vertices to the transformed space.
			hkArray<hkVector4> transformedVertices;
			transformedVertices.setSize(vertices.getSize());
			for(int i = 0; i < vertices.getSize(); i++)
			{
				transformedVertices[i].setTransformedPos(transform, vertices[i].getPosition());

			}

			hkGeometryFiltererTolerances tol;
			hkGeometry* geom = hkGeometryUtil::createFilteredConvexGeometry(& (transformedVertices[0]) , transformedVertices.getSize(), tol);

			hkDisplayGeometry* displayGeom = new hkDisplayGeometry(HK_DISPLAY_GEOMETRY, geom);

			displayGeometries.pushBack(displayGeom);
			break;
		}
		default:
			{
			//	HK_ASSERT2( 0, "Unknown shape type to display" );
			}
        */

	}
}


//==============================================================================

void Render( void )
{
    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();
        draw_Grid( vector3(  -5000,   0,    -5000), 
                   vector3(10000,  0,    0), 
                   vector3(  0,   0, 10000), 
                   xcolor (  0,128,  0), 32 );
    }
    eng_End();


    //==---------------------------------------------------
    //  Render the terrain.
    //==---------------------------------------------------
    if( 0 )
    {
        hkSphere sphereOut;
        s_pTerrainShape->getBoundingSphere( sphereOut );

        eng_Begin();
        vector3 Pos( sphereOut.getPosition()(0), sphereOut.getPosition()(1), sphereOut.getPosition()(2) );
        draw_Sphere( Pos, sphereOut.getRadius() );
        eng_End();
    }

    //==---------------------------------------------------
    //  Render world geom objects
    //==---------------------------------------------------
    {
        eng_Begin();
        for( s32 i=0; i<s_WorldGeom.GetCount(); i++ )
        {
            hkRigidBody& RigidBody = *s_WorldGeom[i];

            RenderShape( RigidBody.getCollidable().getShape(), RigidBody.getTransform() );
        }
        eng_End();
    }

    //==---------------------------------------------------
    //  Render dynamic objects
    //==---------------------------------------------------
    {
        eng_Begin();
        for( s32 i=0; i<s_MyRigidBodyList.GetCount(); i++ )
        {
            hkRigidBody& RigidBody = *s_MyRigidBodyList[i];

            RenderShape( RigidBody.getCollidable().getShape(), RigidBody.getTransform() );
        }
        eng_End();
    }
}

//==============================================================================

void Advance( f32 Seconds )
{
    static f32 LastTime=0;

    // Increment the delta time
    LastTime += Seconds;

    // Note: An optimization could be done this way. (Real games don't run faster than 16fps)
    // Only step the simulation after 1/60 second has passed in real time
    if( LastTime < 0.016f )
        return;

    // Force time step for debug
    LastTime = 0.016f;

    //
    // Advance the logic
    //

    // Step the physical world the specified amount
    s_pWorld->stepDeltaTime( LastTime );

    // Reset the delta to zero
    LastTime = 0;
}

//==============================================================================

void AppMain( s32, char** )
{
    Initialize();
    xtimer Timer;
    Timer.Start() ;
    eng_SetBackColor( xcolor(0x98,0x98,0x98,0xff) );

    while( TRUE )
    {
        if( !HandleInput() )
            break;

        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );
        eng_ActivateView    ( 0 );

        Advance( Timer.TripSec() );

        Render();

        // DONE!
        x_printfxy( 0,0, "%f", eng_GetFPS() );
        eng_PageFlip();
    }

    Shutdown();
}
