//==========================================================================
//
//  SuperDestructible.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  Super Destructible - has it all -
//
//==========================================================================
#ifndef __SUPERDESTRUCTIBLE__ 
#define __SUPERDESTRUCTIBLE__ 

//==------------------------------------------------------------------------
// Includes
//==------------------------------------------------------------------------
#include "Objects\PlaySurface.hpp"
#include "Debris\debris_mgr.hpp"
#include "Decals\DecalPackage.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "Objects\Render\VirtualMeshMask.hpp"

//==------------------------------------------------------------------------
// Globals
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Prototypes
//==------------------------------------------------------------------------

// Class super_destructible_object
class super_destructible_obj : public play_surface
{

public:
    CREATE_RTTI( super_destructible_obj, play_surface, object )

                            super_destructible_obj      ( void );
                            ~super_destructible_obj     ( void );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );

    virtual void            OnEnumProp                  ( prop_enum&    List        );
    virtual xbool           OnProperty                  ( prop_query&   I           );
    virtual void            OnPain                      ( const pain& Pain          ); 
    virtual void            OnRender                    ( void                      );
    virtual void            OnMove                      ( const vector3& NewPos     );      
    virtual void            OnTransform                 ( const matrix4& L2W        );
    virtual void            OnAdvanceLogic              ( f32 DeltaTime             );
    virtual void            OnColCheck                  ( void );
    virtual void            OnColNotify                 ( object& Object );
    virtual void            OnPolyCacheGather           ( void );
    
    virtual f32             GetHealth                   ( void );
            f32             GetMaxHealth                ( void );
            void            SetInitHealth               ( f32 Health );
            
#ifdef X_EDITOR
    virtual s32             OnValidateProperties        ( xstring& ErrorMsg );
#endif

#ifndef X_RETAIL
    virtual void            OnDebugRender               ( void );
#endif // X_RETAIL

    virtual anim_group::handle* GetAnimGroupHandlePtr   ( void );

    const matrix4*          GetBoneL2Ws                 ( void );
    virtual bbox            GetLocalBBox                ( void ) const;      
            s32             GetNumStages                ( void ) { return( m_Stages.GetCount() ); }

            void            BroadcastPain               ( s32 iUsePainFromThisStage );
            void            KillStage                   ( const pain& Pain );
            void            SetCurrentStage             ( s32 NewCurrentStage, xbool bGiveFullHealth );
            void            KillCurrentStage            ( void );
            
            void            CreateSuperPainResponse     ( guid               OwnerGuid,
                                                          const char*        pSoundName, 
                                                          const char*        pFXName,
                                                          f32                Scale,
                                                          const vector3&     Pos,
                                                          const radian3&     Rot,
                                                          u16                Zone );


            void            UpdateZoneTrack             ( void );

            // Stage functions
            void            CreateStage                 ( s32 iRefStage = -1, xbool bAfterRefStage = TRUE );
            void            DeleteStage                 ( s32 Index ) ;
            void            Regenerate                  ( f32 DeltaTime, f32 TotalTime );
            
            xbool           IsDestroyed                 ( void )    { return m_Destroyed; }
                                 
            void            SetParentGuid               ( guid ParentGuid ) { m_ParentGuid = ParentGuid; }                                                       
    virtual guid            GetParentGuid               ( void )            { return m_ParentGuid; }
                                                        
protected:                                              
    anim_group::handle      m_hAnimGroup;
    simple_anim_player      m_AnimPlayer;
    rhandle<char>           m_hAudioPackage;
    rhandle<decal_package>  m_hDecalPackage;
    zone_mgr::tracker       m_ZoneTracker;
    guid                    m_IgnorePainGuid;
    guid                    m_ParentGuid;           // Owner of superdestructible
    s32                     m_DecalGroup;
    f32                     m_DestructionTime;
    s32                     m_CurrentStage;
    s32                     m_AnimIndex;

    // bitfields //////////////////////////////////

    u32                     m_Destroyed         :1;
    u32                     m_AcceptActorPain   :1;
    u32                     m_AcceptObjectPain  :1;
    u32                     m_Invulnerable      :1;
    u32                     m_KillCurrentStage  :1;


//==-------------------------------------------------------------------
    // STRUCTURES
//==-------------------------------------------------------------------
protected:
    // stage
    struct stage
    {
        // Data
        rigid_inst          m_DebrisInst;
        rigid_inst          m_SpecificDebrisInst;
        f32                 m_DebrisCount;
        f32                 m_MinDebrisVelocity;
        f32                 m_MaxDebrisVelocity;
        f32                 m_DebrisLife;
        f32                 m_SpawnWidth;
        f32                 m_SpawnHeight;
        s32                 m_SoundID;
        rhandle<char>       m_hParticleEffect;
        rhandle<char>       m_hPainParticleEffect;
        f32                 m_ParticleScale;
        f32                 m_PainParticleScaleMin;
        f32                 m_PainParticleScaleMax;
        s32                 m_PainSoundID;
        f32                 m_PainRadius;
        f32                 m_PainAmount;
        char                m_PainType[64];
        f32                 m_PainForce;
        f32                 m_Health;
        f32                 m_MaxHealth;
        xbool               m_GlassOnly;
        guid                m_ActivateOnDestruction;
        virtual_mesh_mask   m_VMeshMask;
        virtual_mesh_mask   m_SingleCreateVMeshMask;

        s32                 m_AnimIndex;

        // Functions
        
        // Constructor
        stage();

        // Enumerates properties               
        void    OnEnumProp  ( prop_enum& List, super_destructible_obj& SuperDestructibleObj, s32 iStage );

        // Evaluates properties
        xbool   OnProperty  ( prop_query& I, super_destructible_obj& SuperDestructibleObj, s32 iStage );

        xbool   GetGlassOnly ( void ) { return m_GlassOnly; }
    } ;

    xarray<stage>               m_Stages;


//==-------------------------------------------------------------------
// FRIENDS
//==-------------------------------------------------------------------
friend struct stage;

};


#endif // SUPERDESTRUCTIBLE
