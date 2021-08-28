//=========================================================================
// WEAPON GUASS
//=========================================================================
#ifndef _WEAPON_GAUSS_HPP
#define _WEAPON_GAUSS_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"

//=========================================================================
class weapon_gauss : public new_weapon
{
public:

    struct LED_info
    {
        s8 Mesh;
        s8 Material;
    };

	CREATE_RTTI( weapon_gauss , new_weapon , object )

					weapon_gauss		();
	virtual			~weapon_gauss		();

    virtual void                InitWeapon          ( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid );
	
	virtual	void				InitWeapon			(   const char* pSkinFileName , 
                                                        const char* pAnimFileName , 
                                                        const vector3& rInitPos , 
                                                        const render_state& rRenderState = RENDER_STATE_PLAYER,
                                                        const guid& rParentGuid = NULL );
    
    virtual void                ProcessSfx              ( void );
    virtual void                OnMove                  ( const vector3& NewPos   );      
    virtual void                OnTransform             ( const matrix4& L2W      ); 

    virtual void                BeginPrimaryFire        ( void );
    virtual void                EndPrimaryFire          ( void );
    virtual void                Reload                  ( const ammo_priority& Priority );

    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_SECONDARY; }

    virtual xbool               CanIntereptPrimaryFire  ( s32 nFireAnimIndex );
    virtual xbool               CanIntereptSecondaryFire( s32 nFireAnimIndex );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );

protected:
    
    virtual	xbool				FireWeaponProtected			( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint );
    virtual	xbool				FireSecondaryProtected		( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint );
    virtual void                InitMuzzleFx                ( void ) ;  
            void                SynchGunToCount             ( void ) ;
    
    s32                         m_LoopVoiceId;
    LED_info                    m_LEDInfo[3];
};

#endif
