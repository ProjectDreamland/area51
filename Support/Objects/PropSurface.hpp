#ifndef PROP_SURFACE_HPP
#define PROP_SURFACE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Objects\PlaySurface.hpp"
#include "Debris\debris_mgr.hpp"

//=========================================================================
// DEFINES
//=========================================================================

#define PROP_SURFACE_DATA_VERSION   1000

//=========================================================================
// CLASS
//=========================================================================

class prop_surface : public play_surface
{
public:

    CREATE_RTTI( prop_surface, play_surface, object )

                                prop_surface                ( void );
                                ~prop_surface               ( void );
    virtual bbox                GetLocalBBox                ( void ) const;      
    virtual void                OnEnumProp                  ( prop_enum&    List );
    virtual xbool               OnProperty                  ( prop_query&   I    );
    virtual void                OnRender                    ( void );
    virtual void                OnAdvanceLogic              ( f32 DeltaTime );

    virtual void                OnMove                      ( const vector3& NewPos   );      
    virtual void                OnTransform                 ( const matrix4& L2W      );

    virtual void                OnPain                      ( const pain& Pain ) ;  // Tells object to recieve pain
    virtual void                OnColCheck                  ( void );

            f32                 GetHealth                   ( void )            const;
            void                SetHealth                   ( f32 newHealth );
            xbool               GetOnlyGlassDestructible    ( void )            const;
            void                SetOnlyGlassDestructible    ( xbool onlyGlass );

    virtual const object_desc&  GetTypeDesc                 ( void ) const;
    static  const object_desc&  GetObjectType               ( void );
    virtual void                OnPolyCacheGather           ( void );

#if defined(X_EDITOR)
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

    enum flags
    {
        PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE = 0x0001,
        PROP_FLAG_DESTROYED               = 0x0004,
        PROP_FLAG_BBOX_ONLY               = 0x0008, // for debugging
        PROP_FLAG_CAN_COLLIDE             = 0x0010,
    };

protected:

    rhandle<char>               m_hAudioPackage;
    rhandle<char>               m_hParticleEffect;
    f32                         m_Health;
    f32                         m_Armor;
    debris_mgr::debris_set      m_DebrisSet;
    u32                         m_PropFlags;

    rigid_inst                  m_DebrisInst;
    
    guid                        m_ActivateOnDestruction;
};

//=========================================================================
// INLINE IMPLEMENTATION
//=========================================================================

inline f32 prop_surface::GetHealth ( void ) const
{
    return m_Health;

}

inline void prop_surface::SetHealth ( f32 newHealth )
{
    m_Health = newHealth;
    
}

inline void prop_surface::SetOnlyGlassDestructible( xbool onlyGlass )
{
    m_PropFlags |= onlyGlass ? PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE : 0;
}

inline xbool prop_surface::GetOnlyGlassDestructible( void ) const
{
    return ((m_PropFlags & PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE) == PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE);
}

//=========================================================================
// END
//=========================================================================
#endif