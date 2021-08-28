#include "coll.hpp"
#include "Entropy.hpp"
#include "GameLib\RenderMgr.hpp"
#ifdef EDITOR
#include "ed_db.hpp"
#include "parsing\TextOut.hpp"
#endif
#include "parsing\TextIn.hpp"
extern xbool g_ShowAllMats;
#define VERSION_NUMBER 1004

//==============================================================================

coll::coll ( void )
{
//    m_pTriFlag = NULL;
}

//==============================================================================

coll::~coll ( void )
{
//    if (m_pTriFlag)
//        delete [] m_pTriFlag;
//    m_pTriFlag = NULL;
}

//==============================================================================

void coll::Initialize( void )
{
    x_memset( this, 0, sizeof(*this) );
    m_Version = VERSION_NUMBER;
    m_pStaticVolumeSet = NULL;
//    m_pTriFlag = NULL;
}

//=========================================================================

void coll::GetBBox( bbox& BBox, const matrix4& L2W )
{
    BBox.Clear();
    if (m_pStaticVolumeSet != NULL)
    {
        static_volume_set& SS = *m_pStaticVolumeSet;
        for(s32 count = 0; count < SS.m_nStaticVolumes; count++)
        {
            vector3 p = SS.m_pStaticVolume[count].P0;
            p = L2W.Transform(p);
            BBox += p;
            p = SS.m_pStaticVolume[count].P1;
            p = L2W.Transform(p);
            BBox += p;
            p = SS.m_pStaticVolume[count].P2;
            p = L2W.Transform(p);
            BBox += p;        
        }
    }
    else
    {
        BBox.Set(vector3(0,0,0), vector3(1,1,1));
    }
    return;
}

//==============================================================================

void coll::ApplyGeometry( void )
{
/*
    if (m_pStaticVolumeSet)
    {
        static_volume_set& SS = *m_pStaticVolumeSet;
        for(s32 count = 0; count < SS.m_nStaticVolumes; count++)
        {
            g_CollisionMgr.ApplyTriangle(SS.m_pStaticVolume[count].P0,
                                         SS.m_pStaticVolume[count].P1,
                                         SS.m_pStaticVolume[count].P2, 
                                         0, count);
        }
        return;
    }
*/
    if (m_pStaticVolumeSet)
    {
        // Get shortcut to structure
        static_volume_set& SS = *m_pStaticVolumeSet;

        s32 Sequence = g_CollisionMgr.GetSequence();
        bbox DynamicBBox = g_CollisionMgr.GetDynamicBBox();

        // Are we in the ballpark?
        if( DynamicBBox.Intersect( SS.m_BBox ) )
        {
            // Get Cell range that dynamic bbox overlaps
            f32 W = 1.0f / SS.m_CellSize;
            s32 MinX = (s32)x_floor( (DynamicBBox.Min.X-SS.m_BBox.Min.X) * W );
            s32 MinY = (s32)x_floor( (DynamicBBox.Min.Y-SS.m_BBox.Min.Y) * W );
            s32 MinZ = (s32)x_floor( (DynamicBBox.Min.Z-SS.m_BBox.Min.Z) * W );
            s32 MaxX = (s32)x_floor( (DynamicBBox.Max.X-SS.m_BBox.Min.X) * W );
            s32 MaxY = (s32)x_floor( (DynamicBBox.Max.Y-SS.m_BBox.Min.Y) * W );
            s32 MaxZ = (s32)x_floor( (DynamicBBox.Max.Z-SS.m_BBox.Min.Z) * W );

            // Peg to cells we have
            MinX = MAX(0,MinX);
            MinY = MAX(0,MinY);
            MinZ = MAX(0,MinZ);
            MaxX = MIN(SS.m_nCells[0]-1,MaxX);
            MaxY = MIN(SS.m_nCells[1]-1,MaxY);
            MaxZ = MIN(SS.m_nCells[2]-1,MaxZ);

            ASSERT( (MinX>=0) && (MinX<SS.m_nCells[0]) && (MaxX>=MinX) );
            ASSERT( (MaxX>=0) && (MaxX<SS.m_nCells[0]) );
            ASSERT( (MinY>=0) && (MinY<SS.m_nCells[1]) && (MaxY>=MinY) );
            ASSERT( (MaxY>=0) && (MaxY<SS.m_nCells[1]) );
            ASSERT( (MinZ>=0) && (MinZ<SS.m_nCells[2]) && (MaxZ>=MinZ) );
            ASSERT( (MaxZ>=0) && (MaxZ<SS.m_nCells[2]) );

            // Loop through those cells
            for( s32 CX=MinX; CX<=MaxX; CX++ )
            for( s32 CY=MinY; CY<=MaxY; CY++ )
            for( s32 CZ=MinZ; CZ<=MaxZ; CZ++ )
            {
                // Get Cell index
                s32 CI = CZ*SS.m_nCells[0]*SS.m_nCells[1] + CY*SS.m_nCells[0] + CX;

                // Get offset and count
                s32 Offset = SS.m_CellVolumeOffset[CI];
                s32 Count  = SS.m_CellVolumeCount[CI];

                // Loop through primitives
                for( s32 i=0; i<Count; i++ )
                {
                    // get primitive index
                    s32 PIndex = SS.m_VolumeIndex[ Offset+i ];
                    ASSERT( (PIndex>=0) && (PIndex<SS.m_nStaticVolumes) );

                    // get reference to primitive
                    collision_volume& Prim = SS.m_pStaticVolume[PIndex];

                    //ASSERT( Prim.Type >= PRIMITIVE_STATIC_START );
                    //ASSERT( Prim.Type <= PRIMITIVE_STATIC_END );

                    if( Prim.SearchSeq != Sequence )
                    {
                        Prim.SearchSeq = Sequence;

                        // Check if bboxes intersect
                        if( DynamicBBox.Intersect( Prim.AABBox ) )
                        {
                            g_CollisionMgr.ApplyTriangle( Prim.P0, Prim.P1, Prim.P2, 0, PIndex );
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void coll::OnGatherTriangles( const matrix4& L2W )
{
    if((m_pStaticVolumeSet != NULL) && (m_hasL2W || L2W.IsIdentity()))
    {
        //g_CollisionMgr.SetStaticVolumeSet(m_pStaticVolumeSet);

        // Get shortcut to structure
        static_volume_set& SS = *m_pStaticVolumeSet;

        bbox GatherBBox = g_CollisionMgr.GetGatherBBox();
        s32 Sequence = g_CollisionMgr.GetSequence();

        // Are we in the ballpark?
        if( GatherBBox.Intersect( SS.m_BBox ) )
        {
            // Get Cell range that dynamic bbox overlaps
            f32 W = 1.0f / SS.m_CellSize;
            s32 MinX = (s32)x_floor( (GatherBBox.Min.X-SS.m_BBox.Min.X) * W );
            s32 MinY = (s32)x_floor( (GatherBBox.Min.Y-SS.m_BBox.Min.Y) * W );
            s32 MinZ = (s32)x_floor( (GatherBBox.Min.Z-SS.m_BBox.Min.Z) * W );
            s32 MaxX = (s32)x_floor( (GatherBBox.Max.X-SS.m_BBox.Min.X) * W );
            s32 MaxY = (s32)x_floor( (GatherBBox.Max.Y-SS.m_BBox.Min.Y) * W );
            s32 MaxZ = (s32)x_floor( (GatherBBox.Max.Z-SS.m_BBox.Min.Z) * W );

            // Peg to cells we have
            MinX = MAX(0,MinX);
            MinY = MAX(0,MinY);
            MinZ = MAX(0,MinZ);
            MaxX = MIN(SS.m_nCells[0]-1,MaxX);
            MaxY = MIN(SS.m_nCells[1]-1,MaxY);
            MaxZ = MIN(SS.m_nCells[2]-1,MaxZ);

            ASSERT( (MinX>=0) && (MinX<SS.m_nCells[0]) && (MaxX>=MinX) );
            ASSERT( (MaxX>=0) && (MaxX<SS.m_nCells[0]) );
            ASSERT( (MinY>=0) && (MinY<SS.m_nCells[1]) && (MaxY>=MinY) );
            ASSERT( (MaxY>=0) && (MaxY<SS.m_nCells[1]) );
            ASSERT( (MinZ>=0) && (MinZ<SS.m_nCells[2]) && (MaxZ>=MinZ) );
            ASSERT( (MaxZ>=0) && (MaxZ<SS.m_nCells[2]) );

            // Loop through those cells
            for( s32 CX=MinX; CX<=MaxX; CX++ )
            for( s32 CY=MinY; CY<=MaxY; CY++ )
            for( s32 CZ=MinZ; CZ<=MaxZ; CZ++ )
            {
                // Get Cell index
                s32 CI = CZ*SS.m_nCells[0]*SS.m_nCells[1] + CY*SS.m_nCells[0] + CX;

                // Get offset and count
                s32 Offset = SS.m_CellVolumeOffset[CI];
                s32 Count  = SS.m_CellVolumeCount[CI];

                // Loop through primitives
                for( s32 i=0; i<Count; i++ )
                {
                    // get primitive index
                    s32 PIndex = SS.m_VolumeIndex[ Offset+i ];
                    ASSERT( (PIndex>=0) && (PIndex<SS.m_nStaticVolumes) );

                    // get reference to primitive
                    collision_volume& Prim = SS.m_pStaticVolume[PIndex];

                    //ASSERT( Prim.Type >= PRIMITIVE_STATIC_START );
                    //ASSERT( Prim.Type <= PRIMITIVE_STATIC_END );

                    if( Prim.SearchSeq != Sequence )
                    {
                        Prim.SearchSeq = Sequence;

                        // Check if bboxes intersect
                        if( GatherBBox.Intersect( Prim.AABBox ) )
                        {
                            g_CollisionMgr.GatherTriangle( Prim.P0, Prim.P1, Prim.P2 );
                        }
                    }
                }
            }
        }
    }
    else
    {
        static_volume_set& SS = *m_pStaticVolumeSet;
        bbox GatherBBox = g_CollisionMgr.GetGatherBBox();

        for( s32 i=0; i<SS.m_nStaticVolumes; i++ )
        {
            vector3 P[3];
            P[0] = L2W * SS.m_pStaticVolume[i].P0;
            P[1] = L2W * SS.m_pStaticVolume[i].P1;
            P[2] = L2W * SS.m_pStaticVolume[i].P2;
            bbox BBox;
            BBox += P[0];
            BBox += P[1];
            BBox += P[2];

            if( BBox.Intersect(GatherBBox) )
            {
                g_CollisionMgr.GatherTriangle( P[0], P[1], P[2] );
            }
        }

/*
        if(m_hasL2W || L2W.IsIdentity())
        {
            for(s32 count = 0; count < m_nTris; count++)
            {
                g_CollisionMgr.ApplyTriangle(m_pTris[count].P0,
                    m_pTris[count].P1,
                    m_pTris[count].P2);
            }
        }
        else
        {
            //Transform all the vertices by L2W
            tri*    pTris = new tri[m_nTris];
            for(s32 count2 = 0; count2 < m_nTris; count2++)
            {
                pTris[count2].P0 = L2W.Transform(m_pTris[count2].P0);
                pTris[count2].P1 = L2W.Transform(m_pTris[count2].P1);
                pTris[count2].P2 = L2W.Transform(m_pTris[count2].P2);

            }
            for(s32 count = 0; count < m_nTris; count++)
            {
                g_CollisionMgr.ApplyTriangle(pTris[count].P0,
                    pTris[count].P1,
                    pTris[count].P2);
            }
            delete [] pTris;
        }
*/
    }    
    return;
}

//=========================================================================

void coll::Render( const matrix4& L2W, const xarray<s32>* Active  )
{
#ifndef EDITOR
    (void)Active;
#endif
    ASSERT(m_pStaticVolumeSet);
    static_volume_set& SS = *m_pStaticVolumeSet;
    xcolor Color = XCOLOR_YELLOW;
    if(m_hasL2W)
    {
        for(s32 count = 0; count < SS.m_nStaticVolumes; count++)
        {
            vector3 P0, P1, P2;
            P0 = SS.m_pStaticVolume[count].P0;
            P1 = SS.m_pStaticVolume[count].P1;    
            P2 = SS.m_pStaticVolume[count].P2;

#ifdef EDITOR            
            if (Active && Active->Find(count) != -1)
                Color = XCOLOR_RED;
            else                    
#endif
                Color = XCOLOR_YELLOW;

            if (!g_RenderMgr.DoRenderCollisionSolid)
            {
                // Count common edges with active tri
//                s32 ctr = 0;
//                if (ActiveTri != -1)
//                {
//                    vector3 NoDraw[3];
//                    NoDraw[0] = SS.m_pStaticVolume[ActiveTri].P0;
//                    NoDraw[1] = SS.m_pStaticVolume[ActiveTri].P1;
//                    NoDraw[2] = SS.m_pStaticVolume[ActiveTri].P2;
//                    for ( s32 i = 0; i < 3; i++ )
//                    {
//                        if (NoDraw[i] == P0  || NoDraw[i] == P1 || NoDraw[i] == P2)
//                            ctr++;
//                    }
//                    if (ctr == 2)
//                    {
//                        continue;
//                    }
//                }
                draw_Begin(DRAW_TRIANGLES, DRAW_WIRE_FRAME);
            }
            else
                draw_Begin(DRAW_TRIANGLES);
            draw_Color(Color);
            draw_Vertex(P0);
            draw_Vertex(P1);
            draw_Vertex(P2);

            draw_End();
        }
    }
    else
    {
        vector3 P0,P1,P2;
        if(g_ShowAllMats)
        {
            vector3 Trans = L2W.GetTranslation();
            radian3 Rot   = L2W.GetRotation();
            x_printfxy(10, 10, "Collision L2W :");
            x_printfxy(10, 11, "Translation = (%4.2f, %4.2f, %4.2f)",
                Trans.X, Trans.Y, Trans.Z);
            x_printfxy(10, 12, "Rotation = (%4.2f, %4.2f, %4.2f)",
                Rot.Pitch, Rot.Yaw, Rot.Roll);

            bbox Box = SS.m_BBox;
            Box.Transform(L2W);
            draw_BBox(Box, XCOLOR_YELLOW);
        }
        //Transform all the vertices by L2W
        for(s32 count = 0; count < SS.m_nStaticVolumes; count++)
        {
#ifdef EDITOR
            if (Active && Active->Find(count) != -1)
                Color = XCOLOR_RED;
            else                    
#endif
                Color = XCOLOR_YELLOW;

            P0 = L2W.Transform(SS.m_pStaticVolume[count].P0);
            P1 = L2W.Transform(SS.m_pStaticVolume[count].P1);
            P2 = L2W.Transform(SS.m_pStaticVolume[count].P2);
            if (!g_RenderMgr.DoRenderCollisionSolid)
            {
//               // Count common edges with active tri
//                s32 ctr = 0;
//                if (ActiveTri != -1)
//                {
//                    vector3 NoDraw[3];
//                    NoDraw[0] = L2W.Transform(SS.m_pStaticVolume[ActiveTri].P0);
//                    NoDraw[1] = L2W.Transform(SS.m_pStaticVolume[ActiveTri].P1);
//                    NoDraw[2] = L2W.Transform(SS.m_pStaticVolume[ActiveTri].P2);
//                    for ( s32 i = 0; i < 3; i++ )
//                    {
//                        if (NoDraw[i] == P0  || NoDraw[i] == P1 || NoDraw[i] == P2)
//                            ctr++;
//                    }
//                    if (ctr == 2)
//                    {
//                        continue;
//                    }
//                }
                draw_Begin(DRAW_TRIANGLES, DRAW_WIRE_FRAME);
            }
            else
                draw_Begin(DRAW_TRIANGLES);
            draw_Color(Color);
            draw_Vertex(P0);
            draw_Vertex(P1);
            draw_Vertex(P2);

            draw_End();
        }
    }
    return;
}

//==============================================================================
/*
void coll::tri::Save( platformio& IO )
{
    (void)IO;   
}

//==============================================================================

void coll::tri::Load( X_FILE* Fp )
{
    (void)Fp;    
}*/

//==============================================================================

void coll::Save( X_FILE* Fp, platform Platform )
{
    platformio  IO;
    m_Platform = Platform;

    if( Platform == PLATFORM_PC )
        IO.Init( TRUE );
    else
        IO.Init( FALSE );

    // Save the platform
    IO.Store32( (u32*)&m_Version      );
    IO.Store32( (u32*)&m_Platform     );
    //IO.Store32( (u32*)&m_nTris        );
    IO.Store32( (u32*)&m_hasL2W       );

    s32 count;
/*
    for(count = 0; count < m_nTris; count++)
    {
        tri *pTri = &m_pTris[count];
        IO.Store32( (u32*)&pTri->P0.X );
        IO.Store32( (u32*)&pTri->P0.Y );
        IO.Store32( (u32*)&pTri->P0.Z );
        IO.Store32( (u32*)&pTri->P1.X );
        IO.Store32( (u32*)&pTri->P1.Y );
        IO.Store32( (u32*)&pTri->P1.Z );
        IO.Store32( (u32*)&pTri->P2.X );
        IO.Store32( (u32*)&pTri->P2.Y );
        IO.Store32( (u32*)&pTri->P2.Z );
    }
*/
    //Write the static volume set
    xbool bStaticVolumeSet;
    if(m_pStaticVolumeSet != NULL)
    {
        bStaticVolumeSet = 1;
        IO.Store32( (u32*)&bStaticVolumeSet );
        IO.StoreBBox( &m_pStaticVolumeSet->m_BBox );
        IO.Store32( (u32*)&m_pStaticVolumeSet->m_CellSize );
        IO.Store32( (u32*)&m_pStaticVolumeSet->m_nCells, 3 );
        IO.Store32( (u32*)&m_pStaticVolumeSet->m_nTotalCells );
        IO.Store32( (u32*)&m_pStaticVolumeSet->m_nStaticVolumes );
        for(count = 0; count < m_pStaticVolumeSet->m_nStaticVolumes; count++)
        {
            //IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].Type );
            //IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].SpherePos.X );
            //IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].SpherePos.Y );
            //IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].SpherePos.Z );
            //IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].Radius );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P0.X );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P0.Y );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P0.Z );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P1.X );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P1.Y );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P1.Z );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P2.X );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P2.Y );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].P2.Z );
            IO.StoreBBox( &m_pStaticVolumeSet->m_pStaticVolume[count].AABBox );
            IO.Store32( (u32*)&m_pStaticVolumeSet->m_pStaticVolume[count].SearchSeq );
        }
        s32 temp = m_pStaticVolumeSet->m_CellVolumeCount.GetCount();
        IO.Store32( (u32*)&temp );
        for(count = 0; count < m_pStaticVolumeSet->m_CellVolumeCount.GetCount(); count++)
        {
            temp = m_pStaticVolumeSet->m_CellVolumeCount.GetAt(count);
            IO.Store32((u32*)&temp);
        }
        temp = m_pStaticVolumeSet->m_CellVolumeOffset.GetCount();
        IO.Store32( (u32*)&temp );
        for(count = 0; count < m_pStaticVolumeSet->m_CellVolumeOffset.GetCount(); count++)
        {
            temp = m_pStaticVolumeSet->m_CellVolumeOffset.GetAt(count);
            IO.Store32((u32*)&temp);
        }
        temp = m_pStaticVolumeSet->m_VolumeIndex.GetCount();
        IO.Store32( (u32*)&temp );
        for(count = 0; count < m_pStaticVolumeSet->m_VolumeIndex.GetCount(); count++)
        {
            temp = m_pStaticVolumeSet->m_VolumeIndex.GetAt(count);
            IO.Store32((u32*)&temp);
        }
    }
    else
    {
        bStaticVolumeSet = 0;
        IO.Store32( (u32*)&bStaticVolumeSet );
    }
    IO.WriteToFile( Fp );
}

//==============================================================================

void coll::Load( X_FILE* Fp )
{
     // Load and check version
    x_fread( &m_Version,      1, sizeof(s32), Fp );
    if( m_Version != VERSION_NUMBER )
    {
        ASSERT( 0 );
        e_throw( "Loading geometry with wrong version!" );
    }

    x_fread( &m_Platform,     1, sizeof(s32), Fp );
    //x_fread( &m_nTris,        1, sizeof(s32), Fp );
    x_fread( &m_hasL2W,       1, sizeof(s32), Fp );
    //m_pTris = new tri[m_nTris];
    //x_fread( m_pTris, sizeof(f32)*9, m_nTris, Fp );
    
    xbool bStaticVolumeSet;
    x_fread( &bStaticVolumeSet, sizeof(xbool), 1, Fp );
 //   x_DebugMsg("COLL LOAD: before bStaticVolumeSet, m_nTris = %d bytes so far = %d\n", m_nTris, x_ftell(Fp));
        
    m_pStaticVolumeSet = NULL;
    if(bStaticVolumeSet)
    {
        //x_DebugMsg("COLL LOAD: with bStaticVolumeSet, m_nTris = %d bytes so far = %d\n", m_nTris, x_ftell(Fp));
        m_pStaticVolumeSet = new static_volume_set;
        x_fread(&m_pStaticVolumeSet->m_BBox, sizeof(bbox), 1, Fp);
        x_fread(&m_pStaticVolumeSet->m_CellSize, sizeof(s32), 1, Fp);
        x_fread(&m_pStaticVolumeSet->m_nCells, sizeof(s32), 3, Fp);
        x_fread(&m_pStaticVolumeSet->m_nTotalCells, sizeof(s32), 1, Fp);
        x_fread(&m_pStaticVolumeSet->m_nStaticVolumes, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_pStaticVolume = new collision_volume[m_pStaticVolumeSet->m_nStaticVolumes];
        x_fread(m_pStaticVolumeSet->m_pStaticVolume, sizeof(collision_volume), m_pStaticVolumeSet->m_nStaticVolumes, Fp);

/*
        s32 count;
        s32 readCount;
        s32 *temp;
        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_CellVolumeCount.SetCapacity(readCount);
        temp = new s32[readCount];
        x_fread(temp, sizeof(s32), readCount, Fp);
        for(count = 0; count < readCount; count++)
            m_pStaticVolumeSet->m_CellVolumeCount.Append(temp[count]);
        delete [] temp;
        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_CellVolumeOffset.SetCapacity(readCount);
        temp = new s32[readCount];
        x_fread(temp, sizeof(s32), readCount, Fp);
        for(count = 0; count < readCount; count++)
            m_pStaticVolumeSet->m_CellVolumeOffset.Append(temp[count]);
        delete [] temp;
        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_VolumeIndex.SetCapacity(readCount);
        temp = new s32[readCount];
        x_fread(temp, sizeof(s32), readCount, Fp);
        for(count = 0; count < readCount; count++)
            m_pStaticVolumeSet->m_VolumeIndex.Append(temp[count]);
        delete [] temp;
*/

        s32 readCount;

        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_CellVolumeCount.SetCapacity(readCount);
        m_pStaticVolumeSet->m_CellVolumeCount.SetCount(readCount);
        x_fread(m_pStaticVolumeSet->m_CellVolumeCount.GetPtr(), sizeof(s32), readCount, Fp);

        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_CellVolumeOffset.SetCapacity(readCount);
        m_pStaticVolumeSet->m_CellVolumeOffset.SetCount(readCount);
        x_fread(m_pStaticVolumeSet->m_CellVolumeOffset.GetPtr(), sizeof(s32), readCount, Fp);

        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_VolumeIndex.SetCapacity(readCount);
        m_pStaticVolumeSet->m_VolumeIndex.SetCount(readCount);
        x_fread(m_pStaticVolumeSet->m_VolumeIndex.GetPtr(), sizeof(s32), readCount, Fp);

        // Set up the array of bytes for the surface flags.
//        InitTriFlags();
    }
}

//==============================================================================

#ifdef EDITOR

//==============================================================================

s32 coll::GetNumTriFlags  ( void )
{
    if (m_pStaticVolumeSet)
        return m_pStaticVolumeSet->m_nStaticVolumes;
    return 0;
}

#endif
//==============================================================================

/*
void coll::SanityCheck( void )
{
    ASSERT( m_pStaticVolumeSet->m_BBox.Min.X >= -10000000.0f );
    ASSERT( m_pStaticVolumeSet->m_BBox.Min.Y >= -10000000.0f );
    ASSERT( m_pStaticVolumeSet->m_BBox.Min.Z >= -10000000.0f );

    ASSERT( m_pStaticVolumeSet->m_BBox.Max.X >= -10000000.0f );
    ASSERT( m_pStaticVolumeSet->m_BBox.Max.Y >= -10000000.0f );
    ASSERT( m_pStaticVolumeSet->m_BBox.Max.Z >= -10000000.0f );

    ASSERT( m_pStaticVolumeSet->m_CellSize >= 0 );
    ASSERT( m_pStaticVolumeSet->m_CellSize <  100000 );

    ASSERT( m_pStaticVolumeSet->m_nCells >= 0 );
    ASSERT( m_pStaticVolumeSet->m_nCells < 100000 );

    ASSERT( m_pStaticVolumeSet->m_nTotalCells > 0 );
    ASSERT( m_pStaticVolumeSet->m_nTotalCells < 100000 );

    ASSERT( m_pStaticVolumeSet->m_nStaticVolumes > 0 );
    ASSERT( m_pStaticVolumeSet->m_nStaticVolumes < 100000 );

    s32 i;

    for (s32 i=0; i<m_pStaticVolumeSet->m_nStaticVolumes; i++ )
    {
        m_pStaticVolumeSet->m_pStaticVolume[i]

    }


        m_pStaticVolumeSet->m_pStaticVolume = new collision_volume[m_pStaticVolumeSet->m_nStaticVolumes];
        x_fread(m_pStaticVolumeSet->m_pStaticVolume, sizeof(collision_volume), m_pStaticVolumeSet->m_nStaticVolumes, Fp);
        s32 readCount;
        s32 *temp;
        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_CellVolumeCount.SetCapacity(readCount);
        temp = new s32[readCount];
        x_fread(temp, sizeof(s32), readCount, Fp);
        for(count = 0; count < readCount; count++)
            m_pStaticVolumeSet->m_CellVolumeCount.Append(temp[count]);
        delete [] temp;
        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_CellVolumeOffset.SetCapacity(readCount);
        temp = new s32[readCount];
        x_fread(temp, sizeof(s32), readCount, Fp);
        for(count = 0; count < readCount; count++)
            m_pStaticVolumeSet->m_CellVolumeOffset.Append(temp[count]);
        delete [] temp;
        x_fread(&readCount, sizeof(s32), 1, Fp);
        m_pStaticVolumeSet->m_VolumeIndex.SetCapacity(readCount);
        temp = new s32[readCount];
        x_fread(temp, sizeof(s32), readCount, Fp);
        for(count = 0; count < readCount; count++)
            m_pStaticVolumeSet->m_VolumeIndex.Append(temp[count]);
        delete [] temp;


}
*/

//==============================================================================

