//=============================================================================
//
//  XBOX Specific Shader Management Routines
//
//=============================================================================


#include "../lightmgr.hpp"
#include "../platform_render.hpp"
#include "entropy/xbox/xbox_private.hpp"
#include <xgraphics.h>

#include "xbox_render.hpp"
#include "../LeastSquares/LeastSquares.hpp"



//=============================================================================
//=============================================================================
// Shader manager methods
//=============================================================================
//=============================================================================



//=============================================================================

static DWORD s_dwRigidDesc[]=
{
    D3DVSD_STREAM( 0 ) ,
    D3DVSD_REG   ( 0, D3DVSDT_FLOAT3      ) , // v0-position
    D3DVSD_REG   ( 1, D3DVSDT_NORMPACKED3 ) , // v1-normal
    D3DVSD_REG   ( 2, D3DVSDT_FLOAT2      ) , // v3-uv
    D3DVSD_STREAM( 1 ),
    D3DVSD_REG   ( 3, D3DVSDT_D3DCOLOR    ) , // colour
    D3DVSD_END( )
};

//=============================================================================

static DWORD s_dwSkinDesc[]=
{
    D3DVSD_STREAM( 0 ) ,
    D3DVSD_REG   ( 0, D3DVSDT_FLOAT3      ) , // v0-position
    D3DVSD_REG   ( 1, D3DVSDT_NORMPACKED3 ) , // v1-normal
    D3DVSD_REG   ( 2, D3DVSDT_FLOAT4      ) , // v2-uv/weights
    D3DVSD_REG   ( 3, D3DVSDT_FLOAT2      ) , // v3-bones
    D3DVSD_END( )
};

//=============================================================================

static vector4 s_FogColour[5];
static f32     s_FogStart;

//////////////////////////////////////////////////////////////////////////////
//
//  MATERIAL DESCRIPTIONS: Only these configurations can be used at runtime.
//  This table saves us from having to build every possible combination.
//
//////////////////////////////////////////////////////////////////////////////

#define SHADER_ROOT "C:\\GameData\\A51\\Release\\Xbox"
#define SIZEOF(label) (sizeof(label)/sizeof(label[0]))
#define ENDL(label) }data[SIZEOF(::s_##label##_Name)];
#define LEAF(label) struct label: public leaf{

//////////////////////////////////////////////////////////////////////////////

struct filedesc
{
    const char* pName;
    const char* pLabel;
    u32 Flags;
};

//////////////////////////////////////////////////////////////////////////////
//
//  The NULL,NULL means pass through, stage ignored.

static const filedesc s_oPos_Name[]={ {        NULL , NULL             ,0 },
                                      // c92 to c95 = L2C .....................
                                      // c00 to c03 = L2W .....................
                                      { "VS0000","oPos_Rigid"          ,0 },
                                      // c92 to c95 = L2C .....................
                                      // c00 to c79 = Matrix palette ..........
                                      { "VS0001","oPos_Skin"           ,0 } };
static const filedesc s_oD0__Name[]={ {        NULL , NULL             ,0 },
                                      // c84 to c87 = Light colours ...........
                                      // c88 to c91 = Light directions ........
                                      { "VS0002","oD0_DirectLight"     ,0 },
                                      // c84 to c89 = Point light pos/rgb pairs
                                      { "VS0003","oD0_PointLight"      ,0 },
                                      { "VS0004","oD0_BlackLight"      ,0 },
                                      { "VS0021","oD0_WhiteLight"      ,0 },
                                      { "VS0005","oD0_Diffuse"         ,0 } };
static const filedesc s_oAmb_Name[]={ {        NULL , NULL             ,0 },
                                      {        NULL, "oD0_Ambient"     ,0 } };
static const filedesc s_oT0__Name[]={ {        NULL , NULL             ,0 },
                                      // c-8 to c-9 = Scrolling UVs ...........
                                      { "VS0007","oT0_Normal"          ,0 },
                                      // c-2 to c-5 = Local to eyespace .......
                                      // c-8 to c-9 = Scrolling UVs ...........
                                      { "VS0008","oT0_Cube"            ,0 },
                                      // c-2 to c-5 = Local to eye ............
                                      // c-8 to c-9 = Scrolling UVs ...........
                                      // c-1        = Special consts ..........
                                      { "VS0009","oT0_Env"             ,0 },
                                      // c-1        = Special consts ..........
                                      // c-2        = Distortion consts .......
                                      // c-3 to c-5 = Local to eye ............
                                      { "VS0017","oT0_Distortion"      ,0 },
                                      // c-1        = Special consts ..........
                                      // c-2        = Intensity consts ........
                                      { "VS0025","oT0_Intensity"       ,0 } };
static const filedesc s_oT3__Name[]={ {        NULL , NULL             ,0 },
                                      // c-33 to c-36 = World to projector ....
                                      // c-32         = Flashlight pos ........
                                      { "VS0010","oT3_Flashlight"      ,0 } };
static const filedesc s_oSt__Name[]={ {        NULL , NULL             ,0 },
                                      // c-28 to c-31 = Shadow projector ......
                                      // c-27         = Shadow colour .........
                                      { "VS0011","oSt_CastShadowRigid" ,0 },
                                      // c 00 to c 79 = Matrix palette ........
                                      // c-28 to c-31 = Shadow projector ......
                                      { "VS0012","oSt_CastShadowSkin"  ,0 } };
static const filedesc s_oSi__Name[]={ {        NULL , NULL             ,0 },
                                      // c-31 to c-28 = Shadow projector ......
                                      // c-26         = UV scale
                                      { "VS0016","oT0_ShadowCreate"    ,0 },
                                      // c-24 to c-26 = Shadow mirroring ......
                                      { "VS0013","oT1_ShadowInsert"    ,0 },
                                      // c-24 to c-26 = Shadow mirroring ......
                                      { "VS0014","oT2_ShadowInsert"    ,0 },
                                      // c-24 to c-26 = Shadow mirroring ......
                                      { "VS0015","oT3_ShadowInsert"    ,0 },
                                      // c-28 to c-31 = Shadow projector ......
                                      { "VS0026","oT1_ShadowProject"   ,0 },
                                      // c-29 to c-32 = Shadow projector ......
                                      // c-33 to c-36 = Shadow projector ......
                                      { "VS0027","oT2_ShadowProject"   ,0 },
                                      // c-1        = Special consts ..........
                                      { "VS0028","oT0_ProjectBack"     ,0 } };
static const filedesc s_oLm__Name[]={ {        NULL , NULL             ,0 },
                                      // c84 to c89 = Light pos/colour ........
                                      // c-3        = Active lights ...........
                                      { "VS0020","oT0_LightMapCreate"  ,0 } };
static const filedesc s_oD1__Name[]={ {        NULL , NULL             ,0 },
                                      { "VS0018","oD1_Fog"             ,0 } };
static const filedesc s_oBlt_Name[]={ {        NULL , NULL             ,0 },
                                      { "VS0019","Blitter"             ,0 } };
/*  Vertex shader namespace.
    This provides a nice clean spot to put my shader tree and
    differentiate it from the pixel shader tree.
    */

#define MAX_SHADERS 256

namespace vs
{
    struct detail
    {
        desc Flags;
        shader Shader;
    }
    Details[MAX_SHADERS];
    u32 OldSize;
    u8* pSpace;
    u32 Count;
    u32 Size;
}

#if COMPILE_SHADERS
// Workspace for compiling shaders
char s_Work[16384];
#endif



//////////////////////////////////////////////////////////////////////////////

static const filedesc s_Mat_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0000","bDiffusePerPixelIllum"        , kTEX0                         },
                                     { "PS0001","bDiffusePerPixelEnvAdd"       , kTEX0 | kTEX1                 },
                                     { "PS0002","bDiffusePerPixelEnv"          , kTEX0 | kTEX1                 },
                                     { "PS0003","bAlphaPerPixelIllum"          , kTEX0                         },
                                     { "PS0004","bAlphaPerPolyIllum"           , kTEX0                         },
                                     { "PS0005","bAlphaPerPolyEnv"             , kTEX0 | kTEX1                 },
                                     { "PS0007","bDiffuse"                     , kTEX0                         },
                                     { "PS0008","bAlpha"                       , kTEX0                         },
                                     { "PS0013","bDiffusePerPixelIllum  bProj" , kTEX0                 | kTEX3 },
                                     { "PS0014","bDiffusePerPixelEnvAdd bProj" , kTEX0 | kTEX1         | kTEX3 },
                                     { "PS0015","bDiffusePerPixelEnv    bProj" , kTEX0 | kTEX1         | kTEX3 },
                                     { "PS0016","bAlphaPerPixelIllum    bProj" , kTEX0                 | kTEX3 },
                                     { "PS0017","bAlphaPerPolyIllum     bProj" , kTEX0                 | kTEX3 },
                                     { "PS0018","bAlphaPerPolyEnv       bProj" , kTEX0 | kTEX1         | kTEX3 },
                                     { "PS0021","bDiffuse               bProj" , kTEX0                 | kTEX3 },
                                     { "PS0022","bAlpha                 bProj" , kTEX0                 | kTEX3 } };
static const filedesc s_Det_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0006","bDetailMap"                   ,                 kTEX2         } };
static const filedesc s_Blr_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0028","bDepthBlur"                   , kTEX0 | kTEX1         | kTEX3 } };
static const filedesc s_Sha_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0024","oT1Shadow"                    , kTEX1                         },
                                     { "PS0025","oT2Shadow"                    ,                 kTEX2         },
                                     { "PS0026","oT3Shadow"                    , kTEX3                         },
                                     { "PS0027","oT0Shadow"                    , kTEX0                         },
                                     { "PS0037","oT1Project"                   , kTEX0 | kTEX1                 },
                                     { "PS0038","oT2Project"                   , kTEX0 | kTEX1 | kTEX2         },
                                     { "PS0042","oT1ProjectToLightMap"         , kTEX0 | kTEX1                 },
                                     { "PS0043","oT1ProjectToLightMap"         , kTEX0 | kTEX1 | kTEX2         } };
static const filedesc s_Dis_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0029","bDistortion"                  , kTEX0                         },
                                     { "PS0036","bDistortionEnv"               , kTEX0 | kTEX1                 } };
static const filedesc s_Ppl_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0032","bPerPixelLit"                 , kTEX0 | kTEX1 | kTEX2         },
                                     { "PS0044","bPerPixelLitFog"              ,                               },
                                     { "PS0040","bPerPixelLitProj"             , kTEX0 | kTEX1 | kTEX2 | kTEX3 } };
static const filedesc s_Ppt_Name[]={ {    NULL , NULL                          , 0                             },
                                     {    NULL ,"bPerPixelLitPunchthru"        ,                         kTEX3 } };
static const filedesc s_BxT_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0033","bRgbaByTex0"                  , kTEX0                         } };
static const filedesc s_Pur_Name[]={ {    NULL , NULL                          , 0                             },
                                     {    NULL , NULL                          , 0                             } };
static const filedesc s_Dec_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0034","bDecal0"                      , kTEX0 | kTEX1                 },
                                     { "PS0035","bDecal1"                      , kTEX0                         } };
static const filedesc s_Glw_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0030","bForcedGlow"                  , 0                             } };
static const filedesc s_Xfc_Name[]={ {    NULL , "xfc_Std"                     , 0                             },
                                     {    NULL , "xfc_Fog"                     , 0                             } };
static const filedesc s_Blt_Name[]={ {    NULL , NULL                          , 0                             },
                                     { "PS0009","bBltPostFx"                   , kTEX0 | kTEX1 | kTEX2 | kTEX3 },
                                     { "PS0020","bBltFinal"                    , kTEX0                         },
                                     { "PS0039","bBltx2"                       , kTEX0 | kTEX1                 },
                                     { "PS0031","bBlt"                         , kTEX0                         } };
/* Pixel shader tree.
   This tree enables very rapid shader searches.
   */

namespace ps
{
    struct detail
    {
        desc Flags;
        shader Shader;
    }
    Details[MAX_SHADERS];
    u32 OldSize;
    u8* pSpace;
    u32 Count;
    u32 Size;
}



///////////////////////////////////////////////////////////////////////////////
//
//  This routine loads up the 136 shader addresses on the GPU. It does so by
//  examining the combination bits, loading shader fragments if necessary,
//  and selecting the proper entry point.
//
///////////////////////////////////////////////////////////////////////////////

static void LoadShader( const char* pFilename )
{
    char* pBuffer; if( pFilename )
    {
    #if 1
        HANDLE SectionHandle = XGetSectionHandle( pFilename );
        ASSERT(SectionHandle != INVALID_HANDLE_VALUE );

        DWORD SectionSize = XGetSectionSize( SectionHandle );
        char* SectionData = (char*)XLoadSectionByHandle( SectionHandle );

        s32 Length = x_strlen( s_Work );
        x_memcpy( s_Work+Length,SectionData,SectionSize );
        XFreeSectionByHandle( SectionHandle );
        x_strcat( s_Work,"\n" );
    #else
        // load shader fragment ***********************************************

        X_FILE* hFile= x_fopen( xfs( "%s\\%s",SHADER_ROOT,pFilename),"r" );
        ASSERT( hFile );
        {
            s32 Size = x_flength( hFile );
            pBuffer = new char[ Size+1 ];

            x_fread( pBuffer,Size,1,hFile );
            pBuffer[Size]=0;
        }
        x_fclose( hFile );

        // concatenate to output buffer ***************************************

        x_strcat( s_Work,pBuffer );
        x_strcat( s_Work,"\n" );
        delete[]pBuffer;
    #endif
    }
}

///////////////////////////////////////////////////////////////////////////

static s32 PSCmp( const void* pl,const void* pr )
{
    return( ((ps::detail*)pl)->Flags.Mask > ((ps::detail*)pr)->Flags.Mask );
}

///////////////////////////////////////////////////////////////////////////

template< class t,class a >s32 search( t& Flags,a* Details,s32 n )
{
    s32 high,i,low;
    for( low=-1,high=n;high-low>1; )
    {
        i = (high+low)/2;
        if( Flags.Mask <= Details[i].Flags.Mask )
            high = i;
        else
            low  = i;
    }
    if( Flags.Mask==Details[high].Flags.Mask )
        return high;
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

static bool GetShader( ps::path& Path,shader*& pShader )
{
    // binary search for shader ***********************************************

    s32 i=search( Path.Flags,ps::Details,ps::Count );
    if( i>-1 )
    {
    #ifdef X_DEBUG
        ASSERT( ps::Details[i].Flags.Mask==Path.Flags.Mask );
        if( i<MAX_SHADERS-1 )ASSERT( ps::Details[i].Flags.Mask != ps::Details[i+1].Flags.Mask );
        if( i>1             )ASSERT( ps::Details[i].Flags.Mask != ps::Details[i-1].Flags.Mask );
    #endif
        pShader = &ps::Details[i].Shader;
        return true;
    }

    // get samplers ***********************************************************

#if COMPILE_SHADERS

    u8 Flags = 0;
    {
        if(Path.iLink[ 0])Flags |= s_Mat_Name[Path.iLink[ 0]].Flags;
        if(Path.iLink[ 1])Flags |= s_Det_Name[Path.iLink[ 1]].Flags;
        if(Path.iLink[ 2])Flags |= s_Blr_Name[Path.iLink[ 2]].Flags;
        if(Path.iLink[ 3])Flags |= s_Sha_Name[Path.iLink[ 3]].Flags;
        if(Path.iLink[ 4])Flags |= s_Dis_Name[Path.iLink[ 4]].Flags;
        if(Path.iLink[ 5])Flags |= s_Glw_Name[Path.iLink[ 5]].Flags;
        if(Path.iLink[ 6])Flags |= s_Blt_Name[Path.iLink[ 6]].Flags;
        if(Path.iLink[ 7])Flags |= s_BxT_Name[Path.iLink[ 7]].Flags;
        if(Path.iLink[ 8])Flags |= s_Pur_Name[Path.iLink[ 8]].Flags;
        if(Path.iLink[ 9])Flags |= s_Dec_Name[Path.iLink[ 9]].Flags;
        if(Path.iLink[10])Flags |= s_Ppl_Name[Path.iLink[10]].Flags;
        if(Path.iLink[12])Flags |= s_Ppt_Name[Path.iLink[12]].Flags;
    }

    // Stitch pixel shader fragments ******************************************

    shader& Shader = ps::Details[ps::Count].Shader;
    {
        x_memset( s_Work,0,sizeof(s_Work) );
        x_strcpy( s_Work,"xps.1.1\n\n" );

        if( SWITCH_USE_TEXTURES )
        {
            // validating tweaks ..............................................

            if( Path.Flags.bFogPunchthru )
                Flags |= kTEX0;
            if( Path.Flags.bTexelModded )
                Flags |= kTEX0;
            if( Path.Flags.bDiffusePerPolyEnvAdd )
            {
                Flags |= kTEX0;
                Flags |= kTEX1;
            }
            if( Path.Flags.bTexelOnly )
            {
                Flags |= kTEX0;
                Flags |= kTEX1;
            }
            if( Path.Flags.oT3LightMap )
                Flags |= kTEX3;
            if( Path.Flags.oT3_Mask )
                Flags |= kTEX3;
            if( Path.Flags.bFogPass )
                Flags |= kTEX0;
            if( Path.Flags.oT3_Mask )
                Flags |= kTEX3;
            if( Path.Flags.oT3_Proj )
            {
                Flags |= kTEX0;
                Flags |= kTEX3;
            }
            if( Path.Flags.bCloth )
            {
                Flags |= kTEX0;
                Flags |= kTEX3;
            }

            // determine sampling .............................................

            if( Flags & kTEX0 ) x_strcat( s_Work,"tex t0\n" );
            if( Flags & kTEX1 ) x_strcat( s_Work,"tex t1\n" );
            if( Flags & kTEX2 ) x_strcat( s_Work,"tex t2\n" );
            if( Flags & kTEX3 ) x_strcat( s_Work,"tex t3\n" );

            Path.ts = Flags;

            // special purpose tweaks .........................................

            if( Path.Flags.oV0_Tint )
                x_strcat( s_Work,"mul v0.rgb,v0.rgb,c11.rgb\n" );
            if( Path.Flags.bTexelModded )
                x_strcat( s_Work,"mul r0,t0,v0\n" );
            if( Path.Flags.bTexelOnly )
                // todo: rename bTexelOnly to bAlphakill or something
                x_strcat( s_Work,"mov r0,t1\n" );
            if( Path.Flags.bShadow )
                x_strcat( s_Work,"mov r0,c0\n" );
            if( Path.Flags.bFogPass )
                x_strcat( s_Work,"mov r0.rgb,c0.rgb + mov r0.a,t0.a\n" );
            if( Path.Flags.bFogNoPpl )
                x_strcat( s_Work,"mov r0.rgb,c0.rgb + mov r0.a,v1.a\n" );
            if( Path.Flags.oT3_Mask )
                x_strcat( s_Work,"mov r0,t3\n" );
            if( Path.Flags.oT3_Proj )
                x_strcat( s_Work,"mov_d2 r0,t3.b\n" );

            // second pass cases ..............................................

            if( Path.Flags.bLitProjPunchthru )
                x_strcat( s_Work,"mov_d2 r0,t3.b\n" );
            if( Path.Flags.bShadowZFog )
                x_strcat( s_Work,"mov r0.rgb,v1_sat + mov r0.a,c0.a\n" );
            if( Path.Flags.bApplyZFog )
                x_strcat( s_Work,"mov r0,v1_sat\n" );
            if( Path.Flags.bCloth )
                x_strcat( s_Work,"mul r0.rgb,v0.rgb,t0.rgb\n" );
            if( Path.Flags.bFogPunchthru )
                x_strcat( s_Work,"mad r0,t0,v0,v1_sat\n" );

            // add shader body ................................................

            if(Path.iLink[ 7])LoadShader( s_BxT_Name[Path.iLink[ 7]].pName ); // must be first
            if(Path.iLink[ 0])LoadShader( s_Mat_Name[Path.iLink[ 0]].pName );
            if(Path.iLink[ 1])LoadShader( s_Det_Name[Path.iLink[ 1]].pName );
            if(Path.iLink[ 2])LoadShader( s_Blr_Name[Path.iLink[ 2]].pName );
            if(Path.iLink[ 4])LoadShader( s_Dis_Name[Path.iLink[ 4]].pName );
            if(Path.iLink[ 5])LoadShader( s_Glw_Name[Path.iLink[ 5]].pName );
            if(Path.iLink[ 6])LoadShader( s_Blt_Name[Path.iLink[ 6]].pName );
            if(Path.iLink[ 8])LoadShader( s_Pur_Name[Path.iLink[ 8]].pName );
            if(Path.iLink[ 9])LoadShader( s_Dec_Name[Path.iLink[ 9]].pName );
            if(Path.iLink[10])LoadShader( s_Ppl_Name[Path.iLink[10]].pName );
            if(Path.iLink[12])LoadShader( s_Ppt_Name[Path.iLink[12]].pName );
            if(Path.iLink[ 3])LoadShader( s_Sha_Name[Path.iLink[ 3]].pName );

            // Diffuse per poly code ..........................................

            if( Path.Flags.bDiffusePerPolyEnvAdd )
            {
                x_strcat( s_Work,"mad r0.rgb,t1.rgb,c2.a,t0.rgb\n" );
            }

            if( Path.Flags.bDiffusePerPolyEnv )
            {
                // TODO: Is this material used? If so, then we need to add
                // support for it.
            }

            // final combiners ................................................

            if( Path.Flags.xfc_Sat )
                x_strcat( s_Work,"mov_x2 r0.rgb,r0.rgb\n" );
            if( Path.Flags.xfc_Half )
                // Halve both rgb and a so post effect doesn't screw up glow
                x_strcat( s_Work,"mov_d2 r0.rgb,r0.rgb\n" );
            if( Path.Flags.bPerPixelLitPunchthru )
                // Needs at least one blend instruction
                x_strcat( s_Work,"mov r0.rgb,t3.rgb + mov r0.a,v1.a\n" );
            else
            {
                if( Path.Flags.xfc_Fog )
                    x_strcat( s_Work,"xfc v1.a,v1,r0,zero,zero,zero,r0.a\n" );
                if( Path.Flags.xfc_Std )
                    x_strcat( s_Work,"xfc zero,zero,r0,zero,zero,zero,r0.a\n" );
            }
        }
        else
        {
            x_strcat( s_Work,"mov r0,v0\n" );
            x_strcat( s_Work,"xfc zero,zero,r0,zero,zero,zero,r0.a\n" );
        }
    }
    Path.compile( s_Work,Shader );

    // add new shader object **********************************************

    ASSERT( ps::Count < MAX_SHADERS );
    ps::Details[ps::Count++].Flags.Mask = Path.Flags.Mask;
    ps::Size += Shader.Microcode.size;
    x_qsort(
        ps::Details,
        ps::Count,
        sizeof( ps::detail ),
        PSCmp
    );
    return GetShader( Path,pShader );
#else
    pShader = NULL;
    return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void GetShader( ps::path& Path )
{
#if COMPILE_SHADERS
    shader* pShader; GetShader( Path,pShader );
#endif
}

///////////////////////////////////////////////////////////////////////////

static s32 CmpShader( const void* pl,const void* pr )
{
    return( ((vs::detail*)pl)->Flags.Mask > ((vs::detail*)pr)->Flags.Mask );
}

///////////////////////////////////////////////////////////////////////////////

static bool GetShader( vs::path& Path,shader*& pShader )
{
    // binary search for shader **********************************************

    s32 i=search( Path.Flags,vs::Details,vs::Count );
    if( i>-1 )
    {
    #ifdef X_DEBUG
        ASSERT( vs::Details[i].Flags.Mask==Path.Flags.Mask );
        if( i<MAX_SHADERS-1 )ASSERT( vs::Details[i].Flags.Mask != vs::Details[i+1].Flags.Mask );
        if( i>1             )ASSERT( vs::Details[i].Flags.Mask != vs::Details[i-1].Flags.Mask );
    #endif
        pShader = &vs::Details[i].Shader;
        return true;
    }

    // build shader script ***************************************************

#if COMPILE_SHADERS

    shader& Shader = vs::Details[vs::Count].Shader;
    {
        x_memset( s_Work,0,sizeof(s_Work) );
        x_strcpy( s_Work,"xvs.1.1\n\n" );

        // main sequence fragments ............................................

        if(Path.iLink[0])LoadShader( s_oPos_Name[Path.iLink[0]].pName );
        if(Path.iLink[6])LoadShader( s_oLm__Name[Path.iLink[6]].pName );
        if(Path.iLink[1])LoadShader( s_oD0__Name[Path.iLink[1]].pName );
        if(Path.iLink[2])LoadShader( s_oT0__Name[Path.iLink[2]].pName );
        if(Path.iLink[3])LoadShader( s_oT3__Name[Path.iLink[3]].pName );
        if(Path.iLink[4])LoadShader( s_oSt__Name[Path.iLink[4]].pName );
        if(Path.iLink[5])LoadShader( s_oSi__Name[Path.iLink[5]].pName );
        if(Path.iLink[7])LoadShader( s_oD1__Name[Path.iLink[7]].pName );
        if(Path.iLink[8])LoadShader( s_oBlt_Name[Path.iLink[8]].pName );

        // handle detail map ..................................................

        if( Path.Flags.bDetailMap )
            x_strcat( s_Work,"mul oT2.xy,v2,c-9.zw\n" );
        if( Path.Flags.bPunchthru || Path.Flags.oT3_Mask )
            x_strcat( s_Work,"mov oT3.xy,v2.xy\n" );
    }
    Path.compile( s_Work,Shader );

    // add new shader object **************************************************

    ASSERT( vs::Count < MAX_SHADERS );
    vs::Details[vs::Count++].Flags.Mask = Path.Flags.Mask;
    vs::Size += Shader.Microcode.size;
    x_qsort(
        vs::Details,
        vs::Count,
        sizeof( vs::detail ),
        PSCmp
    );
    return GetShader( Path,pShader );
#else
    pShader = NULL;
    return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void GetShader( vs::path& Path )
{
#if COMPILE_SHADERS
    shader* pShader; GetShader( Path,pShader );
#endif
}

//////////////////////////////////////////////////////////////////////////////

__declspec( naked )__forceinline s32 __fastcall BitScan( u32 Bits )
{
    __asm
    {
        test ecx,ecx
        jne  ou
             mov  eax,ecx
             ret
    ou: bsf  eax,ecx
        inc  eax
        ret
    }
}



//////////////////////////////////////////////////////////////////////////////

vs::path::path( vs::desc& VSDesc )
{
    iLink[0]=BitScan( VSDesc.oPos       );
    iLink[1]=BitScan( VSDesc.oD0        );
    iLink[2]=BitScan( VSDesc.oT0        );
    iLink[3]=BitScan( VSDesc.oT3        );
    iLink[4]=BitScan( VSDesc.oSt        );
    iLink[5]=BitScan( VSDesc.iShadow    );
    iLink[6]=BitScan( VSDesc.iLightMap  );
    iLink[7]=BitScan( VSDesc.oD1        );
    iLink[8]=BitScan( VSDesc.oBlt       );

    Flags.Mask = VSDesc.Mask;
}



//////////////////////////////////////////////////////////////////////////////

static u32 GetMatID( ps::desc& PSDesc )
{
    return u32( PSDesc.MaterialID<<(PSDesc.bProj*8) );
}

//////////////////////////////////////////////////////////////////////////////

ps::path::path( ps::desc& PSDesc )
{
    id = GetMatID( PSDesc );

    iLink[ 0]=BitScan( id                           );
    iLink[ 1]=BitScan( PSDesc.bDetailMap            );
    iLink[ 2]=BitScan( PSDesc.bDepthBlur            );
    iLink[ 3]=BitScan( PSDesc.iShadow               );
    iLink[ 4]=BitScan( PSDesc.Distortion            );
    iLink[ 5]=BitScan( PSDesc.bForcedGlow           );
    iLink[ 6]=BitScan( PSDesc.BltId                 );
    iLink[ 7]=BitScan( PSDesc.bRgbaByTex0           );
    iLink[ 8]=BitScan( PSDesc.bTexelModded          );
    iLink[ 9]=BitScan( PSDesc.Decal                 );
    iLink[10]=BitScan( PSDesc.LightMap              )&3;
    iLink[12]=BitScan( PSDesc.bPerPixelLitPunchthru );
    iLink[11]=BitScan( PSDesc.XfcId                 );

    ts = 0;
    ts |= s_Mat_Name[iLink[ 0]].Flags;
    ts |= s_Det_Name[iLink[ 1]].Flags;
    ts |= s_Blr_Name[iLink[ 2]].Flags;
    ts |= s_Sha_Name[iLink[ 3]].Flags;
    ts |= s_Dis_Name[iLink[ 4]].Flags;
    ts |= s_Glw_Name[iLink[ 5]].Flags;
    ts |= s_Blt_Name[iLink[ 6]].Flags;
    ts |= s_BxT_Name[iLink[ 7]].Flags;
    ts |= s_Pur_Name[iLink[ 8]].Flags;
    ts |= s_Dec_Name[iLink[ 9]].Flags;
    ts |= s_Ppl_Name[iLink[10]].Flags;
    ts |= s_Ppl_Name[iLink[12]].Flags;

    Flags.Mask = PSDesc.Mask;
}



//////////////////////////////////////////////////////////////////////////////

static void SetShader( vs::desc& VSFlags,XGBuffer& Microcode )
{
    s32 o = vs::Count++;
    s32 i = search( VSFlags,vs::Details,o );
    ASSERT( i==-1 );
    {
        u32* pDesc=
            VSFlags.oPos_Rigid
        ?   (u32*)s_dwRigidDesc
        :   (u32*)s_dwSkinDesc
        ;
        vs::detail& Detail = vs::Details[o];
        Detail.Flags.Mask = VSFlags.Mask;
        Detail.Shader.shader::shader(
            SASMT_VERTEXSHADER,
            Microcode,
            pDesc
        );
        x_qsort(
            vs::Details,
            vs::Count,
            sizeof( vs::detail ),
            CmpShader
        );
    }
}



//////////////////////////////////////////////////////////////////////////////

static void SetShader( ps::desc& PSFlags,XGBuffer& Microcode )
{
    s32 o = ps::Count++;
    s32 i = search( PSFlags,ps::Details,o );
    ASSERT( i==-1 );
    {
        ps::detail& Detail = ps::Details[o];
        Detail.Flags.Mask = PSFlags.Mask;
        Detail.Shader.shader::shader(
            SASMT_PIXELSHADER,
            Microcode,
            NULL
        );
        x_qsort(
            ps::Details,
            ps::Count,
            sizeof( ps::detail ),
            CmpShader
        );
    }
}



///////////////////////////////////////////////////////////////////////////////

shader_mgr::shader_mgr( void )
{
    // clear flags ............................................................

    x_memset( this,0,sizeof( shader_mgr ));

    // Allocate PIP texture ...................................................

    m_PIP_Texture = g_TextureFactory.Create( "kPIP"               , // Resource name
                                             0                    , // Pitch
                                             PropW(700)           , // Width
                                             PropH(700)           , // Height
                                             m_PIP_Target         , // Surface
                                             D3DFMT_LIN_A8R8G8B8 ); // Diffuse buffer format

    m_VRAM_PipID = vram_Register( m_PIP_Texture );
    m_pPipData   = (void*)(m_PIP_Target->Data|0x80000000);

    // load cached shaders ....................................................

    #if !PRELOAD_SHADERS
    {
        X_FILE* hFile;

        // load pixel shader **************************************************

        hFile = x_fopen( xfs( "%s\\shaders.psl",SHADER_ROOT),"r" );
        if( !hFile )
        {
            // blue screen of death: indicates shader not found
            g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA( 0,0,128,0),0.0f,0 );
            g_pd3dDevice->Present( 0,0,0,0 );
        __asm int 3
        }
        else
        {
            // read inventory of details ......................................

            x_fread( &ps::Count,sizeof(u32),1,hFile );
            x_fread( &ps::Size ,sizeof(u32),1,hFile );
            x_fread( ps::Details,sizeof(ps::detail),ps::Count,hFile );

            u32* InfoTable = new u32 [ps::Count*2];
            x_fread( InfoTable,sizeof(u32)*2,ps::Count,hFile );

            ps::pSpace = (u8*)x_malloc(ps::Size);
            x_fread( ps::pSpace,ps::Size,1,hFile );
            x_fclose( hFile );

            // hookup all the shaders .........................................

            u32* pInfo = InfoTable;
            for( u32 i=0;i<ps::Count;i++ )
            {
                XGBuffer Microcode;
                Microcode.pData    = ps::pSpace+(pInfo++)[0];
                Microcode.size     = (pInfo++)[0];
                Microcode.refCount = 0;

                ps::Details[i].Shader.shader::shader(
                    SASMT_PIXELSHADER,
                    Microcode,
                    NULL
                );
            }
            delete[]InfoTable;
        }

        // load vertex shader *************************************************

        hFile = x_fopen( xfs( "%s\\shaders.vsl",SHADER_ROOT),"r" );
        if( !hFile )
        {
            // green screen of death: indicates shader not found
            g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA( 0,128,0,0),0.0f,0 );
            g_pd3dDevice->Present( 0,0,0,0 );
        __asm int 3
        }
        else
        {
            // read inventory of details ......................................

            x_fread( &vs::Count,sizeof(u32),1,hFile );
            x_fread( &vs::Size ,sizeof(u32),1,hFile );
            x_fread( vs::Details,sizeof(vs::detail),vs::Count,hFile );

            u32* InfoTable = new u32 [vs::Count*2];
            x_fread( InfoTable,sizeof(u32)*2,vs::Count,hFile );

            vs::pSpace = (u8*)x_malloc(vs::Size);
            x_fread( vs::pSpace,vs::Size,1,hFile );
            x_fclose( hFile );

            // hookup all the shaders .........................................

            u32* pInfo = InfoTable;
            for( u32 i=0;i<vs::Count;i++ )
            {
                XGBuffer Microcode;
                Microcode.pData    = vs::pSpace+(pInfo++)[0];
                Microcode.size     = (pInfo++)[0];
                Microcode.refCount = 0;

                u32* pDesc =
                    vs::Details[i].Flags.oPos_Rigid
                ?   (u32*)s_dwRigidDesc
                :   (u32*)s_dwSkinDesc
                ;
                vs::Details[i].Shader.shader::shader(
                    SASMT_VERTEXSHADER,
                    Microcode,
                    pDesc
                );
            }
            delete[]InfoTable;
        }
    }
    #endif
}

///////////////////////////////////////////////////////////////////////////////

shader_mgr::~shader_mgr( void )
{
}

///////////////////////////////////////////////////////////////////////////////

s32 shader_mgr::SetPerPixelPointLights( const lights* pLights,const matrix4& L2W )
{
    s32 iResult = 0;
    {
        matrix4 Mtx( L2W );
        Mtx.InvertRT( );

        f32 Enabled[4]={0,0,0,0};

        vector4 Const[6];
        vector4 Color[4];

        x_memset( &Const,0,sizeof( Const ));
        x_memset( &Color,0,sizeof( Color ));

        s32 n = pLights ? pLights->Count:0;
        if( n )
        {
            ASSERT( n<=3 );
            iResult = n;

            for( s32 i=0;i<n;i++ )
            {
                lights::point& Light = pLights->pPoint[i];
                f32 R = Light.Pos.GetW();
                {
                    Light.Pos.GetW() = 1.0f;
                    Const[i*2+0] = Mtx.Transform( Light.Pos );
                    Const[i*2+0].GetW() = R;
                    Light.Pos.GetW() = R;

                    Const[i*2+1].GetX() = 1.0f/(2*R); // 1/2r
                    Const[i*2+1].GetY() = 1.0f/(2*R);
                    Const[i*2+1].GetZ() = 1.0f/(2*R);
                    Const[i*2+1].GetW() = R;
                }
                Color[i] = Light.Col;
                Color[i].GetW()=1.0f;
                Enabled[i] = 1.0f;
            }
        }
        else
        {
            Color[0].GetW()=1.0f;
        }

        g_pd3dDevice->SetVertexShaderConstant( 84,Const  ,6 );
        g_pd3dDevice->SetVertexShaderConstant( -3,Enabled,1 );

        static f32 Scale=0.75f;
        Color[3].GetW()=Scale;

        g_pd3dDevice->SetPixelShaderConstant( 0,Color,4 );
    }
    return iResult;
}

///////////////////////////////////////////////////////////////////////////

s32 shader_mgr::SetPointLights( const lights* pLights,const matrix4& L2W )
{
    ASSERT( pLights );

    s32 iResult = 0;
    {
        matrix4 Mtx( L2W );
        Mtx.InvertRT( );

        vector4 Const[6];
        x_memset( &Const,0,sizeof( Const ));

        f32 Extra[4]=
        {
            0.5f,
            0.0f,
            0.0f,
            1.0f
        };

        s32 n = pLights->Count;
        if( n )
        {
            ASSERT( n<=3 );
            iResult = n;

            for( s32 i=0;i<n;i++ )
            {
                lights::point& Light = pLights->pPoint[i];
                {
                    f32 R = Light.Pos.GetW();
                    Light.Pos.GetW() = 1.0f;

                    Const[i*2+0] = Mtx.Transform( Light.Pos );
                    Const[i*2+0].GetW() = R;
                    Light.Pos.GetW() = R;
                }

                f32 R = Light.Col.GetX();
                f32 G = Light.Col.GetY();
                f32 B = Light.Col.GetZ();

                Const[i*2+1].GetX() = R;
                Const[i*2+1].GetY() = G;
                Const[i*2+1].GetZ() = B;
                Const[i*2+1].GetW() = 1.0f;
            }

            g_pd3dDevice->SetVertexShaderConstant( 84,Const,6 );
            g_pd3dDevice->SetVertexShaderConstant( -1,Extra,1 );
        }
    }
    return iResult;
}

//////////////////////////////////////////////////////////////////////////////
//
//  The matrices for the direction and colour are loaded transposed.

void shader_mgr::SetDirLights( const lights* pLights )
{
    ASSERT( pLights );
    {
        matrix4 Mtx[2];
        x_memset( &Mtx,0,sizeof( Mtx ));

        s32 i,n = pLights->Count;
        ASSERT( n<=4 );

        for( i=0;i<n;i++ )
        {
            lights::dir& Light = pLights->pDir[i];

            Mtx[0](i,0)=Light.Dir.GetX();
            Mtx[0](i,1)=Light.Dir.GetY();
            Mtx[0](i,2)=Light.Dir.GetZ();

            f32 R = Light.Col.GetX();
            f32 G = Light.Col.GetY();
            f32 B = Light.Col.GetZ();

            Mtx[1](0,i)=R;
            Mtx[1](1,i)=G;
            Mtx[1](2,i)=B;
            Mtx[1](3,i)=0.0f;
        }
        g_pd3dDevice->SetVertexShaderConstant( 84,Mtx,8 );
    }
}

///////////////////////////////////////////////////////////////////////////////

void shader_mgr::Begin( void )
{
    // Set big constant mode **************************************************

    g_pd3dDevice->SetShaderConstantMode( D3DSCM_192CONSTANTS  );
    m_bFullControl = TRUE;

    m_VSFlags.clear();
    m_PSFlags.clear();

    // Fog effects( x=0 full fog } ********************************************

    g_RenderState.Set( D3DRS_FOGCOLOR    ,D3DCOLOR_RGBA( 0,0,0,0 ));
    g_RenderState.Set( D3DRS_FOGTABLEMODE,D3DFOG_NONE );
    g_RenderState.Set( D3DRS_FOGENABLE   ,FALSE );
}

//////////////////////////////////////////////////////////////////////////////

void shader_mgr::SetCustomFogPalette( const texture::handle& Texture,xbool bSwitchNow,s32 PaletteIndex )
{
    (void)Texture;
    (void)bSwitchNow;
    (void)PaletteIndex;

    if( SWITCH_USE_FOG )
    {
        s32 i;


        ASSERT( PaletteIndex < 4 && PaletteIndex >= 0 );
        m_FogIndex   = PaletteIndex;

        static const u32 W = 8; 
        static const u32 H = 8; 
        u8 FogColour[W*H*4];

        // If we don't have a valid texture, then fade to zero alpha
        texture* pTex = Texture.GetPointer();
        if( pTex == NULL )
        {
            x_memset( FogColour, 0, sizeof(FogColour) );
        }
        else
        {
            s32 VRAMID = pTex->m_Bitmap.GetVRAMID();
            texture_factory::handle Handle = vram_GetSurface( VRAMID );
            u8* pColor = (u8*)Handle->GetPtr();

            // Otherwise unswizzle the image
            ASSERT( pTex->m_Bitmap.GetWidth ()==W );
            ASSERT( pTex->m_Bitmap.GetHeight()==H );
            XGUnswizzleRect( pColor,W,H,NULL,FogColour,W*4,NULL,4 );
        }

        // now do the transitioning
        u32 ColorTotal[4] = { 0, 0, 0, 0 };
        u8* pSrc = FogColour;
        u8* pDst = m_FogPalette[PaletteIndex];
        if( bSwitchNow )
        {
            for( i = 0; i < W*H*4; i+= 4 )
            {
                s32 j;
                for( j = 0; j < 4; j++ )
                {
                    pDst[j] = pSrc[j];
                    ColorTotal[j] += pDst[j];
                }

                pSrc += 4;
                pDst += 4;
            }
        }
        else
        {
            for( i = 0; i < W*H*4; i+=4 )
            {
                s32 j;
                for( j = 0; j < 4; j++ )
                {
                    if( pSrc[j] > pDst[j] )
                        pDst[j]++;
                    else if( pSrc[j] < pDst[j] )
                        pDst[j]--;
                    ColorTotal[j] += pDst[j];
                }

                pSrc += 4;
                pDst += 4;
            }
        }

        // Store out the average color. Ideally we'd use a polynomial for that, too, but
        // the average color will be much more efficient.
        ColorTotal[0] /= (W*H);
        ColorTotal[1] /= (W*H);
        ColorTotal[2] /= (W*H);
        ColorTotal[3] /= (W*H);
        s_FogColour[PaletteIndex].Set(
            f32(ColorTotal[2])/255.0f,
            f32(ColorTotal[1])/255.0f,
            f32(ColorTotal[0])/255.0f,
            f32(ColorTotal[3])/255.0f );

        // store out the useful constants that will be used in the math below
        static const f32 PS2ZConst = 0.5f * (f32)((s32)1<<19);
        const view* pView = eng_GetView();
        f32 ZNear, ZFar;
        pView->GetZLimits( ZNear, ZFar );

        f32 Numer       = (2.0f * PS2ZConst * ZNear * ZFar) / (ZFar - ZNear);
        f32 DenomOffset = -PS2ZConst + (PS2ZConst * (ZFar + ZNear)) / (ZFar - ZNear);
        f32 XboxQ       = ZFar / (ZFar - ZNear);

        f32 PS2ScreenZ, XboxScreenZ;

        static f32 Scale = 1/16.0f;

        // figure out where fog starts on ps2 and map that into the xbox z range
        f32 PS2ZValue   = (f32)(0xffff) * Scale;
        f32 Denom       = PS2ZValue + DenomOffset;
        ASSERT( x_abs(Denom) > 0.001f );
        f32 ViewZValue = Numer / Denom;
        f32 XBoxZValue = ViewZValue * XboxQ - ZNear * XboxQ;
        ASSERT( ViewZValue > 0.001f );
    //  XBoxZValue /= ViewZValue;
        s_FogStart = XBoxZValue;

        // So for each texel color, figure out where that maps to in the ps2 screen space.
        // Work backwards to figure out the z value in view space, then put it into xbox
        // clip space. This is what we will feed into the polynomial approximation for
        // the fog. f(z) = a + b*z + c*(z^2) + d*(z^3)

        least_squares AlphaApprox;

        AlphaApprox.Setup(3);
        static const f32 NSamples = 512.0f;
        const f32 StepSize = (ZFar-ZNear)/NSamples;
        for( f32 ViewZ = ZNear; ViewZ <= ZFar; ViewZ += StepSize )
        {
            // Calculate Z in PS2 space ***************************************

            PS2ScreenZ = (ViewZ*(ZFar+ZNear)/(ZFar-ZNear));
            PS2ScreenZ -= ((2.0f*ZFar*ZNear)/(ZFar-ZNear));

            PS2ScreenZ /= ViewZ;
            PS2ScreenZ *= -PS2ZConst;
            PS2ScreenZ +=  PS2ZConst;

            // Calculate Z in Xbox space **************************************

            XboxScreenZ = (ViewZ*XboxQ)-(ZNear*XboxQ);
        //  XboxScreenZ /= ViewZ;

            // Get colour from bitmap *****************************************

            f32 A;//,R,G,B;
            u32 UPS2SZ = u32(PS2ScreenZ*16.0f+0.5f);
            if( UPS2SZ <= 0xFFFF )
            {
                u32 ClutIX = (((UPS2SZ >> 8) & 0x000000FF)/4);
                A = f32(m_FogPalette[PaletteIndex][ClutIX*4+3])/255.0f;
            }
            else
            {
                A = 0.0f;
            }

            // Approximations *************************************************
            AlphaApprox.AddSample( XboxScreenZ,A );
        }

        // Solve the least-squares polynomial approximation and store the
        // coefficients out for use in the shader.
        if( !AlphaApprox.Solve() )
        {
            AlphaApprox.SetCoeff( 0, f32(FogColour[3]) / 255.0f );
            AlphaApprox.SetCoeff( 1, 0.0f );
            AlphaApprox.SetCoeff( 2, 0.0f );
            AlphaApprox.SetCoeff( 3, 0.0f );
        }

        m_FogConst[PaletteIndex].Set( AlphaApprox.GetCoeff(0),
                                      AlphaApprox.GetCoeff(1),
                                      AlphaApprox.GetCoeff(2),
                                      AlphaApprox.GetCoeff(3) );
    }
}

//////////////////////////////////////////////////////////////////////////////

void shader_mgr::End( void )
{
    g_pd3dDevice->SetShaderConstantMode( D3DSCM_96CONSTANTS  );
    g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
    g_RenderState.Set( D3DRS_FOGENABLE, FALSE );
    m_bFullControl = FALSE;
    m_PSFlags.clear();
    m_VSFlags.clear();

    for( s32 i=0;i<4;i++ )
        g_Texture.Clear( i );
}

//////////////////////////////////////////////////////////////////////////////

void shader_mgr::SetPixelShader( shader_style Style )
{
    vs::desc VSFlags;
    ps::desc PSFlags;

    VSFlags.clear();
    PSFlags.clear();

    switch( Style )
    {
        case kREMOVE:
            g_pd3dDevice->SetPixelShader( NULL );
            m_PSFlags.clear();
            return;

        case kCAST_SHADOW:
            PSFlags.bShadow = true;
            break;

        case kDISTORT_BACK:
            PSFlags.bDiffuse = true;
            PSFlags.xfc_Std  = true;
            break;
        case kSHADOW_BLUR:
            PSFlags.bTexelModded = true;
            break;

        case kFINISH_FRAME:
            PSFlags.bBltFinal = true;
            break;

        case kT0_MASK_T1:
            PSFlags.bTexelOnly = true;
            PSFlags.xfc_Std    = true;
            break;

        case kFULL_POST_EFFECT:
            PSFlags.bBltPostFx = true;
            break;

        case kDIFFUSE_SAT:
            PSFlags.bBltx2 = true;
            break;

        case kDIFFUSE:
            PSFlags.bBlt = true;
            break;
    }
    Link( 0,NULL,VSFlags,NULL,PSFlags,0 );
}

//////////////////////////////////////////////////////////////////////////////

static vector4 s_C0( 0.0f,0.0f,0.0f,1.0f );
static vector4 s_C1( 0.5f,0.5f,0.5f,0.5f );
static vector4 s_C2( 0.0f,0.0f,0.0f,0.0f );
static vector4 s_C3( 0.0f,0.0f,0.0f,0.0f );
 const vector4 s_C5( 0.0f,0.0f,0.0f,1.0f );

static void LoadConstants( ps::desc PSFlags,f32 FixedAlpha )
{
    // Diffuse consts /////////////////////////////////////////////////////////

    if( PSFlags.bPerPixelLitPunchthru )
    {
        s_C0.GetW()=0.0f;
        const f32 Scalar = 0.5f/255.0f;
        s_C0.GetZ()=0.5f-Scalar; // adjust for rounding error in pixel shader hardware
        s_C1.GetW()=1.0f;
    }
    else
    {
        if( PSFlags.bShadow             ||
            PSFlags.bDiffuse            &&
            ! PSFlags.bPerPixelLitProj  &&
            ! g_pPipeline->m_bAlphaBlend )
        {
            s_C0.GetW()=0.0f;
            s_C0.GetZ()=0.0f;
        }
        else
        {
            s_C0.GetW()=1.0f;
            s_C0.GetZ()=0.0f;
        }
        s_C1.GetW()=0.5f;
    }

    // Alpha fading ///////////////////////////////////////////////////////////

    g_pd3dDevice->SetPixelShaderConstant( 5,s_C5,1 );

    // Env mapping ////////////////////////////////////////////////////////////

    if( !(PSFlags.bDiffusePerPixelEnv || PSFlags.bDiffusePerPixelEnvAdd) && (PSFlags.bAlphaPerPolyEnv) )
    {
        s_C0.GetW()=FixedAlpha;
        s_C0.GetZ()=1.0f;
    }

    if( PSFlags.bDiffusePerPolyEnvAdd )
    {
        f32 Const[4] = { 0.0f,0.0f,0.0f,FixedAlpha };
        g_pd3dDevice->SetPixelShaderConstant( 2,Const,1 );
    }

    // Shadow consts //////////////////////////////////////////////////////////

    if( PSFlags.oT1Shadow || PSFlags.oT2Shadow || PSFlags.oT3Shadow )
    {
        s_C0.GetW()=0.5f;
    }

    g_pd3dDevice->SetPixelShaderConstant( 0,&s_C0,1 );
    g_pd3dDevice->SetPixelShaderConstant( 1,&s_C1,1 );

    // Forced glows ///////////////////////////////////////////////////////////

    if( PSFlags.bForcedGlow )
    {
        static f32 Ambient[4]={ 1.0f,1.0f,1.0f,1.0f };
        g_pd3dDevice->SetPixelShaderConstant( 6,Ambient,1 );
        g_RenderState.Set(
            D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_ALL
        );
    }

    // Detail map consts //////////////////////////////////////////////////////

    if( PSFlags.bDetailMap )
    {
        static const f32 Int = 4.0f;
        s_C2.GetW()=Int;

    }
}



///////////////////////////////////////////////////////////////////////////////

u32 shader_mgr::Link( vs::desc& VSFlags,ps::desc& PSFlags,bool bAllocTop )
{
    m_CanContinue = true;

    u32 Address = 0;

    // SPEED UP Z-BUFFER WRITES ***********************************************

    if( g_RenderState.Get(D3DRS_COLORWRITEENABLE)==0 )
        m_PSFlags.clear();

    // COMPILE AND/OR LINK VERTEX SHADERS *************************************

    if( VSFlags.Mask )
    {
        // if flags change relink ---------------------------------------------

        if( VSFlags.Mask != m_VSFlags.Mask )
        {
            shader* pShader; if( GetShader( vs::path( VSFlags ),pShader ))
            {
                if( bAllocTop )
                {
                    u32 ShaderSize = *((u32*)pShader->Microcode.pData)>>16;
                    Address = 136-ShaderSize;
                }

                // double paranoid shader verification ........................

                #ifdef X_DEBUG
                {
                s32 i=search( VSFlags,vs::Details,vs::Count );
                ASSERT( vs::Details[i].Flags.Mask==VSFlags.Mask );
                }
                #endif

                // Load stitched shader .......................................

                ASSERT( pShader->Handle );
                g_pd3dDevice->LoadVertexShaderProgram(( const DWORD* )pShader->Microcode.pData,Address );
                g_pd3dDevice->SelectVertexShader( pShader->Handle,Address );
            }
            else
            {
                m_CanContinue = false;
            }
        }

        // upload important constants -----------------------------------------

        static f32 Lum = 0.05f;
        g_pd3dDevice->SetVertexShaderConstant( -50,vector4( Lum,Lum,Lum,0.0f ),1 );

        if( VSFlags.oD0_WhiteLight )
        {
            vector4 Light(
                m_WhiteConst,
                m_WhiteConst,
                m_WhiteConst,
                m_WhiteConst
            );
            g_pd3dDevice->SetVertexShaderConstant( -7,Light,1 );
        }

        // remember these flags -----------------------------------------------

        m_VSFlags.Mask = VSFlags.Mask;
    }
    else
    {
        m_VSFlags.clear();
        g_pd3dDevice->SetVertexShader( NULL );
    }

    // COMPILE AND/OR LINK PIXEL SHADERS **************************************

    if( PSFlags.Mask )
    {
        if( PSFlags.Mask != m_PSFlags.Mask )
        {
            // special force diffuse case -------------------------------------

            if( (!PSFlags.MaterialID) && (PSFlags.bForcedGlow) )
            {
                PSFlags.bDiffuse = true;
            }

            // get pixel shader object ----------------------------------------

            shader* pShader; if( GetShader( ps::path( PSFlags ),pShader ))
            {
                ASSERT( pShader->Handle );
                g_pd3dDevice->SetPixelShader( pShader->Handle );

                // double paranoid shader verification ........................

                #ifdef X_DEBUG
                {
                s32 i=search( PSFlags,ps::Details,ps::Count );
                ASSERT( ps::Details[i].Flags.Mask==PSFlags.Mask );
                }
                #endif
            }
            else
            {
                m_CanContinue = false;
            }

            // always remember flags ------------------------------------------

            m_PSFlags = PSFlags;
        }
        LoadConstants( PSFlags,m_FixedAlpha );
    }
    else
    {
        g_pd3dDevice->SetPixelShader( NULL );
        m_PSFlags.clear();
    }

    // DEFAULT TO DIFFUSE (This is bad) ****************************************

    if( !m_CanContinue )
    {
        // Construct diffuse only material ....................................

        ps::desc PS; PS.clear();
        vs::desc VS; VS.clear();

        g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );

        VS.oPos       = m_VSFlags.oPos;
        VS.oT0_Normal = true;
        PS.bDiffuse   = true;
        PS.xfc_Std    = true;
        
        // Grab shader and link with it .......................................

        Address = Link( VS,PS );
    }
    return Address;
}


///////////////////////////////////////////////////////////////////////////////

u32 shader_mgr::Link( u32 MtxCount,const matrix4* pMtx,vs::desc& VSFlags,const vector4* pAmbient,ps::desc& PSFlags,bool bAllocTop )
{
    u32 Address = Link( VSFlags,PSFlags,bAllocTop );
    if( VSFlags.Mask )
    {
        s32 ConstAddr = 96;
        u32 i;

        for( i=0;i<MtxCount;i++ )
        {
            ConstAddr-=4;
            ASSERT( ConstAddr >= 83 );
            matrix4 Mtx( pMtx[i] );

            g_pd3dDevice->SetVertexShaderConstant( ConstAddr,&Mtx,4 );
        }
        if( !pAmbient )
        {
            const vector4 Void( 1.0f,1.0f,1.0f,1.0f );
            pAmbient = &Void;
        }
        g_pd3dDevice->SetVertexShaderConstant( 83,pAmbient,1 );
    }
    return Address;
}

//=============================================================================

void shader_mgr::ConstSanityCheck( void )
{
//#ifdef X_DEBUG
//    f32 CmpFogTable[4*MAX_FOG_SIZE];
//    g_pd3dDevice->GetVertexShaderConstant( -94,CmpFogTable,MAX_FOG_SIZE );
//    ASSERT( !x_memcmp( CmpFogTable,m_FogConst,4*MAX_FOG_SIZE ));
//#endif
}

//=============================================================================

s32 shader_mgr::InsertShadow( ps::desc& PSFlags,texture_factory::handle Handle )
{
    s32 iShadow;

    // find first unused stage ................................

    ps::path Path( PSFlags );
    {
        if( !(Path.ts & kTEX0) ) iShadow=0; else
        if( !(Path.ts & kTEX1) ) iShadow=1; else
        if( !(Path.ts & kTEX2) ) iShadow=2; else
        if( !(Path.ts & kTEX3) ) iShadow=3; else return 0;
    }

    // add sampler statements .................................................

    g_TextureStageState.Set( iShadow, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( iShadow, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( iShadow, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( iShadow, D3DTSS_ADDRESSW , D3DTADDRESS_CLAMP );
    g_Texture          .Set( iShadow, Handle );

    return iShadow;
}



//=============================================================================
//=============================================================================
// Shader methods
//=============================================================================
//=============================================================================



//////////////////////////////////////////////////////////////////////////////

shader::shader( u32 dwType,XGBuffer& MC,u32* pShaderDesc ): Microcode( MC )
{
    Id   = dwType;
    Size = Microcode.GetBufferSize( );

    switch( Id )
    {
        case SASMT_VERTEXSHADER:
            if( pShaderDesc )
            {
                LPDWORD pdwBuffer=(LPDWORD)Microcode.GetBufferPointer();
                g_pd3dDevice->CreateVertexShader(
                    (DWORD*)pShaderDesc,
                    pdwBuffer,
                    (DWORD*)&Handle,
                    0
                );
            }
            break;

        case SASMT_PIXELSHADER:
        {
            const D3DPIXELSHADERDEF* pBuffer=( const D3DPIXELSHADERDEF* )Microcode.GetBufferPointer();
            g_pd3dDevice->CreatePixelShader( pBuffer,(DWORD*)&Handle );
            break;
        }

        default:
            ASSERT(0);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////

shader::~shader( void )
{
}
