//==============================================================================
//  
//  dfs.hpp
//
//==============================================================================

#ifndef DFS_HPP
#define DFS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  STRUCTS
//==============================================================================

#define DFS_MAGIC   'XDFS'
#define DFS_VERSION 3

struct dfs_file
{
    u32         FileNameOffset1;    // Offset of filename part 1 in string table
    u32         FileNameOffset2;    // Offset of filename part 2 in string table
    u32         PathNameOffset;     // Offset of pathname in string table
    u32         ExtNameOffset;      // Offset of extension in string table
    u32         DataOffset;         // Offset of data
    u32         Length;             // Length of file
};

struct dfs_subfile
{
    u32         Offset;
    u32         ChecksumIndex;
};

struct dfs_header
{
    s32             Magic;              // Magic number to identify file
    s32             Version;            // Version number of file
    u32             Checksum;           // .DFS file checksum
    s32             SectorSize;         // Sector size in bytes
    u32             SplitSize;          // Split size in bytes (maximum)
    s32             nFiles;             // Total number of files in the filesystem
    s32             nSubFiles;          // Number of sub files (*.000, *.001, etc...)
    s32             StringsLength;      // Length of string table in bytes
    dfs_subfile*    pSubFileTable;      // Pointer to the sub file table
    dfs_file*       pFiles;             // Pointer to file entries
    u16*            pChecksums;         // Pointer to checksum table
    char*           pStrings;           // Pointer to string table
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

dfs_header* dfs_InitHeaderFromRawPtr    ( void* pRawHeaderData  );
void        dfs_DumpFileListing         ( const dfs_header* pHeader, const char* pFileName );

void dfs_Build          ( const xstring& PathName, const xarray<xstring>& Scripts, xbool DoMake, u32 SectorSize, u32 SplitSize, u32 ChunkSize, xbool bEnableCRC );
void dfs_Update         ( const xstring& PathName, const xarray<xstring>& Scripts );
void dfs_Optimize       ( const xstring& PathName );

void dfs_List           ( const xstring& PathName );
void dfs_Extract        ( const xstring& PathName, const xstring& outPath );
void dfs_Verify         ( const xstring& PathName );

void dfs_SetChunkSize   ( u32 nBytes );
void dfs_SectorAlign    ( const char* pExtension );

//==============================================================================
#endif // DFS_HPP
//==============================================================================
