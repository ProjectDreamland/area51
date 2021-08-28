#ifndef	__CORPSE_HPP__
#define __CORPSE_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "PhysicsMgr\PhysicsInst.hpp"

//=========================================================================
// DEFINITIONS
//=========================================================================
enum eCorpseName
{
    CORPSE_GENERIC,       
    CORPSE_BRIDGES,       
    CORPSE_CARSON,        
    CORPSE_CRISPY,        
    CORPSE_CRISPY_MUTATED,
    CORPSE_CHEW,          
    CORPSE_DRCRAY,        
    CORPSE_FERRI,         
    CORPSE_LEONARD,       
    CORPSE_MCCANN,        
    CORPSE_MRWHITE,       
    CORPSE_RAMIREZ,       
    CORPSE_VICTOR,        
    CORPSE_NAME_MAX,
};

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class actor;
class actor_effects;
class character;

//=========================================================================
// CLASS
//=========================================================================

class corpse : public object
{
public:

    CREATE_RTTI( corpse, object, object )
    virtual const object_desc&  GetTypeDesc     ( void ) const;    
    static  const object_desc&  GetObjectType   ( void );


//=========================================================================
// Public functions
//=========================================================================
public:
                        corpse               ( void );
    virtual            ~corpse               ( void );

static      void            LimitCount              ( void );
static      xbool           ReachedMaxActiveLimit   ( void );

            xbool           Initialize              ( actor&                Actor ,
                                                      xbool                 bDoBodyFade = TRUE,
                                                      actor_effects*        pActorEffects = NULL );
            xbool           Initialize              ( const char*           pGeomName,
                                                      const char*           pAnimGroupName,
                                                      const char*           pAnimName,
                                                            s32             AnimFrame );
            xbool           InitializeEditorPlaced  ( void );

    virtual bbox            GetLocalBBox		    ( void ) const;
    virtual s32             GetMaterial				( void ) const { return m_Material; }
    virtual void            OnRender                ( void );
    
#ifndef X_RETAIL
    virtual void            OnDebugRender           ( void );   
    virtual void            OnColRender             ( xbool bRenderHigh );
#endif
    
    virtual void            OnRenderTransparent     ( void );
    virtual void            OnRenderShadowCast      ( u64 ProjMask );
    virtual void            OnAdvanceLogic          ( f32 DeltaTime );  
    virtual void            OnActivate              ( xbool Flag );            
    virtual void            OnPain                  ( const pain& Pain );    
    virtual void            OnColCheck              ( void );
    virtual void            OnKill                  ( void );
    virtual void            OnMove                  ( const vector3& NewPos );        
    virtual void            OnTransform             ( const matrix4& L2W    ); 
    virtual f32             GetHealth               ( void ) { return 0.0f; }

			physics_inst&   GetPhysicsInst          ( void );    
            void            SetPermanent            ( xbool Permanent );

            xcolor          GetFloorColor           ( void ) { return m_FloorProperties.GetColor(); }
    
            void            ChangeObjectGuid        ( guid NewGuid );

            skin_inst&      GetSkinInst             ( void ) { return m_PhysicsInst.GetSkinInst(); }
            const skin_inst& GetSkinInst            ( void ) const { return m_PhysicsInst.GetSkinInst(); }
    
            void            SetDrainable            ( xbool isDrainable )   { m_bDrainable = isDrainable; }
            xbool           GetDrainable            ( void )                { return ( m_bDrainable != 0 ); }
            
    virtual render_inst*        GetRenderInstPtr      ( void ) { return &m_PhysicsInst.GetSkinInst(); }
    virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return &m_PhysicsInst.GetAnimGroupHandle(); }

    virtual const char*         GetLogicalName      ( void ) { return "DEADBODY";}
            eCorpseName         NameToEnum          ( const char* pName );
            xstring             GetEnumStringCorpse ( void );
            const char*         EnumToName          ( eCorpseName theCorpse );
			const char*         GetScanIdentifier   ( void );

            void                StartFading         ( void );            
            void                StartFading         ( f32 NewFadeOutTime );            

            const decal_package* GetBloodDecalPackage( void ) const;
                  s32            GetBloodDecalGroup  ( void ) const;

            xbool               IsBloodEnabled       ( void ) const;
            xbool               IsRagdollEnabled     ( void ) const;

            void                CreateSplatDecalOnGround( void );
            void                CreateImpactEffect      ( const pain& Pain );

//=========================================================================
// Editor functions
//=========================================================================
protected:
    
    virtual void    OnEnumProp      ( prop_enum&    List );
    virtual xbool   OnProperty      ( prop_query&   I    );

//=========================================================================
// Data
//=========================================================================
protected:  

        // Static data
static  s32                     m_ActiveCount;          // # of active (moving) corpses
    
        // Misc
        guid                    m_OriginGuid;               // Guid of object that created it (if any)
        
        // Flags
        u32                     m_bActive            : 1;   // TRUE if physics should be active when editor placed
        u32                     m_bCreatedBlood      : 1;   // TRUE if blood has been created
        u32                     m_bCanDelete         : 1;   // TRUE if corpse can be deleted
        u32                     m_bPermanent         : 1;   // TRUE if corpse can never be deleted (editor placed)
        u32                     m_bDestroy           : 1;   // TRUE if corpse should be destroyed
        u32                     m_bDrainable         : 1;   // TRUE if corpse is available to be drained by a BO. 
        u32                     m_bActorCollision    : 1;   // TRUE if corpse gets pushed by actors
        u32                     m_bWorldCollision    : 1;   // TRUE if corpse collides with the world
        u32                     m_bActiveWhenVisible : 1;   // TRUE if bodies are always active when visible

        // Zone tracking
        zone_mgr::tracker       m_ZoneTracker;          // Tracks the zones.

        // Logic
        physics_inst            m_PhysicsInst;          // Physics instance
        f32                     m_TimeAlive;            // Used for time out delete
        floor_properties        m_FloorProperties;      // Floor tracking class
        actor_effects*          m_pActorEffects;        // any attached special FX (such as flame particles)
        guid                    m_FlamingDamageField;
        f32                     m_FadeOutTime; 

        // Properties (for editor created dead body)    
        s32                     m_AnimGroupName;        // Animation group to use
        s32                     m_AnimName;             // Animation to use
        s32                     m_AnimFrame;            // Animation frame to use
        f32                     m_SimulationTime;       // Time to run ragdoll for pose

	    // Blood properties
	    s32                     m_Material;             // Material type
        rhandle<decal_package>  m_hBloodDecalPackage;   // Blood package to use
        s32                     m_BloodDecalGroup;      // Decal group to use
        eCorpseName             m_CorpseName;           // Name of the corpse
        
        // Audio
        f32                     m_ImpactSfxTimer;       // Timer count down since last impact

};

//=========================================================================

inline
physics_inst& corpse::GetPhysicsInst( void )
{
	return m_PhysicsInst;
}

//=========================================================================

inline
void corpse::SetPermanent( xbool Permanent )
{
    m_bPermanent = Permanent;
}

//=========================================================================

inline
const decal_package* corpse::GetBloodDecalPackage( void ) const
{
    return m_hBloodDecalPackage.GetPointer();
}

//=========================================================================

inline
s32 corpse::GetBloodDecalGroup( void ) const
{
    return m_BloodDecalGroup;
}    

//=========================================================================

//=========================================================================
// END
//=========================================================================
#endif
