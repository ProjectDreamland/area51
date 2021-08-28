//=========================================================================
// WEAPON SCANNER
//=========================================================================
#ifndef _WEAPON_SCANNER_HPP__
#define _WEAPON_SCANNER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"

enum eScanState
{
    SCAN_NONE,
    SCAN_LORE,
    SCAN_AIR,
    SCAN_CHARACTER,
    SCAN_CORPSE,
    SCAN_OBJECT,
    SCAN_PLAYER,
    SCAN_MAX
};

enum eVOIdentifiers
{
    VO_SCAN_SELF,
    VO_SCAN_SELF_MUT,
    VO_SCAN_LORE,
    VO_SCAN_NOTHING,
};

//=========================================================================
class weapon_scanner : public new_weapon
{
public:
	CREATE_RTTI( weapon_scanner , new_weapon , object )

								weapon_scanner		();
	virtual						~weapon_scanner	();

    virtual s32                 GetTotalSecondaryAmmo   ( void );
            void                FireBullet              ( const vector3& Pos, const radian3& Rot, const vector3& Speed, guid Owner, const xbool isHit );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );
    
    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }

    virtual void                OnAdvanceLogic          ( f32 DeltaTime );      

    virtual	void	            OnEnumProp		        ( prop_enum& list );
	virtual	xbool	            OnProperty		        ( prop_query& rPropQuery );
    virtual const char*         GetLogicalName          ( void ) {return "SCANNER";}
    virtual xbool               CanFire                 ( xbool bIsAltFire );
    virtual void                ClearScan               ( void );    
    virtual xbool               DoCollisionCheck        ( vector3 &EndPoint, guid &ObjGuid );
            xbool               GetObjectPositionalInfo ( guid objGuid, radian &AngleBetween, vector3 &StartPos, vector3 &EndPos );

    virtual void                RenderWeapon            ( xbool bDebug, const xcolor& Ambient, xbool Cloaked );

            f32                 GetCurrentScanTime      ( void ) { return m_fScanTime; }
            f32                 GetMaxScanTime          ( void );

    virtual xbool               GetFlashlightTransformInfo  ( matrix4& incMatrix,  vector3 &incVect );

    virtual void                OnRenderTransparent         ( void );

    virtual void                BeginSwitchFrom             ( void );
    virtual void                BeginSwitchTo               ( void );
    virtual void                EndSwitchTo                 ( void );
            void                FlashLogic                  ( f32 DeltaTime );
            void                CheckForScanComplete        ( void );
            guid                GetScannedGuid              ( void ) { return m_ScannedGuid; }
            void                GetScannerVOIdent           ( char *pIdentifier, eVOIdentifiers Ident );
    virtual xbool               CanIntereptPrimaryFire      ( s32 nFireAnimIndex );

protected:
    
    virtual	xbool				FireWeaponProtected         ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireSecondaryProtected	    ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual	xbool				FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    virtual guid                CheckHitObject              ( void );
            void                DrawScanningLaser           ( void );
            void                GetLaserHitLocation         ( player* pPlayer, vector3& EndPos, xbool bIsOnLoreObject );
            void                GetSamplingLaserHitLocation ( player* pPlayer, vector3& EndPos, xbool bIsOnLoreObject );
            void                DrawEnclosures              ( void );
            void                TESTEnclosures              ( void );

            void                KillLaserSound              ( void );
            void                UpdateScanEffect            ( void );
            void                KillScanEffect              ( void );
            void                PlayLaserSound              ( void );
            xbool               CanScan                     ( void );

            void                UpdateLoreCount             (f32 DeltaTime);
    
    radian                      m_ZoomFOV;
    f32                         m_ViewChangeRate;
    f32                         m_StickSensitivity;

    f32                         m_fScanTime;
    guid                        m_ScanStartGuid;
    guid                        m_ScanEndGuid;
    xbool                       m_bCanScan;
    xbool                       m_bBootUpAnimFinished;
    xbool                       m_bInitialScan;

    s32                         m_LaserOnLoopId;

    rhandle<xbitmap>            m_LaserBitmap;
    rhandle<xbitmap>            m_LaserFixupBitmap;
    rhandle<char>               m_FXScannerBox;

    vector3                     m_BBoxVerts[8];
    u32                         m_ScanIndex;
    u16                         m_ScanTimesPerFace;

    xbool                       m_bFlashEnclosures;
    f32                         m_CurrentFlashTime;
    u32                         m_NumberOfFlashes;

    f32                         m_LastLoreUpdate;
    eScanState                  m_ScanState;
    guid                        m_ScannedGuid;

    guid                        m_ScanEffect;
    fx_handle                   m_ScanBoxHandle;
};

//===========================================================================

inline
s32 weapon_scanner::GetTotalSecondaryAmmo( void )
{
    // Scanner uses no ammo.
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
}

#endif