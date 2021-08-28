#include "Lighting.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Objects\LightObject.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "Render\Render.hpp"
#include "..\WorldEditor\WorldEditor.hpp"
#include "RaycastLighting.hpp"
#include "ManagerRegistration.hpp"
#include "..\Apps\Editor\Project.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "3rdParty/DirectX9/d3d9.h"


//=============================================================================

struct info
{
    object*       pObject;
    geom*         pGeom;
    rigid_inst*   pRigidInst;
};

//=============================================================================

static vector3              s_LightDir( 0.3f, 1.0f, 0.8f );
static raycast_lighting     s_RayCastSystem;
static void*                s_pPlaySurfaceColors = NULL;
static xcolor               s_ZoneColors[256];

//=============================================================================
static 
xcolor ConvertColorTo4444( xcolor Color )
{
    if( 0 )
    {
        s16 R[3],G[3],B[3];
        s16 A = ((s16)(((Color.A / 255.0f)*15.0f)+0.5f))<<4;
        R[0]  = ((s16)(((Color.R / 255.0f)*15.0f)+0.5f))<<4;
        G[0]  = ((s16)(((Color.G / 255.0f)*15.0f)+0.5f))<<4;
        B[0]  = ((s16)(((Color.B / 255.0f)*15.0f)+0.5f))<<4;
        R[1]  = MIN((R[0]+16),255) & 0xF0;
        R[2]  = MAX((R[0]-16),  0) & 0xF0;
        G[1]  = MIN((G[0]+16),255) & 0xF0;
        G[2]  = MAX((G[0]-16),  0) & 0xF0;
        B[1]  = MIN((B[0]+16),255) & 0xF0;
        B[2]  = MAX((B[0]-16),  0) & 0xF0;

        f32 OrigRG = (Color.G) ? ( (f32)Color.R/(f32)Color.G ) : ( (f32)Color.R );
        f32 OrigGB = (Color.B) ? ( (f32)Color.G/(f32)Color.B ) : ( (f32)Color.G );
        f32 OrigRB = (Color.B) ? ( (f32)Color.R/(f32)Color.B ) : ( (f32)Color.R );
        f32 BestErr = F32_MAX;
        for( s32 r=0; r<3; r++ )
        for( s32 g=0; g<3; g++ )
        for( s32 b=0; b<3; b++ )
        {
            f32 RG = (G[g]) ? ( (f32)R[r]/(f32)G[g] ) : ( (f32)R[r] );
            f32 GB = (B[b]) ? ( (f32)G[g]/(f32)B[b] ) : ( (f32)G[g] );
            f32 RB = (B[b]) ? ( (f32)R[r]/(f32)B[b] ) : ( (f32)R[r] );
            f32 Err = x_sqr(RG-OrigRG) + x_sqr(GB-OrigGB) + x_sqr(RB-OrigRB);

            if( Err < BestErr )
            {
                Color.R = (u8)R[r];
                Color.G = (u8)G[g];
                Color.B = (u8)B[b];
                BestErr = Err;
            }
        }

        // 
        Color.A = (u8)A;

        
        Color.R >>=1;
        Color.G >>=1;
        Color.B >>=1;
        Color.A >>=1;
    }
    else 
    {
        u8 R = (Color.R >> 3) & 0x1F;
        u8 G = (Color.G >> 3) & 0x1F;
        u8 B = (Color.B >> 3) & 0x1F;

        // 128 is 1.0 on PS2
        R >>= 1;
        G >>= 1;
        B >>= 1;

        // Back to 8bits
        Color.R = R<<3;
        Color.G = G<<3;
        Color.B = B<<3;
        
    }

    return Color;
}

//=============================================================================

static xbool GetInfo( platform Platform, guid Guid, info& Info )
{
    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    
    if( pObject == NULL )
        x_throw( "Unable to find object in Object Manager" );
    
    // Make sure that we have to bake the lighting for this object
    if( pObject->GetTypeDesc().IsBurnVertexLighting() == FALSE )
        return FALSE ;
    
    rigid_inst* pRigidInst = g_WorldEditor.GetRigidInstForObject(pObject);

    ASSERT( pRigidInst );
    rigid_inst& RigidInst = *pRigidInst;

    
    // Get the Rigid Instance
    
    
    // Get the Geom
    geom* pGeom = RigidInst.GetGeom();
    
    if( pGeom == NULL )
        return( FALSE );

    // Setup output structure
    Info.pObject    = pObject;
    Info.pGeom      = pGeom;
    Info.pRigidInst = &RigidInst;

    return( TRUE );
}

//=============================================================================

void LightVertexDistance( const matrix4&            L2W,
                          const matrix4&            W2L,
                          const xarray<light_obj*>& lLight,
                          rigid_geom::vertex_pc&    Vertex )
{
    f32 TotalR = 0.0f;
    f32 TotalG = 0.0f;
    f32 TotalB = 0.0f;

    vector3 WorldPos( L2W.Transform( Vertex.Pos ) );

    for( s32 i=0; i<lLight.GetCount(); i++ )
    {
        light_obj& LightObject = *lLight[i];
    
		if( LightObject.GetBBox().Intersect( WorldPos ) == TRUE )
		{
			vector3 LightPos   ( LightObject.GetPosition() );
			xcolor  LightColor ( LightObject.GetColor()    );
			f32     LightRadius( LightObject.GetRadious()  );

			// Compute attenuation
			f32 L = MIN( (LightPos - WorldPos).Length(), LightRadius );
			f32 A = 1.0f - ( (L * L) / (LightRadius * LightRadius) );
        
			// Compute directional lighting
			vector3 LightDir( W2L.Transform( LightPos - WorldPos ) );
			LightDir.Normalize();
			f32 I = MINMAX( 0.0f, LightDir.Dot( Vertex.Normal ), 1.0f );

            A *= LightObject.GetLightIntensity();

            if( LightObject.GetAngleAccent() )
            {
                I *= 2;
                I = (I*I)/3;
            }

			// Apply attenuation
			I *= A;
        
			// Add contribution of light
			TotalR += (LightColor.R / 255.0f) * I;
			TotalG += (LightColor.G / 255.0f) * I;
			TotalB += (LightColor.B / 255.0f) * I;

            // Add ambient lighting
            xcolor Ambient = LightObject.GetAmbient();

            TotalR += (Ambient.R / 255.0f)*A;
            TotalG += (Ambient.G / 255.0f)*A;
            TotalB += (Ambient.B / 255.0f)*A;
		}
    }
    
    // Clamp lighting
    TotalR = fMax( 0.0f, fMin( TotalR, 1.0f ));
    TotalG = fMax( 0.0f, fMin( TotalG, 1.0f ));
    TotalB = fMax( 0.0f, fMin( TotalB, 1.0f ));
    
    Vertex.Color.R = (u8)(TotalR * 255.0f);
    Vertex.Color.G = (u8)(TotalG * 255.0f);
    Vertex.Color.B = (u8)(TotalB * 255.0f);
    //Vertex.Color = ConvertColorTo4444( Vertex.Color );

}


//=============================================================================

void LightVertexByRaycast(const matrix4&            L2W,
                          const matrix4&            W2L,
                          const xarray<light_obj*>& lLight,
                          rigid_geom::vertex_pc&    Vertex )
{
    f32 TotalR = 0.0f;
    f32 TotalG = 0.0f;
    f32 TotalB = 0.0f;

    vector3 WorldPos( L2W.Transform( Vertex.Pos + Vertex.Normal ) );

    for( s32 i=0; i<lLight.GetCount(); i++ )
    {
        light_obj& LightObject = *lLight[i];
    
		if( LightObject.GetBBox().Intersect( WorldPos ) == TRUE )
		{
			vector3 LightPos   ( LightObject.GetPosition() );
			f32     LightRadius( LightObject.GetRadious()  );
            
            g_CollisionMgr.LineOfSightSetup( LightObject.GetGuid(), WorldPos , LightPos );
            g_CollisionMgr.CheckCollisions(object::TYPE_PLAY_SURFACE);

			// Compute attenuation
			f32 L = MIN( (LightPos - WorldPos).Length(), LightRadius );
			f32 A = 1.0f - ( (L * L) / (LightRadius * LightRadius) );
            A *= LightObject.GetLightIntensity();
            
            //  if zero collisions then we can see this thing
            if( !(g_CollisionMgr.m_nCollisions ) )
            {
			    xcolor  LightColor ( LightObject.GetColor()    );
        
			    // Compute directional lighting
			    vector3 LightDir( W2L.Transform( LightPos - WorldPos ) );
			    LightDir.Normalize();
			    f32 I = MINMAX( 0.0f, LightDir.Dot( Vertex.Normal ), 1.0f );

                if( LightObject.GetAngleAccent() )
                {
                    I *= 2;
                    I = (I*I)/3;
                }

			    // Apply attenuation
			    I *= A;
        
			    // Add contribution of light
			    TotalR += (LightColor.R / 255.0f) * I;
			    TotalG += (LightColor.G / 255.0f) * I;
			    TotalB += (LightColor.B / 255.0f) * I;
            }

            // Add ambient lighting
            xcolor Ambient = LightObject.GetAmbient();

            TotalR += (Ambient.R / 255.0f)*A;
            TotalG += (Ambient.G / 255.0f)*A;
            TotalB += (Ambient.B / 255.0f)*A;
		}
    }
    
    // Clamp lighting
    TotalR = fMax( 0.0f, fMin( TotalR, 1.0f ));
    TotalG = fMax( 0.0f, fMin( TotalG, 1.0f ));
    TotalB = fMax( 0.0f, fMin( TotalB, 1.0f ));
    
    Vertex.Color.R = (u8)(TotalR * 255.0f);
    Vertex.Color.G = (u8)(TotalG * 255.0f);
    Vertex.Color.B = (u8)(TotalB * 255.0f);
}

//=============================================================================

static void GetLights( object& Object, xarray<light_obj*>& lList, xbool Dynamic, xbool Static )
{
    if ( Dynamic )
    {
        g_ObjMgr.SelectBBox( object::ATTR_ALL, Object.GetBBox(), object::TYPE_CHARACTER_LIGHT );
        slot_id SlotID = g_ObjMgr.StartLoop();
        for( ;SlotID != SLOT_NULL; SlotID = g_ObjMgr.GetNextResult( SlotID ) )
        {
            object*    pLight      = g_ObjMgr.GetObjectBySlot( SlotID );
            light_obj& LightObject = light_obj::GetSafeType( *pLight );
            lList.Append( &LightObject );
        }
        g_ObjMgr.EndLoop();
    }

    if ( Static )
    {
        g_ObjMgr.SelectBBox( object::ATTR_ALL, Object.GetBBox(), object::TYPE_LIGHT );
        slot_id SlotID = g_ObjMgr.StartLoop();
        for( ;SlotID != SLOT_NULL; SlotID = g_ObjMgr.GetNextResult( SlotID ) )
        {
            object*    pLight      = g_ObjMgr.GetObjectBySlot( SlotID );
            light_obj& LightObject = light_obj::GetSafeType( *pLight );
            lList.Append( &LightObject );
        }
        g_ObjMgr.EndLoop();
    }
}

//=============================================================================

void InitZoneColors( void )
{
    s32 i;

    for ( i = 0; i < 256; i++ )
    {
        s_ZoneColors[i].R = x_irand(32,255);
        s_ZoneColors[i].G = x_irand(32,255);
        s_ZoneColors[i].B = x_irand(32,255);
        s_ZoneColors[i].A = 255;
    }
}

//=============================================================================

xcolor GetZoneColor( u16 Zone1, u16 Zone2 )
{
    ASSERT( (Zone1>=0) && (Zone1<256) );
    ASSERT( (Zone2>=0) && (Zone2<256) );
    xcolor Color1 = s_ZoneColors[Zone1];
    xcolor Color2 = s_ZoneColors[Zone2];
    if ( (Zone2 == 0) || (Zone1 == Zone2) )
        return Color1;

    s32 R = Color1.R + Color2.R;
    s32 G = Color1.G + Color2.G;
    s32 B = Color1.B + Color2.B;
    
    // divide by two to get an average, and divide by two again to make it a darker color
    R >>= 2;
    G >>= 2;
    B >>= 2;

    return xcolor(R,G,B,255);
}

//=============================================================================

void lighting_Initialize( void )
{
    g_RegGameMgrs.AddManager( "Lighting\\RayCast", &s_RayCastSystem );
}

//=============================================================================

void lighting_LightObject( platform       Platform,
                           guid           Guid,
                           const matrix4& L2W,
                           s32            Mode )
{
    if( Platform != PLATFORM_PC )
        x_throw( "Can only light the objects in the PC version" );

    // Take the world light vector into the local space of the object
    matrix4 W2L( L2W );
    W2L.SetTranslation( vector3( 0.0f, 0.0f, 0.0f ) );
    W2L.Transpose();
    
    vector3 LDir( W2L.Transform( s_LightDir ) );
    LDir.Normalize();

    info Info;
    if( GetInfo( Platform, Guid, Info ) == FALSE )
        return;

    // Build a list of all the lights which affect this object
    xarray<light_obj*> lLight;

    if( Mode == LIGHTING_DYNAMIC )
    {
        GetLights( *Info.pObject, lLight, TRUE, FALSE );
    }
    else
    {
        GetLights( *Info.pObject, lLight, FALSE, TRUE );
    }

    rigid_geom& RigidGeom = *(rigid_geom*)Info.pGeom;

    // Loop through all Display Lists
    for( s32 iSubMesh = 0; iSubMesh < RigidGeom.m_nSubMeshes; iSubMesh++ )
    {
        // Get the display list
        rigid_geom::submesh&  GeomSubMesh = RigidGeom.m_pSubMesh[iSubMesh];
        rigid_geom::dlist_pc& DList       = RigidGeom.m_System.pPC[GeomSubMesh.iDList];

        // Get the vertex buffer
        rigid_geom::vertex_pc* pVertex =
            (rigid_geom::vertex_pc*)render::LockRigidDListVertex( Info.pRigidInst->GetInst(), iSubMesh );

        s32 iVert;
        switch ( Mode )
        {
        case LIGHTING_WHITE:
            for ( iVert = 0; iVert < DList.nVerts; iVert++ )
            {
                pVertex[iVert].Color.Set( 128, 128, 128, 255 );
            }
            break;

        case LIGHTING_DIRECTIONAL:
            for ( iVert = 0; iVert < DList.nVerts; iVert++ )
            {
                f32 Ambient = 0.5f;
                f32 D = LDir.Dot( vector3(pVertex[iVert].Normal) );
                s32 I = (s32)( MINMAX( Ambient, D, 1.0f ) * 128.0f );

                pVertex[iVert].Color.R = (u8)I & 0xF8;
                pVertex[iVert].Color.G = (u8)I & 0xF8;
                pVertex[iVert].Color.B = (u8)I & 0xF8;
            }
            break;

        case LIGHTING_DYNAMIC :
        case LIGHTING_DISTANCE :
            for ( iVert =0; iVert < DList.nVerts; iVert++ )
            {
                LightVertexDistance( L2W, W2L, lLight, pVertex[iVert] );
            }
            break;

        case LIGHTING_RAYCAST :
            {
                object::type ObjType = Info.pObject->GetType();
                s32 SlotID = Info.pObject->GetSlot();
                xbool CastShadow = FALSE;

                // only objects that don't move should cast shadows
                if ( (ObjType == object::TYPE_PLAY_SURFACE) ||
                     (ObjType == object::TYPE_ANIM_SURFACE) )
                {
                    CastShadow = TRUE;
                }

                // transparent materials shouldn't cast shadows
                geom::submesh&  SubMesh = RigidGeom.m_pSubMesh[iSubMesh];
                geom::material& Mat     = RigidGeom.m_pMaterial[SubMesh.iMaterial];
                switch ( Mat.Type )
                {
                default:
                    ASSERTS( 0, xfs("Trying to Raycast an unknown material type %d", (s32)Mat.Type) );
                    break;

                case Material_Diff:
                case Material_Diff_PerPixelEnv:
                case Material_Diff_PerPixelIllum:
                    // Since we don't do texture lookups, it also doesn't
                    // make sense for punch-through to cast shadows either.
                    if ( Mat.Flags & geom::material::FLAG_IS_PUNCH_THRU )
                    {
                        CastShadow = FALSE;
                    }
                    break;

                case Material_Alpha:
                case Material_Alpha_PerPolyEnv:
                case Material_Alpha_PerPixelIllum:
                case Material_Alpha_PerPolyIllum:
                case Material_Distortion:
                case Material_Distortion_PerPolyEnv:
                    CastShadow = FALSE;
                    break;
                }

                s32     IndexVOffset;
                u16*    pIndex = (u16*)render::LockRigidDListIndex( Info.pRigidInst->GetInst(), iSubMesh, IndexVOffset );
                for ( s32 j = 0; j < DList.nIndices; j++ )
                {
                    const s32   iFacet = j;
                    vector3     P[3];
                    vector3     N[3];

                    // Go throw all the indices/facets
                    for( s32 v=0; v<3; v++, j++ )
                    {
                        s32     iV = pIndex[j] - IndexVOffset;

                        ASSERT( iV < DList.nVerts );
                        ASSERT( iV >= 0 );

                        P[v] = L2W * pVertex[ iV ].Pos;
                        N[v] = L2W.RotateVector( pVertex[ iV ].Normal );
                    }

                    // After v we are too far sice we are about to increment in the next loop
                    j--;

                    // Add the actual facet into the system
                    s_RayCastSystem.AddFacet( 
                        P[0], N[0], P[1], N[1], P[2], N[2], 
                        SlotID, 
                        CastShadow ? raycast_lighting::FLAG_CAST_SHADOW : 0,
                        iSubMesh, iFacet );
                }
                
                render::UnlockRigidDListIndex( Info.pRigidInst->GetInst(), iSubMesh );
            }
            break;
     
        case LIGHTING_ZONE:
            {
                xcolor ZoneColor = GetZoneColor( Info.pObject->GetZone1(), Info.pObject->GetZone2() );
                for ( iVert = 0; iVert < DList.nVerts; iVert++ )
                {
                    pVertex[iVert].Color = ZoneColor;
                }
            }
            break;
        
        default :
        
            x_throw( "Unknown lighting mode!" );
            break;
        }

        // unlock the vertex buffer
        render::UnlockRigidDListVertex( Info.pRigidInst->GetInst(), iSubMesh );
    }
}

//=============================================================================

void lighting_LightObjects( platform            Platform,
                            const xarray<guid>& lGuid,
                            s32                 Mode )
{
    s32 i;

    if (g_WorldEditor.m_pHandler) g_WorldEditor.m_pHandler->SetProgressRange(0,lGuid.GetCount());

    //
    // If we are going to do the zone lighting, then build the zone colors
    //
    if( Mode == LIGHTING_ZONE )
    {
        InitZoneColors();
    }

    //
    // If we are going to do the raycast lighting make sure that we are ready for it
    //
    if( Mode == LIGHTING_RAYCAST )
    {
        s_RayCastSystem.Clear();
    }

    //
    // Light objects
    //
    for( i=0; i<lGuid.GetCount(); i++ )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( lGuid[i] );
        
        if (g_WorldEditor.m_pHandler) g_WorldEditor.m_pHandler->SetProgress(i);

        if( pObject )
        {
            lighting_LightObject( Platform, lGuid[i], pObject->GetL2W(), Mode );
        }
    }

    //
    // Okay get ready to compute the ray casting
    //
    if( Mode == LIGHTING_RAYCAST )
    {
        // First lets add all the lights 
        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_LIGHT );
        for( ; SlotID != SLOT_NULL; SlotID = g_ObjMgr.GetNext( SlotID ) )
        {
            object*    pLight      = g_ObjMgr.GetObjectBySlot( SlotID );
            light_obj& LightObject = light_obj::GetSafeType( *pLight );

           // Filter the dynamic lights
            if( pLight->IsKindOf( dynamic_light_obj::GetRTTI() ) )
            {
                continue;
            }
             
		    vector3 P   ( LightObject.GetPosition() );
		    xcolor  C   ( LightObject.GetColor()    );
		    f32     R   ( LightObject.GetRadious()  );
            xcolor  A   ( LightObject.GetAmbient()  );

            s_RayCastSystem.AddLight( P, R, C, A, LightObject.GetLightIntensity() );
        }

        // Now we must compile the data. (Such merging vertex, etc)
        s_RayCastSystem.CompileData();

        // Now we compute the hold lighting
        s_RayCastSystem.ComputeLighting();

        //
        // We must be done so we must start collecting the vertex color for each facet
        //
        u16*                    pIndex;
        rigid_geom::vertex_pc*  pVertex;
        s32                     IndexVOffset;
        render::hgeom_inst      hInstCache    = HNULL;
        u32                     iSubMeshCache = 0xffffffff;
        guid                    GuidCache     = 0;

        for( s32 i=0; i<s_RayCastSystem.GetFacetCount(); i++ )
        {
            s32     Index[3];
            guid    Guid;
            u32     UserData[2];
            s32     iFacet;
            s32     SlotID;


            s_RayCastSystem.GetFacetData( i, Index, SlotID, UserData );

            object* pObj = g_ObjMgr.GetObjectBySlot( SlotID );
            if( pObj ) Guid = pObj->GetGuid();
            else       Guid = 0;

            iFacet = UserData[1];

            if( GuidCache != Guid || iSubMeshCache != UserData[0] )
            {
                info Info;
                if( GetInfo( Platform, Guid, Info ) == FALSE )
                    continue;

                // Invalidate previous cache (If this is not the first time)
                if( GuidCache )
                {
                    ASSERT( hInstCache.IsNonNull() );
                    render::UnlockRigidDListVertex( hInstCache, iSubMeshCache );
                    render::UnlockRigidDListIndex ( hInstCache, iSubMeshCache );
                }

                // Set the cache entries
                hInstCache    = Info.pRigidInst->GetInst();
                iSubMeshCache = UserData[0];
                GuidCache     = Guid;

                // Lock vertex and Index buffers
                pVertex = (rigid_geom::vertex_pc*)render::LockRigidDListVertex( hInstCache, iSubMeshCache );
                pIndex  = (u16*)render::LockRigidDListIndex( hInstCache, iSubMeshCache, IndexVOffset );
            }

            //
            // Light each of the vertices of the facet
            //
            for( s32 v=0; v<3; v++ )
            {
                s32            iV = pIndex[iFacet + v] - IndexVOffset;
                pVertex[iV].Color = s_RayCastSystem.GetVertexColor( Index[v] );
            }
        }

        // Tell raycast system it is finished
        s_RayCastSystem.Clear();

        //
        // Make sure that the last dlist gets also unlock
        //
        if( GuidCache )
        {
            render::UnlockRigidDListVertex( hInstCache, iSubMeshCache );
            render::UnlockRigidDListIndex ( hInstCache, iSubMeshCache );
        }
    }
}

//=============================================================================

static s32 LoadPlatformGeom( platform Platform, const info& InfoPC, xarray<rigid_geom*>& lRigidGeom )
{
    s32 nColors = 0;

    switch( Platform )
    {
        case PLATFORM_XBOX :
        {
            const char* pFileNamePC = InfoPC.pRigidInst->GetRigidGeomName();
            if( pFileNamePC == NULL )
                x_throw( "Unable to get Rigid Geom name" );

            char pFileName[256];
            x_splitpath( pFileNamePC, NULL, NULL, pFileName, NULL );
            
            rigid_geom* pRigidGeomXbox = NULL;

            // Load the Xbox Rigid Geom
            xstring FileNameXbox( xfs( "%s\\Xbox\\%s.rigidgeom", g_Settings.GetReleasePath(), pFileName ) );
            fileio File;
            File.Load( FileNameXbox, pRigidGeomXbox );
            if( pRigidGeomXbox == NULL )
                x_throw( xfs( "Unable to load Rigid Geom [%s]", (const char*)FileNameXbox ) );

            lRigidGeom.Append() = pRigidGeomXbox;
            rigid_geom& RigidGeom = *pRigidGeomXbox;

            // Count the number of colors used
            for( s32 i=0; i<RigidGeom.m_nSubMeshes; i++ )
            {
                geom::submesh         & GeomSubMesh = RigidGeom.m_pSubMesh[i];
                rigid_geom::dlist_xbox& DList       = RigidGeom.m_System.pXbox[ GeomSubMesh.iDList ];
                nColors += DList.nVerts;
            }
            break;
        }

        case PLATFORM_PS2 :
        {
            // Convert PC Geom filename to PS2
            const char* pFileNamePC = InfoPC.pRigidInst->GetRigidGeomName();
            
            if( pFileNamePC == NULL )
                x_throw( "Unable to get Rigid Geom name" );

            char pFileName[256];
            x_splitpath( pFileNamePC, NULL, NULL, pFileName, NULL );
            
            rigid_geom* pRigidGeomPS2 = NULL;

            // Load the PS2 Rigid Geom
            xstring FileNamePS2( xfs( "%s\\PS2\\%s.rigidgeom", g_Settings.GetReleasePath(), pFileName ) );
            fileio File;
            File.Load( FileNamePS2, pRigidGeomPS2 );

            if( pRigidGeomPS2 == NULL )
                x_throw( xfs( "Unable to load Rigid Geom [%s]", (const char*)FileNamePS2 ) );

            lRigidGeom.Append() = pRigidGeomPS2;
            rigid_geom& RigidGeom = *pRigidGeomPS2;
            
            // Count the number of colors used
            for( s32 i=0; i<RigidGeom.m_nSubMeshes; i++ )
            {
                geom::submesh&         GeomSubMesh = RigidGeom.m_pSubMesh[i];
                rigid_geom::dlist_ps2& DList       = RigidGeom.m_System.pPS2[ GeomSubMesh.iDList ];
                
                nColors += ALIGN_16( sizeof( u16 ) * DList.nVerts ) / sizeof( u16 );
            }
            break;
        }

        case PLATFORM_PC :
            nColors += InfoPC.pGeom->GetNVerts();
            break;
    }

    return( nColors );
}

//=============================================================================

static void UnloadAllPlatformGeom( const xarray<rigid_geom*>& lRigidGeom )
{
    for( s32 i=0; i<lRigidGeom.GetCount(); i++ )
    {
        delete lRigidGeom[i];
    }
}

//=============================================================================

static s32 CopyColorPS2( const rigid_geom::vertex_pc& VertexPC, const rigid_geom::dlist_ps2& DList, u16* pCol )
{
    s32 nColorWrites = 0;
    
    // Look for PC vertex in PS2 display list
    for( s32 i=0; i<DList.nVerts; i++ )
    {
        if( (VertexPC.Pos.X == DList.pPosition[i].GetX()) &&
            (VertexPC.Pos.Y == DList.pPosition[i].GetY()) &&
            (VertexPC.Pos.Z == DList.pPosition[i].GetZ()) )
        {
            s8 X = (s8)(VertexPC.Normal.X * (0xff>>1));
            s8 Y = (s8)(VertexPC.Normal.Y * (0xff>>1));
            s8 Z = (s8)(VertexPC.Normal.Z * (0xff>>1));
            
            if( (X == DList.pNormal[ (i*3)+0 ]) &&
                (Y == DList.pNormal[ (i*3)+1 ]) &&
                (Z == DList.pNormal[ (i*3)+2 ]) )
            {
                u8 R = (VertexPC.Color.R >> 3) & 0x1F;
                u8 G = (VertexPC.Color.G >> 3) & 0x1F;
                u8 B = (VertexPC.Color.B >> 3) & 0x1F;

                // 128 is 1.0 on PS2
                // R >>= 1;
                //G >>= 1;
                //B >>= 1;
                
                // Copy color to Color Table
                pCol[i] = 0x8000 | (B << 10) | (G << 5) | R;
                
                nColorWrites++;
            }
        }
    }
    
    return( nColorWrites );
}

//=============================================================================

static s32 CopyColors( platform Platform, const info& InfoPC, const rigid_geom* pRigidGeom, void* pColBase )
{
    s32 nColors = 0;

    rigid_geom& RigidGeomPC = *(rigid_geom*)InfoPC.pGeom;

    switch( Platform )
    {
        case PLATFORM_XBOX :
        {
            D3DCOLOR* pCol=( D3DCOLOR* )pColBase;

            for( s32 i=0; i<RigidGeomPC.m_nSubMeshes; i++ )
            {
                // Get the PC display list
                geom::submesh&        GeomSubMesh = RigidGeomPC.m_pSubMesh[i];
                rigid_geom::dlist_pc& DListPC     = RigidGeomPC.m_System.pPC[ GeomSubMesh.iDList ];

                // Get the instance handle
                render::hgeom_inst hInst = InfoPC.pRigidInst->GetInst();

                // Get the vertex buffer
                rigid_geom::vertex_pc* pVertex = (rigid_geom::vertex_pc*)render::LockRigidDListVertex( hInst, i );

                // Loop through all the vertices in the display list
                for( s32 j=0; j<DListPC.nVerts; j++ )
                {
                    xcolor& C = pVertex[j].Color;

                   *pCol = D3DCOLOR_RGBA( C.R,C.G,C.B,C.A );
                    pCol++;
                }

                nColors += DListPC.nVerts;

                render::UnlockRigidDListVertex( hInst, i );
            }
            
            ASSERT( nColors == RigidGeomPC.GetNVerts() );
            break;
        }

        case PLATFORM_PS2 :
        {
            u16* pCol=( u16* )pColBase;

            const rigid_geom& RigidGeomPS2 = *pRigidGeom;
        
            for( s32 i=0; i<RigidGeomPS2.m_nSubMeshes; i++ )
            {
                // Get the instance handle
                render::hgeom_inst hInst = InfoPC.pRigidInst->GetInst();
                geom::submesh&         GeomSubMesh = RigidGeomPS2.m_pSubMesh[i];
                rigid_geom::dlist_pc&  DListPC     = RigidGeomPC.m_System.pPC  [ GeomSubMesh.iDList ];
                rigid_geom::dlist_ps2& DListPS2    = RigidGeomPS2.m_System.pPS2[ GeomSubMesh.iDList ];

                // Get the vertex buffer
                rigid_geom::vertex_pc* pVertex = (rigid_geom::vertex_pc*)render::LockRigidDListVertex( hInst, i );

                s32 nColorWrites = 0;

                // Copy each PC vertex color into the PS2 color table.
                for( s32 j=0; j<DListPC.nVerts; j++ )
                {
                    nColorWrites += CopyColorPS2( pVertex[j], DListPS2, pCol + nColors );
                }
                if( nColorWrites < DListPS2.nVerts )
                    x_DebugMsg(xfs("Color count < number of vertices:   %s\n",pRigidGeom->GetMeshName( 0 ) ) );

                nColors += ALIGN_16( sizeof( u16 ) * DListPS2.nVerts ) / sizeof( u16 );

                render::UnlockRigidDListVertex( hInst, i );
            }

            ASSERT( nColors >= RigidGeomPS2.GetNVerts() );
            break;
        }

        case PLATFORM_PC :
        {
            u16* pCol=( u16* )pColBase;

            for( s32 i=0; i<RigidGeomPC.m_nSubMeshes; i++ )
            {
                // Get the PC display list
                geom::submesh&        GeomSubMesh = RigidGeomPC.m_pSubMesh[i];
                rigid_geom::dlist_pc& DListPC     = RigidGeomPC.m_System.pPC[ GeomSubMesh.iDList ];

                // Get the instance handle
                render::hgeom_inst hInst = InfoPC.pRigidInst->GetInst();
                
                // Get the vertex buffer
                rigid_geom::vertex_pc* pVertex = (rigid_geom::vertex_pc*)render::LockRigidDListVertex( hInst, i );
            
                // Loop through all the vertices in the display list
                for( s32 j=0; j<DListPC.nVerts; j++ )
                {
                    xcolor& C = pVertex[j].Color;
                    
                    u8 R = (C.R >> 3) & 0x1F;
                    u8 G = (C.G >> 3) & 0x1F;
                    u8 B = (C.B >> 3) & 0x1F;
                    
                    // Copy color to Color Table
                    *pCol = (B << 10) | (G << 5) | R;
                    pCol++;
                }
            
                nColors += DListPC.nVerts;
                
                render::UnlockRigidDListVertex( hInst, i );
            }
            
            ASSERT( nColors == RigidGeomPC.GetNVerts() );
            break;
        }
    }

    return( nColors );
}

//=============================================================================

void lighting_ExportTo3DMax( const xarray<guid>& lGuid, const char* pFileName )
{
    ASSERT( pFileName );

    s32 i;
    X_FILE* Fp = x_fopen( xfs("C:\\%s", pFileName ), "wt" );
    if( Fp == NULL ) x_throw( xfs("Unable to open file (%s)", pFileName) );

    s32 nObjects=0;
    for( i=0; i<lGuid.GetCount(); i++ )
    {
        info Info;
        if ( GetInfo( PLATFORM_PC, lGuid[i], Info ) == FALSE )
            continue;

        object* pObject = g_ObjMgr.GetObjectByGuid( lGuid[i] );
        if( pObject )nObjects++;
    }

    x_fprintf( Fp, "Object Count: %d \n", nObjects );

    nObjects = 0;
    for( s32 k=0; k<lGuid.GetCount(); k++ )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( lGuid[k] );
        
        if (g_WorldEditor.m_pHandler) g_WorldEditor.m_pHandler->SetProgress(k);

        if( pObject )
        {
            info Info;

            if ( GetInfo( PLATFORM_PC, lGuid[k], Info ) == FALSE )
                continue;

            const char* pObjectName = Info.pRigidInst->GetRigidGeomName();

            x_fprintf( Fp, "\nObject: (%d)%s\n", nObjects, pObjectName?pObjectName:"UNKNOWN" );
            nObjects++;

            rigid_geom& RigidGeom = *(rigid_geom*)Info.pGeom;

            matrix4 L2W = pObject->GetL2W();

            //
            // Count Materials 
            //
            x_fprintf( Fp, "Materials: %d\n", RigidGeom.m_nSubMeshes );

            //
            // Print out materials
            //
            for( i=0; i<RigidGeom.m_nSubMeshes; i++ )
            {
                rigid_geom::submesh&  GeomSubMesh = RigidGeom.m_pSubMesh[i];

                char Drive[256];
                char Dir[256];
                char FileName[256];
                const char* pTextureName = RigidGeom.GetTextureName( RigidGeom.m_pMaterial[GeomSubMesh.iMaterial].iTexture );
                x_splitpath( pTextureName, Drive, Dir, FileName, NULL );
                x_fprintf( Fp, "%s%s%s.tga\n", Drive, Dir, FileName );
            }

            //
            // Count facets
            //
            s32 nFacets = 0;
            for( i=0; i<RigidGeom.m_nSubMeshes; i++ )
            {
                rigid_geom::submesh&  GeomSubMesh = RigidGeom.m_pSubMesh[i];
                rigid_geom::dlist_pc& DList       = RigidGeom.m_System.pPC[ GeomSubMesh.iDList ];
                nFacets += DList.nIndices/3;
            }
            x_fprintf( Fp, "Facets: %d\n", nFacets );

            //
            // Print out facets
            //
            for( i=0; i<RigidGeom.m_nSubMeshes; i++ )
            {
                // Get the Display List
                rigid_geom::submesh&  GeomSubMesh = RigidGeom.m_pSubMesh[i];
                rigid_geom::dlist_pc& DList       = RigidGeom.m_System.pPC[ GeomSubMesh.iDList ];
        
                // Get the vertex buffer
                render::hgeom_inst hInst = Info.pRigidInst->GetInst();
                rigid_geom::vertex_pc* pVertex = 
                    (rigid_geom::vertex_pc*)render::LockRigidDListVertex( hInst, i );
                s32 j;

                // Go throw all the facets
                s32     IndexVOffset;
                u16*    pIndex = (u16*)render::LockRigidDListIndex( hInst, i, IndexVOffset );

                for( j=0; j<DList.nIndices; j++ )
                {
                    const s32   iFacet = j;
                    vector3     P[3];
                    vector3     N[3];

                    // Go throw all the indices/facets
                    for( s32 v=0; v<3; v++, j++ )
                    {
                        s32     iV = pIndex[j] - IndexVOffset;

                        ASSERT( iV < DList.nVerts );
                        ASSERT( iV >= 0 );

                        P[v] = L2W * pVertex[ iV ].Pos;
                        N[v] = L2W.RotateVector( pVertex[ iV ].Normal );

                        x_fprintf( Fp, " %f %f %f ", P[v].GetX(), P[v].GetY(), P[v].GetZ() );
                        x_fprintf( Fp, " %f %f %f ", N[v].GetX(), N[v].GetY(), N[v].GetZ() );
                        x_fprintf( Fp, " %d %d %d ", MIN(255,pVertex[ iV ].Color.R*2), MIN(255,pVertex[ iV ].Color.G*2), MIN(255,pVertex[ iV ].Color.B*2) );
                        x_fprintf( Fp, " %f %f ",    pVertex[ iV ].UV.X, pVertex[ iV ].UV.Y );
                    }

                    x_fprintf( Fp, " %d  ", i );
                    x_fprintf( Fp, "\n" );

                    // After v we are too far sice we are about to increment in the next loop
                    j--;
                }
                
                render::UnlockRigidDListIndex( hInst, i );
                render::UnlockRigidDListVertex( hInst, i );
            }
        }
    }

    x_fclose( Fp );
}


//=============================================================================

void lighting_CreateColorTable( platform Platform, const xarray<guid>& lGuid, const char* pFileName )
{
    xarray<info>        lInfo;
    xarray<rigid_geom*> lRigidGeom;

    s32 TotalColors = 0;
    s32 i;

    // Collect information about the objects
    for( i=0; i<lGuid.GetCount(); i++ )
    {
        info Info;
        if( GetInfo( Platform, lGuid[i].Guid, Info ) == FALSE )
            continue;

        // Load the destination platform Rigid Geom
        s32 nColors = LoadPlatformGeom( Platform, Info, lRigidGeom );
        if( nColors== 0 )
        {
            UnloadAllPlatformGeom( lRigidGeom );
            x_throw( "No colors were found" );
        }
        
        TotalColors += nColors;
        
        lInfo.Append() = Info;
    }

    // Allocate memory for the color table
    color_info RigidColor;
    {
        RigidColor.SetCount( TotalColors );
        switch( Platform )
        {
            case PLATFORM_XBOX:
                RigidColor.Set( new u32[ TotalColors ] );
                break;
            case PLATFORM_PS2:
            case PLATFORM_PC:
                RigidColor.Set( new u16[ TotalColors ] );
                break;

            default:
                ASSERT(0);
                break;                    
        }
    }

    void* pCol = RigidColor;
    if( ! pCol )
    {
        UnloadAllPlatformGeom( lRigidGeom );
        x_throw( "Out of memory" );
    }

    s32 iColor = 0;
    // Build the color table
    for( i=0; i<lInfo.GetCount(); i++ )
    {
        info& Info  = lInfo[i];
        s32 nColors = 0;

        // PC Version doesnt have any loaded Geom's for other platforms
        if( lRigidGeom.GetCount  ( ))
            nColors  = CopyColors( Platform,
                                  Info,
                                  lRigidGeom[i],
                                  pCol );
        else
            nColors = CopyColors( Platform,
                                  Info,
                                  NULL,
                                  pCol );

        // NULL because we're just building it, and this is only a temp ptr
        Info.pRigidInst->SetColorTable( NULL, iColor, nColors );

        iColor += nColors;

        switch( Platform )
        {
            case PLATFORM_XBOX:
                pCol = ((u32*)RigidColor)+iColor;
                break;
            case PLATFORM_PS2:
            case PLATFORM_PC:
                pCol = ((u16*)RigidColor)+iColor;
                break;
        }
    }

    // Save the Rigid Color Table
    fileio File;
    File.Save( pFileName, RigidColor, FALSE );
    
    // Free up any allocated memory
    UnloadAllPlatformGeom( lRigidGeom );

    switch( Platform )
    {
        case PLATFORM_XBOX:
            ASSERT( pCol == ((u32*)RigidColor)+TotalColors );
            delete[](u32*)RigidColor;
            break;
        case PLATFORM_PS2:
        case PLATFORM_PC:
            ASSERT( pCol == ((u16*)RigidColor)+TotalColors );
            delete[](u16*)RigidColor;
            break;
    }
}

//=============================================================================

void lighting_CreatePlaySurfaceColors( platform Platform, const xarray<guid>& lGuid )
{
    delete []s_pPlaySurfaceColors;
    s_pPlaySurfaceColors = NULL;

    xarray<info>        lInfo;
    xarray<rigid_geom*> lRigidGeom;

    s32 TotalColors = 0;
    s32 i;

    // Collect information about the objects
    for( i=0; i<lGuid.GetCount(); i++ )
    {
        info Info;
        if( GetInfo( Platform, lGuid[i].Guid, Info ) == FALSE )
            continue;

        // Load the destination platform Rigid Geom
        x_try;

        s32 nColors = LoadPlatformGeom( Platform, Info, lRigidGeom );
// CJ:       if( nColors == 0 )
// CJ:        {
// CJ:            UnloadAllPlatformGeom( lRigidGeom );
// CJ:            x_throw( "No colors were found" );
// CJ:        }

        TotalColors += nColors;
        lInfo.Append() = Info;

        x_catch_display;
    }

    // Allocate memory for the color table
    color_info RigidColor;
    {
        RigidColor.SetCount( TotalColors );
        switch( Platform )
        {
            case PLATFORM_XBOX:
                RigidColor.Set( new u32[ TotalColors ] );
                x_memset( RigidColor, 0, TotalColors * sizeof(u32)  );
                break;
            case PLATFORM_PS2:
            case PLATFORM_PC:
                RigidColor.Set( new u16[ TotalColors ] );
                x_memset( RigidColor, 0, TotalColors * sizeof(u16)  );
                break;

            default:
                ASSERT(0);
        }
    }

    s_pPlaySurfaceColors = RigidColor;
    void* pCol = RigidColor;
    if( ! pCol )
    {
        UnloadAllPlatformGeom( lRigidGeom );
        x_throw( "Out of memory" );
    }

    s32 iColor = 0;
    
    // Build the color table
    for( i=0; i<lInfo.GetCount(); i++ )
    {
        info& Info  = lInfo[i];
        s32 nColors = 0;

        // PC Version doesnt have any loaded Geom's for other platforms
        if( lRigidGeom.GetCount() == 0 )
        {
            nColors = CopyColors( Platform, Info, NULL, pCol );
        }
        else
        {
            nColors = CopyColors( Platform, Info, lRigidGeom[i], pCol );
        }
        
        // s_pPlaySurfaceColors is only temporary, we'll need to clear the
        // color table later to be safe...
        Info.pRigidInst->SetColorTable( s_pPlaySurfaceColors, iColor, nColors );
        
        iColor += nColors;

        switch( Platform )
        {
            case PLATFORM_XBOX:
                pCol = ((u32*)RigidColor)+iColor;
                break;
            case PLATFORM_PS2:
            case PLATFORM_PC:
                pCol = ((u16*)RigidColor)+iColor;
                break;
        }
    }

    switch( Platform )
    {
        case PLATFORM_XBOX:
            ASSERT( pCol == ((u32*)RigidColor)+TotalColors );
            break;
        case PLATFORM_PS2:
        case PLATFORM_PC:
            ASSERT( pCol == ((u16*)RigidColor)+TotalColors );
            break;
    }
}

//=============================================================================

void lighting_KillPlaySurfaceColors( platform Platform, const xarray<guid>& lGuid )
{
    xarray<info>        lInfo;

    s32 i;

    // Collect information about the objects
    for( i=0; i<lGuid.GetCount(); i++ )
    {
        info Info;
        if( GetInfo( Platform, lGuid[i].Guid, Info ) == FALSE )
            continue;

        lInfo.Append() = Info;
    }

    // Clear the color tables
    for( i=0; i<lInfo.GetCount(); i++ )
    {
        info& Info  = lInfo[i];
        Info.pRigidInst->SetColorTable( NULL, 0, 0 );
    }

    // now we can delete the temp data
    delete []s_pPlaySurfaceColors;
    s_pPlaySurfaceColors = NULL;
}

//=============================================================================
