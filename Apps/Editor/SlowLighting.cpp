
#include "SlowLighting.hpp"

//=========================================================================

slow_lighting::slow_lighting( void )
{
    Clear();
}

//=========================================================================

void slow_lighting::Clear( void )
{
    m_lVert.Clear();
    m_lTri.Clear();
    m_lLight.Clear();
    m_lQuickBox.Clear();
    m_ZoneBBox.Clear();

    m_CurrentIndex = 0;
    m_bAdding      = FALSE;
}

//=========================================================================

void slow_lighting::AddBegin( void )
{
    ASSERT( m_bAdding == FALSE );
    m_bAdding = TRUE;
}

//=========================================================================

void slow_lighting::AddEnd( void )
{
    ASSERT( m_bAdding == TRUE );
    m_bAdding = FALSE;

    CollapseVerts();
    ComputeSpatialDBase();
}

//=========================================================================

void slow_lighting::AddTriangle( vector3* pPos, vector3* Normal, guid Guid, u32 UserData )
{
    ASSERT( m_bAdding );

    s32   i;
    tri&    Tri      = m_lTri.Add();
    vert*   pVert[3] = { &m_lVert.Add(Tri.hVertex[0]), &m_lVert.Add(Tri.hVertex[1]), &m_lVert.Add(Tri.hVertex[2]) };
    
    //
    // Setup the triangle
    //
    Tri.gObject   = Guid;
    Tri.uUserData = UserData;

    //
    // Create the 3 verts
    //
    for( i=0; i<3; i++ )
    {
        pVert[i]->Position    = pPos[i];
        pVert[i]->Normal      = Normal[i];
        pVert[i]->C1.Zero();
        pVert[i]->C2.Zero();
        pVert[i]->W = 0;

        // Set the zone size
        m_ZoneBBox.Intersect( pPos[i] );
    }
}

//=========================================================================

xbool slow_lighting::TempVCompare( const vert& A, const vert& B ) const
{
    vector3 T;
    f32     d;

    // Check positions
    static const f32 PEpsilon = 0.001f; 
    T = A.Position - B.Position;
    d = T.Dot( T );
    if( d > PEpsilon ) return FALSE;

    // Check normals
    static const f32 NEpsilon = 0.001f;
    T = B.Normal - A.Normal;
    d = T.Dot( T );
    if( d > NEpsilon ) return FALSE;

    return TRUE;
}

//=========================================================================

void slow_lighting::CollapseVerts( void )
{
    struct hash
    {
        s32 iVRemap;
        s32 iNext;
    };

    struct tempv
    {
        xhandle hRemapHandle;   // Which is the handle for this vertex       
        xhandle hHandle;        // Handle to the original Vertex
        s32     iNext;          // next node in the hash 
    };

    s32         i;
    hash*       pHash     = NULL;
    tempv*      pTempV    = NULL;
    s32         HashSize  = MAX( 20, m_lVert.GetCount()*10 );
    f32         MaxX, MinX, Shift;

    // begin the error block
    x_try;

    // Allocate memory
    pHash   = new hash  [ HashSize ];
    pTempV  = new tempv [ m_lVert.GetCount() ];

    if( pTempV == NULL || pHash == NULL )
        x_throw( "Out of memory" );

    // Initialize the hash with terminators
    for( i=0; i<HashSize; i++) 
    {
        pHash[i].iNext = -1;
    }

    // Fill the nodes for each of the dimensions
    MaxX = m_lVert[0].Position.GetX();
    MinX = MaxX;
    for( i=0; i<m_lVert.GetCount(); i++) 
    {
        pTempV[i].hHandle       =  m_lVert.GetHandleByIndex(i);
        pTempV[i].iNext         = -1;
        pTempV[i].hRemapHandle  =  pTempV[i].hHandle;
       
        MaxX = MAX( MaxX, m_lVert[i].Position.GetX() );
        MinX = MIN( MinX, m_lVert[i].Position.GetX() );
    }

    // Hash all the vertices into the hash table
    Shift = HashSize/(MaxX-MinX+1);
    for( i=0; i<m_lVert.GetCount(); i++) 
    {
        s32 OffSet = (s32)(( m_lVert[i].Position.GetX() - MinX ) * Shift);

        ASSERT(OffSet >= 0 );
        ASSERT(OffSet < HashSize );

        pTempV[i].iNext  = pHash[ OffSet ].iNext;
        pHash[ OffSet ].iNext = i;
    }

    // Now do a seach for each vertex
    for( i=0; i<HashSize; i++ )
    {
        for( s32 k = pHash[i].iNext;  k != -1; k = pTempV[k].iNext )
        {
            s32 j;

            // This vertex has been remap
            if( pTempV[k].hRemapHandle != pTempV[k].hHandle )
                continue;

            // Seach in the current hash 
            for( j = pTempV[k].iNext; j != -1; j = pTempV[j].iNext )
            {                
                // This vertex has been remap
                if( pTempV[j].hRemapHandle != pTempV[j].hHandle )
                    continue;

                // If both vertices are close then remap vertex
                if( TempVCompare( m_lVert[ k ], m_lVert[ j ] ))
                    pTempV[j].hRemapHandle    = m_lVert.GetHandleByIndex( k );
            }

            // Searchin the hash on the left
            if( (i+1)< HashSize )
            {
                for( j = pHash[i+1].iNext; j != -1; j = pTempV[j].iNext )
                {                
                    // This vertex has been remap
                    if( pTempV[j].hRemapHandle != pTempV[j].hHandle )
                        continue;

                    // If both vertices are close then remap vertex
                    if( TempVCompare( m_lVert[ k ], m_lVert[ j ] ))
                        pTempV[j].hRemapHandle    = m_lVert.GetHandleByIndex( k );
                }
            }
        }
    }

    // OKay now get all the facets and remap their indices
    for( i=0; i<m_lTri.GetCount(); i++ )
    {
        for( s32 j=0; j<3; j++ )
        {
            // Right now handles and indices should be maching 1 to 1.
            // This is a bit of a hack since I am assuming that no one has touch the
            // vertices untill this point.
            ASSERT( m_lTri[i].hVertex[j] == pTempV[ m_lTri[i].hVertex[j] ].hHandle );

            m_lTri[i].hVertex[j] = pTempV[ m_lTri[i].hVertex[j] ].hRemapHandle;
        }
    }

    // Finally nuck any vertex that has been remap
    for( i=0; i<m_lVert.GetCount(); i++ )
    {
        if( pTempV[i].hRemapHandle != pTempV[i].hHandle )
        {
            m_lVert.DeleteByHandle( pTempV[i].hHandle );
        }
    }

    delete []pHash;
    delete []pTempV;

    // Handle the errors
    x_catch_begin;

    if( pHash   ) delete []pHash;
    if( pTempV  ) delete []pTempV;

    x_catch_end_ret;
}

//=========================================================================

void slow_lighting::AddLight( const vector3& Pos, f32 AttenR, xcolor Color, xcolor AmbLight )
{
    ASSERT( m_bAdding );

    light& Light    = m_lLight.Add();
    Light.Pos       = Pos;
    Light.AttenR    = AttenR;
    Light.Color     = Color;
    Light.AmbLight  = AmbLight;

    Light.BBox.Set( Light.Pos, Light.AttenR );
}

//=========================================================================
// Some generic spacial data base. Very slow but not time for more fanzy stuff.
//=========================================================================
void slow_lighting::ComputeSpatialDBase( void )
{
    xharray<xhandle> TriList;
    s32              i;

    //
    // Create a copy of all the references to the triangles
    //
    for( i=0; i<m_lTri.GetCount(); i++ )
    {
        TriList.Add() = m_lTri.GetHandleByIndex( i );
    }

    //
    // Now Start creating bboxes and try to fit as many triangles as we can
    //
    const vector3& BBoxSize = m_ZoneBBox.GetSize();
    vector3 MainSize( 100.0f/BBoxSize.GetX(),
                      100.0f/BBoxSize.GetY(),
                      100.0f/BBoxSize.GetZ() );

    while( TriList.GetCount() > 0 )
    {
        quickbox& QBBox = m_lQuickBox.Add();

        // Add the first triangle
        QBBox.hTri.Append() = TriList[0];

        QBBox.BBox.Clear();
        QBBox.BBox.Intersect( m_lVert( m_lTri(TriList[0]).hVertex[0]).Position );
        QBBox.BBox.Intersect( m_lVert( m_lTri(TriList[0]).hVertex[1]).Position );
        QBBox.BBox.Intersect( m_lVert( m_lTri(TriList[0]).hVertex[2]).Position );

        for( s32 j=1; j<TriList.GetCount(); j++ )
        {
            bbox Test( QBBox.BBox );

            Test.Intersect( m_lVert( m_lTri(TriList[j]).hVertex[0]).Position );
            Test.Intersect( m_lVert( m_lTri(TriList[j]).hVertex[1]).Position );
            Test.Intersect( m_lVert( m_lTri(TriList[j]).hVertex[2]).Position );

            vector3 TestSize = Test.GetSize();
            vector3 BBoxSize = QBBox.BBox.GetSize();

            TestSize = TestSize - BBoxSize;
            TestSize = TestSize * MainSize;

            // If the bbox grow less than 10% in any of the dimensions then we want to 
            // add that triangle in our list.
            if( TestSize.GetX() < 10 && TestSize.GetY() < 10 && TestSize.GetZ() < 10 )
            {
                // Add triangle into our list
                QBBox.hTri.Append() = TriList[j];

                // Romove the triangle reference
                TriList.DeleteByIndex( j );

                // Make sure to go over the new j
                j--;
            }
        }
    }   
}

//=============================================================================
static
xbool ComputeRayTriCollision   ( const vector3* Tri,
                                 const vector3& Start, 
                                 const vector3& End, 
                                       f32&     T,
                                       vector3& Normal )
{
    s32 i;
    plane Plane;
    Plane.Setup( Tri[0], Tri[1], Tri[2] );

    // Set the normal
    Normal = Plane.Normal;

    // Are we completely in front or starting from behind?
    if ( !(Plane.InFront( Start ) && Plane.InBack( End )) )
    {
        return FALSE;
    }
        
    // Find where we hit the plane
    Plane.Intersect( T, Start, End );

    if ( (T < 0.0f) || (T > 1.0f) )
    {
        return FALSE;
    }

    vector3 HitPoint = Start + ((End - Start) * T);

    // See if hit point is inside tri
    vector3 EdgeNormal;
    
    for ( i = 0; i < 3; ++i )
    {
        EdgeNormal = Plane.Normal.Cross( Tri[(i+1)%3] - Tri[i] );
        if( EdgeNormal.Dot( HitPoint - Tri[i] ) < -0.001f )
        {
            return FALSE;
        }
    }

    // Collision
    return TRUE;
}

//=========================================================================
// Check Whether it can see the light
//=========================================================================
xbool slow_lighting::CanSeeLight( const light& Light, const vector3& P1 ) const
{
    s32 i;

    for( i=0; i<m_lQuickBox.GetCount(); i++ )
    {
        const quickbox&  QBBox  = m_lQuickBox[i];
        f32              t;
        vector3          p0, p1;

        p0 = Light.Pos;
        p1 = P1;
        
        if( QBBox.BBox.Intersect( t, p0, p1 ) )
        {
            // Check for collisions
            for( s32 j=0; j<QBBox.hTri.GetCount(); j++ )
            {
                vector3 Normal;
                vector3 P[3];

                P[0] = m_lVert( m_lTri( QBBox.hTri[j] ).hVertex[0] ).Position;
                P[1] = m_lVert( m_lTri( QBBox.hTri[j] ).hVertex[1] ).Position;
                P[2] = m_lVert( m_lTri( QBBox.hTri[j] ).hVertex[2] ).Position;

                if( ComputeRayTriCollision( P, p0, p1, t, Normal ) )
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

//=========================================================================

void slow_lighting::LightingBegin( void )
{

}

//=========================================================================
#define LIGHTING_STEPS 4

void slow_lighting::LightingTriangle( s32 i )
{
    f32 Step = 1.0f/((f32)LIGHTING_STEPS);
    f32 u,v,w;
    tri& Tri = m_lTri[i];

    for( u=0; u<=1.0f; u+=Step )
    for( v=0; v<=(1-u); v+=Step )
    {
        w = 1 - u - v;

        vector3 P = u*m_lVert( Tri.hVertex[0] ).Position + 
                    v*m_lVert( Tri.hVertex[1] ).Position + 
                    w*m_lVert( Tri.hVertex[2] ).Position;

        vector3 N = u*m_lVert( Tri.hVertex[0] ).Normal + 
                    v*m_lVert( Tri.hVertex[1] ).Normal + 
                    w*m_lVert( Tri.hVertex[2] ).Normal;

        N.Normalize();

        vector3 L1(0,0,0);
        vector3 L2(0,0,0);

        // Accumulate lighting
        for( s32 j=0; j<m_lLight.GetCount(); j++ )
        {
            light&  Light    = m_lLight[j];
            vector3 LightPos = Light.Pos;

            if( Light.BBox.Intersect( P ) == FALSE )
                continue;

            vector3 Diff = LightPos - P;
            f32 D = Diff.Length();

            // Compute lighting
            xbool InShadow = CanSeeLight( Light, (P+(N*0.5f)) );

            f32 NI = N.Dot( Diff ) / D;
            if( NI < 0 ) NI = 0;

            f32 I = (Light.AttenR - D) / (Light.AttenR);
            if( I < 0 ) I = 0;
            if( I > 1 ) I = 1;

            I *= NI;
            if( I < 0 ) I = 0;
            if( I > 2 ) I = 2;


            vector3 LightColor( Light.Color.R, Light.Color.G, Light.Color.B );
            if( !InShadow )
            {
                L2 += I * LightColor;
            }

            L1 += I * LightColor;
        }

        L1.Min( 255.0f );
        L1.Max( 0.0f   );
        L2.Min( 255.0f );
        L2.Max( 0.0f   );

//**** Add critical section here if we do multiprocessing
        m_lVert( Tri.hVertex[0] ).C1 += L1 * u;
        m_lVert( Tri.hVertex[1] ).C1 += L1 * v;
        m_lVert( Tri.hVertex[2] ).C1 += L1 * w;
        m_lVert( Tri.hVertex[0] ).C2 += L2 * u;
        m_lVert( Tri.hVertex[1] ).C2 += L2 * v;
        m_lVert( Tri.hVertex[2] ).C2 += L2 * w;
        m_lVert( Tri.hVertex[0] ).W += u;
        m_lVert( Tri.hVertex[1] ).W += v;
        m_lVert( Tri.hVertex[2] ).W += w;
//**** End here
    }
}

//=========================================================================

void slow_lighting::LightingEnd( void )
{
    for( s32 i=0; i<m_lVert.GetCount(); i++ )
    {
        vector3 L1 = m_lVert[i].C1;
        if( m_lVert[i].W == 0 )
            L1.Set( 0,0,0 );
        else
            L1 /= m_lVert[i].W;

//        L1.X += m_AmbientLight.R;
//        L1.Y += m_AmbientLight.G;
//        L1.Z += m_AmbientLight.B;
        L1.Min( 255.0f );
        L1.Max( 0.0f   );


        vector3 L2 = m_lVert[i].C2;
        if( m_lVert[i].W == 0 )
            L2.Set( 0,0,0 );
        else
            L2 /= m_lVert[i].W;

//        L2.X += m_AmbientLight.R;
//        L2.Y += m_AmbientLight.G;
//        L2.Z += m_AmbientLight.B;
        L2.Min( 255.0f );
        L2.Max( 0.0f   );

        f32 t = 0;
        vector3 Final = L2;//L1 + t*( L2 - L1 );

        m_lVert[i].FinalColor = xcolor( (byte)Final.GetX(), (byte)Final.GetY(), (byte)Final.GetZ() );
    }
}



