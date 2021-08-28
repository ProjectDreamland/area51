//==============================================================================
//  
//  x_memfile.hpp
//  
//==============================================================================

#ifndef X_MEMFILE_HPP
#define X_MEMFILE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

class xstring;
class xwstring;

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  XMEMFILE
//==============================================================================
//
//  A simple memfile class to buffer reading and writing of binary files
//
//==============================================================================

class xmemfile
{
private:
        void            EnsureCapacity  ( s32 Capacity );

public:
                        xmemfile        ( void );
                        xmemfile        ( const xmemfile& MemFile );
                       ~xmemfile        ( void );

        s32             GetLength       ( void ) const;

        void            Clear           ( void );
        void            FreeExtra       ( void );

        xbool           IsEOF           ( void ) const;
        s32             Tell            ( void ) const;
        s32             Seek            ( s32   Offset, s32 Origin );
        s32             Read            ( byte* pBuffer, s32 Count );
        s32             Write           ( byte* pBuffer, s32 Count );

        void            Write_s8        ( s8    Value );
        void            Write_s16       ( s16   Value );
        void            Write_s32       ( s32   Value );
        void            Write_f32       ( f32   Value );
        void            Write_xstring   ( const xstring&    String );
        void            Write_xwstring  ( const xwstring&   String );

        s8              Read_s8         ( void );
        s16             Read_s16        ( void );
        s32             Read_s32        ( void );
        f32             Read_f32        ( void );
        xstring         Read_xstring    ( void );
        xwstring        Read_xwstring   ( void );

const   xmemfile&       operator =      ( const xmemfile&   MemFile );
const   xmemfile&       operator +=     ( const xmemfile&   MemFile );
const   xmemfile&       operator +=     ( s8                Data    );
const   xmemfile&       operator +=     ( s16               Data    );
const   xmemfile&       operator +=     ( s32               Data    );
const   xmemfile&       operator +=     ( f32               Data    );
const   xmemfile&       operator +=     ( const xstring&    String  );
const   xmemfile&       operator +=     ( const xwstring&   String  );

        xbool           LoadFile        ( const char* pFileName );
        xbool           SaveFile        ( const char* pFileName ) const;

protected:
        byte*       m_pData;
        s32         m_Capacity;
        s32         m_EOF;
        s32         m_Position;
};

//==============================================================================
#endif // X_MEMFILE_HPP
//==============================================================================
