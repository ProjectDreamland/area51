#ifndef RIGIDINST_HPP
#define RIGIDINST_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Render\RigidGeom.hpp"
#include "Objects\Render\RenderInst.hpp"

//=========================================================================
// CLASS
//=========================================================================

class rigid_inst : public render_inst
{
public:
                        rigid_inst          ( void );
                       ~rigid_inst          ( void );

    virtual void        OnEnumProp          ( prop_enum&    List );
    virtual xbool       OnProperty          ( prop_query&   I    );

			xbool		SetUpRigidGeom		( const char* pFileName );
    
    virtual geom*       GetGeom             ( void     ) const;
    virtual const char* GetGeomName         ( void     ) const;

    rigid_geom*         GetRigidGeom        ( void     ) const;
    s32                 GetNumColors        ( void     ) const;
    const void*         GetColorTable       ( void     ) const;
    const void*         GetColorTable       ( platform ) const;
    const char*         GetRigidGeomName    ( void     ) const;

    void                SetColorTable       ( const void* pColorTable, s32 iColor, s32 nColors );
    virtual void        LoadColorTable      ( const char* pFileName );

    void                Render              ( const matrix4* pL2W, u32 Flags );
    void                Render              ( const matrix4* pL2W, u32 Flags, u64 Mask );
    void                Render              ( const matrix4* pL2W, u32 Flags, u64 Mask, u8 Alpha );
    void                Render              ( const matrix4* pL2W, u32 Flags, u32 VTextureMask, s32 Alpha = 255 );

protected:

    rhandle<rigid_geom>         m_hRigidGeom;       // Handle to the Rigid Geom
    const void*                 m_pRigidColor;      // Ptr to colors

    s32                         m_nColors;          // Number of colours used by instance
    s32                         m_iColor;           // Index into colour table


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

inline rigid_geom* rigid_inst::GetRigidGeom( void ) const
{
    return( m_hRigidGeom.GetPointer() );
}

//=============================================================================

inline geom* rigid_inst::GetGeom( void ) const
{
    return GetRigidGeom();
}

//=========================================================================

inline const char* rigid_inst::GetGeomName( void ) const
{
    return GetRigidGeomName();
}

//=========================================================================

#endif

