#include "entropy.hpp"
#include "..\entropy\e_Memcard.hpp"

//==============================================================================
//=============================== Class Functions ==============================
//==============================================================================

//------------------------------------------------------------------------------

memcard_hardware::memcard_hardware( void )
{
}

//------------------------------------------------------------------------------

memcard_hardware::~memcard_hardware( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::Init( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::Kill( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::SendMessage( s32 Message )
{
    (void) Message;
}

//------------------------------------------------------------------------------

void memcard_hardware::SetIOParams( const char* FileName, byte* pBuffer, s32 Offset, s32 nBytes )
{
    (void) FileName;
    (void) pBuffer;
    (void) Offset;
    (void) nBytes;
}
//------------------------------------------------------------------------------

void memcard_hardware::FreeIOBuffer( void )
{
}


//------------------------------------------------------------------------------

void memcard_hardware::SetOperation( memcard_op Operation )
{
    (void) Operation;
}

//------------------------------------------------------------------------------

void memcard_hardware::InitiateOperation( void )
{
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetFileList( xarray<xstring>& FileList )
{
    (void) FileList;
return 0;
}

//------------------------------------------------------------------------------

void memcard_hardware::InvalidateFileList( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::Process( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessMount( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessUnmount( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessReadFile( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessWriteFile( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessDeleteFile( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessFormat( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessReadFileList( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessPurgeFileList( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessGetFileLength( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessRepair( void )
{
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetMaxCards( void )
{
    return 0;
}

//------------------------------------------------------------------------------

xbool memcard_hardware::IsCardConnected( s32 CardID )
{
    (void) CardID;
    return FALSE;
}

