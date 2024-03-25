///////////////////////////////////////////////////////////////////////////////
//
//  particle_emitter.hpp
//
//
///////////////////////////////////////////////////////////////////////////////


#ifndef _PARTICLE_EMITTER_HPP
#define _PARTICLE_EMITTER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\Obj_mgr.hpp"
#include "..\auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "..\Auxiliary\Miscutils\Dictionary.hpp"
#include "MiscUtils\SimpleUtils.hpp"

//=========================================================================
// CLASS
//=========================================================================

struct fx_handle_wrapper
{
    //=========================================================================
    
    fx_handle_wrapper( void ) {}
    
    //=========================================================================
    
    fx_handle_wrapper( const char*  pFXName )
    {
        Init ( pFXName );
    }
    
    //=========================================================================
    
    ~fx_handle_wrapper      ( void )
    {
        Unload();
    }
    
    //=========================================================================
    
    void        Init        ( const char*  pFXName )
    {
        ASSERT( pFXName );
        
        Unload();
        
        m_Resource.SetName( pFXName );
        
#ifndef X_RETAIL        
        x_strncpy( m_ParticleName, pFXName, 64 );
#endif
        
        if (m_Resource.GetPointer())
        {
            m_Fx.InitInstance(  m_Resource.GetPointer() );
        }
    }
    
    //=========================================================================

    const char* GetName( void )
    {
        return m_Resource.GetName();
    }
    
    //=========================================================================
    
    void        Unload      ( void )
    {
        m_Fx.KillInstance();
        m_Resource.SetName("NULL");
    }
    
    //=========================================================================
    
    xbool       IsValid     ( void )
    {
        if (m_Resource.GetPointer() == NULL)
            return FALSE;
        
        return m_Fx.Validate();
    }
    
    //=========================================================================
    
    rhandle<char>   m_Resource;
    fx_handle       m_Fx;
    
#ifndef X_RETAIL        
    char            m_ParticleName[64];     // For debug only...
#endif    
};

//=========================================================================
// PARTICLE_EMITTER
//=========================================================================

class particle_emitter : public object
{
public:
  
    enum    particle_type               // THIS IS A HACK!!!!, NEEDS TO GET CLEANED UP ASAP!!!!!!!
    {
        // Special types
        INVALID_PARTICLE            = -1,       // Undefined
        UNINITIALIZED_PARTICLE      = -2,       // Particle type is chosen based on pain etc
        GENERIC_DYNAMIC_PARTICLE    = -3,       // .fxo name is passed into create function
        
        // START OF PRESET LIST MUST BE ZERO!!!
        PRESET_START = 0,
        
        HARD_SPARK = PRESET_START,
        SOFT_SPARK,
        ENERGY_SPARK,
        DIRT_PUFF,

        GRENADE_EXPLOSION,

        JUMPING_BEAN_TRAIL,
        JUMPING_BEAN_EXPLOSION,

        GRAV_GRENADE_TRAIL,
        GRAV_GRENADE_EXPLOSION,

        MSN_SECONDARY_TRAIL,
        MSN_SECONDARY_EXPLOSION,
        MSN_PROJ_EXPLOSION,

        IMPACT_FLESH_POP,
        IMPACT_FLESH_CLOUD,
        IMPACT_FLESH_HIT,
        IMPACT_FABRIC_HIT,
        
        // MUST BE AT END!!!
        PRESET_COUNT
    };
  
    enum load_states
    {
        INVALID_LOAD_STATE = -1,
            
        UNLOADED,
        LOADED,
        LOAD_FAIL,

        LOAD_STATES_END
    };

    CREATE_RTTI( particle_emitter, object, object )
                             particle_emitter                       ( void );
                            ~particle_emitter                       ( void );
                            
    virtual         bbox    GetLocalBBox                            ( void ) const ;
    virtual         s32     GetMaterial                             ( void ) const { return MAT_TYPE_NULL; }
           
    virtual         void    OnEnumProp                              ( prop_enum& rList );
    virtual         xbool   OnProperty                              ( prop_query& rPropQuery );
    
    virtual         void    OnMove                                  ( const vector3& NewPos   );
    virtual         void    OnTransform                             ( const matrix4& L2W      ); 
    virtual         void    OnColCheck                              ( void );
    virtual         void    OnActivate                              ( xbool Flag );    

                    void    Restart                                 ( void );
                    void    InitParticleFromName                    ( const char* pName );
                    void    DestroyParticle                         ( void );
                    void    StopParticle                            ( void );

                    void    SetScale                                ( f32 NewScale );
                    f32     GetScale                                ( void ) { return m_Scale; }
                    xbool   IsDestroyed                             ( void );
                    xbool   HasPlayedOnce                           ( void ) { return m_PlayedOnce; }
                    void    SetColor                                ( xcolor Color );

    //=========================================================================
    // Interface for creating particles emitter objects at runtime and utility functions...

    static          guid    CreateGenericParticle                   ( const char* rParticlePath, u16 Zone1 = 0 );
    static          void    ProcessCollisionSfx                     ( s32 MaterialType, const vector3& Positon, u16 ZoneId, guid OwnerGuid );
    static          guid    CreatePresetParticle                    ( particle_type ParticleType, u16 Zone1 = 0 );
    static          void    CreateProjectileCollisionEffect         ( const collision_mgr::collision& rCollision, guid OwnerGuid );

    static          void    CreateOnPainEffect                      ( const pain&   Pain,            
                                                                      f32           DisplaceAmount,
                                                                      particle_type ParticleType,
                                                                      xcolor        Color );
    
    static          guid    CreatePresetParticleAndOrient           ( particle_type ParticleType, const vector3& rDir, const vector3& rPos, u16 Zone1 = 0  );
    static          guid    CreatePresetParticleAndOrient           ( const char* rParticlePath, const vector3& rDir, const vector3& rPos, u16 Zone1 = 0  );    
    static          guid    CreatePersistantParticle                ( particle_type ParticleType, u16 Zone1 = 0  );
    static          guid    CreatePersistantParticle                ( const char* pFXName, u16 Zone1 = 0  );
 
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

public:
    
    enum    particle_logic_type
    {
        INVALID_EMISSION_TYPE = -1,
        PLAY_ONCE,
        PLAY_REPEATED,
        PLAY_FOREVER,
        PLAY_PERSISTANT,
        PLAY_END
    };
    
    enum    particle_endroutine_type
    {
        INVALID_END_TYPE = -1,
        END_DESTROY,
        END_DEACTIVATE,
        END_END
    };

    enum    particle_visibility_type
    {
        INVALID_VIS_TYPE = -1,
        VIS_ALL,
        VIS_HUMAN,
        VIS_MUTANT,
        VIS_END
    };

public:

    static          guid        CreateParticleEmitter   ( particle_type ParticleType, particle_logic_type LogicType );
    static          guid        CreateParticleEmitter   ( const char* pFXName, particle_logic_type LogicType );
    
                    void        SetParticleType         ( particle_type ParticleType    );
                    void        SetParticleType         ( const char* pFXName );
                    void        SetLogicType            ( particle_logic_type LogicType );
                    void        ResizeParticleEffect    ( void );

protected:

    virtual         void        OnInit                  ( void );
    virtual         void        OnRender                ( void );
    virtual         void        OnRenderTransparent     ( void );
    virtual         void        OnAdvanceLogic          ( f32 DeltaTime );    

#ifndef X_RETAIL
    virtual         void        OnDebugRender           ( void );    
#endif // X_RETAIL

                    void        ComputeLocalBBox        ( const matrix4& L2W );
                    void        UnloadCurrentEffect     ( void );

                    void        InitGenricParticle      ( const char* rParticlePath  );
                    void        InitPresetParticle      ( particle_type ParticleType );
    
                    void        PlayOnceLogic           ( f32 DeltaTime );
                    void        PlayRepeatedLogic       ( f32 DeltaTime );
                    void        PlayForeverLogic        ( f32 DeltaTime );
                    void        PlayPresistantLogic     ( f32 DeltaTime );
     
public:

    particle_type           m_Type;                     // This is the type of the particle

protected:

    particle_logic_type     m_LogicType;                // This is the logic associated with the particle
    particle_endroutine_type m_EndRoutine;                     
    particle_visibility_type m_Visibility;                     
    fx_handle_wrapper       m_FxHandle;                 // The handle of the effect, used to access the effect in the FXMngr
    xcolor                  m_Color;                    // Global color of FX (defaults to white)
    xbool                   m_PlayedOnce;               // Boolean used for PLAY_ONCE
    f32                     m_RandomWaitMin;            // Minimum random time to wait, used by both PLAY_REPEATED and PLAY_FOREVER
    f32                     m_RandomWaitMax;            // Maximum random time to wait, used by both PLAY_REPEATED and PLAY_FOREVER
    f32                     m_RandomWait;               // Current amount of time to wait until the next effect is played
    s32                     m_RepeatTimes;              // Number of times to play the effect, only valid for  PLAY_REPEATED
    f32                     m_Scale;                    // Uniform scaling value
    xbool                   m_OnActivate;               // On activate flag turns the object on or off
    rhandle<char>           m_hAudioPackage;
    f32                     m_EmiterArea;               // Area of which we will start to render this emiter ( growth to player bbox )

#ifdef X_EDITOR
    med_string              m_FxSourceFile;             // String to store name in Editor mode..
#endif // X_EDITOR
    
    bbox                    m_ParticleBBox;             // BBox of entire particle effect.
    f32                     m_TimeSinceLastRender;      // Timer for turning off onadvance logic.
    xbool                   m_bDestroyed;               // Are we in the process of destroying the particle fx.

protected:
      
    typedef enum_pair<particle_logic_type>              logic_pair;
    typedef enum_table<particle_logic_type>             logic_table;

    typedef enum_pair<particle_endroutine_type>         endroutine_pair;
    typedef enum_table<particle_endroutine_type>        endroutine_table;

    typedef enum_pair<particle_visibility_type>         visibility_pair;
    typedef enum_table<particle_visibility_type>        visibility_table;

protected:

    static logic_pair           s_LogicPairTable[];
    static logic_table          s_LogicEnumTable;
    
    static const char*          s_PresetPathTable[];

    static endroutine_pair      s_EndRoutinePairTable[];
    static endroutine_table     s_EndRoutineEnumTable;

    static visibility_pair      s_VisibilityPairTable[];
    static visibility_table     s_VisibilityEnumTable;
};

//==========================================================================
// INLINE FUNCTIONS
//==========================================================================

inline
void particle_emitter::SetColor( xcolor Color )
{
    m_Color = Color;
}

//==========================================================================
// END
//=========================================================================
#endif

