#include <stdio.h>

#include "x_files.hpp"
#include "x_array.hpp"
#include "x_string.hpp"
#include "io_mgr.hpp"


void io_fs_Init( const char* pFilesystemFile )
{
}

void io_fs_Kill( void )
{
}

X_FILE* io_fs_Open( const char* pPathName, const char* pMode )
{
    return NULL;
}

void io_fs_Close( X_FILE* pFile )
{
}

s32 io_fs_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    return 0;
}

s32 io_fs_Write( X_FILE* pFile, const byte* pBuffer, s32 Bytes )
{
    return 0;
}

s32 io_fs_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    return 0;
}

s32 io_fs_Tell( X_FILE* pFile )
{
    return 0;
}

s32 io_fs_Flush( X_FILE* pFile )
{
    return 0;
}

xbool io_fs_EOF( X_FILE* pFile )
{
    return 0;
}

s32 io_fs_Length( X_FILE* pFile )
{
    return 0;
}
