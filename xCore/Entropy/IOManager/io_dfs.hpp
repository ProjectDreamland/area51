#ifndef IO_DFS_HPP
#define IO_DFS_HPP

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
    u32             Magic;              // Magic number to identify file
    s32             Version;            // Version number of file
    u32             Checksum;           // .DFS file checksum
    s32             SectorSize;         // Sector size in bytes
    u32             SplitSize;          // Split size in bytes (maximum)
    s32             nFiles;             // Total number of files in the filesystem
    s32             nSubFiles;          // Number of sub files (*.000, *.001, etc...)
    s32             StringsLength;      // Length of string table in bytes
    dfs_subfile*    pSubFileTable;      // Pointer to the sub file table
    dfs_file*       pFiles;             // Pointer to file entries
    u16*            pChecksums;         // Pointer to checksums
    char*           pStrings;           // Pointer to string table

};

dfs_header* dfs_InitHeaderFromRawPtr    ( void* pRawHeaderData, s32 Length  );
void        dfs_DumpFileListing         ( const dfs_header* pHeader, const char* pFileName );
void        dfs_BuildFileName           ( const dfs_header* pHeader, s32 iFile, char* pFileName );

#endif // IO_DFS_HPP
