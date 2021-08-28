//=========================================================================
// WEAPON DUAL SMP
//=========================================================================
#ifndef _WEAPON_DUAL_SMP_HPP
#define _WEAPON_DUAL_SMP_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"


//=========================================================================

class weapon_dual_smp : public new_weapon
{
public:
	CREATE_RTTI( weapon_dual_smp , new_weapon , object )

								weapon_dual_smp		        ( void );
	virtual		  			   ~weapon_dual_smp		        ( void );
    
    //--------------------------------------------------------------------
    // OBJECT METHODS
    //--------------------------------------------------------------------
    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );

    virtual	void	            OnEnumProp		        ( prop_enum& list );
	virtual	xbool	            OnProperty		        ( prop_query& rPropQuery );

    virtual void                OnAdvanceLogic          ( f32 DeltaTime );      

    virtual void                OnMove                  ( const vector3& NewPos   );      
    virtual void                OnTransform             ( const matrix4& L2W      ); 
    //--------------------------------------------------------------------

    virtual void                ProcessSfx				( void );    
    virtual s32                 GetTotalSecondaryAmmo   ( void );

    virtual void                InitWeapon              ( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid );
    virtual	void				InitWeapon			    (   const char* pSkinFileName , 
                                                            const char* pAnimFileName , 
                                                            const vector3& rInitPos , 
                                                            const render_state& rRenderState = RENDER_STATE_PLAYER,
                                                            const guid& rParentGuid = NULL 
                                                        );
    
    virtual xbool               CanReload               ( const ammo_priority& Priority );
    //--------------------------------------------------------------------
    // FIRING STATES
    //--------------------------------------------------------------------
    virtual void                BeginAltFire        ( void );
    virtual void                EndAltFire          ( void );
    virtual void                BeginPrimaryFire        ( void );
    virtual void                EndPrimaryFire          ( void );
    virtual void                ReleaseAudio            ( void );


    virtual xbool               CanIntereptPrimaryFire  ( s32 nFireAnimIndex );

    
    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }
    virtual const char*         GetLogicalName           ( void ) {return "SMP";}
    virtual xbool               GetFlashlightTransformInfo  ( matrix4& incMatrix,  vector3 &incVect );

protected:
       
    //--------------------------------------------------------------------
    // FIRING ROUTINES
    //--------------------------------------------------------------------
    virtual	xbool				FireWeaponProtected		    ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool bHitLiving  );

    //--------------------------------------------------------------------
    // DATA
    //--------------------------------------------------------------------
    s32                         m_LoopVoiceId;
    f32                         m_ZoomFOV;
    f32                         m_ViewChangeRate;
    dual_weapon_anim_controller m_DualAnimController;
};

//===========================================================================

inline
s32 weapon_dual_smp::GetTotalSecondaryAmmo( void )
{
    // SMP uses primary ammo for both shots.
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
}

//=========================================================================
// END
//=========================================================================    
#endif