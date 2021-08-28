//=========================================================================
// WEAPON SHOTGUN
//========================================================================= 
#ifndef _WEAPONSHOTGUN_HPP
#define _WEAPONSHOTGUN_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"

//=========================================================================
class weapon_shotgun : public new_weapon
{
public:
	CREATE_RTTI( weapon_shotgun , new_weapon , object )

								weapon_shotgun		();
	virtual						~weapon_shotgun		();

    virtual	xbool	            OnProperty		        ( prop_query& rPropQuery );
    virtual void                ProcessSfx              ( void );
    virtual s32                 GetTotalSecondaryAmmo   ( void );
            void                FireBullet              ( const vector3& Pos, const radian3& Rot, 
                                                          const vector3& InheritedVelocity, guid Owner, 
                                                          const xbool bHitLiving, const xbool bIsPrimary, 
                                                          pain_handle& PainHandle );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }

    virtual xbool               GetFlashlightTransformInfo  ( matrix4& incMatrix,  vector3 &incVect );

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

protected:
    
    virtual	xbool				FireWeaponProtected         ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireSecondaryProtected	    ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    virtual	xbool				FireNPCSecondaryProtected	( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
            void                GetPainHandle               ( pain_handle& OutHandle, guid Owner, xbool bIsPrimary );
            xbool               HasLOS                      ( const vector3& A, const vector3& B );

};

//===========================================================================

inline
s32 weapon_shotgun::GetTotalSecondaryAmmo( void )
{
    // Shotgun uses primary ammo for both shots.  I'm subtracting one b/c shotgun's alt fire uses two ammo.
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount - 1;
}


#endif