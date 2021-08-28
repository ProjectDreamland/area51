//=========================================================================
// WEAPON DESERT EAGLE
//=========================================================================
#ifndef _WEAPON_DESERT_EAGLE_HPP__
#define _WEAPON_DESERT_EAGLE_HPP__

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"

//=========================================================================
class weapon_desert_eagle : public new_weapon
{
public:
	CREATE_RTTI( weapon_desert_eagle , new_weapon , object )

								weapon_desert_eagle		();
	virtual						~weapon_desert_eagle	();

    virtual void                ProcessSfx              ( void );
    virtual s32                 GetTotalSecondaryAmmo   ( void );
            void                FireBullet              ( const vector3& Pos, const radian3& Rot, const vector3& Speed, guid Owner, const xbool isHit );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );
    
    virtual xbool               CanFastTapFire          ( void ) { return TRUE; }
    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }

    virtual void                OnAdvanceLogic          ( f32 DeltaTime );      

    virtual	void	            OnEnumProp		        ( prop_enum& list );
	virtual	xbool	            OnProperty		        ( prop_query& rPropQuery );
    virtual const char*         GetLogicalName          ( void ) {return "DEAGLE";}

    virtual xbool               GetFlashlightTransformInfo  ( matrix4& incMatrix,  vector3 &incVect );

    virtual xbool               ShouldUpdateReticle         ( void );
    virtual xbool               ShouldDrawReticle           ( void );

protected:
    
    virtual	xbool				FireWeaponProtected         ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireSecondaryProtected	    ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    
    radian                      m_ZoomFOV;
    f32                         m_ViewChangeRate;
    f32                         m_StickSensitivity;
};

//===========================================================================

inline
s32 weapon_desert_eagle::GetTotalSecondaryAmmo( void )
{
    // Eagle uses primary ammo for both shots.
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
}


#endif