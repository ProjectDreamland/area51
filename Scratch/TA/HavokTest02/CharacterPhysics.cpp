/* 
 * 
 * Havok Game Dynamics SDK (C) Copyright 2000/2003 Telekinesys Research Limited.
 *  See end of file for license agreement.
 * Use of this software is subject to the terms of that license.
 * 
 */
#include "CharacterPhysics.hpp"
#include <hkutilities2/hkHavok2Common.h>
#include <hkdynamics2/phantom/hkCharacterPhantom.h>
#include <hkcollide2/dispatch/hkCollisionEnvironment.h>




CharacterControlAction::CharacterControlAction(		hkCharacterPhantom* phantom,
													hkWorld* world,
													const hkVector4& characterSize )
{

	m_displacement.setZero4();
	m_displacementTrim.setZero4();
	m_world = world;
	m_phantom = phantom;

	m_velocity.setZero4();
	m_minimumDistance = 0.f;
	m_localUp.set(0.f,1.f,0.f);
	m_characterSize = characterSize;

	m_rayLength = 0.7f * characterSize(1); //compatible with the 1.6 version
	
	// initialize the surface details
	updateSurfaceDetails();

	phantom->addReference();
	

}


CharacterControlAction::~CharacterControlAction()
{
	
	m_phantom->removeReference();
	m_phantom = HK_NULL;
}



bool CharacterControlAction::updateSurfaceDetails()
{
	//initialize details
	m_surfaceDetails.m_collidable = HK_NULL;
	m_surfaceDetails.m_distance = -1;
	m_surfaceDetails.m_linearVelocity.setZero4();
	m_surfaceDetails.m_angularVelocity.setZero4(); 

	const hkVector4& rayStartWS = m_phantom->getTransform().getTranslation();


	hkVector4 rayEndWS = rayStartWS;

	rayEndWS.addMul4(-m_rayLength , m_localUp);

	hkReal	mindist = 1.f;

	m_surfaceDetails.m_collidable = m_phantom->castRay(rayStartWS, rayEndWS, m_surfaceDetails.m_normal , m_surfaceDetails.m_distance);
	
	//m_surfaceDetails.m_rigidBody =0;
	if (m_surfaceDetails.m_collidable)
	{
		m_surfaceDetails.m_distance *= m_rayLength;
		
		// check for rigidbody type, and do an upcast
		if (m_surfaceDetails.m_collidable->getType() == HK_BROAD_PHASE_ENTITY)
		//if (m_surfaceDetails.m_rigidBody->getType() == HK_RIGID_BODY)
		{
			hkRigidBody* rb = (hkRigidBody*)m_surfaceDetails.m_collidable->getOwner();
			//if (rb->getType() == HK_RIGID_BODY)
			{
				// fill in velocities
				hkVector4 contactPoint;
				contactPoint.setInterpolate4(rayStartWS,rayEndWS,mindist);

				rb->getPointVelocity(contactPoint,m_surfaceDetails.m_linearVelocity);
				m_surfaceDetails.m_angularVelocity = rb->getAngularVelocity();
			}

		} else
		{
			m_surfaceDetails.m_collidable = HK_NULL;
		}
		
	}

	return m_surfaceDetails.valid();
}

bool CharacterControlAction::attemptDisplacement()
{
	// store original positions
	hkTransform oldTransform = m_phantom->getTransform();
	hkVector4 lastPos = m_phantom->getTransform().getTranslation();

	// move body back to new position
	hkVector4 newTrans; newTrans.setAdd4( lastPos, m_displacement );
	m_phantom->setTranslation( newTrans );
		
	
	//return closest distance + normal
	
	bool penetration = false;


	int minindex = m_phantom->updateCollisions( *m_world->getCollisionEnvironment() );

	if (minindex >= 0 && (m_phantom->getCollisionDetails()[minindex]->getClosestDistance() < m_world->getCollisionEnvironment()->m_tolerance))
	{
		m_minimumDistance = m_phantom->getCollisionDetails()[minindex]->getClosestDistance();
		//either colliding or penetrating
		

		if (m_minimumDistance > 0.f) //m_world->getCollisionEnvironment()->m_tolerance/2.f)
		{
			penetration = false;
		} 
		else
		{
			penetration =  true;
		}

	}
	else
	{
		// no collisions, full movement
		m_minimumDistance = m_world->getCollisionEnvironment()->m_tolerance;

		return true;

	}


	// check for collisions
	// check for interpenetrations
	//those two checks are similar to checking for closestDistance
	
	
	if (penetration)
	{
		//don't move at all
				// move failed so place body back to original position
		m_phantom->setTranslation( lastPos );

		// can't move anywhere
		m_displacement.setZero4();

		// interpenetration
		m_minimumDistance = 0.f; 

		return false;

	}

	
	
	// trim displacement based on all collisions 
	hkVector4 newDisplacement	= m_displacement;
	
	hkVector4 sumNormals;
	sumNormals.setZero4();

	
		
	const hkArray<hkSimpleClosestPointsDetails*>& collisionDetails = m_phantom->getCollisionDetails();

	for ( int i = 0; i < collisionDetails.getSize(); i++ ) 
	{

		//every overlapping object can have more then 1 collidable pair 
		//for example one object is a mopp, with both ground triangles and wall triangles
		//the one overlapping object (mopp) can have both ground and wall collisions with the character
		
		hkSimpleClosestPointsDetails* details = collisionDetails[i];

		const hkArray<hkContactPoint>& points = details->getContactPoints();

		for ( int j = 0; j < points.getSize(); j++ )
		{
					
			hkVector4  contactNormal;
			contactNormal.setNeg4( points[j].getNormal() );
				
			// accumulate normals
			sumNormals.add4(contactNormal);
	
		}

	}

	// trim displacement based on collision normals
	sumNormals.normalize3();
	hkReal	distance;
	distance = newDisplacement.dot3( sumNormals );
	if( 0.f > distance )
	{
		newDisplacement.addMul4(-distance,sumNormals);
	}

	// move body to the new position
	hkVector4 moveVec; moveVec.setAdd4( lastPos, newDisplacement );
	m_phantom->setTranslation( moveVec );

	// return final movement vector
	m_displacement = newDisplacement;

	return true;

}


void CharacterControlAction::applyAction(const hkStepInfo& stepInfo)
{
	// accumulate and apply forces (user and gravity)
		
	//predict a 'safe motion'
	
	
	// add on the velocity component (Euler integrate)
	
	
	if( m_surfaceDetails.valid() )
	{
		// if the body is standing on something 
		// make sure it is at the right distance above 
		// the surface and that it moves with it
		if( m_surfaceDetails.m_distance < m_rayLength )
		{
			// this upward displacement need not happen in one go
			// a damping value would make a smoother rise.
			// a factor of the tolerance is removed from the surface 
			// distance to ensure that the raycast keeps succeeding
			// if the character is not moved. (in order to prevent jitter)
			
			m_displacement.addMul4((m_rayLength - m_surfaceDetails.m_distance - 
				0.01f * m_world->getCollisionEnvironment()->m_tolerance) , m_localUp);


		}

		// have surface velocity effect displacement directly
		// adding to m_velocity can create slingshot effects
		// e.g. being thrown from a horizontaly spinning 
		// platform such as a tree trunk floating in water
		m_displacement.addMul4(stepInfo.m_deltaTime , m_surfaceDetails.m_linearVelocity);
	}

	//todo: add on the acceleration

	// add on the velocity component 
	m_displacement.addMul4(stepInfo.m_deltaTime , m_velocity);
	
		// was the last displacement vector trimmed?
	bool	displacementTrimmed			= 0.00001f < m_displacementTrim.length3();

	
	// store the displacement vector before trimming
	const hkVector4	untrimmedDisplacement = m_displacement;

	if( displacementTrimmed )
	{
		hkVector4 normalizedDisplacementTrim	= m_displacementTrim;
		hkReal len = normalizedDisplacementTrim.lengthSquared3();
		if ( len > 0.00001f )
		{
			normalizedDisplacementTrim.mul4(hkMath::sqrtInverse(len));	
		
			// if the body hit an object on its travels then we 
			// should kill any velocity in that direction
			// NOTE: this is a bit of a bitch as it can currently
			// gain a bit of lateral velocity if it slides down 
			// from another object. this is why the velocity is
			// being killed when there is a surface (see above).
			if( 0.00001f < m_velocity.length3() )
			{
				// rather than kill we could affect velocity based on 
				// restitution values of body and surface. 
				const hkReal	distance = m_velocity.dot3( normalizedDisplacementTrim );
				if( 0.f > distance )
				{
					hkVector4 sub; sub.setMul4(distance, normalizedDisplacementTrim);
					m_velocity.sub4( sub );
				}
			}

			// also kill displacement based on trim from previous iteration
			// NOTE: for large displacements, if everything is trimmed then the body 
			// is most likely to get an immediate interpenetration and stop.
			// hence the minimum collision distance is added to help allow sliding 
			// along surfaces even with large displacements.
			if( 0.00001f < m_displacement.length3() )
			{
				const hkReal dot = m_displacement.dot3( normalizedDisplacementTrim );
				const hkReal distance = dot + m_minimumDistance;
				if( 0.f > distance )
				{
					m_displacement.addMul4(-distance , normalizedDisplacementTrim);
				}
			}
		}
	}

	// need the last minimum distance to check to see
	// if trimming should be stopped
	const hkReal	lastMinimumDistance = m_minimumDistance;


	//
	// try to move the body
	//
	attemptDisplacement( );


	// store difference for next iteration
	m_displacementTrim = m_displacement;
	m_displacementTrim.sub4( untrimmedDisplacement );

	// trimming is on or off depending on the whether there was any 
	// displacement trimmed in last apply. the length of this
	// trim vector may not change if the body slides 
	// of the end of an object. hence we need to do a further check
	// to zero this trim vector if the body has entered an open space.
	// the body is only in an open space if there were no collision
	// i.e. if the minimum collision distance is the same as the 
	// subspace tolerance. to make this check smoother the minimum
	// distance is also compared with the value before displacement
	// was attempted.
	if(( m_world->getCollisionEnvironment()->m_tolerance == lastMinimumDistance) && (lastMinimumDistance == m_minimumDistance) )
	{
		m_displacementTrim.setZero4();
	}


	//
	// Apply gravity if the character is not on the ground
	//

	if (updateSurfaceDetails())
	{
		m_velocity.setZero4();
	}
	else
	{
		m_velocity		= m_velocity;
		m_velocity.addMul4( stepInfo.m_deltaTime , m_world->getGravity());
	}


	//
	// Update the bounding box of the character phantom to the new position (this must be done explicitly)
	//

	hkVector4 start_ws = m_phantom->getTransform().getTranslation();
	start_ws.add4( m_characterSize );
	hkVector4 end_ws = m_phantom->getTransform().getTranslation();
	end_ws.sub4( m_characterSize );

	hkPhantomConstructionInfo pci;
	pci.m_min.setMin4(start_ws,end_ws);
	pci.m_max.setMax4(start_ws,end_ws);

	
	hkAabb aabb(pci.m_min,pci.m_max);
	m_phantom->updateAabb(aabb);


}

void	CharacterControlAction::addVelocity(const hkVector4& impulse)
{
	m_velocity.add4(impulse);
}

void	CharacterControlAction::setDisplacement(const hkVector4& displacement) 
{
	m_displacement = displacement;
}


hkVector4 CharacterControlAction::getVelocityForJumpHeight( hkReal height )
{
	// v^2 = u^2 + 2as
	// v = zero at top of jump
	// i.e. u = sqrt( -2as )

	const hkReal	a = m_world->getGravity().length3();
	const hkReal	s = hkMath::fabs( height );

	const hkReal	IuI = hkMath::sqrt( 2 * a * s );

	hkVector4 result;
	
	result.setMul4((-IuI / a), m_world->getGravity());

	return result;
}


/*
 * Havok Game Dynamics SDK  - DEMO RELEASE, BUILD(#20030206)
 * 
 * (C) Copyright 2000/2002 Telekinesys Research Limited. All Rights Reserved.  The Telekinesys
 * Logo, Havok and the Havok buzzsaw logo are trademarks of Telekinesys
 * Research Limited. 
 * 
 * Use of this software is subject to and indicates acceptance of the End User licence
 * Agreement for this product. A copy of the license is included with this software and 
 * is also available from info@havok.com.
 * 
 * This software is owned by Telekinesys
 * Research Limited and is protected by Irish copyright, United States
 * copyright, and other intellectual property laws.  Title, ownership
 * rights, and intellectual property rights in the Software remain in
 * Telekinesys Research Limited and/or its suppliers. 
 */

