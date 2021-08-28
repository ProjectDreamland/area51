//=========================================================================
// WEAPON SNIPER
//=========================================================================
#ifndef _WEAPONSNIPER_HPP
#define _WEAPONSNIPER_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"

//=========================================================================
class weapon_sniper_rifle : public new_weapon
{
public:

	CREATE_RTTI( weapon_sniper_rifle , new_weapon , object )

    //---------------------------------------------------------------------
	enum sniper_state
	{
		STATE_ZOOM_UNDEFINED = -1,
		STATE_NO_ZOOM,
		STATE_ZOOMING,
		STATE_WAITING_FOR_RESET,
		STATE_ZOOM_MAX
	};
    //---------------------------------------------------------------------
public:

								weapon_sniper_rifle			();
	virtual						~weapon_sniper_rifle		();

    virtual void                InitWeapon          ( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid );

    virtual	void				InitWeapon			(   const char* pSkinFileName , 
                                                        const char* pAnimFileName , 
                                                        const vector3& rInitPos , 
                                                        const render_state& rRenderState = RENDER_STATE_PLAYER,
                                                        const guid& rParentGuid = NULL );
    virtual s32                 GetTotalSecondaryAmmo   ( void );
	virtual	void                OnAdvanceLogic      ( f32 DeltaTime );   
    virtual	xbool	            OnProperty		    ( prop_query& rPropQuery );
	virtual	void				RenderWeapon	    ( xbool bDebug, const xcolor& Ambient, xbool Cloaked );

			void				SetState			( const sniper_state& rNewState );
            sniper_state        GetState            ( void ){ return m_CurrentState; }
			void				BeginState			( void );
			void				UpdateState			( const f32& rDeltaTime );
			void				EndState			( void );

			void				OnZoomMode			( const f32& DeltaTime );

    virtual void                ResetWeapon         ( void );

			void				OnExitSniper		( void );
    virtual const object_desc&  GetTypeDesc			( void ) const;
    static  const object_desc&  GetObjectType		( void );
    virtual s32                 IncrementZoom       ( void );
    virtual void                ClearZoom           ( void );
    virtual const char*         GetLogicalName       ( void ) {return "SNIPER";}

    virtual xbool               HasFlashlight               ( void )        { return FALSE; }

    virtual xbool               ShouldUpdateReticle         ( void );
    
    //---------------------------------------------------------------------
protected:
    
    virtual	xbool				FireWeaponProtected		( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint);
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit = TRUE );
    virtual	xbool				FireNPCSecondaryProtected	( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit = TRUE );

            void				BeginNoZoom			    ( void );
			void				UpdateNoZoom		    ( const f32& rDeltaTime );

			void				BeginZooming		    ( void );
			void				UpdateZooming		    ( const f32& rDeltaTime );

			void				UpdateWaitForReset	    ( const f32& rDeltaTime );

    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }


	sniper_state				m_CurrentState;		//current state of the sniper rifle's zoom

	f32							m_RateOfViewChange;	//radians per second that the view changes while zooming
    f32                         m_ViewClickDimension;
    s32                         m_SniperZoomSound;
};

//===========================================================================

inline
s32 weapon_sniper_rifle::GetTotalSecondaryAmmo( void )
{
    // Sniper has only one attack.  Must return 1 for secondary ammo count so that we are able to zoom.
    return 1;
}

//=========================================================================
// END
//=========================================================================
#endif
