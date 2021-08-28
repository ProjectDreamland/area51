#ifndef CONVEX_HULL_HPP
#define CONVEX_HULL_HPP

#include "x_files.hpp"

//============================================================================
//  convex_hull
//============================================================================

class convex_hull
{
protected:

    struct edge
    {
        edge() {}
        edge( s32 v0, s32 v1 ) { v[0] = v0 ; v[1] = v1; }
        s32     v[2];
    };

    struct face
    {
        face() {}
        face( const xarray<vector3>& Verts, s32 v0, s32 v1, s32 v2 )
        {
            v[0] = v0 ; v[1] = v1 ; v[2] = v2;
            p.Setup( Verts[v0], Verts[v1], Verts[v2] );
        }

        xbool InBack( const vector3& Vert )
        {
            return p.InBack( Vert );
        }

        s32     v[3];
        plane   p;
    };

    xarray<vector3>     m_Verts;            // Vertices to build hull
    xarray<edge>        m_EdgeStack;        // Edges of hull
    xarray<face>        m_Faces;            // Faces

    xarray<edge>        m_RenderEdges;      // Edges to render

protected:
    void                ClearEdgeStack          ( void );
    void                PushEdge                ( s32 v0, s32 v1 );

public:
                        convex_hull             ( );
                       ~convex_hull             ( );

    s32                 AddVertex               ( const vector3& Position );
    s32                 AddRenderEdge           ( s32 v0, s32 v1 );

    void                ComputeHull             ( void );

    void                Render                  ( const matrix4& L2W );
};

//============================================================================

#endif
