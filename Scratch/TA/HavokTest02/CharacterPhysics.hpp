/* 
 * 
 * Havok Game Dynamics SDK (C) Copyright 2000/2003 Telekinesys Research Limited.
 *  See end of file for license agreement.
 * Use of this software is subject to the terms of that license.
 * 
 */

#ifndef HK_CHARACTERCONTROLACTION_H
#define HK_CHARACTERCONTROLACTION_H


#include <hkdynamics2/action/hkAction.h>
#include <hkcollide2/hkCollidable.h>

class hkWorld;
class hkCharacterPhantom;

class CharacterControlAction : public hkReferencedObject
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_DEMO)
			
			// constructor
		CharacterControlAction(		hkCharacterPhantom* phantom,
									hkWorld* world,
									const hkVector4& characterSize );

			// destructor
		virtual ~CharacterControlAction();

			// implementation of hkAction interface
		virtual void applyAction( const hkStepInfo& stepInfo );

		
		hkCharacterPhantom* getPhantom() { return m_phantom; }


		//
		// Character Controller API
		//

		void addVelocity(const hkVector4& impulse);

		void setDisplacement(const hkVector4& displacement);

		hkVector4 getVelocityForJumpHeight(hkReal	height);


		struct	SurfaceDetails
		{
			hkCollidable*		m_collidable;
			hkReal			m_distance;
			hkVector4		m_normal;
			hkVector4		m_linearVelocity;
			hkVector4		m_angularVelocity;

			bool	valid() const	{ return (HK_NULL != m_collidable); }
		};	

		const SurfaceDetails& getSurfaceDetails() { return m_surfaceDetails; }


	protected:

		bool	updateSurfaceDetails();
		bool	attemptDisplacement();

	private:

		hkVector4 m_characterSize;
		hkVector4	m_velocity;
		hkVector4	m_displacement; // user-controlled destination pos (warp instead of velocity!)
		hkVector4	m_displacementTrim;
		
		hkWorld* m_world;
		hkReal		m_minimumDistance;

		//this is the representation of the character
		hkCharacterPhantom* m_phantom;



		SurfaceDetails	m_surfaceDetails;
		
		hkVector4	m_localUp;
		
		hkReal		m_rayLength;

};

#endif // HK_CHARACTERCONTROLACTION_H


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

