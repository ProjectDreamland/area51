//==============================================================================
//
//  fx_Sphere.hpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct fx_edef_sphere : public fx_element_def
{
    s32     BitmapIndex;
    xbool   PlanarMap;  
    f32     TileU;      
    f32     TileV;      
    f32     ScrollU;    
    f32     ScrollV;    
    s32     NSegmentsU;
    s32     NSegmentsV;
    f32     Top;
    f32     Bottom;
    xcolor  ColorTop;
    xcolor  ColorBottom;
}; 

//==============================================================================

class fx_sphere : public fx_element
{
//------------------------------------------------------------------------------
public:
    void    Initialize      ( const fx_element_def* pElementDef, f32* pInput );
    void    AdvanceLogic    ( const fx_effect_base* pEffect,     f32  DeltaTime );
    void    Render          ( const fx_effect_base* pEffect ) const;
    void    ApplyColor      ( void );

//------------------------------------------------------------------------------
protected:
    s32         m_NSIndexes;
    s32         m_NTIndexes;
    s32         m_NVerts;
    f32         m_OffsetU;
    f32         m_OffsetV;
    xbool       m_CapTop;
    xbool       m_CapBottom;
    vector3*    m_pVertex;
    vector2*    m_pUV;
    xcolor*     m_pColor;
    s16*        m_pSIndex;
    s16*        m_pTIndex;
};

//==============================================================================
