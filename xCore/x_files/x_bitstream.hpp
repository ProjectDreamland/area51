//=============================================================================
//
//  BITSTREAM.HPP
//
//=============================================================================
#ifndef BITSTREAM_HPP
#define BITSTREAM_HPP

#include "x_types.hpp"
#include "x_math.hpp"
//#include "ObjectMgr/Object.hpp"
//#include "netlib/netlib.hpp"

//=============================================================================


//=============================================================================
// BITSTREAM CLASS
//=============================================================================

class bitstream
{

public:

                        bitstream               ( void );
                       ~bitstream               ( void );

            void        Init                    ( s32 DataSize );
            void        Init                    ( const byte* pData, s32 DataSize );
            void        Kill                    ( void );
            void        SetOwnsData             ( xbool OwnsData );
            void        Grow                    ( void );
            void        SetMaxGrowSize          ( s32 MaxGrowSize );
                        
            s32         GetNBytes               ( void ) const;
            s32         GetNBits                ( void ) const;
            s32         GetNBytesUsed           ( void ) const;
            s32         GetNBytesFree           ( void ) const;
            s32         GetNBitsUsed            ( void ) const;
            s32         GetNBitsFree            ( void ) const;
            byte*       GetDataPtr              ( void ) const;
            xbool       IsFull                  ( void ) const;
            void        Clear                   ( void );
                        
            s32         GetCursor               ( void ) const;
            void        SetCursor               ( s32 BitIndex );
            s32         GetCursorRemaining      ( void ) const;

            // Jumps cursor to aligned position. 
            // Remember we are talking about bits...not bytes so
            // (2) = nibble alignment because (2^2) ==  4 bits == 0.5 bytes
            // (3) = byte   alignment because (2^3) ==  8 bits == 1 byte
            // (4) = u16    alignment because (2^4) == 16 bits == 2 bytes
            void        AlignCursor             ( s32 PowerOfTwo );

            // Integer Helpers
            void        WriteU64                ( u64 Value, s32 NBits=64 );
            void        WriteS32                ( s32 Value, s32 NBits=32 );
            void        WriteU32                ( u32 Value, s32 NBits=32 );
            void        WriteS16                ( s16 Value, s32 NBits=16 );
            void        WriteU16                ( u16 Value, s32 NBits=16 );
            void        WriteRangedS32          ( s32 Value, s32 Min, s32 Max );
            void        WriteRangedU32          ( u32 Value, s32 Min, s32 Max );
            void        WriteVariableLenS32     ( s32 Value );
            void        WriteVariableLenU32     ( u32 Value );
                        
            void        ReadU64                 ( u64& Value, s32 NBits=64 ) const;
            void        ReadS32                 ( s32& Value, s32 NBits=32 ) const;
            void        ReadU32                 ( u32& Value, s32 NBits=32 ) const;
            void        ReadS16                 ( s16& Value, s32 NBits=16 ) const;
            void        ReadU16                 ( u16& Value, s32 NBits=16 ) const;
            void        ReadRangedS32           ( s32& Value, s32 Min, s32 Max ) const;
            void        ReadRangedU32           ( u32& Value, s32 Min, s32 Max ) const;
            void        ReadVariableLenS32      ( s32& Value ) const;
            void        ReadVariableLenU32      ( u32& Value ) const;

            // Float helpers
            void        WriteF32                ( f32 Value );
            void        WriteRangedF32          ( f32 Value, s32 NBits, f32 Min, f32 Max );
            void        WriteVariableLenF32     ( f32 Value );


            void        ReadF32                 ( f32& Value ) const;
            void        ReadRangedF32           ( f32& Value, s32 NBits, f32 Min, f32 Max ) const;
            void        ReadVariableLenF32      ( f32& Value ) const;

     static void        TruncateRangedF32       ( f32& Value, s32 NBits, f32 Min, f32 Max );
     static void        TruncateRangedVector    ( vector3& N, s32 NBits, f32 Min, f32 Max );

            // Vectors
            void        WriteVector             ( const vector3& N );
            void        WriteRangedVector       ( const vector3& N, s32 NBits, f32 Min, f32 Max );
            void        WriteVariableLenVector  ( const vector3& N );
            void        WriteUnitVector         ( const vector3& N, s32 TotalBits );

            void        ReadVector              ( vector3& N ) const;
            void        ReadRangedVector        ( vector3& N, s32 NBits, f32 Min, f32 Max ) const;
            void        ReadVariableLenVector   ( vector3& N ) const;
            void        ReadUnitVector          ( vector3& N, s32 TotalBits ) const;

            // Quaternion
            void        WriteQuaternion         ( const quaternion& Q );
            void        ReadQuaternion          ( quaternion& Q ) const;

            // Radian3
            void        WriteRadian3            ( const radian3& Radian );
            void        WriteRangedRadian3      ( const radian3& Radian, s32 NBits );

            void        ReadRadian3             ( radian3& Radian ) const;
            void        ReadRangedRadian3       ( radian3& Radian, s32 NBits ) const;

            // Color
            void        WriteColor              ( xcolor Color );
            void        ReadColor               ( xcolor& Color ) const;

            // String
            void        WriteString             ( const char* pBuf );
            void        ReadString              ( char* pBuf, s32 MaxLength=0) const;

            // Wide string
            void        WriteWString            ( const xwchar* pBuf );
            void        ReadWString             ( xwchar* pBuf, s32 MaxLength=0 ) const;

            // Handles a full matrix... full precision
            void        WriteMatrix4            ( const matrix4& M );
            void        ReadMatrix4             ( matrix4& M ) const;
   
            // Raw bits
            void        WriteBits               ( const void* pData, s32 NBits );
            void        ReadBits                ( void* pData, s32 NBits ) const;
            xbool       WriteFlag               ( xbool Value );
            xbool       ReadFlag                ( void ) const;
            xbool       ReadFlag                ( xbool& Flag ) const;

            // Marker support
            void        WriteMarker             ( void );
            void        ReadMarker              ( void ) const;

            // Overwrite control 
            xbool       Overwrite               ( void )                            const;
            void        ClearOverwrite          ( void );                           

            // Section support
            xbool       OpenSection             ( xbool Flag = TRUE );
            xbool       CloseSection            ( void );


private:

        byte*           m_Data;
        s32             m_DataSize;
        s32             m_DataSizeInBits;
        s32             m_HighestBitWritten;
        xbool           m_bOwnsData;
        s32             m_MaxGrowSize;

        mutable s32     m_Cursor;
        s32             m_SectionCursor;
        xbool           m_bOverwrite;
                        
        void            WriteRawBits            ( const void* pData, s32 NBits );
        void            ReadRawBits             ( void* pData, s32 NBits ) const;
        void            WriteRaw32              ( u32 Value, s32 NBits );
        u32             ReadRaw32               ( s32 NBits ) const;

};

//=============================================================================
// INLINED ROUTINES
//=============================================================================
//=========================================================================

inline void    bitstream::WriteS32        ( s32 Value, s32 NBits )
{
    WriteRaw32(Value, NBits);
}

//=========================================================================

inline void    bitstream::WriteU32        ( u32 Value, s32 NBits )
{
    WriteRaw32(Value, NBits);
}

//=========================================================================

inline void    bitstream::WriteS16        ( s16 Value, s32 NBits )
{
    WriteRaw32(Value, NBits);
}

//=========================================================================

inline void    bitstream::WriteU16        ( u16 Value, s32 NBits )
{
    WriteRaw32(Value, NBits);
}

//=========================================================================

inline void    bitstream::WriteMarker  ( void )
{
#if defined(X_DEBUG)
    WriteU32( 0xDEADBEEF );
#endif
}

//=========================================================================

inline void    bitstream::ReadU32         ( u32& Value, s32 NBits ) const
{
    Value = ReadRaw32( NBits );
}

//=========================================================================

inline void    bitstream::ReadU16         ( u16& Value, s32 NBits ) const
{
    Value = ReadRaw32( NBits );
}

//=========================================================================

inline void    bitstream::ReadS16         ( s16& Value, s32 NBits ) const
{
    Value = ReadRaw32( NBits );
    
    if( NBits==16 ) return;
    
    // extend sign bit
    if( Value & (1<<(NBits-1)) )
        Value |= (0xFFFF & (~((1<<NBits)-1)));
}

//=========================================================================

inline void    bitstream::ReadS32         ( s32& Value, s32 NBits ) const
{
    Value = ReadRaw32( NBits );
    
    if( NBits==32 ) return;
    
    // extend sign bit
    if( Value & (1<<(NBits-1)) )
        Value |= (0xFFFFFFFF & (~((1<<NBits)-1)));
}

//=============================================================================
#endif
//=============================================================================