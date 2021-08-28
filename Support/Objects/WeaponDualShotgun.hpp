//=========================================================================
// WEAPON DUAL SHOTGUN
//========================================================================= 
#ifndef _WEAPON_DUAL_SHOTGUN_HPP
#define _WEAPON_DUAL_SHOTGUN_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"

//=========================================================================
class weapon_dual_shotgun : public new_weapon
{
public:
	CREATE_RTTI( weapon_dual_shotgun , new_weapon , object )

								weapon_dual_shotgun		();
	virtual						~weapon_dual_shotgun	();

    virtual void                ProcessSfx          ( void );
    virtual s32                 GetTotalSecondaryAmmo   ( void );

    virtual void                InitWeapon              ( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid );
    virtual	void				InitWeapon			    (   const char* pSkinFileName , 
                                                            const char* pAnimFileName , 
                                                            const vector3& rInitPos , 
                                                            const render_state& rRenderState = RENDER_STATE_PLAYER,
                                                            const guid& rParentGuid = NULL 
                                                        );

            void                FireBullet          ( const vector3& Pos, const radian3& Rot, const vector3& InheritedVelocity, guid Owner, const xbool bHitLiving, const xbool bIsPrimary, pain_handle& PainHandle, s32 iFirePoint );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }

    virtual void                Reload                  ( const ammo_priority& Priority );
    virtual xbool               ContinueReload          ( void );
    virtual xbool               CanFire                 ( xbool bIsAltFire );
    virtual const char*         GetLogicalName           ( void ) {return "SHOTGUN";}

            void                HandleSweeperCone       (   const vector3&  Pos,
                                                            const vector3&  Dir,
                                                            const radian3&  Rot,
                                                            const pain_handle& Pain,
                                                            radian          ConeAngle, 
                                                            f32             Distance );
    
    virtual xbool               CanReload               ( const ammo_priority& Priority );
    virtual xbool               GetFlashlightTransformInfo  ( matrix4& incMatrix,  vector3 &incVect );

protected:
    
    virtual	xbool				FireWeaponProtected         ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireSecondaryProtected	    ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    virtual	xbool				FireNPCSecondaryProtected	( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
            void                GetPainHandle               ( pain_handle& OutHandle, guid Owner, xbool bIsPrimary );
            xbool               HasLOS                      ( const vector3& A, const vector3& B );


//--------------------------------------------------------------------
// DATA
//--------------------------------------------------------------------
    dual_weapon_anim_controller m_DualAnimController;

};

//===========================================================================

inline
s32 weapon_dual_shotgun::GetTotalSecondaryAmmo( void )
{
    // Shotgun uses primary ammo for both shots.  I'm subtracting one b/c shotgun's alt fire uses two ammo.
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount - 1;
}


#endif