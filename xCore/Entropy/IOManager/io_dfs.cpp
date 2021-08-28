//==============================================================================
//==============================================================================
//==============================================================================
// IO_DFS.CPP
//==============================================================================
//==============================================================================
//==============================================================================

#include "x_files.hpp"
#include "io_dfs.hpp"

//==============================================================================

extern u16 crc16Table[];
#define crc16ApplyByte( v, crc ) (u16)((crc << 8) ^  crc16Table[((crc >> 8) ^ (v)) & 255])

//==============================================================================

dfs_header* dfs_InitHeaderFromRawPtr( void* pRawHeaderData, s32 Length )
{
    (void)Length;
    dfs_header* pHeader = (dfs_header*)pRawHeaderData;

#if defined(TARGET_XBOX)
    extern void OnIOErrorCallback( void );

    // Test the checksum.
    u8* pData        = (u8*)pRawHeaderData;
    s32 Checksum     = 0;
    s32 OrigChecksum = pHeader->Checksum;
    pHeader->Checksum = 0;
    while( Length-- )
    {
        Checksum = crc16ApplyByte( *pData, Checksum );
        pData++;
    }
    //ASSERT( Checksum == OrigChecksum );
    if( Checksum != OrigChecksum )
    {
        OnIOErrorCallback();
    }

#endif

    // Endian swap the header...
    pHeader->Magic          = LITTLE_ENDIAN_32( pHeader->Magic          );
    pHeader->Version        = LITTLE_ENDIAN_32( pHeader->Version        );
    pHeader->SectorSize     = LITTLE_ENDIAN_32( pHeader->SectorSize     );
    pHeader->SplitSize      = LITTLE_ENDIAN_32( pHeader->SplitSize      );
    pHeader->nFiles         = LITTLE_ENDIAN_32( pHeader->nFiles         );
    pHeader->nSubFiles      = LITTLE_ENDIAN_32( pHeader->nSubFiles      );
    pHeader->StringsLength  = LITTLE_ENDIAN_32( pHeader->StringsLength  );
    pHeader->pSubFileTable  = (dfs_subfile*)LITTLE_ENDIAN_32( pHeader->pSubFileTable  );
    pHeader->pFiles         = (dfs_file*)   LITTLE_ENDIAN_32( pHeader->pFiles         );     
    pHeader->pChecksums     = (u16*)        LITTLE_ENDIAN_32( pHeader->pChecksums     );     
    pHeader->pStrings       = (char*)       LITTLE_ENDIAN_32( pHeader->pStrings       );

    // Make sure its valid!
    if( (pHeader->Version == DFS_VERSION) )
    {
        dfs_file* pEntry;
        s32       i;

        // Now fixup the offsets in the header.
        u32 BaseAddr = (u32)pHeader;
        pHeader->pSubFileTable  = (dfs_subfile*)(BaseAddr + (u32)pHeader->pSubFileTable  );
        pHeader->pFiles         = (dfs_file*)   (BaseAddr + (u32)pHeader->pFiles         );
        if( pHeader->pChecksums )
        {
            pHeader->pChecksums = (u16*)        (BaseAddr + (u32)pHeader->pChecksums     );
        }
        pHeader->pStrings       = (char*)       (BaseAddr + (u32)pHeader->pStrings       );

        // Byte swap the filesize / checksum index table.
        dfs_subfile* pTable = pHeader->pSubFileTable;
        for( i=0 ; i<pHeader->nSubFiles ; i++ )
        {
            pTable[i].Offset        = LITTLE_ENDIAN_32( pTable[i].Offset );
            pTable[i].ChecksumIndex = LITTLE_ENDIAN_32( pTable[i].ChecksumIndex );
        }

        // Byte swap the file entries.
        for( i=0, pEntry=pHeader->pFiles ; i<pHeader->nFiles ; i++,pEntry++ )
        {
            // Byte swap the 32-bit values.
            pEntry->FileNameOffset1 = LITTLE_ENDIAN_32( pEntry->FileNameOffset1 );
            pEntry->FileNameOffset2 = LITTLE_ENDIAN_32( pEntry->FileNameOffset2 );
            pEntry->PathNameOffset  = LITTLE_ENDIAN_32( pEntry->PathNameOffset  );
            pEntry->ExtNameOffset   = LITTLE_ENDIAN_32( pEntry->ExtNameOffset   );

            // Byte swap the 32-bit values.
            pEntry->DataOffset = LITTLE_ENDIAN_32( pEntry->DataOffset );
            pEntry->Length     = LITTLE_ENDIAN_32( pEntry->Length     );  
        }

        // Woot!
        return pHeader;
    }

    return NULL;
}

//==============================================================================

void dfs_DumpFileListing( const dfs_header* pHeader, const char* pFileName )
{
    X_FILE* f;
    f = x_fopen( pFileName, "w+t" );
    if( f )
    {
        dfs_file*   pEntry  = pHeader->pFiles;

        for( s32 i=0 ; i<pHeader->nFiles ; i++, pEntry++ )
        {
            x_fprintf( f,"%8d\t%8d\t%8d\t%s\t%s%s\t%s\n",
                i,
                pEntry->Length,
                pEntry->DataOffset,
                pHeader->pStrings + (u32)pEntry->PathNameOffset,
                pHeader->pStrings + (u32)pEntry->FileNameOffset1,
                pHeader->pStrings + (u32)pEntry->FileNameOffset2,
                pHeader->pStrings + (u32)pEntry->ExtNameOffset);
        }

        x_fclose( f );
    }
}

//==============================================================================

void dfs_BuildFileName( const dfs_header* pHeader, s32 iFile, char* pFileName )
{
    ASSERT( (iFile>=0) && (iFile<pHeader->nFiles) );

    dfs_file*   pEntry  = &pHeader->pFiles[ iFile ];

    x_sprintf(pFileName,"%s%s%s%s",
        pHeader->pStrings + (u32)pEntry->PathNameOffset,
        pHeader->pStrings + (u32)pEntry->FileNameOffset1,
        pHeader->pStrings + (u32)pEntry->FileNameOffset2,
        pHeader->pStrings + (u32)pEntry->ExtNameOffset);
}

//==============================================================================


