//==============================================================================
//
//  fx_ShockWave.hpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct fx_edef_shockwave : public fx_element_def
{
    s32     BitmapIndex;
    xbool   PlanarMap;
    f32     TileU;
    f32     TileV;
    f32     ScrollU;
    f32     ScrollV;
    f32     CenterV;
    s32     NSegments;
    f32     Width;
    f32     Center;
    xbool   IsFlat;
    xcolor  InnerColor;
    xcolor  OuterColor;
}; 

//==============================================================================

class fx_shockwave : public fx_element
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
    f32         m_OffsetU;
    f32         m_OffsetV;
    vector3*    m_pVertex;
    vector2*    m_pUV;
    xcolor*     m_pColor;
    s16*        m_pIndex;
};

//==============================================================================
