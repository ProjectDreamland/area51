//=========================================================================
//
// MAIN.CPP
//
//=========================================================================

#include "x_files.hpp"
#include "Auxiliary\CommandLine\commandline.hpp"
#include "md5.h"

//=========================================================================

#define SECTOR_SIZE (2048)

//=========================================================================

s32 GeneratePrimeNear1000( s32 Value );
 
//=========================================================================

xbool   bVerbose = FALSE;
xstring PatchFileName;
xstring SourceFileName;
xstring DestFileName;
xstring ExecFileName;
s32     DataSpecified=0;

//=========================================================================

struct payload_header
{
    u32 ID0;
    u32 ID1;
    s64 PayloadOffset;
    s64 PayloadSize;
};

payload_header PayloadHeader;

//=========================================================================

xbool ReadPayloadHeader( void )
{
    //x_printf("ReadPayloadHeader: %s\n",ExecFileName);
    X_FILE* fp = x_fopen(ExecFileName,"rb");
    ASSERT(fp);
    s32 Size = x_flength( fp );
    x_fseek(fp,Size-sizeof(payload_header),X_SEEK_SET);
    x_fread(&PayloadHeader,sizeof(payload_header),1,fp);
    x_fclose(fp);

    if( (PayloadHeader.ID0 == 0xDEADBEEF) &&
        (PayloadHeader.ID1 == 0xFEEDC0DE) )
        return TRUE;

    return FALSE;
}

//=========================================================================

void InitPayloadHeader( X_FILE* fp )
{
    s32 Offset = x_ftell(fp);

    PayloadHeader.PayloadOffset = Offset;
    PayloadHeader.PayloadSize   = 0;
    PayloadHeader.ID0           = 0xDEADBEEF;
    PayloadHeader.ID1           = 0xFEEDC0DE;
}

//=========================================================================

void WritePayloadHeader( X_FILE* fp)
{
    s32 Offset = x_ftell( fp );
    PayloadHeader.PayloadSize = Offset - PayloadHeader.PayloadOffset;
    x_fwrite( &PayloadHeader, sizeof(payload_header), 1, fp );
}

//=========================================================================
//=========================================================================
//=========================================================================

u32     x_OpenFileForWrite  ( const char* pFileName );
u32     x_OpenFileForRead   ( const char* pFileName );
s32     x_ReadFromFile      ( u32 Handle, void* pDst, s32 nBytes );
s32     x_WriteToFile       ( u32 Handle, void* pDst, s32 nBytes );
void    x_SeekInFile        ( u32 Handle, s64 ByteOffset );

//=========================================================================

u32 x_OpenFileForWrite( const char* pFileName )
{
    X_FILE* fp = x_fopen(pFileName,"wb");
    return (u32)fp;
}

u32 x_OpenFileForRead( const char* pFileName )
{
    X_FILE* fp = x_fopen(pFileName,"rb");
    return (u32)fp;
}

s32 x_ReadFromFile( u32 Handle, void* pDst, s32 nBytes )
{
    s32 C = x_fread( pDst, 1, nBytes, (X_FILE*)Handle );
    return C;
}

s32 x_WriteToFile( u32 Handle, void* pSrc, s32 nBytes )
{
    s32 C = x_fwrite( pSrc, 1, nBytes, (X_FILE*)Handle );
    return C;
}

void x_SeekInFile( u32 Handle, s64 ByteOffset )
{
    x_fseek( (X_FILE*)Handle, (s32)ByteOffset, X_SEEK_SET );
}
//=========================================================================
//=========================================================================
//=========================================================================

struct checksum
{
    inline s32 ComputeHash( s32 HashTableSize )
    {
        return (D[0]^D[1]^D[2]^D[3]) % HashTableSize;
    };

    inline xbool IsEqual( const checksum& CS )
    {
        return ((D[0]==CS.D[0]) && (D[1]==CS.D[1]) && (D[2]==CS.D[2]) && (D[3]==CS.D[3]));
    }

    inline void Compute( void* pData, s32 Len )
    {
        MD5Context md5;
        MD5Init( &md5 );
        MD5Update( &md5, (const u8*)pData, Len );
        MD5Final( &md5, (u8*)D );
    }

    xstring BuildString( void )
    {
        xstring S;
        S += xfs("%08X",D[0]);
        S += xfs(":%08X",D[1]);
        S += xfs(":%08X",D[2]);
        S += xfs(":%08X",D[3]);
        return S;
    }

    u32    D[4];
};

//=========================================================================

struct isomorph_header
{
    char        m_SrcISOFileName[256];
    char        m_DstISOFileName[256];
    checksum    m_SrcChecksum;
    checksum    m_DstChecksum;
    s64         m_DstFileSize;
};

struct isomorph_cmd
{
    u32 Command:1,
        SourceSectorOffset:31;
};

//=========================================================================

struct ref
{
    checksum Checksum;
};

//=========================================================================

checksum BuildChecksumFromFile( X_FILE* fp )
{
    byte Buffer[SECTOR_SIZE];

    MD5Context FullMD5;
    MD5Init( &FullMD5 );

    s32 C=0;
    while( 1 )
    {
        s32 nBytesRead = x_fread( Buffer, 1, SECTOR_SIZE, fp );
        //x_printf("[%6d] nBytesRead %d\n",C,nBytesRead);
        C++;
        //if( C==4000 ) break;
        if( nBytesRead==0 )
            break;
        MD5Update( &FullMD5, Buffer, nBytesRead );
    }
//    x_printf("READ %d SECTORS\n",C);

    checksum Checksum;
    MD5Final( &FullMD5, (u8*)Checksum.D );

    return Checksum;
}

//=========================================================================

void BuildSourceSectors( X_FILE* fp, s32& nSectors, ref*& pRef )
{
    const s32 MAX_BUFFER_SIZE = 512*1024;

    // This is written in this manner to take into account huge files
    x_fseek( fp, 0, X_SEEK_SET );

    s32 nSectorsAllocated = 0;
    nSectors = 0;
    pRef  = NULL;

    // Setup a buffer size that is a multiple of the reference size
    s32 BuffSize = (MAX_BUFFER_SIZE / SECTOR_SIZE);
    if( BuffSize < 1) BuffSize = 1;
    BuffSize *= SECTOR_SIZE;
    byte* pBuff = (byte*)x_malloc( BuffSize );
    ASSERT(pBuff);

    s32 TotalSectors=0;
    s32 TotalSize=0;
    xbool bEOF = FALSE;
    while( !bEOF )
    {
        // Read 
        s32 nBytesRead = x_fread( pBuff, 1, BuffSize, fp );
        
        // Compute number of refs read based on bytes read
        s32 nSectorsRead = (nBytesRead / SECTOR_SIZE);
        if( nSectorsRead*SECTOR_SIZE < nBytesRead )
            nSectorsRead++;

        // Check if we read the full buffer
        if( nBytesRead < BuffSize )
        {
            x_memset( pBuff+nBytesRead, 0, BuffSize-nBytesRead );
            bEOF = TRUE;
            if( nBytesRead==0 )
                break;
        }

        // Add these sectors to the list 
        for( s32 i=0; i<nSectorsRead; i++ )
        {
            if( nSectors == nSectorsAllocated )
            {
                nSectorsAllocated += 1024*128;
                pRef = (ref*)x_realloc( pRef, nSectorsAllocated*sizeof(ref) );
            }

            ref& RF = pRef[nSectors];

            // Compute the checksum for this block
            RF.Checksum.Compute( pBuff + SECTOR_SIZE*i, SECTOR_SIZE );

            //x_printf("Ref [%05d] (%08X)\n",nSectors,RF.Checksum);
            nSectors++;
        }
    }

    // resize ref array back down to normal
    pRef = (ref*)x_realloc( pRef, nSectors*sizeof(ref) );
    x_free(pBuff);
}

//=========================================================================

void BuildPatches( void )
{
    x_printf("Source File:  %s\n",SourceFileName);
    x_printf("Dest File:    %s\n",DestFileName);
    x_printf("-----------------------------------------------------\n");

    x_printf("Building SelfExecuting Patch File...\n");

    // Copy Executable into buffer
    byte* ExecBuffer;
    s32   ExecSize;
    {
        x_printf("  - Copying executable into buffer.\n");
        X_FILE* fp = x_fopen(ExecFileName,"rb");
        ASSERT(fp);
        ExecSize = x_flength( fp );
        ExecBuffer = (byte*)x_malloc(ExecSize);
        ASSERT(ExecBuffer);
        x_fread( ExecBuffer, ExecSize, 1, fp );
        x_fclose(fp);
    }

    // Build patch file name
    PatchFileName = "";
    {
        char PATH[X_MAX_PATH];
        char DRIVE[X_MAX_DRIVE];
        char DIR[X_MAX_DIR];
        char FNAME[X_MAX_FNAME];
        char EXT[X_MAX_EXT];
        x_splitpath(DestFileName,DRIVE,DIR,FNAME,EXT);
        x_makepath(PATH,DRIVE,DIR,xfs("isomorph_%s",FNAME),".exe");
        PatchFileName = PATH;
        x_printf("  - Exec name generated\n");
        x_printf("  - %s\n",PatchFileName);
    }
    
    // Open the patch file and write the executable
    x_printf("  - Writing executable\n");
    X_FILE* pPF = x_fopen(PatchFileName,"wb");
    if( !pPF )
    {
        x_printf("  - Could not open file for writing!!!\n");
        return;
    }
    x_fwrite( ExecBuffer, ExecSize, 1, pPF );
    InitPayloadHeader( pPF );


    s32     nSrcSectors;
    ref*    pSrcRef;
    xtimer  T;

    // Build checksum of source file
    checksum FullSourceChecksum;
    checksum FullDestChecksum;

    // Open source file and compute full checksum
    x_printf("  - Source File: %s\n",SourceFileName);
    X_FILE* pSF = x_fopen(SourceFileName,"rb");
    {
        if( !pSF )
        {
            x_printf("  - Could not open source file for reading!!!\n");
            return;
        }
        x_printf("    - Computing source checksum.\n");
        FullSourceChecksum = BuildChecksumFromFile(pSF);
        x_fseek(pSF,0,X_SEEK_SET);
        x_printf("    - Full Checksum:        %s\n",FullSourceChecksum.BuildString());
    }

    // Open dest file and compute full checksum
    x_printf("  - Dest File: %s\n",DestFileName);
    X_FILE* pDF = x_fopen(DestFileName,"rb");
    {
        if( !pDF )
        {
            x_printf("  - Could not open dest file for reading!!!\n");
            return;
        }
        x_printf("    - Computing dest checksum.\n");
        FullDestChecksum = BuildChecksumFromFile(pDF);
        x_fseek(pDF,0,X_SEEK_SET);
        x_printf("    - Full Checksum:        %s\n",FullDestChecksum.BuildString());
    }


    // Build source file references
    x_printf("  - Computing source file sector hash values.\n");
    {
        T.Reset(); T.Start();
        BuildSourceSectors( pSF, nSrcSectors, pSrcRef );
        T.Stop();
        x_printf("    - Execution Time:       %1.2f sec\n",T.ReadSec());
        x_printf("    - Total Source Sectors: %d\n",nSrcSectors);
    }

    // Build hash table for SrcSectors
    s32 nHashEntries;
    s32* pHash;
    {
        x_printf("    - Building source sector database.\n");
        T.Reset(); T.Start();

        nHashEntries = GeneratePrimeNear1000( nSrcSectors*3 );
        //nHashEntries = nSrcSectors*3;
        pHash = (s32*)x_malloc(sizeof(s32)*nHashEntries);
        x_memset(pHash,0xFFFFFFFF,sizeof(s32)*nHashEntries);
        ASSERT(pHash);

        s32 nCollisions=0;
        for( s32 i=0; i<nSrcSectors; i++ )
        {
            s32 I = pSrcRef[i].Checksum.ComputeHash(nHashEntries);
            s32 nLoops=0;
            while( 1 )
            {
                if( pHash[I]==-1 )
                {
                    pHash[I] = i;
                    break;
                }

                nLoops++;
                I++;
                if( I==nHashEntries ) I = 0;
            }
            if( nLoops>0 )
                nCollisions++;
        }

        T.Stop();
        x_printf("    - HashTable %d entries, %d bytes, %1.2f sec\n",nHashEntries,sizeof(s32)*nHashEntries,T.ReadSec());
        x_printf("    - nCollisions %d\n",nCollisions);
    }

    isomorph_header Header;
    x_memset( &Header, 0, sizeof(isomorph_header) );
    x_fseek( pPF, ExecSize, X_SEEK_SET );
    x_fwrite( &Header, sizeof(isomorph_header), 1, pPF );

    //X_FILE* pTemp = x_fopen("sectorA.txt","wt");

    // Loop through destination sectors and write .morph file
    T.Reset(); T.Start();
    s32 nAdds=0;
    s32 nCopies=0;
    s32 TotalSize=0;
    {
        x_printf("-----------------------------------------------------\n");
        x_printf("Writing Patches\n");

        // This is written in this manner to take into account huge files
        x_fseek( pDF, 0, X_SEEK_SET );

        byte Buffer[ SECTOR_SIZE ];

        s32 TotalSectors=0;
        xbool bEOF = FALSE;
        while( !bEOF )
        {
            // Read 
            s32 nBytesRead = x_fread( Buffer, 1, SECTOR_SIZE, pDF );
            Header.m_DstFileSize+=nBytesRead;
            
            // Check if we read the full buffer
            if( nBytesRead < SECTOR_SIZE )
            {
                x_memset( Buffer+nBytesRead, 0, SECTOR_SIZE-nBytesRead );
                bEOF = TRUE;
                if( nBytesRead==0 )
                    break;
            }

            checksum SectorChecksum;

            // Compute the checksum for this block
            SectorChecksum.Compute( Buffer, SECTOR_SIZE );
            //x_fprintf(pTemp,"[%08d] %s\n",TotalSectors,SectorChecksum.BuildString());

            TotalSectors++;

            // Search for sector in source
            s32 iSourceSector=-1;
            {
                s32 I = SectorChecksum.ComputeHash(nHashEntries);
                while( 1 )
                {
                    if( pHash[I] == -1 )
                    {
                        nAdds++;
                        TotalSize += sizeof(isomorph_cmd) + SECTOR_SIZE;

                        // Write an Add
                        isomorph_cmd CMD;
                        CMD.Command = 0;
                        CMD.SourceSectorOffset = 0;
                        x_fwrite(&CMD,sizeof(CMD),1,pPF);
                        x_fwrite(Buffer,SECTOR_SIZE,1,pPF);
                        break;
                    }

                    if( pSrcRef[ pHash[I] ].Checksum.IsEqual( SectorChecksum ) )
                    {
                        nCopies++;
                        TotalSize += sizeof(isomorph_cmd);

                        // Write as Copy
                        isomorph_cmd CMD;
                        CMD.Command = 1;
                        CMD.SourceSectorOffset = pHash[I];
                        x_fwrite(&CMD,sizeof(CMD),1,pPF);
                        break;
                    }

                    I++;
                    if( I==nHashEntries ) I = 0;
                }
            }

            if( (TotalSectors%10000)==0 )
            {
                x_printf("  - C:%d\tA:%d\tTS:%d\n",nCopies,nAdds,TotalSize);
            }
        }
    }
    T.Stop();
    x_printf("  - C:%d\tA:%d\tTS:%d\n",nCopies,nAdds,TotalSize);

    // Write payload identifier on end of executable
    WritePayloadHeader( pPF );

    // Build file names to store in header
    char EXT[X_MAX_EXT];
    x_splitpath(SourceFileName,NULL,NULL,Header.m_SrcISOFileName,EXT);
    x_strcat(Header.m_SrcISOFileName,EXT);
    x_splitpath(DestFileName,NULL,NULL,Header.m_DstISOFileName,EXT);
    x_strcat(Header.m_DstISOFileName,EXT);

    // Fill out checksums
    Header.m_SrcChecksum = FullSourceChecksum;
    Header.m_DstChecksum = FullDestChecksum;

    // rewrite final header
    x_fseek( pPF, ExecSize, X_SEEK_SET );
    x_fwrite( &Header, sizeof(isomorph_header), 1, pPF );

    // Close all files and free memory
    x_fclose(pPF);
    x_fclose(pSF);
    x_fclose(pDF);
    x_free(pHash);

    // Leave final remarks.
    x_printf("-----------------------------------------------------\n");
    x_printf("  - Patch File Finished. Congratulations.\n");
    x_printf("-----------------------------------------------------\n");
}

//=========================================================================

void PrintHelp( void )
{
    x_printf( "  usage:\n"
              "         IsoMorph [-opt [param]] \n"
              "\n"
              "options:\n"
              "         -s      <src file>      - source data file\n"
              "         -d      <dst file>      - destination data file\n"
              "-----------------------------------------------------\n"
             );
}

//=========================================================================

void ProcessCommandLine( command_line& CommandLine )
{
    s32 iOption;

    // Read V option
    iOption = CommandLine.FindOption( xstring("V") );
    if( iOption != -1 )
    {
        bVerbose = TRUE;
    }

    // Read Source file
    iOption = CommandLine.FindOption( xstring("S") );
    if( iOption != -1 )
    {
        SourceFileName = CommandLine.GetOptionString( iOption );
        DataSpecified++;
    }

    // Read Destination file
    iOption = CommandLine.FindOption( xstring("D") );
    if( iOption != -1 )
    {
        DestFileName = CommandLine.GetOptionString( iOption );
        DataSpecified++;
    }
}

//=========================================================================
/*
struct isomorph_header
{
    char        m_SrcISOFileName[256];
    char        m_DstISOFileName[256];
    checksum    m_SrcChecksum;
    checksum    m_DstChecksum;
    s64         m_DstFileSize;
};

*/
void BuildDestISOFromPatches( void )
{
//    byte Buffer[SECTOR_SIZE];

    // Open the executable file and advance to where data is stored
    X_FILE* pPF = x_fopen(ExecFileName,"rb");
    ASSERT(pPF);
    x_fseek( pPF, (s32)PayloadHeader.PayloadOffset, X_SEEK_SET );

    // Read the header
    isomorph_header Header;
    x_fread( &Header, sizeof(Header), 1, pPF );

    x_printf("  - Src Name       %s\n",Header.m_SrcISOFileName);
    x_printf("  - Src Checksum   %s\n",Header.m_SrcChecksum.BuildString());
    x_printf("  - Dst Name       %s\n",Header.m_DstISOFileName);
    x_printf("  - Dst Checksum   %s\n",Header.m_DstChecksum.BuildString());
    x_printf("  - Dst Final Size %d\n",Header.m_DstFileSize);
    x_printf("-----------------------------------------------------\n");

    // Check if the source iso has been provided.  If it hasn't
    // then just display the header info.
    X_FILE* pSF = x_fopen(SourceFileName,"rb");
    if( !pSF )
    {
        x_printf("  - Unable to open source file.\n");
        x_printf("  - %s\n",SourceFileName);
        return;
    }

    // Confirm checksum of source file
    {
        x_printf("  - Source file provided\n");
        x_printf("  - %s\n",SourceFileName);
        x_printf("  - Confirming checksum\n");

        checksum FullSrcChecksum = BuildChecksumFromFile( pSF );
        x_fseek( pSF, 0, X_SEEK_SET );

        x_printf("  - Src Checksum   %s\n",FullSrcChecksum.BuildString());
        if( FullSrcChecksum.IsEqual( Header.m_SrcChecksum ) )
        {
            x_printf("  - Source checksum matches!!!\n");
        }
        else
        {
            x_printf("  - Source checksum failed.  Wrong file!!!\n");
            return;
        }
    }

    // Build dest file name.  Use dest name in patch file and use path
    // of executable.
    {
        char PATH[X_MAX_PATH];
        char DRIVE[X_MAX_DRIVE];
        char DIR[X_MAX_DIR];
        char FNAME[X_MAX_FNAME];
        char EXT[X_MAX_EXT];
        x_splitpath(ExecFileName,DRIVE,DIR,FNAME,EXT);
        x_makepath(PATH,DRIVE,DIR,Header.m_DstISOFileName,NULL);
        DestFileName = PATH;
        x_printf("  - Dest name generated\n");
        x_printf("  - %s\n",DestFileName);
    }


    X_FILE* pDF = x_fopen(DestFileName,"wb");
    if( !pDF )
    {
        x_printf("  - Could not open dest file for writing!!!\n");
        return;
    }

    ASSERT( pPF && pSF && pDF );

    x_printf("-----------------------------------------------------\n");
    x_printf("Rebuilding Dest from Source and Patches\n");
    x_printf("-----------------------------------------------------\n");

    byte Buffer[SECTOR_SIZE];

    // Loop through the patch file
    s32 TotalSectors=0;
    s64 TotalBytesToWrite=Header.m_DstFileSize;
    s32 nSectorsPerDot = 1 + (TotalBytesToWrite/SECTOR_SIZE)/40;
    while( 1 )
    {
        isomorph_cmd CMD;
        if( !x_fread( &CMD, sizeof(CMD), 1, pPF ) )
            break;

        s32 nBytesToWrite = (s32)(MIN( TotalBytesToWrite, SECTOR_SIZE ));
        if( nBytesToWrite==0 )
            break;

        if( CMD.Command )
        {
            // Copy command
            x_fseek( pSF, ((s32)CMD.SourceSectorOffset)*SECTOR_SIZE, X_SEEK_SET );
            s32 nBytesRead = x_fread( Buffer, 1, SECTOR_SIZE, pSF );
            if( nBytesRead != SECTOR_SIZE )
            {
                x_memset( Buffer+nBytesRead, 0, SECTOR_SIZE-nBytesRead );
            }
            x_fwrite( Buffer, 1, nBytesToWrite, pDF );
        }
        else
        {
            // Add command
            x_fread( Buffer, SECTOR_SIZE, 1, pPF );
            x_fwrite( Buffer, 1, nBytesToWrite, pDF );
        }

        TotalBytesToWrite -= nBytesToWrite;

        if( (TotalSectors%nSectorsPerDot)==0 )
            x_printf(".");

        TotalSectors++;
    }
    x_printf("Finished\n");
    x_printf("-----------------------------------------------------\n");

    x_fclose(pPF);
    x_fclose(pSF);
    x_fclose(pDF);

    // Do final checksum on destination file
    {
        checksum DstChecksum;

        x_printf("  - Confirming destination checksum\n");
        X_FILE* fp = x_fopen(DestFileName,"rb");
        if( fp )
        {
            x_printf("  - Opened filed %s.\n",DestFileName);
            DstChecksum = BuildChecksumFromFile( fp );
            x_fclose(fp);
        }

        x_printf("  - Dst Checksum   %s\n",DstChecksum.BuildString());
        if( DstChecksum.IsEqual( Header.m_DstChecksum ) )
        {
            x_printf("  - Destination checksum matches!!!\n");
        }
        else
        {
            x_printf("  - Destination checksum failed!!!\n");
            x_printf("  - Unknown error\n");
            x_printf("-----------------------------------------------------\n");
            return;
        }
    }    

    x_printf("-----------------------------------------------------\n");
    x_printf("  - Patching Finished. Congratulations.\n");
    x_printf("-----------------------------------------------------\n");
}

//=========================================================================

void main( s32 argc, char* argv[] )
{
    x_Init(argc,argv);
    x_try;

    x_printf("\n");
    x_printf("-----------------------------------------------------\n");
    x_printf("IsoMorph (c)2005 Midway Studios Austin.\n");
    x_printf("-----------------------------------------------------\n");


    // Dump args
    if( 0 )
    {
        for( s32 i=0; i<argc; i++ )
            x_printf("[%d] %s\n",i,argv[i]);
    }

    // Capture executable name
    ExecFileName = argv[0];
    if( ExecFileName.Find(".exe")==-1 )
        ExecFileName += ".exe";
//    x_printf("EXEC NAME: %s\n",ExecFileName);

    // Check for payload
    if( ReadPayloadHeader() )
    {
        SourceFileName = "NoName";
        if( argc==2 )
            SourceFileName = argv[1];
        BuildDestISOFromPatches();

        while(1);
    }
    else
    {
        // Specify all the options
        command_line CommandLine;
        CommandLine.AddOptionDef( "V"     , command_line::SWITCH );
        CommandLine.AddOptionDef( "S"     , command_line::STRING );
        CommandLine.AddOptionDef( "D"     , command_line::STRING );

        // Parse the command line
        CommandLine.Parse( argc, argv );
        
        // Do the script
        ProcessCommandLine( CommandLine );

        if( DataSpecified != 2 )
        {
            PrintHelp();
            return;
        }

        BuildPatches();
        while(1);
    }

    x_catch_begin;
    #ifdef X_EXCEPTIONS
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    #endif
    x_catch_end;

    x_Kill();
}

//=========================================================================
