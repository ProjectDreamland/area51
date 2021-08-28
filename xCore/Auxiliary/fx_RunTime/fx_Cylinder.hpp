//==============================================================================
//
//  fx_Cylinder.hpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct fx_edef_cylinder : public fx_element_def
{
    s32     BitmapIndex;
    xbool   PlanarMap;
    f32     TileU;
    f32     TileV;
    f32     ScrollU;
    f32     ScrollV;
    s32     NSegments;
    f32     SizeTop;
    f32     SizeBottom;
    xcolor  ColorTop;
    xcolor  ColorBottom;
}; 

//==============================================================================

class fx_cylinder : public fx_element
{
//------------------------------------------------------------------------------
public:
    void    Initialize      ( const fx_element_def* pElementDef, f32* pInput );
    void    AdvanceLogic    ( const fx_effect_base* pEffect,     f32  DeltaTime );
    void    Render          ( const fx_effect_base* pEffect ) const;
    void    ApplyColor      ( void );

//------------------------------------------------------------------------------
protected:
    s32         m_NIndexes;
    s32         m_NVerts;
    s32         m_Cone;
    f32         m_OffsetU;
    f32         m_OffsetV;
    vector3*    m_pVertex;
    vector2*    m_pUV;
    xcolor*     m_pColor;
    s16*        m_pIndex;
};

//==============================================================================
