#ifndef PROJECTOR_HPP
#define PROJECTOR_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Render\Texture.hpp"

//=========================================================================
// CLASS
//=========================================================================
class projector_obj : public object
{
public:

    CREATE_RTTI( projector_obj, object, object )
    
                            projector_obj   ( void );
    virtual                ~projector_obj   ( void );
    virtual bbox            GetLocalBBox    ( void ) const;
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );

    xbool                   IsDynamic           ( void ) const;
    xbool                   IsShadow            ( void ) const;
    xbool                   IsActive            ( void ) const;
    xbool                   IsFlashlight        ( void ) const;
    radian                  GetFOV              ( void ) const;
    f32                     GetLength           ( void ) const;
    const texture::handle&  GetTexture          ( void ) const;

    void                    SetShadow           ( xbool IsShadow );
    void                    SetActive           ( xbool IsActive );
    void                    SetIsFlashlight     ( xbool Flashlight );
    void                    SetFOV              ( radian FOV );
    void                    SetLength           ( f32 Length );
    void                    SetTextureHandle    ( texture::handle Texture );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

protected:

    virtual void            OnRender        ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender   ( void );
#endif // X_RETAIL

    xbool                   m_bIsDynamic;
    xbool                   m_bIsShadow;
    xbool                   m_bIsActive;
    xbool                   m_bIsFlashlight;
    radian                  m_FOV;
    f32                     m_Length;
    texture::handle         m_hTexture;     // Handle to the texture
    
// Make friends here
};

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================

inline xbool projector_obj::IsDynamic( void ) const
{
    return m_bIsDynamic;
}

//=========================================================================

inline xbool projector_obj::IsShadow( void ) const
{
    return m_bIsShadow;
}

//=========================================================================

inline xbool projector_obj::IsActive( void ) const
{
    return m_bIsActive;
}

//=========================================================================

inline xbool projector_obj::IsFlashlight( void ) const
{
    return m_bIsFlashlight;
}

//=========================================================================

inline radian projector_obj::GetFOV( void ) const
{
    return m_FOV;
}

//=========================================================================

inline f32 projector_obj::GetLength( void ) const
{
    return m_Length;
}

//=========================================================================

inline const texture::handle& projector_obj::GetTexture( void ) const
{
    return m_hTexture;
}

//=========================================================================

inline void projector_obj::SetShadow( xbool IsShadow )
{
    m_bIsShadow = IsShadow;
}

//=========================================================================

inline void projector_obj::SetActive( xbool IsActive )
{
    m_bIsActive = IsActive;
}

//=========================================================================

inline void projector_obj::SetIsFlashlight( xbool Flashlight )
{
    m_bIsFlashlight = Flashlight;
}

//=========================================================================

inline void projector_obj::SetFOV( radian FOV )
{
    m_FOV = FOV;
}

//=========================================================================

inline void projector_obj::SetLength( f32 Length )
{
    m_Length = Length;
}

//=========================================================================

inline void projector_obj::SetTextureHandle( texture::handle Texture )
{
    m_hTexture = Texture;
}

//=========================================================================
// END
//=========================================================================
#endif

