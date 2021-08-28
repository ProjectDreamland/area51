#ifndef PORTAL_OBJECT_HPP
#define PORTAL_OBJECT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#define MAX_PORTAL_NAME 32

//=========================================================================
// CLASS
//=========================================================================
class zone_portal : public object
{
public:

    CREATE_RTTI( zone_portal, object, object )
    
                            zone_portal     ( void );
    virtual bbox            GetLocalBBox    ( void ) const;
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    
    u8                      GetZone1        ( void ) { return m_Zone1; }
    u8                      GetZone2        ( void ) { return m_Zone2; }
    void                    SetZone1        ( u8 Zone ) { m_Zone1 = Zone; }
    void                    SetZone2        ( u8 Zone ) { m_Zone2 = Zone; }
    f32                     GetWidth        ( void ) const { return m_Width; } 
    f32                     GetHeight       ( void ) const { return m_Height; }

    xbool                   IsPortalValid   ( void );
    
    void                    SetSoundOcclusion   ( f32 SoundOcclusion )  { m_SoundOcclusion = SoundOcclusion; }
    f32                     GetSoundOcclusion   ( void )                { return m_SoundOcclusion; }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

#ifndef X_RETAIL
    void                    RenderVolume        ( void );
#endif // X_RETAIL

protected:

#if !defined( CONFIG_RETAIL )
    virtual void            OnRender            ( void );
    virtual void            OnRenderTransparent ( void );
#endif // !defined( CONFIG_RETAIL )

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

    f32                     m_Width;
    f32                     m_Height;
    u8                      m_Zone1;
    u8                      m_Zone2;
    f32                     m_SoundOcclusion;
};

//=========================================================================
// END
//=========================================================================
#endif

