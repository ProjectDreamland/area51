#ifndef GLASS_SURFACE_HPP
#define GLASS_SURFACE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Objects\PlaySurface.hpp"
#include "Debris\debris_mgr.hpp"

//=============================================================================
// TYPE
//=============================================================================
struct glass_surface_desc : public object_desc
{
    CREATE_RTTI( glass_surface_desc, object_desc, object_desc )

    //-------------------------------------------------------------------------

    glass_surface_desc( void ) : object_desc( 

            object::TYPE_GLASS_SURFACE, 
            "Glass Surface", 
            "PROPS",

            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS      | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_PAIN_LOS        | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_DAMAGEABLE, 

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING  ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void );

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }
};

//=============================================================================
// INSTANCE
//=============================================================================
class glass_surface : public play_surface
{
public:

    CREATE_RTTI( glass_surface, play_surface, object )

                            glass_surface    ( void );
                            ~glass_surface   ( void );
    virtual     bbox        GetLocalBBox     ( void ) const;      
    virtual void            OnColCheck       ( void );
    virtual void            OnEnumProp       ( prop_enum&    List );
    virtual xbool           OnProperty       ( prop_query&   I    );

    virtual void            OnPain          ( const pain& Pain ) ;  // Tells object to recieve pain
    virtual f32             GetHealth       ( void );
    virtual void            SetHealth       ( f32 newHealth );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

protected:

    rhandle<char>               m_hAudioPackage;
    rhandle<char>               m_hParticleEffect;
    f32                         m_Health;
    f32                         m_Armor;
    debris_mgr::debris_set      m_DebrisSet;
    xbool                       m_Destroyed;

    rigid_inst                  m_DebrisInst;

    static glass_surface_desc   s_GlassSurface_Desc;
};

//=========================================================================
// INLINE IMPLEMENTATION
//=========================================================================

inline f32   glass_surface::GetHealth ( void )
{
    return m_Health;
}

inline void  glass_surface::SetHealth ( f32 newHealth )
{
    m_Health = newHealth;   
}

//=========================================================================
// END
//=========================================================================
#endif