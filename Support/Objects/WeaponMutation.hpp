//=========================================================================
// WEAPON MUTATION
//=========================================================================
#ifndef _WEAPONMUTATION_HPP
#define _WEAPONMUTATION_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "NewWeapon.hpp"
#include "Objects\Player.hpp"

#define MAX_TENDRIL_CONSTRAINTS 3
#define CORPSE_PIN              0

//=========================================================================
class weapon_mutation : public new_weapon
{
public:

	CREATE_RTTI( weapon_mutation , new_weapon , object )
    
    //---------------------------------------------------------------------
public:
    							weapon_mutation			();
	virtual						~weapon_mutation		();

    virtual void                InitWeapon          ( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid );

    virtual	       void	        OnEnumProp		        ( prop_enum& list );
	virtual	       xbool	    OnProperty		        ( prop_query& rPropQuery );
    virtual s32                 GetTotalSecondaryAmmo   ( void );

			void				OnExitMutation		    ( void );
    virtual const object_desc&  GetTypeDesc			    ( void ) const;
    static  const object_desc&  GetObjectType		    ( void );

    virtual xbool               GetFiringStartPosition      ( vector3 &Pos );
            xbool               GetAltFiringStartPosition   ( vector3 &Pos );
            xbool               CanFire                     ( xbool bIsAltFire );
            xbool               FireTendril                 ( const vector3& InitPos , const vector3& BaseVelocity, const radian3& InitRot, const guid& Owner, xbool bLeft );
            xbool               RetractTendril              ( xbool bLeft );            

            void                CreateCorpse                ( actor *pActor );
            void                CreateCorpsePin             ( corpse *pCorpse, s32 Index );
            void                GiveMutagenFromTendrils     ( void );
            void                UpdateCorpsePin             ( void );
            void                ClearCorpseConstraints      ( void );
            void                GetCorpseDirection          ( vector3 &StartPos, vector3 &EndPos );

    virtual void                OnRenderTransparent         ( void );
    virtual xbool               HasFlashlight               ( void )        { return FALSE; }

    virtual const char*         GetLogicalName              ( void ) {return "MUTATION";}
    virtual void                BeginSwitchFrom             ( void );
            
    //---------------------------------------------------------------------
    // Melee stuff
    //---------------------------------------------------------------------
    void            Setup               ( const guid& PlayerGuid, const player::animation_state& AnimState );
    radian          GetXFOV             ( void );
    virtual void    OnAdvanceLogic      ( f32 DeltaTime );
    virtual void    TendrilLogic        ( player *pPlayer, f32 DeltaTime );

    void            DoExtremeMelee      ( void );
    void            SetMeleeComplete    ( xbool bComplete );
    
    //---------------------------------------------------------------------

    virtual xbool               FireGhostPrimary            ( s32 iFirePoint, xbool bUseFireAt, vector3& FireAt );
    virtual xbool               FireGhostSecondary          ( s32 iFirePoint );

protected:
    
    virtual	xbool				FireWeaponProtected		( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint);
    virtual	xbool				FireSecondaryProtected  ( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint);

    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }
            void                SpawnMutagen            ( object *pObject ) const;
            xbool               ImposeMutagenCosts      ( object *pOwner, xbool bIsPrimaryFire = TRUE );
            guid                FindDumbTarget          ( const vector3& InitPos, const radian3& InitRot, guid Owner );

            xbool               TriggerContagion       ( void );

    //---------------------------------------------------------------------
    // Melee stuff
    //---------------------------------------------------------------------
    
    void                        CausePain               ( player *pPlayer, object *pObject ) const;    
    void                        ChangeFOV               ( f32 DeltaTime );    
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // Camera stuff
    //---------------------------------------------------------------------
    // Zoom stuff
    f32             m_ViewChangeRate;       // how fast we change the FOV
    radian		    m_CurrentViewX;         // Current x-view
    
    xbool           m_bMeleeComplete;        // this may be obsolete    
    guid            m_HostPlayerGuid;       // what player guid are we tied to?
    //---------------------------------------------------------------------

    // Particle Property stuff
    rhandle<char>   m_hMeleeParticle_Hit;
    s32             m_nPrimaryParasitesPerShot;
    s32             m_nSecondaryParasitesPerShot;
	f32             m_PrimaryFireDelay;
    f32             m_TimeSinceLastPrimaryFire;
    s32             m_MutationZoomSound;

    s32                     m_MutagenTemplateID;
    player::animation_state m_MeleeState;    

    guid            m_ContagionGuid;
    guid            m_LeftTendril;
    guid            m_RightTendril;
    guid            m_CorpseGuid;
    s32             m_iConstraint[MAX_TENDRIL_CONSTRAINTS];
    s32             m_iBone[MAX_TENDRIL_CONSTRAINTS];
    vector3         m_ConstraintPosition[MAX_TENDRIL_CONSTRAINTS];
    vector3         m_ConstraintOffset[MAX_TENDRIL_CONSTRAINTS];
    f32             m_LiftHeight;
};

//===========================================================================

inline
s32 weapon_mutation::GetTotalSecondaryAmmo( void )
{
    return 0;
}

//=========================================================================
// END
//=========================================================================
#endif
