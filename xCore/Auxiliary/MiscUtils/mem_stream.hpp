
#ifndef MEM_STREAM_HPP
#define MEM_STREAM_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_stdio.hpp"

//=========================================================================
// MEMORY STREAM
//=========================================================================
class mem_stream
{
public:
            mem_stream      ( void );
           ~mem_stream      ( void );
    void    GrowBy          ( s32 Count );
    void    Write           ( const void* pData, s32 Count );
    void    WriteAt         ( s32 Pos, const void* pData, s32 Count );
    s32     Tell            ( void );
    void    SeekPos         ( s32 Pos );
    void    GotoEnd         ( void );
    void    Grow            ( s32 Count );
    void    Preallocate     ( s32 Count, xbool bUpdatePos = FALSE );
    void    Preallocate32   ( s32 Count, xbool bUpdatePos = FALSE );
    void    Save            ( X_FILE* Fp );
    s32     GetLength       ( void );

protected:
    s32     m_GrowSize;
    s32     m_CurrentSize;
    s32     m_EndPos;
    s32     m_Pos;
    byte*   m_pPtr;
};

//=========================================================================
// END
//=========================================================================
#endif