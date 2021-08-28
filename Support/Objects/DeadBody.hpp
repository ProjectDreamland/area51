#ifndef	DEAD_BODY_HPP
#define DEAD_BODY_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "ragdoll\Ragdoll.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Characters\FloorProperties.hpp"
#include "Objects\Actor\Actor.hpp"

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class character;
class actor_effects;

//=========================================================================
// CLASS
//=========================================================================

class dead_body : public object
{
public:

    CREATE_RTTI( dead_body, object, object )
    virtual const object_desc&  GetTypeDesc     ( void ) const;    
    static  const object_desc&  GetObjectType   ( void );


//=========================================================================
// Public functions
//=========================================================================
public:
                        dead_body               ( void );
    virtual            ~dead_body               ( void );

            void        LimitDeadBodyCount      ( void );
            xbool       Initialize              ( actor&                Actor ,
                                                  xbool                 doBodyFade = TRUE,
                                                  actor_effects*        pActorEffects = NULL ) ;
            xbool       Initialize              ( const char*           pGeomName,
                                                  const char*           pAnimGroupName,
                                                  const char*           pAnimName,
                                                        s32             AnimFrame,
                                                        ragdoll::type   RagdollType ) ;
            xbool       InitializeEditorPlaced  ( void ) ;

    virtual bbox        GetLocalBBox		    ( void ) const;
    virtual s32         GetMaterial				( void ) const { return MAT_TYPE_FLESH; }
    virtual void        OnRender                ( void );
    virtual void        OnRenderTransparent     ( void );
    virtual void        OnRenderShadowCast      ( u64 ProjMask );
    virtual void        OnAdvanceLogic          ( f32 DeltaTime );  
    virtual void        OnPain                  ( const pain& Pain );    
    virtual void        OnColCheck              ( void ) ;
    virtual void        OnKill                  ( void );
    virtual void        OnMove                  ( const vector3& NewPos );        
    virtual void        OnTransform             ( const matrix4& L2W    ); 
    virtual f32         GetHealth               ( void ) { return 0.0f; }

			ragdoll*	GetRagdollPointer		( void );    
            void        SetPermanent            ( xbool Permanent );

            xcolor      GetFloorColor           ( void ) { return m_FloorProperties.GetColor(); }
    
            void        ChangeObjectGuid        ( guid NewGuid ) ;

            skin_inst&  GetSkinInst             ( void ) { return m_Ragdoll.GetSkinInst(); }
            void        SetDrainable            ( xbool isDrainable )   { m_bDrainable = isDrainable; }
            xbool       GetDrainable            ( void )                { return m_bDrainable; }

            
    virtual render_inst*        GetRenderInstPtr      ( void ) { return &m_Ragdoll.GetSkinInst(); }
    virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return &m_Ragdoll.GetAnimGroupHandle(); }

    
//=========================================================================
// Private functions
//=========================================================================
protected:
            matrix4*    GetBonesForRender       ( u64 LODMask, s32& nActiveBones );
            void        CreateBloodPool         ( void );
            void        CreateSplatDecalOnGround( const pain &Pain );

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

    // Flags
    u32                 m_bPhysics                : 1 ; // TRUE if ragdoll physics should advance
    u32                 m_bCreatedBlood           : 1 ; // TRUE if blood has been created
    u32                 m_bCanDelete              : 1 ; // TRUE if object can be deleted
    u32                 m_bPermanent              : 1 ; // TRUE if dead body can never be deleted
    u32                 m_bDestroy                : 1 ; // TRUE if object should be destroyed
    u32                 m_bDrainable              : 1 ; // TRUE if object is available to be drained by a BO. 

    // Zone tracking
    zone_mgr::tracker   m_ZoneTracker;              // Tracks the zones.

    // Logic
    ragdoll             m_Ragdoll ;                 // Ragdoll container
    f32                 m_TimeAlive ;               // Used for time out delete
    f32                 m_NoEnergyTimer;            // Time with no energy
    floor_properties    m_FloorProperties;          // Floor tracking class
    smem_matrix_cache   m_CachedL2Ws;               // Matrix allocation class
    actor_effects*      m_pActorEffects;            // any attached special FX (such as flame particles)

    // Properties (for editor created dead body)
    s32                 m_AnimGroupName ;           // Animation group to use
    s32                 m_AnimName ;                // Animation to use
    s32                 m_AnimFrame ;               // Animation frame to use
    f32                 m_SimulationTime ;          // Time to run ragdoll for pose
    ragdoll::type       m_RagdollType ;             // Type of ragdoll rig to use

	// Blood properties
    rhandle<decal_package>  m_hBloodDecalPackage;
    s32                     m_BloodDecalGroup;

};

//=========================================================================

inline
ragdoll* dead_body::GetRagdollPointer( void )
{
	return& m_Ragdoll;
}

//=========================================================================

inline
void dead_body::SetPermanent( xbool Permanent )
{
    m_bPermanent = Permanent;
}

//=========================================================================
// END
//=========================================================================
#endif
