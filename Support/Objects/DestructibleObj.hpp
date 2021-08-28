#ifndef DESTRUCTIBLE_OBJ_HPP
#define DESTRUCTIBLE_OBJ_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Objects\PlaySurface.hpp"
#include "Debris\debris_mgr.hpp"
#include "Decals\DecalPackage.hpp"

//=============================================================================
// INSTANCE
//=============================================================================
class destructible_obj : public play_surface
{
public:

    CREATE_RTTI( destructible_obj, play_surface, object )

                            destructible_obj    ( void );
                            ~destructible_obj   ( void );
    virtual     bbox        GetLocalBBox        ( void ) const;      

    virtual void            OnEnumProp          ( prop_enum&    List );
    virtual xbool           OnProperty          ( prop_query&   I    );

    virtual void            OnPain              ( const pain& Pain ) ;  // Tells object to recieve pain
    virtual f32             GetHealth           ( void );
    virtual void            SetHealth           ( f32 newHealth );
    virtual void            OnAdvanceLogic      ( f32 DeltaTime );
    virtual void            OnRender            ( void );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual xbool           IsDestroyed         ( void );

protected:
    virtual void            SetDestroyed        ( void );
    virtual void            OnColCheck          ( void );
    virtual void            OnColNotify         ( object& Object );    

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

            void            BroadcastPain       ( void );
    
    rigid_inst                  m_DebrisInst;
    rigid_inst                  m_SpecificDebrisInst;
    rhandle<char>               m_hAudioPackage;
    rhandle<char>               m_hParticleEffect;
    rhandle<char>               m_hPainParticleEffect;
    rhandle<decal_package>      m_hDecalPackage;
    s32                         m_DecalGroup;
    f32                         m_DestructionTime;
    f32                         m_ParticleScale;
    f32                         m_PainParticleScaleMin;
    f32                         m_PainParticleScaleMax;
    f32                         m_Health;
    f32                         m_Armor;
    f32                         m_PainRadius;
    f32                         m_PainAmount;
    f32                         m_DebrisCount;
    f32                         m_MinDebrisVelocity;
    f32                         m_MaxDebrisVelocity;
    f32                         m_DebrisLife;
    xbool                       m_Destroyed;
    guid                        m_ActivateOnDestruction;
    s32                         m_SoundID;  
    s32                         m_PainSoundID;
};

//=========================================================================
// INLINE IMPLEMENTATION
//=========================================================================

inline f32   destructible_obj::GetHealth ( void )
{
    return m_Health;
}

inline void  destructible_obj::SetHealth ( f32 newHealth )
{
    m_Health = newHealth;   
}

//=========================================================================
// END
//=========================================================================
#endif