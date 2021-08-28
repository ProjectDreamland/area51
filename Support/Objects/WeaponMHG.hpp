//===========================================================================
// Weapon Meson Hand Gun
//===========================================================================
#ifndef _WEAPON_MHG_HPP
#define _WEAPON_MHG_HPP

//===========================================================================
// INCLUDES
//===========================================================================
#include "NewWeapon.hpp"

//===========================================================================
class weapon_mhg : public new_weapon
{
public:
	CREATE_RTTI( weapon_mhg , new_weapon , object )

								weapon_mhg		        ( void );
	virtual						~weapon_mhg		        ( void );

    virtual s32                 GetTotalSecondaryAmmo   ( void );
            void                FireBullet              ( const vector3& Pos, const radian3& Rot, 
                                                          const vector3& Speed, const guid& Owner );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );
    
    virtual void                OnAdvanceLogic          ( f32 DeltaTime );      
    virtual void                InitWeapon              ( const vector3& rInitPos, render_state rRenderState, 
                                                          guid OwnerGuid );

    virtual	void				InitWeapon			    ( const char* pSkinFileName , 
                                                          const char* pAnimFileName , 
                                                          const vector3& rInitPos , 
                                                          const render_state& rRenderState = RENDER_STATE_PLAYER,
                                                          const guid& rParentGuid = NULL );

    virtual void                BeginIdle               ( void );
    virtual void                EndIdle                 ( void );
    virtual xbool               CanSwitchIdleAnim       ( void );

    virtual ammo_priority       GetPrimaryAmmoPriority  ( void )    { return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void )    { return AMMO_PRIMARY; }
    
    //virtual xbool               CanIntereptPrimaryFire  ( s32 nFireAnimIndex );
    virtual xbool				CanReload			    ( const ammo_priority& Priority );
    virtual void                SetCurrentRenderIndex   ( s32 nRenderIndex );
    

    virtual	void	            OnEnumProp		        ( prop_enum& List );
	virtual	xbool	            OnProperty		        ( prop_query& PropQuery );
    virtual void                OnMove                  ( const vector3& NewPos   );      
    virtual void                OnTransform             ( const matrix4& L2W      ); 

    //----------------------------------------------------------------------
protected:
    
    virtual	xbool				FireWeaponProtected         ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint );
    virtual	xbool				FireSecondaryProtected	    ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint );
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    virtual	xbool				FireNPCSecondaryProtected	( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    virtual void                InitMuzzleFx                ( void );  

            void                InitReloadFx                ( void );
            void                UpdateReloadFx              ( void );
            void                DestoryReloadFx             ( void );

    f32                         m_CurrentWaitTime;
    f32                         m_ReloadWaitTime;
    xbool                       m_bStartReloadFx;
    s32                         m_ReloadBoneIndex;
    s32                         m_AltReloadBoneIndex;
    guid                        m_ReloadFxGuid;
    guid                        m_AltReloadFxGuid;
    rhandle<char>               m_ReloadFx;             // The reload particle effect.
    rhandle<char>               m_PrimaryProjectileFx;  // The primary fire energy projectile particle effect.
    rhandle<char>               m_SecondayProjectileFx; // The secondary fire energy projectile particle effect.

    f32                         m_SecondaryFireBaseDamage;
    f32                         m_SecondaryFireMaxDamage;
    f32                         m_SecondaryFireSpeed;
    f32                         m_SecondaryFireBaseForce;
    f32                         m_SecondaryFireMaxForce;
};

//===========================================================================

inline
s32 weapon_mhg::GetTotalSecondaryAmmo( void )
{
    // Shotgun uses primary ammo for both shots.  I'm subtracting one b/c shotgun's alt fire uses two ammo.
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount - 1;
}


#endif