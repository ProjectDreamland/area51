#ifndef PS2_STRIP_HPP
#define PS2_STRIP_HPP
//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"

//==============================================================================
// CLASS
//==============================================================================
class ps2_strip
{
public:
                ps2_strip           ( void );
               ~ps2_strip           ( void );

    void        Begin               ( s32 nVertices, s32 CacheSize );
    void        AddTri              ( s32 I1, s32 I2, s32 I3 );
    void        End                 ( void );

    s32         GetIndexCount       ( void  ) const;
    s32         IsIndexNewStrip     ( s32 I ) const;
    s32         GetIndex            ( s32 I ) const;
    f32         GetFinalScore       ( void ) const;
    s32         GetCacheSize        ( void ) const;

protected:

    xhandle     GetNextTri          ( xhandle hTri, s32 iD1, s32 iD2 );
    void        GetNextDirection    ( xhandle hNext, xhandle hTri, s32& iD1, s32& iD2 );    
    void        UpdateRun           ( void );
    s32         ComputeRunScore     ( xhandle hTri, s32 iD1, s32 iD2 );
    xhandle     GetBestRun          ( s32& iD1, s32& iD2, s32 Count );
    void        RemoveVert          ( s32 iVert, xhandle hTri );
    void        RemoveTri           ( xhandle hTri );
    void        BuildStrip          ( xhandle hTri, s32 iD1, s32 iD2, s32 Count );
    s32         GetLastIndex        ( xhandle hTri, s32 iD1, s32 iD2 );

protected:

    struct triangle
    {
        s32     RunNum;
        s32     iV[3];
    };

    struct vertex
    {
        s32     nRef;
        xhandle hTriangle[32];
    };

    s32                 m_CacheSize;
    s32                 m_RunNum;
    s32                 m_nTriangles;
    s32                 m_nVertices;
    vertex*             m_pVertex;
    xharray<triangle>   m_lTriangle;
    xarray<s32>         m_Index;
};

//==============================================================================
// END
//==============================================================================
#endif