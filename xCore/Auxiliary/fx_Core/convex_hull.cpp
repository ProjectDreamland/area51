#include "entropy.hpp"
#include "convex_hull.hpp"

//============================================================================
// Constructor

convex_hull::convex_hull( )
{
}

convex_hull::~convex_hull()
{
}

//============================================================================

void convex_hull::ClearEdgeStack( void )
{
    m_EdgeStack.SetCount( 0 );
}

//============================================================================

void convex_hull::PushEdge( s32 v0, s32 v1 )
{
    // Note: This is not a simple stack, it has the following logic
    //       IF (v0,v1) is being pushed and (v1,v0) is already in the stack
    //       THEN remove (v1,v0)
    //       ELSE add (v0,v1)
    //
    // For example, if 2 faces have their edges addes to this stack and the faces
    // shared an edge, then that edge will be added and subsequently removed when
    // the second face applies it's edges to the stack (due to winding order).
    // Only the silhouette would be left. This is intended for tracking holes
    // we are punching in the convex hull so we can fill them with new faces later.

    // Loop through all edges in stack
    for( s32 i=0 ; i<m_EdgeStack.GetCount() ; i++ )
    {
        edge& Edge = m_EdgeStack[i];
        ASSERT( !((v0 == Edge.v[0]) && (v1 == Edge.v[1])) );

        // Delete edge if it's reverse is added to the stack and exit
        if( (v0 == Edge.v[1]) && (v1 == Edge.v[0]) )
        {
            m_EdgeStack.Delete( i );
            return;
        }
    }

    // Not found so add to stack
    m_EdgeStack.Append( edge(v0,v1) );
}

//============================================================================

s32 convex_hull::AddVertex( const vector3& Position )
{
    m_Verts.Append( Position );
    return m_Verts.GetCount()-1;
}

//============================================================================

s32 convex_hull::AddRenderEdge( s32 v0, s32 v1 )
{
    ASSERT( v0 >= 0 );
    ASSERT( v0 < m_Verts.GetCount() );
    ASSERT( v1 >= 0 );
    ASSERT( v1 < m_Verts.GetCount() );

    m_RenderEdges.Append( edge(v0,v1) );
    return m_RenderEdges.GetCount()-1;
}

//============================================================================

void convex_hull::ComputeHull( void )
{
    ASSERT( m_Verts.GetCount() >= 3 );

    // Make first 2 faces
    m_Faces.Append( face(m_Verts, 0,1,2) );
    m_Faces.Append( face(m_Verts, 0,2,1) );

    // Iterate while the Changed flag has been set
    xbool Changed = TRUE;
    while( Changed )
    {
        // Start this iteration with the changed flag cleared
        Changed = FALSE;

        // Loop through all verts
        for( s32 i=0 ; i<m_Verts.GetCount() ; i++ )
        {
            s32 j;
            const vector3& Vert = m_Verts[i];

            // Is this vert already contained in the hull?
            xbool Contained = FALSE;
            for( j=0 ; j<m_Faces.GetCount() ; j++ )
            {
                face& Face = m_Faces[j];
                if( (Face.v[0] == i) ||
                    (Face.v[1] == i) ||
                    (Face.v[2] == i) )
                {
                    Contained = TRUE;
                    break;
                }
            }

            // If the vert was contained then no need for further evaluation
            if( Contained )
                continue;

            // Loop through all faces
            j = 0;
            while( j<m_Faces.GetCount() )
            {
                face& Face = m_Faces[j];

                // Vert in back of face?
                if( Face.InBack( Vert ) )
                {
                    // Yes, then add all it's edges to the edge stack and delete the face
                    PushEdge( Face.v[0], Face.v[1] );
                    PushEdge( Face.v[1], Face.v[2] );
                    PushEdge( Face.v[2], Face.v[0] );
                    m_Faces.Delete( j );
                }
                else
                {
                    // No, move onto next face
                    j++;
                }
            }

            // Did we get any disconnected edges?
            if( m_EdgeStack.GetCount() > 0 )
            {
                // Set changed flag to force a new iteration through everything
                Changed = TRUE;

                // Build all the new faces
                for( s32 j=0 ; j<m_EdgeStack.GetCount() ; j++ )
                {
                    edge& Edge = m_EdgeStack[j];
                    m_Faces.Append( face( m_Verts, Edge.v[0], Edge.v[1], i ) );
                }

                // Clear the edge stack
                m_EdgeStack.SetCount(0);
            }
        }
    }
}

//============================================================================

void convex_hull::Render( const matrix4& L2W )
{
    s32             i,j,k;
    xarray<s32>     VisibleFaces;

    // Get camera position in local space
    const view* pView = eng_GetView();
    vector3 ViewPos = pView->GetPosition();
    matrix4 W2L = L2W;
    W2L.InvertSRT();
    ViewPos = W2L.Transform( ViewPos );

    // Determine which faces are visible
    VisibleFaces.SetCapacity( m_Faces.GetCount() );
    for( i=0 ; i<m_Faces.GetCount() ; i++ )
    {
        if( m_Faces[i].p.InBack( ViewPos ) )
            VisibleFaces.Append( i );
    }

    // Begin rendering of triangles
    draw_Begin ( DRAW_TRIANGLES, DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE );
    draw_SetL2W( L2W );
    draw_Color ( xcolor(255,255,255,48) );

    // Render all the faces
    for( i=0 ; i<VisibleFaces.GetCount() ; i++ )
    {
        face& Face = m_Faces[VisibleFaces[i]];

        draw_Vertex( m_Verts[Face.v[0]] );
        draw_Vertex( m_Verts[Face.v[2]] );
        draw_Vertex( m_Verts[Face.v[1]] );
    }
    draw_End();

    // Begin rendering of lines
    draw_Begin( DRAW_LINES, DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE );
    draw_SetL2W( L2W );
    draw_Color( xcolor(128,128,128) );

    // Render all visible edges
    for( i=0 ; i<m_RenderEdges.GetCount() ; i++ )
    {
        xbool Visible = FALSE;
        edge& Edge = m_RenderEdges[i];

        // Is the edge contained in the visible faces of the hull?
        for( j=0 ; (j<VisibleFaces.GetCount()) && !Visible ; j++ )
        {
            // Loop through the face edges testing if this edge is visible
            face& Face = m_Faces[VisibleFaces[j]];
            for( k=0 ; k<3 ; k++ )
            {
                if( ((Face.v[k] == Edge.v[0]) && (Face.v[(k+1)%3] == Edge.v[1])) ||
                    ((Face.v[k] == Edge.v[1]) && (Face.v[(k+1)%3] == Edge.v[0])) )
                {
                    // Found it
                    Visible = TRUE;
                    break;
                }
            }
        }

        // If visible then render it
        if( Visible )
        {
            draw_Vertex( m_Verts[Edge.v[0]] );
            draw_Vertex( m_Verts[Edge.v[1]] );
        }
    }
    draw_End();
}
