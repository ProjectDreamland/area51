#ifndef SKININST_HPP
#define SKININST_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Render\SkinGeom.hpp"
#include "Objects\Render\RenderInst.hpp"
#include "Objects\Render\VirtualTextureMask.hpp"

//=========================================================================
// CLASS
//=========================================================================

class skin_inst : public render_inst
{
public:
                        skin_inst           ( void );
                       ~skin_inst           ( void );

    virtual void        OnEnumProp          ( prop_enum&    List );
    virtual xbool       OnProperty          ( prop_query&   I    );
    virtual geom*       GetGeom             ( void ) const;
    virtual const char* GetGeomName         ( void ) const;

    skin_geom*          GetSkinGeom         ( void ) const;
    const char*         GetSkinGeomName     ( void ) const;
    
    virtual void        SetUpSkinGeom       ( const char* fileName);

    void                Render              ( const matrix4* pL2W,
                                              const matrix4* pBone,
                                              s32            nBone,
                                              u32            Flags,
                                              u64            LODMask,
                                              const xcolor&  Ambient = xcolor(64,64,64,255) );

    void                RenderDistortion    ( const matrix4* pL2W,
                                              const matrix4* pBone,
                                              s32            nBone,
                                              u32            Flags,
                                              u64            LODMask,
                                              const radian3& NormalRot,
                                              const xcolor&  Ambient = xcolor(0,0,0,255) );

    void                RenderShadowCast    ( const matrix4* pL2W,
                                              const matrix4* pBone,
                                              s32            nBone,
                                              u32            Flags,
                                              u64            LODMask,
                                              u64            ProjMask );

    void                SetMinAmbient           ( xcolor MinAmbient );                                              
    void                SetOtherAmbientAmount   ( f32 OtherAmbientAmount );                                              
    
    
    const skin_inst& operator = ( const skin_inst& Skin );
    

    void                SetVirtualTexture   ( const char*   pVTextureName,
                                              const char*   pDiffuseTextureDesc );
    void                SetVirtualTexture   (       s32     VTexture );

protected:

    rhandle<skin_geom>          m_hSkinGeom;            // Handle to the Skin Geom
    virtual_texture_mask        m_VTextureMask;         // virtual texture mask for bitmap swapping
    xcolor                      m_MinAmbient;           // ambient color this instance will always receive
    f32                         m_OtherAmbientAmount;   // amount that other ambient sources (such as the floor color or lights) will contribute

#ifdef X_EDITOR
    // these variables are for creating a linked-list of render instances.
    // when we go to refresh newly compiled geometry in the editor, we can
    // have it automagically unregister the old geometry and register
    // the new geometry
    virtual void        UnregiserInst   ( void );
    virtual void        RegisterInst    ( void );
#endif // X_EDITOR
};

//=========================================================================

inline skin_geom* skin_inst::GetSkinGeom( void ) const
{
    return( m_hSkinGeom.GetPointer() );
}

//=========================================================================

inline geom* skin_inst::GetGeom( void ) const
{
    return GetSkinGeom();
}

//=========================================================================

inline const char* skin_inst::GetGeomName( void ) const
{
    return GetSkinGeomName();
}

//=============================================================================

inline
void skin_inst::SetMinAmbient( xcolor MinAmbient )
{
    m_MinAmbient = MinAmbient;
}

//=========================================================================

inline
void skin_inst::SetOtherAmbientAmount( f32 OtherAmbientAmount )
{
    m_OtherAmbientAmount = OtherAmbientAmount;
}

//=========================================================================

#endif


