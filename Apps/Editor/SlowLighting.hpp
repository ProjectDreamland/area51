
#ifndef SLOW_LIGHTING_HPP
#define SLOW_LIGHTING_HPP

//=========================================================================

#include "x_files.hpp"

//=========================================================================

class slow_lighting
{
public:
            slow_lighting     ( void );
    void    Clear             ( void );

    void    AddBegin          ( void );
    void    AddTriangle       ( vector3* pPos, vector3* pNorm, guid Guid, u32 UserData );
    void    AddLight          ( const vector3& Pos, f32 AttenR, xcolor Color, xcolor AmbLight );
    void    AddEnd            ( void );

//    s32     GetTriangleCount  ( void );
//    void    LightNextTriangle ( void );

protected:

    struct light
    {
        vector3     Pos;
        f32         AttenR;
        xcolor      Color;
        bbox        BBox;
        xcolor      AmbLight;
    };

    struct vert
    {
        vector3     Normal;
        vector3     Position;
        vector3     C1;
        vector3     C2;
        f32         W;
        xcolor      FinalColor;
    };

    struct tri
    {
        xhandle     hVertex[3];
        guid        gObject;
        u32         uUserData;
    };

    struct quickbox
    {
        bbox                BBox;
        xarray<xhandle>     hTri;
    };

protected:

    void    ComputeSpatialDBase     ( void );
    void    CollapseVerts           ( void );
    xbool   CanSeeLight             ( const light& Light, const vector3& P1 ) const;
    xbool   TempVCompare            ( const vert& A, const vert& B ) const;

    void    LightingBegin           ( void );
    void    LightingTriangle        ( s32 i );
    void    LightingEnd             ( void );

protected:

    xharray<vert>       m_lVert;
    xharray<tri>        m_lTri;
    xharray<light>      m_lLight;
    xharray<quickbox>   m_lQuickBox;
    s32                 m_CurrentIndex;
    xbool               m_bAdding;
    bbox                m_ZoneBBox;
};

//=========================================================================
// END
//=========================================================================
#endif