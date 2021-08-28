//=========================================================================
//
//  ANIMCOMPILER.CPP
//
//  TODO: Decompress into tighter formats
//  TODO: Write endian swap
//  TODO: Check endian of bitstream data
//  TODO: Try DCT, Predictors
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include <windows.h>
#include <stdio.h>
#include "animcompiler.hpp"
#include "parsing/tokenizer.hpp"
#include "x_bitstream.hpp"
#include "meshutil/rawanim.hpp"
#include "meshutil/rawanim.hpp"

#define DELETE_DUMMY_BONES

char g_EventTypes[NUM_EVENT_TYPES][EVENT_MAX_STRING_LENGTH+1] =
{
    "Old Event",
    "Do Not Use",
    "Hot Point",
    "Audio",
    "Particle",
    "Generic",
    "Intensity",
    "World Collision",
    "Gravity",
    "Weapon",
    "Pain",
    "Debris",
    "Set Mesh",
    "Swap Mesh",
    "Fade Geometry",
    "Swap Virtual Texture"
};
//=========================================================================
extern xbool g_Verbose;
extern void DisplayAnimCompressStats( const char* pFileName );

extern
void CompressAnimationData( bitstream&          BS, 
                            const anim_key*     pKeys,
                            const anim_bone*    pBone,
                            s32                 nBones,
                            const u32*          pStreamFlags,
                            s32                 TotalFrames,
                            s32                 TotalStreams,
                            s32                 StartFrame,
                            s32                 EndFrame,
                            f32*                pLenOfBonesToVerts,
                            s32&                DecompressedDataSize,
                            s32&                iAnimBoneMin,
                            s32&                iAnimBoneMax );

//=========================================================================

compiler_anim::~compiler_anim( void )
{
}

//=========================================================================

compiler_anim::compiler_anim( void )
{
}

//=========================================================================
// ANIM_COMPILER
//=========================================================================

anim_compiler::anim_compiler( void )
{
    m_Anim = new compiler_anim[MAX_ANIMS];
    m_nAnims = 0;
    m_MaxDistToVerts=NULL;
    
    x_strcpy( m_SourceFile, "Unknown source file" );
    x_strcpy( m_UserName, "Unknown user" );
    x_strcpy( m_ComputerName, "Unknown computer" );
    m_ExportDate        [0] = 0;
    m_ExportDate        [1] = 0;
    m_ExportDate        [2] = 0;
}

//=========================================================================

anim_compiler::~anim_compiler( void )
{
    delete[] m_Anim;
    x_free(m_MaxDistToVerts);
}

//=========================================================================

// *INEV* *SB* - Added for A51
void anim_compiler::SetBindPose( const char* pBindPose )
{
    ASSERT( pBindPose );
    ASSERT( m_nAnims == 0 );

    x_strcpy(m_BindFileName, pBindPose);
    if( m_Bind.Load( pBindPose ) == FALSE ) 
        ThrowError(xfs("Unable to load the bind pose [%s]",pBindPose));

    if( m_BindMesh.Load( pBindPose ) == FALSE ) 
        ThrowError(xfs("Unable to load the bind pose [%s]",pBindPose));

    SetUserInfo( m_Bind );

    if( m_Bind.m_nBones > MAX_ANIM_BONES )
        ThrowError(xfs("Bind pose [%s] has more than %d bones (%d). Can't handle that", pBindPose, MAX_ANIM_BONES, m_Bind.m_nBones));

    SetUserInfo( m_BindMesh );

	if( m_BindMesh.m_nVertices <= 0 )
		ThrowError(xfs("Bind pose [%s] had zero vertices. Re-export with vertices!", pBindPose ));

    x_printf("  Number Of Bones: %d\n", m_Bind.m_nBones );

#ifdef DELETE_DUMMY_BONES
    m_Bind.DeleteDummyBones();
#endif

    ClearUserInfo();

    // SB - ALWAYS Add bind pose animation so we can get at the hot point events etc.
    compiler_anim::params Params ;
    Params.FileName = pBindPose ;
    Params.Name     = "BIND_POSE" ;

    AddAnimation( Params ) ;
}

//=========================================================================

void anim_compiler::AddAnimation( const compiler_anim::params& Params )
{
    ASSERT( x_strlen(Params.FileName)) ;
    ASSERT( x_strlen(Params.Name)) ;
    
    ClearUserInfo();
    
    // Validate name
    if( x_strlen(Params.Name) >= 32 )        
        ThrowError( xfs("The name of the animation [%s] is too long - only 31 characters allowed", Params.Name) );

    // Copy params
    m_Anim[m_nAnims].Params = Params ;

    // Try load
    if( !m_Anim[m_nAnims].RawAnim.Load( m_Anim[m_nAnims].Params.FileName ) )
        ThrowError( xfs("Could not load [%s].\n", m_Anim[m_nAnims].Params.FileName));

    SetUserInfo( m_Anim[m_nAnims].RawAnim );

#ifdef DELETE_DUMMY_BONES
    m_Anim[m_nAnims].RawAnim.DeleteDummyBones();
#endif

    // Next anim
    m_nAnims++;
    if( m_nAnims >= MAX_ANIMS )
        ThrowError( xfs( "Too many animations for the compiler to handle. Talk to a programmer") );
        
    ClearUserInfo();        
}

//=========================================================================

xbool anim_compiler::Compile( const char * pBindPoseFile, const char* pBindMeshName, int nAnims, xstring *AnimNames, xstring *AnimFiles, anim_group& AnimGroup )
{
    if(!Setup( pBindPoseFile, pBindMeshName, nAnims, AnimNames, AnimFiles ))
        return FALSE;
    return Compile(AnimGroup);
}
    
//=========================================================================

xbool anim_compiler::Compile( const char* pInputFile, anim_group& AnimGroup )
{
    if(!Setup(pInputFile))
        return FALSE;
    return Compile(AnimGroup);
}

//=========================================================================

xbool anim_compiler::Setup( const char *pBindPoseFile, const char* pBindMeshName, int nAnims, xstring *AnimNames, xstring *AnimFiles )
{
    x_strcpy(m_BindFileName, pBindPoseFile );
    x_strcpy(m_BindMeshName, pBindMeshName );

    if( !x_strlen(pBindPoseFile) || !m_Bind.Load( m_BindFileName ) )
        return FALSE;

    if( !x_strlen(pBindPoseFile) || !m_BindMesh.Load( m_BindFileName ) )
        return FALSE;

    for(s32 count = 0; count < nAnims; count++)
    {
        // Setup params
        compiler_anim::params Params ;
        Params.Name     = (const char *)AnimNames[count];
        Params.FileName = (const char *)AnimFiles[count];
        m_Anim[count].Params = Params ;

        // Try load anim
        if( !m_Anim[count].RawAnim.Load( m_Anim[count].Params.FileName ) )
        {
            x_DebugMsg("Could not load (%s).\n",m_Anim[count].Params.FileName);
            return FALSE;
        }
        #ifdef AUX_DEBUG_OUTPUT
        x_DebugMsg("%s LOADED.\n",m_Anim[count].Params.FileName);   
        #endif
    }
    m_nAnims = nAnims;
    return TRUE;
}

//=========================================================================

xbool anim_compiler::Setup( const char* pInputFile )
{
    x_strcpy(m_BindMeshName,"NONE");

    //
    // Open description file
    //
    token_stream TOK;
    if( !(x_strlen(pInputFile)) || !TOK.OpenFile( pInputFile ) )
    {
        MessageBox(NULL, TEXT("Could not open anim list file"), TEXT("Error"), MB_OK);            
        return FALSE;
    }

    //
    // Find and load the bind pose
    //
    {
        VERIFY( TOK.Find("BINDPOSE",TRUE) );
        VERIFY( TOK.Read() == token_stream::TOKEN_STRING );
        x_strcpy( m_BindFileName, TOK.String() );

        if( !m_Bind.Load( m_BindFileName ) )
        {
            MessageBox(NULL, TEXT("Could not load bind pose file"), TEXT("Error"), MB_OK);
            return FALSE;
        }
        if( !m_BindMesh.Load( m_BindFileName ) )
        {
            MessageBox(NULL, TEXT("Could not load bind pose file"), TEXT("Error"), MB_OK);
            return FALSE;
        }
        #ifdef DELETE_DUMMY_BONES
        m_Bind.DeleteDummyBones();
        #endif
    }

    // 
    // Loop through animation files
    //
    {
        TOK.Rewind();
        while( TOK.Find("ANIM") )
        {
            ASSERT( m_nAnims < MAX_ANIMS );

            VERIFY( TOK.Read() == token_stream::TOKEN_NUMBER );
            m_Anim[m_nAnims].Params.bLooping = ( TOK.Int() ) ? (TRUE):(FALSE);

            VERIFY( TOK.Read() == token_stream::TOKEN_NUMBER );
            m_Anim[m_nAnims].Params.FPS = TOK.Int();

            VERIFY( TOK.Read() == token_stream::TOKEN_NUMBER );
            m_Anim[m_nAnims].Params.Downsample = TOK.Float();

            VERIFY( TOK.Read() == token_stream::TOKEN_NUMBER );
            m_Anim[m_nAnims].Params.HandleAngle = DEG_TO_RAD(TOK.Float());

            VERIFY( TOK.Read() == token_stream::TOKEN_SYMBOL );
            m_Anim[m_nAnims].Params.Name = TOK.String();

            VERIFY( TOK.Read() == token_stream::TOKEN_STRING );
            m_Anim[m_nAnims].Params.FileName = TOK.String() ;

            if( !m_Anim[m_nAnims].RawAnim.Load( m_Anim[m_nAnims].Params.FileName ) )
            {
                xstring S; S.Format("Could not load (%s).\n",m_Anim[m_nAnims].Params.FileName);
                MessageBox(NULL, (const char*)S, TEXT("Error"), MB_OK);
                x_DebugMsg("Could not load (%s).\n",m_Anim[m_nAnims].Params.FileName);
                return FALSE;
            }

            #ifdef AUX_DEBUG_OUTPUT
            x_DebugMsg("%s LOADED.\n",m_Anim[m_nAnims].Params.FileName);
            #endif
            m_nAnims++;
        }
    }

    TOK.CloseFile();


    return TRUE;
}

//=========================================================================

void anim_compiler::AttachUncompressedData( anim_group& AG )
{
    #define NUM_SECTIONS 4

    s32     i;
    s32     Size[NUM_SECTIONS];
    byte**  Ptr[NUM_SECTIONS];

    //
    // Get data into arrays
    //
    Size[0] = sizeof(anim_bone)*AG.m_nBones;
    Size[1] = sizeof(anim_info)*AG.m_nAnims;
    Size[2] = sizeof(anim_prop)*AG.m_nProps;
    Size[3] = sizeof(anim_key_block)*AG.m_nKeyBlocks;
    Ptr[0]  = (byte**)&AG.m_pBone;
    Ptr[1]  = (byte**)&AG.m_pAnimInfo;
    Ptr[2]  = (byte**)&AG.m_pProp;
    Ptr[3]  = (byte**)&AG.m_pKeyBlock;

    //
    // Compute total size including alignment padding
    //
    AG.m_UncompressedDataSize = 0;
    for( i=0; i<NUM_SECTIONS; i++ )
        AG.m_UncompressedDataSize += ALIGN_4(Size[i]);

    //
    // Allocate buffer and clear
    //
    AG.m_pUncompressedData = (byte*)x_malloc( AG.m_UncompressedDataSize );
    ASSERT(AG.m_pUncompressedData);
    x_memset( AG.m_pUncompressedData, 0, AG.m_UncompressedDataSize );

    //
    // Loop through and copy sections into place
    //
    byte* pD = AG.m_pUncompressedData;
    for( i=0; i<NUM_SECTIONS; i++ )
    {
        // Copy section into current location in buffer
        x_memcpy( pD, *Ptr[i], Size[i] );

        // Free old section
        x_free( *Ptr[i] );

        // Aim section ptr into new buffer
        *Ptr[i] = pD;

        // Step buffer to next section location
        pD += ALIGN_4(Size[i]);
    }
}

//=========================================================================

void anim_compiler::AttachCompressedData( anim_group& AG )
{
    // Get size of data from bitstream
    AG.m_CompressedDataSize = m_BS.GetNBytesUsed();

    // Allocate buffer for data
    AG.m_pCompressedData = (byte*)x_malloc(AG.m_CompressedDataSize);
    ASSERT(AG.m_pCompressedData);

    // Copy over
    x_memcpy( AG.m_pCompressedData, m_BS.GetDataPtr(), AG.m_CompressedDataSize );
}

//=========================================================================

extern s32 s_ScaleCompUses[8];
extern s32 s_ScaleCompSize[8];
extern s32 s_ScaleCompSamples[8];
extern s32 s_RotationCompUses[8];
extern s32 s_RotationCompSize[8];
extern s32 s_RotationCompSamples[8];
extern s32 s_TranslationCompUses[8];
extern s32 s_TranslationCompSize[8];
extern s32 s_TranslationCompSamples[8];
extern s32 s_TotalNBones;
extern s32 s_TotalFrames;

void anim_compiler::DisplayAnimCompressStats( anim_group& AG, const char* pFileName )
{
    s32 i;
    s32 Sum[3]={0};
    X_FILE* fp = x_fopen(pFileName,"wt");
    ASSERT(fp);
    if( !fp )
        return;

    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"--                       ANIMGROUP COMPILED                           --\n");
    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"Anims           %8d = %8d bytes\n",AG.m_nAnims,AG.m_nAnims*sizeof(anim_info));
    x_fprintf(fp,"Bones           %8d = %8d bytes\n",AG.m_nBones,AG.m_nBones*sizeof(anim_bone));
    x_fprintf(fp,"Events          %8d = %8d bytes\n",AG.m_nEvents,AG.m_nEvents*sizeof(anim_event));
    x_fprintf(fp,"Props           %8d = %8d bytes\n",AG.m_nProps,AG.m_nProps*sizeof(anim_prop));
    x_fprintf(fp,"Blocks          %8d = %8d bytes\n",AG.m_nKeyBlocks,AG.m_nKeyBlocks*sizeof(anim_key_block));
    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"TotalAnimTime   %8.2f seconds\n",AG.m_TotalNFrames / 30.0f);
    x_fprintf(fp,"TotalFrames     %8d\n",AG.m_TotalNFrames);
    x_fprintf(fp,"TotalKeys       %8d\n",AG.m_TotalNKeys);
    x_fprintf(fp,"UncompDataSize  %8d bytes\n",sizeof(anim_group) + AG.m_UncompressedDataSize );
    x_fprintf(fp,"CompDataSize    %8d bytes\n",AG.m_CompressedDataSize );
    x_fprintf(fp,"TotalDataSize   %8d bytes\n",sizeof(anim_group) + AG.m_UncompressedDataSize + AG.m_CompressedDataSize );
    x_fprintf(fp,"BytesPerKey     %8.2f\n",(AG.m_CompressedDataSize)/(f32)AG.m_TotalNKeys );

    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"--                       ANIM COMPRESS STATS                          --\n");
    x_fprintf(fp,"------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[0] += s_ScaleCompSize[i];
        x_fprintf(fp,"%1d Scale       %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_ScaleCompUses[i], 
            (s_ScaleCompSize[i]+7)/8, 
            (s_ScaleCompSamples[i])?(((s_ScaleCompSize[i]+7)/8)/(f32)s_ScaleCompSamples[i]):(0));
    }

    x_fprintf(fp,"------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[1] += s_RotationCompSize[i];
        x_fprintf(fp,"%1d Rotation    %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_RotationCompUses[i], 
            (s_RotationCompSize[i]+7)/8, 
            (s_RotationCompSamples[i])?(((s_RotationCompSize[i]+7)/8)/(f32)s_RotationCompSamples[i]):(0) );
    }

    x_fprintf(fp,"------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[2] += s_TranslationCompSize[i];
        x_fprintf(fp,"%1d Translation %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_TranslationCompUses[i], 
            (s_TranslationCompSize[i]+7)/8, 
            (s_TranslationCompSamples[i])?(((s_TranslationCompSize[i]+7)/8)/(f32)s_TranslationCompSamples[i]):(0));
    }

    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"Scale       bytes = %8d\n",(Sum[0]+7)/8);
    x_fprintf(fp,"Rotation    bytes = %8d\n",(Sum[1]+7)/8);
    x_fprintf(fp,"Translation bytes = %8d\n",(Sum[2]+7)/8);
    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fclose(fp);

    fp = x_fopen("c:\\temp\\animcompilerstats.csv","at");
    if( fp )
    {
        for( i=0; i<6; i++ )
            x_fprintf(fp,"%d,%d,", s_ScaleCompUses[i],  (s_ScaleCompSize[i]+7)/8);
        x_fprintf(fp,",");

        for( i=0; i<6; i++ )
            x_fprintf(fp,"%d,%d,", s_RotationCompUses[i],  (s_RotationCompSize[i]+7)/8);
        x_fprintf(fp,",");
        
        for( i=0; i<6; i++ )
            x_fprintf(fp,"%d,%d", s_TranslationCompUses[i],  (s_TranslationCompSize[i]+7)/8);
        x_fprintf(fp,"\n");

        x_fclose(fp);
    }
}

//=========================================================================

xbool anim_compiler::Compile( anim_group& AG, xbool bKeepBind )
{
    // Show hierarchy?
    if( g_Verbose )
        m_Bind.PrintHierarchy();

    /*
    {
        // Write out BIND/ANIM pairs
        X_FILE* fp = x_fopen("c:\\temp\\animlist.csv","at");
        if( fp )
        {
            char OPName[512];
            x_strcpy(OPName,m_OutputFileName);
            x_strtoupper(OPName);

            char BFName[512];
            x_strcpy(BFName,m_BindFileName);
            x_strtoupper(BFName);

            for( s32 i=0; i<m_nAnims; i++ )
            {
                char FName[512];
                x_strcpy(FName,m_Anim[i].Params.FileName);
                x_strtoupper(FName);

                x_fprintf(fp,"%s,%s,%s\n",OPName,BFName,FName);
            }
            x_fclose(fp);
        }
    }
    */
    //
    // Build final source animations
    //
    PrepareSourceAnims( bKeepBind );

    //
    // Clear anim_group and setup version
    //
    AG.Clear();
    //x_memset( &AG, 0, sizeof(anim_group) );
    AG.m_Version = ANIM_DATA_VERSION;

    //
    // Build skeleton information
    //
    BuildSkeleton( AG );

    //
    // Build animation info structures.  This also computes the number of
    // events and props.  It does not build keyframe data
    //
    BuildAnimInfo( AG );

    //
    // Builds anim group and individual anims bboxes
    //
    BuildBounds( AG );

    //
    // Build Events
    //
    BuildEvents( AG );

    //
    // Build Prop structures.  This does not build keyframe data.
    //
    BuildProps( AG );

    //
    // Build Anim Keysets
    //
    BuildAnimKeys( AG );

    //
    // Merge the uncompressed data into a single block of memory
    //
    AttachUncompressedData( AG );

    //
    // Build Compressed data
    //
    AttachCompressedData( AG );

    //
    // Display a final summary
    //
    /*
    char FILENAME[256];
    x_splitpath( m_BindFileName, NULL, NULL, FILENAME, NULL );
    DisplayAnimCompressStats(AG,xfs("c:\\temp\\animcompiler\\%s.txt",FILENAME));
    */

    return TRUE;
}

//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
// SUPPORT ROUTINES
//=========================================================================
//=========================================================================
//=========================================================================

class CompareAnimNames : public x_compare_functor<const compiler_anim&>
{
public:
    s32 operator()( const compiler_anim& Anim1, const compiler_anim& Anim2 )
    {
        return x_stricmp( Anim1.Params.Name, Anim2.Params.Name) ;
    }
};


void anim_compiler::PrepareSourceAnims( xbool bKeepBind )
{
    s32 i;

    // *INEV* *SB* - Commented out for A51
    
    /*

    //
    // Remove bones not used by geometry
    //
    if( m_BindMeshName[0] != 0 )
    {
        m_BindMesh.CollapseGroups();
        //x_DebugMsg("ANIM ---- BindMeshName %s\n",m_BindMeshName );

        rawmesh2 IsolatedMesh;
        if( m_BindMesh.IsolateSubmesh(m_BindMeshName,IsolatedMesh) )
        {
            for( i=0; i<m_Bind.m_nBones; i++ )
            {
                // Get core name
                if( !IsolatedMesh.IsBoneUsedByMesh(i,m_BindMeshName) )
                {
                    if( m_Bind.m_nBones > 1 )
                    {
                        //x_DebugMsg("ANIM ---- Deleting Bone %s\n",m_Bind.m_pBone[i].Name);
                        m_Bind.DeleteBone( i );
                        IsolatedMesh.DeleteBone( i );
                        i=-1;
                    }
                }
            }
        }

        // Display final bones
        x_DebugMsg("--------------- FINAL SKELETON ----------------\n");
        for( i=0; i<m_Bind.m_nBones; i++ )
            x_DebugMsg("%d %s\n",i,m_Bind.m_pBone[i].Name);
        x_DebugMsg("-----------------------------------------------\n");
    }

    //
    // Convert all anims to bind pose
    //
    for( i=0; i<m_nAnims; i++ )
    {
        m_Anim[i].RawAnim.ApplyNewSkeleton( m_Bind );
        m_Anim[i].RawAnim.BakeBindingIntoFrames( TRUE, TRUE, FALSE );
        //m_Anim[i].RawAnim.DumpFrames( xfs("F_%s.txt",m_Anim[i].FileName), TRUE );
    }
    */

    // SB:
    // Sort anims so that all animations of the same name are grouped together ready for the engine
    // random weight playback.
    // NOTE: The BINDPOSE - Anim[0] is not sorted since we always want it at slot zero!
    // NOTE: Using a special swap because of the funky combination of classes and structs.
    if (m_nAnims > 1)
    {
        s32 i,j;

        for( i=1; i<m_nAnims; i++ )
        {
            s32 BestI = i;
            for( j=i+1; j<m_nAnims; j++ )
            {
                if( x_stricmp( m_Anim[BestI].Params.Name,  m_Anim[j].Params.Name ) > 0 )
                    BestI = j;
            }
            if( BestI != i )
                m_Anim[i].SpecialSwap( m_Anim[BestI] );
        }            
    }

    //
    // Convert all anims and mesh to bind pose skeleton
    //
    m_BindMesh.ApplyNewSkeleton( m_Bind );
    for( i=0; i<m_nAnims; i++ )
    {
        SetUserInfo( m_Anim[i].RawAnim );
        
        m_Anim[i].RawAnim.ApplyNewSkeleton( m_Bind ) ;
        m_Anim[i].RawAnim.BakeBindingIntoFrames( !bKeepBind, !bKeepBind, FALSE );
        //m_Anim[i].RawAnim.DumpFrames( xfs("F_%s.txt",m_Anim[i].FileName), TRUE );
    }
    
    ClearUserInfo();
}

//=========================================================================
//=========================================================================
//=========================================================================
// BUILD ROUTINES
//=========================================================================
//=========================================================================
//=========================================================================

void anim_compiler::BuildSkeleton( anim_group& AG )
{
    s32 i,j,k ;

    x_DebugMsg("Building Skeleton...\n");

    SetUserInfo( m_Bind );
    
    m_Bind.BakeBindingIntoFrames( TRUE, TRUE, FALSE );

    //=====================================================================
    // Setup bones
    //=====================================================================

    AG.m_nBones = m_Bind.m_nBones;
    AG.m_pBone  = (anim_bone*)x_malloc( sizeof(anim_bone) * AG.m_nBones );
    for(i=0; i<AG.m_nBones; i++ )
    {
        anim_bone* pBS = &AG.m_pBone[i];
        rawanim::bone* pBB = &m_Bind.m_pBone[i];

        pBS->LocalTranslation   = m_Bind.m_pFrame[i].Translation;
        pBS->BindTranslation    = pBB->BindTranslation;
        pBS->BindMatrixInv      = pBB->BindMatrixInv  ;
        pBS->iBone              = pBB->iBone          ;
        pBS->iParent            = pBB->iParent        ;
        pBS->nChildren          = pBB->nChildren      ;

        x_strncpy(pBS->Name, pBB->Name, 31);
    }

    //=====================================================================
    // Compute MaxDistToVerts for each bone
    //=====================================================================
    {
        m_MaxDistToVerts = (f32*)x_malloc(sizeof(f32)*AG.m_nBones);
        ASSERT( m_MaxDistToVerts );

        // Loop through bones from child to parent
        i = AG.m_nBones-1;
        while( i >= 0 )
        {
            m_MaxDistToVerts[i] =  0;

            // Initialize distance to distance of verts from bone
            for( j=0; j<m_BindMesh.m_nVertices; j++ )
            for( k=0; k<m_BindMesh.m_pVertex[j].nWeights; k++ )
            if( m_BindMesh.m_pVertex[j].Weight[k].iBone == i )
            {
                f32 BoneToVertDist = (m_BindMesh.m_pVertex[j].Position - AG.m_pBone[i].BindTranslation).Length();
                m_MaxDistToVerts[i] = MAX( m_MaxDistToVerts[i], BoneToVertDist );
            }

            // Loop through children and see if their distance is greater
            for( j=i+1; j<AG.m_nBones; j++ )
            if( AG.m_pBone[j].iParent == i )
            {
                m_MaxDistToVerts[i] = MAX( m_MaxDistToVerts[i], m_MaxDistToVerts[j] + AG.m_pBone[j].LocalTranslation.Length() );
            }

            // Move to next bone higher in hierarchy
            i--;
        }

        for( i=0; i<AG.m_nBones; i++ )
            x_DebugMsg("%3d] %f\n",i,m_MaxDistToVerts[i]);
    }
    
    ClearUserInfo();
}

//=========================================================================

void anim_compiler::BuildAnimInfo( anim_group& AG )
{
    x_DebugMsg("Building AnimInfo...\n");

    s32 i,j;
    s32 nEvents=0;
    s32 nProps=0;

    // First downsample any animations that need to
    for( i=0; i<m_nAnims; i++ )
    {
        // Lookup info
        anim_info&              Anim       = AG.m_pAnimInfo[i];
        compiler_anim&          SrcAnim    = m_Anim[i] ;
        compiler_anim::params&  SrcParams  = SrcAnim.Params ;
        
        SetUserInfo( SrcAnim.RawAnim );
        
        if( SrcParams.Downsample != 1.0f )
        {
            rawanim&    SrcRawAnim = SrcAnim.RawAnim ;
            s32 NewNFrames = (s32)(SrcRawAnim.m_nFrames * SrcParams.Downsample);
            if( NewNFrames < 2 ) NewNFrames = 2;
            x_DebugMsg("Downsampling from %d to %d\n",SrcRawAnim.m_nFrames,NewNFrames);
            SrcRawAnim.Resample( NewNFrames );
        }
    }

    AG.m_nAnims = m_nAnims;
    AG.m_pAnimInfo  = (anim_info*)x_malloc( sizeof(anim_info) * AG.m_nAnims );
    x_memset( AG.m_pAnimInfo, 0, sizeof(anim_info) * AG.m_nAnims );

    for( i=0; i<m_nAnims; i++ )
    {
        // Lookup info
        anim_info&              Anim       = AG.m_pAnimInfo[i];
        compiler_anim&          SrcAnim    = m_Anim[i] ;
        compiler_anim::params&  SrcParams  = SrcAnim.Params ;
        rawanim&                SrcRawAnim = SrcAnim.RawAnim ;

        SetUserInfo( SrcRawAnim );

        // Setup anim count and wieght of matching anims
        Anim.m_nAnims      = 1 ;
        Anim.m_AnimsWeight = SrcParams.Weight ;
        for (j = i+1 ; j < m_nAnims ; j++)
        {
            // Match?
            if (x_stricmp(SrcParams.Name, m_Anim[j].Params.Name) == 0)
            {
                // Update total count and total weight
                Anim.m_nAnims      += 1 ;
                Anim.m_AnimsWeight += m_Anim[j].Params.Weight ;
            }
        }

        // Setup properties
        x_strcpy(Anim.m_Name, SrcParams.Name) ;
        
        Anim.m_Weight           = SrcParams.Weight ;
        Anim.m_BlendTime        = SrcParams.BlendTime;
        
        Anim.m_nFrames          = SrcRawAnim.m_nFrames;
        Anim.m_iLoopFrame       = SrcParams.iLoopFrame;
        Anim.m_EndFrameOffset   = SrcParams.EndFrameOffset;

        Anim.m_nEvents          = SrcRawAnim.m_nEvents + SrcRawAnim.m_nSuperEvents;
        Anim.m_iEvent           = nEvents;
        nEvents                 += Anim.m_nEvents;

        Anim.m_nProps           = SrcRawAnim.m_nProps;
        Anim.m_iProp            = nProps;
        nProps                  += Anim.m_nProps;

        // Setup chain info
        Anim.m_iChainAnim = -1;
        for( j = 0; j < m_nAnims; j++ )
        {
            if (x_stricmp(SrcParams.ChainAnim, m_Anim[j].Params.Name) == 0)
            {
                Anim.m_iChainAnim = j;
                break;
            }
        }
        Anim.m_nChainFramesMin = (s16)(SrcParams.ChainCyclesMin * (SrcRawAnim.m_nFrames-1));
        Anim.m_nChainFramesMax = (s16)(SrcParams.ChainCyclesMax * (SrcRawAnim.m_nFrames-1));
        Anim.m_iChainFrame     = SrcParams.iChainFrame;

        // Default to no animated bones - the compress function will update these values when animated bones
        // are found
        Anim.m_iAnimBoneMin = -1;
        Anim.m_iAnimBoneMax = -1;

        // Setup Flags
        Anim.m_Flags  = 0;
        Anim.m_Flags |= (SrcParams.bLooping)            ? (ANIM_DATA_FLAG_LOOPING) : (0);
        Anim.m_Flags |= (SrcRawAnim.IsMaskedAnim())     ? (ANIM_DATA_FLAG_HAS_MASKS) : (0);
        Anim.m_Flags |= (SrcParams.bAccumHorizMotion)   ? (ANIM_DATA_FLAG_ACCUM_HORIZ_MOTION) : (0);
        Anim.m_Flags |= (SrcParams.bAccumVertMotion)    ? (ANIM_DATA_FLAG_ACCUM_VERT_MOTION) : (0);
        Anim.m_Flags |= (SrcParams.bAccumYawMotion)     ? (ANIM_DATA_FLAG_ACCUM_YAW_MOTION) : (0);
        Anim.m_Flags |= (SrcParams.bGravity)            ? (ANIM_DATA_FLAG_GRAVITY) : (0);
        Anim.m_Flags |= (SrcParams.bWorldCollision)     ? (ANIM_DATA_FLAG_WORLD_COLLISION) : (0);
        Anim.m_Flags |= (SrcParams.bChainCyclesInteger) ? (ANIM_DATA_FLAG_CHAIN_CYCLES_INTEGER) : (0);
        Anim.m_Flags |= (SrcParams.bBlendFrames)        ? (ANIM_DATA_FLAG_BLEND_FRAMES) : (0);
        Anim.m_Flags |= (SrcParams.bBlendLoop)          ? (ANIM_DATA_FLAG_BLEND_LOOP) : (0);

        // Setup FPS
        ASSERT( (SrcParams.FPS>=1) && (SrcParams.FPS<=255) );
        Anim.m_FPS = SrcParams.FPS;

        // Setup computed info about animation
        {
            rawanim::frame& Key0 = SrcRawAnim.m_pFrame[ 0 * SrcRawAnim.m_nBones ];
            rawanim::frame& Key1 = SrcRawAnim.m_pFrame[ (SrcRawAnim.m_nFrames-1) * SrcRawAnim.m_nBones ];

            radian R;

            R = m_Anim[i].Params.HandleAngle;
            while( R > R_360 ) R -= R_360;
            while( R < R_0   ) R += R_360;
            Anim.m_HandleAngle = (u16)( R * (65535.0f / R_360) );

            R = Anim.m_TotalTranslation.GetYaw();
            while( R > R_360 ) R -= R_360;
            while( R < R_0   ) R += R_360;
            Anim.m_TotalMoveDir = (u16)( R * (65535.0f / R_360) );

            R = x_MinAngleDiff(Key1.Rotation.GetRotation().Yaw,Key0.Rotation.GetRotation().Yaw);
            while( R > R_360 ) R -= R_360;
            while( R < R_0   ) R += R_360;
            Anim.m_TotalYaw = (u16)( R * (65535.0f / R_360) );

            Anim.m_TotalTranslation = Key1.Translation - Key0.Translation;
            
            // These bboxes are computed in "BuildBounds" - see below
            Anim.m_BBox.Min.Zero();
            Anim.m_BBox.Max.Zero();
        }
    }

    AG.m_nEvents = nEvents;
    AG.m_nProps = nProps;
    
    ClearUserInfo();
}

//=========================================================================

bbox anim_compiler::ComputeBounds( anim_group& AG, s32 iAnim, xbool bUseAccumFlags )
{
    ASSERT( m_Bind.m_nBones <= MAX_ANIM_BONES );
    matrix4 Matrices[ MAX_ANIM_BONES ];

    // Lookup anim info
    compiler_anim& CompilerAnim = m_Anim[ iAnim ];
    rawanim&       RawAnim      = CompilerAnim.RawAnim;
    anim_info&     AnimInfo     = AG.m_pAnimInfo[ iAnim ];

    // Setup accumulate flags
    xbool bAccumHoriz = bUseAccumFlags && CompilerAnim.Params.bAccumHorizMotion;
    xbool bAccumVert  = bUseAccumFlags && CompilerAnim.Params.bAccumVertMotion;
    xbool bAccumYaw   = bUseAccumFlags && CompilerAnim.Params.bAccumYawMotion;
    
    // Clear bbox
    bbox BBox;
    BBox.Clear();
    
    // If there are no verts, use default bbox
    if( m_BindMesh.m_nVertices == 0 )
    {
        BBox.Set( vector3( 0.0f, 0.0f, 0.0f ), 100.0f );    
    }
    
    // Push all verts through every animation frame to compute bounds
    for( s32 iFrame = 0; iFrame < RawAnim.m_nFrames; iFrame++ )
    {
        // Compute bones L2W for this frame
        ASSERT( m_BindMesh.m_nBones == m_Bind.m_nBones );
        ASSERT( RawAnim.m_nBones    == m_Bind.m_nBones );
        RawAnim.ComputeBonesL2W( Matrices, iFrame, bAccumHoriz, bAccumVert, bAccumYaw );

        // Loop through all verts in bind pose mesh
        for( s32 iVert = 0; iVert < m_BindMesh.m_nVertices; iVert++ )
        {
            // Lookup vert
            rawmesh2::vertex& Vert = m_BindMesh.m_pVertex[iVert];

            // Sum up total of weights to compute local pos?
            vector3 AnimPos( 0, 0, 0 );
            for( s32 iWeight = 0; iWeight < Vert.nWeights; iWeight++ )
            {
                // Lookup weight
                rawmesh2::weight& Weight = Vert.Weight[ iWeight ];

                // Add to local position
                ASSERT( Weight.iBone >= 0 );
                ASSERT( Weight.iBone < m_Bind.m_nBones );
                AnimPos += Matrices[ Weight.iBone ] * Vert.Position * Weight.Weight;
            }

            // Add vert bind pos and animation pos to bounding box
            BBox += Vert.Position;
            BBox += AnimPos;
        }
    }
    
    // Add tolerance
    BBox.Inflate( 10.0f, 10.0f, 10.0f );
    return BBox;
}

//=========================================================================

void anim_compiler::BuildBounds( anim_group& AG )
{
    x_DebugMsg("Building bounds...\n");

    // Compute the bounding boxes of all the animations and anim group
    AG.m_BBox.Clear();
    for( s32 iAnim = 0; iAnim < m_nAnims; iAnim++ )
    {
        // Lookup anim info
        anim_info& AnimInfo = AG.m_pAnimInfo[ iAnim ];

        // Compute animation bounds taking accumulation flags into account
        AnimInfo.m_BBox = ComputeBounds( AG, iAnim, TRUE  );

        // Compute anim bounds ignoring accumulation flags so that anim surfaces
        // use the correct bounding box (anim surfaces also ignore accumulation flags)
        AG.m_BBox += ComputeBounds( AG, iAnim, FALSE );
    }
}

//=========================================================================

void anim_compiler::BuildEvents( anim_group& AG )
{
    x_DebugMsg("Building Events...\n");
    

    s32 i,j,k;

    // The number of events was already computed in BuildAnimInfo

    // Allocate events
    AG.m_pEvent = new anim_event[AG.m_nEvents];
    ASSERT( AG.m_pEvent );
    x_memset( AG.m_pEvent, 0, sizeof(anim_event)*AG.m_nEvents );

    // Setup events
    s32 EC=0;
    s32 nEvents=0;
    for( i=0; i<m_nAnims; i++ )
    {
        anim_info& Anim = AG.m_pAnimInfo[i];
        rawanim&   RAnim = m_Anim[i].RawAnim;
        nEvents += RAnim.m_nEvents;
        nEvents += RAnim.m_nSuperEvents;

        SetUserInfo( RAnim );

        // Copy event data into animation
        for( j=0; j<RAnim.m_nEvents; j++ )
        {
            anim_event& EV = AG.m_pEvent[ j + Anim.m_iEvent ];
            rawanim::event& REV = RAnim.m_pEvent[j];
            event_data EventData;
            
            // Figure out which bone anim is attached to.  If not attached to any
            // known bone then attach to root
            s32 BoneIdx = AG.GetBoneIndex( REV.ParentName );
            if ( -1 == BoneIdx ) BoneIdx = 0;
            EventData.StoreInt( anim_event::INT_IDX_BONE, BoneIdx );

            // Make sure event name is not too big
            if( x_strlen( REV.Name ) > EVENT_MAX_STRING_LENGTH )
            {
                ThrowError( xfs( "Event name [%s] is too long!\nIt must be <= %d characters!\n", 
                                  REV.Name,
                                  EVENT_MAX_STRING_LENGTH ) );
            }

            // Copy other basic info
            EventData.SetType( g_EventTypes[0] );
            EventData.SetName( REV.Name );
            EventData.StoreInt( anim_event::INT_IDX_OLD_TYPE,       REV.Type );
            EventData.StoreInt( anim_event::INT_IDX_START_FRAME,    REV.Frame0 );

            s32 EndFrame;
            if ( REV.Frame1 == REV.Frame0 )
            {
                EndFrame = MIN( Anim.GetNFrames()-1, REV.Frame1+1 );
                REV.Frame1 = EndFrame;
            }
            else
            {
                EndFrame = REV.Frame1;
            }
           
            EventData.StoreInt( anim_event::INT_IDX_END_FRAME,      EndFrame );
            EventData.StoreFloat( anim_event::FLOAT_IDX_RADIUS,     REV.Radius );

            // Compute Offset relative to parent bone
            matrix4 ParentInvM;
            s32 RawAnimBoneIdx = RAnim.GetBoneIDFromName( REV.ParentName );
            if ( -1 == RawAnimBoneIdx ) RawAnimBoneIdx = 0;

            RAnim.ComputeBoneL2W( RawAnimBoneIdx, ParentInvM, 0 );
            ParentInvM.Invert();
            EventData.StorePoint( anim_event::POINT_IDX_OFFSET, ParentInvM * REV.Position );
            EV.SetData( EventData );

/*
#if _DEBUG
            // make sure this all works like it used to
            {
                s32 iBone = AG.GetBoneIndex( REV.ParentName );
                if( iBone == -1 )
                    iBone = 0;
                ASSERT( EV.GetInt( anim_event::INT_IDX_BONE ) == iBone );

                ASSERT( EV.GetInt( anim_event::INT_IDX_OLD_TYPE )== REV.Type );
                ASSERT( EV.StartFrame() == REV.Frame0 );
                ASSERT( EV.EndFrame() == REV.Frame1 );
                ASSERT( EV.GetFloat( anim_event::FLOAT_IDX_RADIUS ) == REV.Radius );

                matrix4 Temp;
                RAnim.ComputeBoneL2W( EV.GetInt( anim_event::INT_IDX_BONE ), Temp, 0 );
                Temp.Invert();
                ASSERT( (EV.GetPoint( anim_event::POINT_IDX_OFFSET ) - (Temp * REV.Position)).LengthSquared() < 0.0001f );
            }
#endif // _DEBUG
*/

        }
        //
        // Now copy the super events
        //
        // Setup count and pointer to events
        s32 nOldEvents = RAnim.m_nEvents;
        EC += RAnim.m_nSuperEvents;

        // Copy super event data into animation
        for( j=nOldEvents; j<Anim.m_nEvents; j++ )
        {
            rawanim::super_event& REV = RAnim.m_pSuperEvent[j-nOldEvents];
            event_data EventData;

            // Figure out which bone anim is attached to.  If not attached to any
            // known bone then attach to root
            s32 Bone = AG.GetBoneIndex( REV.ParentName );
            if( Bone == -1 )
                Bone = 0;
            EventData.StoreInt( anim_event::INT_IDX_BONE, Bone );

            // Make sure event name is not too big
            if( x_strlen( REV.Name ) > EVENT_MAX_STRING_LENGTH )
            {
                ThrowError( xfs( "Event name [%s] is too long!\nIt must be <= %d characters!\n", 
                                 REV.Name,
                                 EVENT_MAX_STRING_LENGTH ) );
            }

            // Copy other basic info
            ASSERT( REV.Type < NUM_EVENT_TYPES );
            EventData.SetType( g_EventTypes[REV.Type] );
            EventData.SetName( REV.Name );
            EventData.StoreInt( anim_event::INT_IDX_START_FRAME,    REV.StartFrame );

            s32 EndFrame;
            if ( REV.EndFrame == REV.StartFrame )
            {
                EndFrame = MIN( Anim.GetNFrames()-1, REV.EndFrame+1 );
            }
            else
            {
                EndFrame = REV.EndFrame;
            }
            EventData.StoreInt( anim_event::INT_IDX_END_FRAME,      EndFrame );

            EventData.StoreFloat( anim_event::FLOAT_IDX_RADIUS,     REV.Radius );

            // Compute Offset relative to parent bone
            matrix4 ParentInvM;
            RAnim.ComputeBoneL2W( Bone, ParentInvM, 0 );
            ParentInvM.Invert();
            EventData.StorePoint( anim_event::POINT_IDX_OFFSET, ParentInvM * REV.Position );
            //EventData.StorePoint( anim_event::POINT_IDX_ROTATION, REV.Rotation );

            matrix4 MR;
            MR.Identity();
            //MR.RotateZ( R_90 );
            //MR.RotateY( R_90 );
            MR.RotateX( R_90 );
            MR.Rotate( REV.Rotation );
            MR = ParentInvM * MR;

            radian3 RR = MR.GetRotation();
            vector3 VR( RR.Pitch, RR.Yaw, RR.Roll );

            EventData.StorePoint( anim_event::POINT_IDX_ROTATION, VR );


            // Set event type-specific data
            switch ( REV.Type )
            {
            case 1:
                // Event type not handled
                EventData.StoreString( 0, "INVALID EVENT DO NOT USE" );
            break;
            
            case EVENT_HOT_POINT:
                EventData.StoreString( anim_event::STRING_IDX_HOTPOINT_TYPE, REV.Strings[0] );
            break;

            case EVENT_TYPE_AUDIO:
                EventData.StoreString( anim_event::STRING_IDX_AUDIO_SOUND_ID, REV.Strings[0] );

                if ( x_strlen( REV.Strings[1] ) == 0 )
                {
                    EventData.StoreString( anim_event::STRING_IDX_AUDIO_LOCATION, "Center of Object" );
                }
                else
                {
                    EventData.StoreString( anim_event::STRING_IDX_AUDIO_LOCATION, REV.Strings[1] );
                }

                if ( x_strlen( REV.Strings[2] ) == 0 )
                {
                    EventData.StoreString( anim_event::STRING_IDX_AUDIO_TYPE, "One Shot" );
                }
                else
                {
                    EventData.StoreString( anim_event::STRING_IDX_AUDIO_TYPE, REV.Strings[2] );
                }
                
                if( x_strlen( REV.Strings[3] ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_AUDIO_DATA, 0 );   
                }
                else if( x_strcmp( REV.Strings[3], "Default" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_AUDIO_DATA, 0 );   
                }
                else if( x_strcmp( REV.Strings[3], "Gun Shot" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_AUDIO_DATA, 1 );   
                }
                else if( x_strcmp( REV.Strings[3], "Explosion" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_AUDIO_DATA, 2 );   
                }
                else if( x_strcmp( REV.Strings[3], "Voice" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_AUDIO_DATA, 3 );   
                }
                else
                {
                    EventData.StoreInt( anim_event::INT_IDX_AUDIO_DATA, 0 );
                }
                break;
	        
            case EVENT_TYPE_PARTICLE:
                EventData.StoreString( anim_event::STRING_IDX_PARTICLE_TYPE,                REV.Strings[0] );
                EventData.StoreBool  ( anim_event::BOOL_IDX_PARTICLE_EVENT_ACTIVE,          REV.Bools[0] );
                EventData.StoreBool  ( anim_event::BOOL_IDX_PARTICLE_DONOT_APPLY_TRANSFORM, REV.Bools[1] );
            break;		

            case EVENT_TYPE_GENERIC:
                EventData.StoreString( anim_event::STRING_IDX_GENERIC_TYPE, REV.Strings[0] );
            break;

            case EVENT_TYPE_INTENSITY:
                
                EventData.StoreFloat( anim_event::FLOAT_IDX_CONTROLLER_INTENSITY,   REV.Floats[0] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_CONTROLLER_DURATION,    REV.Floats[1] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_CAMERA_SHAKE_TIME,      REV.Floats[2] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_CAMERA_SHAKE_AMOUNT,    REV.Floats[3] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_CAMERA_SHAKE_SPEED,     REV.Floats[4] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_BLUR_INTENSITY,         REV.Floats[5] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_BLUR_DURATION,          REV.Floats[6] );
            break;

            case EVENT_TYPE_WORLD_COLLISION:
                
                EventData.StoreBool( anim_event::BOOL_IDX_WORLD_COLLISION,          REV.Bools[0] );
            break;

            case EVENT_TYPE_GRAVITY:
                
                EventData.StoreBool( anim_event::BOOL_IDX_GRAVITY,                  REV.Bools[0] );
            break;

            case EVENT_TYPE_WEAPON:

                if( x_strlen( REV.Strings[0] ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 0 );   
                }
                else if( x_strcmp( REV.Strings[0], "Fire" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 1 );   
                }
                else if( x_strcmp( REV.Strings[0], "Alt Fire" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 2 );   
                }
                else if( x_strcmp( REV.Strings[0], "Grenade" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 3 );   
                }
                else if( x_strcmp( REV.Strings[0], "Alt Grenade" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 4 );   
                }
                else if( x_strcmp( REV.Strings[0], "Primary Fire Left" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 5 );   
                }
                else if( x_strcmp( REV.Strings[0], "Alt Fire Left" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 6 );   
                }
                else if( x_strcmp( REV.Strings[0], "Primary Fire Right" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 7 );   
                }
                else if( x_strcmp( REV.Strings[0], "Alt Fire Right" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 8 );   
                }
                else if( x_strcmp( REV.Strings[0], "Cinema Fire" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 9 );   
                }
                else
                {
                    EventData.StoreInt( anim_event::INT_IDX_WEAPON_DATA, 0 );
                }
            break;

            case EVENT_TYPE_PAIN:
                if( x_strlen( REV.Strings[0] ) == 0 || x_strcmp( REV.Strings[0], "Melee" ) == 0)
                {
                    EventData.StoreInt( anim_event::INT_IDX_PAIN_TYPE, 0 );   
                }
                else if( x_strcmp( REV.Strings[0], "Leap-Charge" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_PAIN_TYPE, 1 );   
                }
                else if( x_strcmp( REV.Strings[0], "Special" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_PAIN_TYPE, 2 );   
                }
                else
                {
                    ASSERTS( FALSE,"Compiling a Super Pain Event with a bad pain type" );
                }
            break;

            case EVENT_TYPE_DEBRIS:

                EventData.StoreString( anim_event::STRING_IDX_DEBRIS_TYPE,          REV.Strings[0] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_DEBRIS_MIN_VELOCITY,    REV.Floats[0] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_DEBRIS_MAX_VELOCITY,    REV.Floats[1] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_DEBRIS_LIFE,            REV.Floats[2] );
                EventData.StoreBool( anim_event::BOOL_IDX_DEBRIS_BOUNCE,            REV.Bools[0] );
            break;

            case EVENT_TYPE_SET_MESH:
                EventData.StoreString( anim_event::STRING_IDX_SET_MESH,             REV.Strings[0] );
                EventData.StoreString( anim_event::STRING_IDX_SET_MESH_ON_OR_OFF,   REV.Strings[1] );
            break;

            case EVENT_TYPE_SWAP_MESH:
                EventData.StoreString( anim_event::STRING_IDX_SWAP_MESH_ON,        REV.Strings[0] );
                EventData.StoreString( anim_event::STRING_IDX_SWAP_MESH_OFF,       REV.Strings[1] );
            break;

            case EVENT_TYPE_FADE_GEOMETRY:
                if( x_strlen( REV.Strings[0] ) == 0 || x_strcmp( REV.Strings[0], "Fade-Out" ) == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_FADE_DIRECTION, -1 );
                }
                else
                if( x_strcmp( REV.Strings[0], "Fade-In") == 0 )
                {
                    EventData.StoreInt( anim_event::INT_IDX_FADE_DIRECTION, 1 );
                }
                EventData.StoreFloat( anim_event::FLOAT_IDX_GEOMETRY_FADE_TIME, REV.Floats[0] );
            break;

            case EVENT_TYPE_SWAP_TEXTURE:
                EventData.StoreString( anim_event::STRING_IDX_SET_TEXTURE,      REV.Strings[0] );
                EventData.StoreString( anim_event::STRING_IDX_USE_TEXTURE,      REV.Strings[1] );
                break;

            case EVENT_TYPE_CAMERA_FOV:
                EventData.StoreFloat( anim_event::FLOAT_IDX_CAMERA_FOV,         REV.Floats[0] );
                EventData.StoreFloat( anim_event::FLOAT_IDX_CAMERA_FOV_TIME,    REV.Floats[1] );
                break;

            default:
                ThrowError( "Unhandled Event in AnimCompiler" );
                //ASSERT( 0 ); // event type not handled
               
            }

            Anim.m_pAnimGroup = &AG;
            Anim.SetEventData( j, EventData );
        }

        // Sort the events in order of starting frame
        anim_event* pEV = &AG.m_pEvent[ Anim.m_iEvent ];

        for( j=0; j<Anim.m_nEvents; j++ )
        {
            s32 BestI = j;
            for( k=j+1; k<Anim.m_nEvents; k++ )
            {
                if( pEV[k].StartFrame() < pEV[BestI].StartFrame() )
                    BestI = k;
            }
            anim_event AE = pEV[j];
            pEV[j]        = pEV[BestI];
            pEV[BestI]    = AE;
        }
    }
    
    ClearUserInfo();
}

//=========================================================================

void anim_compiler::BuildProps( anim_group& AG )
{
    x_DebugMsg("Building Props...\n");

    s32 i,j;

    // The number of props was already computed in BuildAnimInfo

    //
    // Allocate props
    //
    AG.m_pProp = (anim_prop*)x_malloc(sizeof(anim_prop)*AG.m_nProps);
    ASSERT( AG.m_pProp );
    x_memset( AG.m_pProp, 0, sizeof(anim_prop)*AG.m_nProps );

    //
    // Fill out prop structures
    //
    for( i=0; i<m_nAnims; i++ )
    {
        anim_info& Anim = AG.m_pAnimInfo[i];
        rawanim&   RAnim = m_Anim[i].RawAnim;

        // Copy prop data
        for( j=0; j<Anim.m_nProps; j++ )
        {
            anim_prop& P = AG.m_pProp[ j + Anim.m_iProp ];
            rawanim::prop& RP = RAnim.m_pProp[j];
            
            // Figure out which bone prop is attached to.  If not attached to any
            // known bone then attach to root
            P.m_iBone      = AG.GetBoneIndex( RP.ParentName );

            // Copy other basic info
            x_strncpy(P.m_Type, RP.Type, 31);
        }
    }
}
//=========================================================================

void anim_compiler::BuildAnimKeys( anim_group& AG )
{
    x_DebugMsg("Building Compressed Keyframe Data...\n");

    s32 i,j,k;

    //
    // Initialize compressed data stream
    //
    m_BS.Init( 500*1024 );
    m_BS.SetMaxGrowSize( 1024*1024*1024 );

    //
    // Compute total number of keyblocks needed and allocate array
    //
    ASSERT( AG.m_nKeyBlocks == 0 );

    for( i=0; i<m_nAnims; i++ )
    {
        anim_info& Anim     = AG.m_pAnimInfo[i];
        anim_keys& Keys     = Anim.m_AnimKeys;

        Keys.m_nBones       = AG.m_nBones;
        Keys.m_nProps       = Anim.m_nProps;
        Keys.m_nFrames      = Anim.m_nFrames;
        Keys.m_iKeyBlock    = AG.m_nKeyBlocks;
        Keys.m_nKeyBlocks   = 0;

        AG.m_TotalNFrames   += Keys.m_nFrames;
        AG.m_TotalNKeys     += (Keys.m_nBones + Keys.m_nProps)*Keys.m_nFrames;

        s32 SF = 0;
        for( SF=0; SF<Anim.m_nFrames-1; )
        {
            Keys.m_nKeyBlocks++;
            SF = SF + MAX_KEYS_PER_BLOCK;
        }

        AG.m_nKeyBlocks     += Keys.m_nKeyBlocks;
    }

    //
    // Allocate the keyblocks
    //
    AG.m_pKeyBlock = (anim_key_block*)x_malloc( sizeof(anim_key_block) * AG.m_nKeyBlocks );
    ASSERT( AG.m_pKeyBlock );
    x_memset( AG.m_pKeyBlock, 0, sizeof(anim_key_block) * AG.m_nKeyBlocks );

    //
    // Initialize raw key buffer and stream flags
    //
    s32         nRawKeysAllocated = 0;
    anim_key*   pKey = NULL;
    s32         nStreamFlagsAllocated = 0;
    u32*        pStreamFlag = NULL;

    //
    // Loop through and build keysets
    //
    for( i=0; i<m_nAnims; i++ )
    {
        anim_info&              Anim     = AG.m_pAnimInfo[i];
        anim_keys&              Keys     = Anim.m_AnimKeys;
        rawanim::frame*         pRawKey  = m_Anim[i].RawAnim.m_pFrame;
        rawanim::prop_frame*    pPropKey = m_Anim[i].RawAnim.m_pPropFrame;
        
        SetUserInfo( m_Anim[i].RawAnim );
        
        x_DebugMsg("compressing...");
        s32 BitstreamCursor = m_BS.GetCursor();
        ASSERT( (BitstreamCursor%8) == 0 );

        // Clear animated bone min/max indices
        s32 iAnimBoneMin = -1;
        s32 iAnimBoneMax = -1;

        //
        // Compute size of rawkey buffer and realloc if we need to
        //
        s32 nRawKeys = (Keys.m_nBones + Keys.m_nProps) * Keys.m_nFrames;
        if( nRawKeys > nRawKeysAllocated )
        {
            nRawKeysAllocated = nRawKeys;
            pKey = (anim_key*)x_realloc( pKey, nRawKeysAllocated*sizeof(anim_key) );
            ASSERT( pKey );
        }

        //
        // Compute size of stream flags buffer and realloc if we need to
        //
        s32 nStreamFlags = (Keys.m_nBones + Keys.m_nProps);
        if( nStreamFlags > nStreamFlagsAllocated )
        {
            nStreamFlagsAllocated = nStreamFlags;
            pStreamFlag = (u32*)x_realloc( pStreamFlag, nStreamFlagsAllocated*sizeof(u32) );
            ASSERT( pStreamFlag );
        }

        //
        // Clear stream flags and copy in masked bit
        //
        x_memset( pStreamFlag, 0, sizeof(u32)*nStreamFlagsAllocated );
        for( j=0; j<Keys.m_nBones; j++ )
        if( m_Anim[i].RawAnim.m_pBone[j].bIsMasked )
            pStreamFlag[j] |= STREAM_FLAG_MASKED;

        //
        // I - is a running index in the raw key data
        //
        s32 I=0;

        //
        // Copy Bone keys into raw buffer
        //
        for( j=0; j<Keys.m_nBones; j++ )
        {
            // If bone is masked then nuke values
            if( pStreamFlag[j] & STREAM_FLAG_MASKED )
            {
                for( k=0; k<Keys.m_nFrames; k++ )
                {
                    pKey[I].Scale = vector3(1,1,1);
                    pKey[I].Rotation.Identity();
                    pKey[I].Translation.Zero();
                    I++;
                }
            }
            else
            {
                for( k=0; k<Keys.m_nFrames; k++ )
                {
                    pKey[I].Scale       = pRawKey[k*Keys.m_nBones + j].Scale;
                    pKey[I].Rotation    = pRawKey[k*Keys.m_nBones + j].Rotation;
                    pKey[I].Translation = pRawKey[k*Keys.m_nBones + j].Translation;
                    I++;
                }
            }
        }

        //
        // Copy Prop keys into bottom of raw buffer
        //
        for( j=0; j<Keys.m_nProps; j++ )
        {
            // Gather all the keys for this prop
            for( k=0; k<Keys.m_nFrames; k++ )
            {
                matrix4 PM;
                rawanim::prop_frame* pF  = &m_Anim[i].RawAnim.m_pPropFrame[k*Keys.m_nProps+j];
                PM.Setup( pF->Scale, pF->Rotation, pF->Translation );

                // Make relative to parent if attached to one
                s32 iParent = AG.m_pProp[ j + Anim.m_iProp ].m_iBone ;
                if (iParent != -1)
                {
                    matrix4 ParentInvM;
                    m_Anim[i].RawAnim.ComputeRawBoneL2W( AG.m_pProp[ j + Anim.m_iProp ].m_iBone, ParentInvM, k );
                    ParentInvM.Invert();
                    PM = ParentInvM * PM;
                }
                
                pKey[I].Scale       = PM.GetScale();
                pKey[I].Rotation    = PM.GetQuaternion();
                pKey[I].Translation = PM.GetTranslation();
                I++;
            }
        }
        
        //
        // Force all quaternions to have positive W
        //
        if( 1 )
        {
            for( j=0; j<(Keys.m_nBones+Keys.m_nProps)*Keys.m_nFrames; j++ )
            if( pKey[j].Rotation.W < 0 )
            {
                pKey[j].Rotation.X = -pKey[j].Rotation.X;
                pKey[j].Rotation.Y = -pKey[j].Rotation.Y;
                pKey[j].Rotation.Z = -pKey[j].Rotation.Z;
                pKey[j].Rotation.W = -pKey[j].Rotation.W;
            }
        }

        //
        // Nuke all keyframes
        //
        if( 0 )
        {
            for( j=0; j<(Keys.m_nBones+Keys.m_nProps)*Keys.m_nFrames; j++ )
            {
                pKey[j].Scale = vector3(1,1,1);
                pKey[j].Rotation.Identity();
                pKey[j].Translation.Zero();
            }
        }

        //
        // The entire raw key buffer is setup.
        // Now loop through each key_block to be built
        //
        for( j=0; j<Keys.m_nKeyBlocks; j++ )
        {
            // Get access to keyblock structure
            anim_key_block& KeyBlock = AG.m_pKeyBlock[ j + Keys.m_iKeyBlock ];

            // Store byte offset of where this block's comp data starts
            ASSERT( (m_BS.GetCursor()%8) == 0 );
            KeyBlock.CompressedDataOffset = (m_BS.GetCursor() >> 3);
            s32 DataStartOffset = (m_BS.GetCursor() >> 3);

            // Clear pointer to factored out compressed data
            KeyBlock.pFactoredCompressedData = NULL;

            // Compute starting and ending frames for this block
            s32 SF = j*MAX_KEYS_PER_BLOCK;
            s32 EF = MIN( SF+MAX_KEYS_PER_BLOCK, Anim.m_nFrames-1 );

            KeyBlock.nFrames = (EF-SF)+1;

            s32 DecompDataSize;

            // Call the compression routine
            CompressAnimationData(  m_BS, 
                                    pKey,
                                    AG.m_pBone,
                                    AG.m_nBones,
                                    pStreamFlag,
                                    Keys.m_nFrames,
                                    Keys.m_nBones+Keys.m_nProps,
                                    SF, EF,
                                    m_MaxDistToVerts,
                                    DecompDataSize,
                                    iAnimBoneMin,
                                    iAnimBoneMax );

            KeyBlock.DecompressedDataSize = DecompDataSize;

            {
                ASSERT( (m_BS.GetCursor()%8) == 0 );
                s32 DataEndOffset = (m_BS.GetCursor() >> 3);
                const byte* pDataPtr = m_BS.GetDataPtr();
                KeyBlock.Checksum = x_chksum( pDataPtr + DataStartOffset, DataEndOffset-DataStartOffset );
            }
/*
            {
                static X_FILE* fp = NULL;
                if( !fp ) fp = x_fopen("c:/temp/animblock.csv","at");
                if( fp )
                {
                    x_fprintf(fp,"%08X,%8d,%8d,%s,\n",KeyBlock.Checksum,KeyBlock.nFrames,KeyBlock.DecompressedDataSize,Anim.m_Name);
                    x_fflush(fp);
                }
            }
*/
        }

        // Store animated bone min/max indices
        AG.m_pAnimInfo[i].m_iAnimBoneMin = iAnimBoneMin;
        AG.m_pAnimInfo[i].m_iAnimBoneMax = iAnimBoneMax;

        s32 CompressedSize = m_BS.GetCursor() - BitstreamCursor;
        x_DebugMsg("[%8d] %s\n",(CompressedSize+7)/8,Anim.m_Name);
    }

    ClearUserInfo();

    //
    // Release rawkey buffer and flag buffer
    //
    x_free( pKey );
    x_free( pStreamFlag );
}

//=========================================================================
/*
void anim_compiler::BuildScaleKeys( const anim_key*    pSrcKey,
                                    s32          nKeys,
                                    s32&         ScaleKeyFormat,
                                    byte*&       pScaleKeyData,
                                    s32&         nScaleKeyDataBytes )
{
    s32 i,j;

    vector3 ScaleMin;
    vector3 ScaleMax;
    f32 MinValue[3] = {F32_MAX,F32_MAX,F32_MAX};
    f32 MaxValue[3] = {-F32_MAX,-F32_MAX,-F32_MAX};
    for( i=0; i<nKeys; i++ )
    {
        f32* pF = (f32*)&pSrcKey[i].Scale;
        for( j=0; j<3; j++ )
        {
            MinValue[j] = MIN(MinValue[j],pF[j]);
            MaxValue[j] = MAX(MaxValue[j],pF[j]);
        }
    }

    //
    // Check if all scales = 1.0f
    //
    if( 1 )
    {
        f32 E = 0.001f;

        for( i=0; i<nKeys; i++ )
        {
            if( x_abs( pSrcKey[i].Scale.X - 1.0f ) > E )
                break;
            if( x_abs( pSrcKey[i].Scale.Y - 1.0f ) > E )
                break;
            if( x_abs( pSrcKey[i].Scale.Z - 1.0f ) > E )
                break;
        }
        if( i==nKeys )
        {
            nScaleKeyDataBytes = 0;
            ScaleKeyFormat = anim_key_set::KNOWN_CONSTANT;
            pScaleKeyData = NULL;
            return;
        }
    }

    //
    // Check if all scales are the same
    //
    if( 1 )
    {
        f32 E = 0.05f;
        vector3 S = pSrcKey[0].Scale;

        for( i=0; i<nKeys; i++ )
        {
            if( x_abs( pSrcKey[i].Scale.X - S.X ) > E )
                break;
            if( x_abs( pSrcKey[i].Scale.Y - S.Y ) > E )
                break;
            if( x_abs( pSrcKey[i].Scale.Z - S.Z ) > E )
                break;
        }

        if( i==nKeys )
        {
        //x_DebugMsg("SCALE SAME (%f,%f,%f) (%f,%f,%f)\n",
        //    MinValue[0],MinValue[1],MinValue[2],
        //    MaxValue[0],MaxValue[1],MaxValue[2]);

            nScaleKeyDataBytes = sizeof(vector3);
            vector3* pS = (vector3*)new byte[ nScaleKeyDataBytes ];
            pS[0] = pSrcKey[0].Scale;
            ScaleKeyFormat = anim_key_set::SINGLE_VALUE;
            pScaleKeyData = (byte*)pS;
            return;
        }
    }

    //
    // Store full precision
    //
    {
        //x_DebugMsg("SCALE RANGE (%f,%f,%f) (%f,%f,%f)\n",
        //    MinValue[0],MinValue[1],MinValue[2],
        //    MaxValue[0],MaxValue[1],MaxValue[2]);

        nScaleKeyDataBytes = sizeof(vector3)*nKeys;
        vector3* pS = (vector3*)new byte[ nScaleKeyDataBytes ];

        for( i=0; i<nKeys; i++ )
        {
            pS[i] = pSrcKey[i].Scale;
        }

        ScaleKeyFormat = anim_key_set::FULL_PRECISION;
        pScaleKeyData = (byte*)pS;
    }
}

//=========================================================================
s32 nRotSetsPassed=0;
void anim_compiler::BuildRotationKeys(   const anim_key*    pSrcKey,
                                    s32          nKeys,
                                    s32&         RotKeyFormat,
                                    byte*&       pRotKeyData,
                                    s32&         nRotKeyDataBytes )
{
    s32 i,j;

    //
    // Compression ideas:
    //
    // Range with different bits per sample
    // Watch for single axis with just angle changing
    //

    //
    // Get numerical ranges
    //
    f32 MinValue[4] = {1,1,1,1};
    f32 MaxValue[4] = {-1,-1,-1,-1};
    for( i=0; i<nKeys; i++ )
    {
        f32* pF = (f32*)&pSrcKey[i].Rotation;
        for( j=0; j<4; j++ )
        {
            MinValue[j] = MIN(MinValue[j],pF[j]);
            MaxValue[j] = MAX(MaxValue[j],pF[j]);
        }
    }

    //
    // Check if all rotations are the same
    //
    if( 1 )
    {
        f32 E = 0.001f;
        quaternion R = pSrcKey[0].Rotation;

        for( i=0; i<nKeys; i++ )
        {
            quaternion Q = pSrcKey[i].Rotation;
            
            if( ((Q.X>0)&&(R.X<0)) || ((Q.X>0)&&(R.X<0)))
                break;
            if( ((Q.Y>0)&&(R.Y<0)) || ((Q.Y>0)&&(R.Y<0)))
                break;
            if( ((Q.Z>0)&&(R.Z<0)) || ((Q.Z>0)&&(R.Z<0)))
                break;

            if( x_abs( pSrcKey[i].Rotation.X - R.X ) > E )
                break;
            if( x_abs( pSrcKey[i].Rotation.Y - R.Y ) > E )
                break;
            if( x_abs( pSrcKey[i].Rotation.Z - R.Z ) > E )
                break;
            if( x_abs( pSrcKey[i].Rotation.W - R.W ) > E )
                break;
        }
        if( i==nKeys )
        {
            nRotKeyDataBytes = sizeof(quaternion);
            quaternion* pR = (quaternion*)new byte[ nRotKeyDataBytes ];
            pR[0] = pSrcKey[0].Rotation;
            RotKeyFormat = anim_key_set::SINGLE_VALUE;
            pRotKeyData = (byte*)pR;
            return;
        }
    }

    //
    // Check if rotations can be 8bit
    //
    if( m_bUseCompression )
    {
        RotKeyFormat = anim_key_set::PRECISION_8;
        nRotKeyDataBytes = (sizeof(quaternion)*2)+(nKeys*4);
        byte* pR = (byte*)new byte[nRotKeyDataBytes];
        pRotKeyData = (byte*)pR;

        // Setup initial quaternions
        ((f32*)pR)[0] = MinValue[0];
        ((f32*)pR)[1] = MinValue[1];
        ((f32*)pR)[2] = MinValue[2];
        ((f32*)pR)[3] = MinValue[3];
        ((f32*)pR)[4] = MaxValue[0];
        ((f32*)pR)[5] = MaxValue[1];
        ((f32*)pR)[6] = MaxValue[2];
        ((f32*)pR)[7] = MaxValue[3];
        pR += sizeof(f32)*8;

        // Setup bytes per quaternion
        for( i=0; i<nKeys; i++ )
        {
            quaternion Q = pSrcKey[i].Rotation;
            s32 XI = (s32)(255.0f*(Q.X-MinValue[0]) / (MaxValue[0]-MinValue[0]));
            s32 YI = (s32)(255.0f*(Q.Y-MinValue[1]) / (MaxValue[1]-MinValue[1]));
            s32 ZI = (s32)(255.0f*(Q.Z-MinValue[2]) / (MaxValue[2]-MinValue[2]));
            s32 WI = (s32)(255.0f*(Q.W-MinValue[3]) / (MaxValue[3]-MinValue[3]));
            ASSERT( (XI>=0) && (XI<=255) );
            ASSERT( (YI>=0) && (YI<=255) );
            ASSERT( (ZI>=0) && (ZI<=255) );
            ASSERT( (WI>=0) && (WI<=255) );
            pR[0] = (byte)XI;
            pR[1] = (byte)YI;
            pR[2] = (byte)ZI;
            pR[3] = (byte)WI;
            pR += 4;
        }

        return;
    }

    //
    // Compress in 8 packs
    //
    if( 0 && m_bUseCompression )
    {
        static f32 ITable[16] = {0,0.142857f,0.28571f,0.428571f,0.571428f,0.714285f,0.857143f,1.0f,
                                0.1f,0.2f,0.3f,0.5f,0.6f,0.7f,0.8f,0.9f};

        s32 nPacks = nKeys / 8;
        if( nPacks*8 < nKeys ) nPacks++;

        nRotKeyDataBytes = sizeof(u64)*2*nPacks;
        u64* pR = (u64*)new byte[ nRotKeyDataBytes ];

        f32 WorstDiffForPacks = 0;
        for( i=0; i<nPacks; i++ )
        {
            u64& PA = pR[(i*2)+0];
            u64& PB = pR[(i*2)+1];

            // Determine first and last frames
            s32 FrameA = (i+0)*8;
            s32 FrameB = FrameA+7;
            if( FrameB >= nKeys ) FrameB = nKeys-1;

            // Get boundary quaternions
            quaternion QA = pSrcKey[FrameA].Rotation;
            quaternion QB = pSrcKey[FrameB].Rotation;

            if( QA.W < 0 ) {QA.X *= -1; QA.Y*=-1; QA.Z*=-1; QA.W*=-1;}
            if( QB.W < 0 ) {QB.X *= -1; QB.Y*=-1; QB.Z*=-1; QB.W*=-1;}

            // Pack boundary quaternions into PA & PB
            s64 XA = (QA.X>0) ? ((s32)((QA.X*4095.0f)+0.5f)) : ((s32)((QA.X*4095.0f)-0.5f));
            s64 YA = (QA.Y>0) ? ((s32)((QA.Y*4095.0f)+0.5f)) : ((s32)((QA.Y*4095.0f)-0.5f));
            s64 ZA = (QA.Z>0) ? ((s32)((QA.Z*4095.0f)+0.5f)) : ((s32)((QA.Z*4095.0f)-0.5f));
            s64 WA = (QA.W>0) ? ((s32)((QA.W*4095.0f)+0.5f)) : ((s32)((QA.W*4095.0f)-0.5f));
            s64 XB = (QB.X>0) ? ((s32)((QB.X*4095.0f)+0.5f)) : ((s32)((QB.X*4095.0f)-0.5f));
            s64 YB = (QB.Y>0) ? ((s32)((QB.Y*4095.0f)+0.5f)) : ((s32)((QB.Y*4095.0f)-0.5f));
            s64 ZB = (QB.Z>0) ? ((s32)((QB.Z*4095.0f)+0.5f)) : ((s32)((QB.Z*4095.0f)-0.5f));
            s64 WB = (QB.W>0) ? ((s32)((QB.W*4095.0f)+0.5f)) : ((s32)((QB.W*4095.0f)-0.5f));

            u32 Mask = ((1<<13)-1);
            PA =    ((XA & Mask) << (64-(13*1))) | ((YA & Mask) << (64-(13*2))) |
                    ((ZA & Mask) << (64-(13*3))) | ((WA & Mask) << (64-(13*4)));
            PB =    ((XB & Mask) << (64-(13*1))) | ((YB & Mask) << (64-(13*2))) |
                    ((ZB & Mask) << (64-(13*3))) | ((WB & Mask) << (64-(13*4)));

            // Decide on interpolation values for middle keys
            s32 IIndex[6];
            
            quaternion BlendQ[16];
            matrix4    BlendM[16];
            for( j=0; j<16; j++ )
            {
                BlendQ[j] = Blend(QA,QB,ITable[j]);
                BlendM[j].Identity();
                BlendM[j].Setup( BlendQ[j] );
            }

            f32 WorstDiff=0;
            for( j=0; j<6; j++ )
            {
                s32 F = FrameA + j + 1;
                if( F >= nKeys ) F = nKeys-1;

                quaternion Q = pSrcKey[F].Rotation;
                if( Q.W < 0 ) {Q.X *= -1; Q.Y*=-1; Q.Z*=-1; Q.W*=-1;}

                matrix4 MQ;
                MQ.Identity();
                MQ.Setup( Q );

                s32 BestI=0;
                f32 BestDiff=F32_MAX;
                for( s32 k=0; k<16; k++ )
                {
                    f32 D = MQ.Difference( BlendM[k] );
                    //f32 D = Q.Difference(BlendQ[k]);//Q.X*BlendQ[k].X + Q.Y*BlendQ[k].Y + Q.Z*BlendQ[k].Z + Q.W*BlendQ[k].W;
                    if( D < BestDiff )
                    {
                        BestDiff = D;
                        BestI = k;
                    }
                }
                IIndex[j] = BestI;
                WorstDiff = MAX(WorstDiff,BestDiff);
            }

            if( WorstDiff > 0.0001f )
            {
                x_DebugMsg("Overriding interp choices\n");
                for( j=0; j<6; j++ )
                    IIndex[j] = (j+1);
            }
            WorstDiffForPacks = MAX(WorstDiffForPacks,WorstDiff);
                
            // Pack indices into PA & PB
            PA |= (IIndex[0]<<8) | (IIndex[1]<<4) | (IIndex[2]<<0);
            PB |= (IIndex[3]<<8) | (IIndex[4]<<4) | (IIndex[5]<<0);
        }

        x_DebugMsg("WORST DIFF FOR PACKS: %f\n",WorstDiffForPacks);

        RotKeyFormat = anim_key_set::QUAT_8_PACK;
        pRotKeyData = (byte*)pR;
        return;
    }

    //
    // Check if we can use 16bit range
    //
    if( m_bUseCompression )
    {
        nRotKeyDataBytes = sizeof(s16)*4*nKeys;
        s16* pR = (s16*)new byte[ nRotKeyDataBytes ];

        for( i=0; i<nKeys; i++ )
        {
            pR[(i*4)+0] = (s16)( pSrcKey[i].Rotation.X * 16384 );
            pR[(i*4)+1] = (s16)( pSrcKey[i].Rotation.Y * 16384 );
            pR[(i*4)+2] = (s16)( pSrcKey[i].Rotation.Z * 16384 );
            pR[(i*4)+3] = (s16)( pSrcKey[i].Rotation.W * 16384 );
        }

        RotKeyFormat = anim_key_set::PRECISION_16;
        pRotKeyData = (byte*)pR;
        return;
    }

    //
    // Store full precision
    //
    {
        nRotKeyDataBytes = sizeof(quaternion)*nKeys;
        quaternion* pR = (quaternion*)new byte[ nRotKeyDataBytes ];

        for( i=0; i<nKeys; i++ )
        {
            pR[i] = pSrcKey[i].Rotation;
        }

        RotKeyFormat = anim_key_set::FULL_PRECISION;
        pRotKeyData = (byte*)pR;
    }
}

//=========================================================================

void anim_compiler::BuildTranslationKeys( const anim_key*    pSrcKey,
                                    s32          nKeys,
                                    s32&         TransKeyFormat,
                                    byte*&       pTransKeyData,
                                    s32&         nTransKeyDataBytes )
{
    s32 i;

    //
    // Check if all translations are the same
    //
    if( 1 )
    {
        f32 E = 0.01f;
        vector3 T = pSrcKey[0].Translation;

        for( i=0; i<nKeys; i++ )
        {
            if( x_abs( pSrcKey[i].Translation.X - T.X ) > E )
                break;
            if( x_abs( pSrcKey[i].Translation.Y - T.Y ) > E )
                break;
            if( x_abs( pSrcKey[i].Translation.Z - T.Z ) > E )
                break;
        }
        if( i==nKeys )
        {
            // Decide on 32bit or 16bit
            if( (x_abs(T.X)*16 > 32767) ||
                (x_abs(T.Y)*16 > 32767) ||
                (x_abs(T.Z)*16 > 32767) )
            {
                // 32 bit
                nTransKeyDataBytes = sizeof(vector3);
                vector3* pT = (vector3*)new byte[ nTransKeyDataBytes ];
                pT[0] = pSrcKey[0].Translation;
                TransKeyFormat = anim_key_set::SINGLE_VALUE;
                pTransKeyData = (byte*)pT;
                return;
            }
            else
            {
                //x_DebugMsg("SINGLE TRANSLATION: %f %f %f\n",T.X,T.Y,T.Z);
                // 16 bit
                nTransKeyDataBytes = sizeof(s16)*3;
                nTransKeyDataBytes = ALIGN_4(nTransKeyDataBytes);
                s16* pT = (s16*)new byte[ nTransKeyDataBytes ];
                pT[0] = (s16)(T.X*16.0f);
                pT[1] = (s16)(T.Y*16.0f);
                pT[2] = (s16)(T.Z*16.0f);
                TransKeyFormat = anim_key_set::SINGLE_VALUE_16;
                pTransKeyData = (byte*)pT;
                return;
            }
        }
    }

    if( m_bUseCompression )
    {
        //
        // Check if we can use 16bit
        //
        if( 1 )
        {
            // Check if all keys fit in bounds using 1/64th of a centimeter accuracy
            for( i=0; i<nKeys; i++ )
            {
                s32 X = (s32)(pSrcKey[i].Translation.X*16);
                s32 Y = (s32)(pSrcKey[i].Translation.Y*16);
                s32 Z = (s32)(pSrcKey[i].Translation.Z*16);
                if( (X>32767) || (X<-32768) || 
                    (Y>32767) || (Y<-32768) ||  
                    (Z>32767) || (Z<-32768) )
                    break;
            }
        
            // All the keys fit within 16bit range
            if( i==nKeys )
            {
                TransKeyFormat = anim_key_set::PRECISION_16;
                nTransKeyDataBytes = sizeof(s16)*nKeys*3;
                nTransKeyDataBytes = ALIGN_4(nTransKeyDataBytes);
                s16* pT = (s16*)new byte[nTransKeyDataBytes];
                for( i=0; i<nKeys; i++ )
                {
                    s16 X = (s32)(pSrcKey[i].Translation.X*16);
                    s16 Y = (s32)(pSrcKey[i].Translation.Y*16);
                    s16 Z = (s32)(pSrcKey[i].Translation.Z*16);
                    pT[(i*3)+0] = X;
                    pT[(i*3)+1] = Y;
                    pT[(i*3)+2] = Z;
                }
                pTransKeyData = (byte*)pT;
                return;
            }
        }
    }

    //
    // Store full precision
    //
    {
        nTransKeyDataBytes = sizeof(vector3)*nKeys;
        vector3* pT = (vector3*)new byte[ nTransKeyDataBytes ];

        for( i=0; i<nKeys; i++ )
        {
            pT[i] = pSrcKey[i].Translation;
        }

        TransKeyFormat = anim_key_set::FULL_PRECISION;
        pTransKeyData = (byte*)pT;
    }
}

//=========================================================================

void anim_compiler::BuildCompressedKeys( anim_key_set& KeySet, const anim_key* pKey, s32 nFrames, xbytestream& DataStream )
{
    s32     Format;
    byte*   pData;
    s32     nBytes;

    KeySet.m_nFrames = nFrames;

    //static X_FILE* fp = NULL;
    //if( fp == NULL ) fp = x_fopen("c:/temp/animcomp2.txt","wt");

    //
    // Build scale keys
    //
    BuildScaleKeys( pKey, nFrames, Format, pData, nBytes );
    KeySet.m_pScale = (byte*)DataStream.GetLength();
    KeySet.m_ScaleFormat = (byte)Format;
    DataStream.Append( pData, nBytes );
    delete[] pData;
    //x_fprintf(fp,"%1d %5d   ",KeySet.m_ScaleFormat,nBytes);

    //
    // Build rotation keys
    //
    BuildRotationKeys( pKey, nFrames, Format, pData, nBytes );
    KeySet.m_pRotation      = (byte*)DataStream.GetLength();
    KeySet.m_RotationFormat = (byte)Format;
    DataStream.Append( pData, nBytes );
    delete[] pData;
    //x_fprintf(fp,"%1d %5d   ",KeySet.m_RotationFormat,nBytes);

    //
    // Build Translation keys
    //
    BuildTranslationKeys( pKey, nFrames, Format, pData, nBytes );
    KeySet.m_pTranslation      = (byte*)DataStream.GetLength();
    KeySet.m_TranslationFormat = (byte)Format;
    DataStream.Append( pData, nBytes );
    delete[] pData;
    //x_fprintf(fp,"%1d %5d   \n",KeySet.m_TranslationFormat,nBytes);
    //x_fflush(fp);
}
*/

//=========================================================================

void anim_compiler::SetOutputName( const char* pFileName )
{
    x_strcpy(m_OutputFileName,pFileName);
}

//=========================================================================

void anim_compiler::ClearUserInfo( void )
{
    x_strcpy( m_SourceFile,   "Unknown source file" );
    x_strcpy( m_UserName,     "Unknown user" );
    x_strcpy( m_ComputerName, "Unknown computer" );
    m_ExportDate        [0] = 0;
    m_ExportDate        [1] = 0;
    m_ExportDate        [2] = 0;
}

//=========================================================================

void  anim_compiler::SetUserInfo( const rawmesh2& RawMesh )
{
    // Copy anim user info
    x_strcpy( m_SourceFile,   RawMesh.m_SourceFile );
    x_strcpy( m_UserName,     RawMesh.m_UserName );
    x_strcpy( m_ComputerName, RawMesh.m_ComputerName );
    m_ExportDate[0] = RawMesh.m_ExportDate[0];
    m_ExportDate[1] = RawMesh.m_ExportDate[1];
    m_ExportDate[2] = RawMesh.m_ExportDate[2];
}

//=============================================================================

void  anim_compiler::SetUserInfo( const rawanim& RawAnim )
{
    // Copy anim user info
    x_strcpy( m_SourceFile,   RawAnim.m_SourceFile );
    x_strcpy( m_UserName,     RawAnim.m_UserName );
    x_strcpy( m_ComputerName, RawAnim.m_ComputerName );
    m_ExportDate[0] = RawAnim.m_ExportDate[0];
    m_ExportDate[1] = RawAnim.m_ExportDate[1];
    m_ExportDate[2] = RawAnim.m_ExportDate[2];
}

//=============================================================================

void anim_compiler::ReportWarning( const char* pWarning )
{
    x_printf( "WARNING: %s\n", pWarning );
    x_printf( "  Error in [%s]\n  Last exported by: %s, on: %d/%d/%d\n", m_SourceFile, m_UserName, m_ExportDate[0], m_ExportDate[1], m_ExportDate[2] );
}

//=============================================================================

void anim_compiler::ReportError( const char* pError )
{
    x_printf( "ERROR: %s\n", pError );
    x_printf( "  Error in [%s]\n  Last exported by: %s, on: %d/%d/%d\n", m_SourceFile, m_UserName, m_ExportDate[0], m_ExportDate[1], m_ExportDate[2] );
}

//=============================================================================

void anim_compiler::ThrowError( const char* pError )
{
    ReportError( pError );
    x_throw( "AnimCompiler Error Occured" );
}

//=============================================================================
