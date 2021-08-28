//==============================================================================
//
//  x_bitmap_inline.hpp
//
//==============================================================================

#ifndef X_BITMAP_INLINE_HPP
#define X_BITMAP_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================
        
inline const byte* xbitmap::GetPixelData( s32 Mip ) const
{
    ASSERT( Mip >= -1 );

    if( Mip == -1 )
    {
        return( m_Data.pPixel );
    }
    else
    if( m_NMips == 0 )
    {
        ASSERT( Mip == 0 );
        return( m_Data.pPixel );
    }
    else
    {
        ASSERT( Mip <= m_NMips );
        return( m_Data.pPixel + m_Data.pMip[Mip].Offset );
    }
}

//==============================================================================

inline const byte* xbitmap::GetClutData( void ) const
{
    return( m_pClut );
}

//==============================================================================

inline s32 xbitmap::GetDataSize( void ) const
{
    return( m_DataSize );
}

//==============================================================================

inline s32 xbitmap::GetClutSize( void ) const
{
    return( m_ClutSize );
}

//==============================================================================

inline s32 xbitmap::GetMipDataSize( s32 Mip ) const
{
    ASSERT( Mip >= 0 );

    if( m_NMips == 0 )
    {
        ASSERT( Mip == 0 );
        return( m_DataSize );
    }
    else
    {
        ASSERT( Mip <= m_NMips );
        return( (m_Data.pMip[Mip].Width * 
                 m_Data.pMip[Mip].Height * 
                 m_FormatInfo[ m_Format ].BPP) >> 3 );
    }
}

//==============================================================================

inline s32 xbitmap::GetBPP( void ) const
{
    return( m_FormatInfo[ m_Format ].BPP );
}

//==============================================================================

inline s32 xbitmap::GetBPC( void ) const
{
    return( m_FormatInfo[ m_Format ].BPC );
}

//==============================================================================

inline s32 xbitmap::GetWidth( s32 Mip ) const
{
    ASSERT( Mip >= 0 );

    if( m_NMips == 0 )
    {
        ASSERT( Mip == 0 );
        return( m_Width );
    }
    else
    {
        ASSERT( Mip <= m_NMips );
        return( m_Data.pMip[Mip].Width );
    }
}

//==============================================================================

inline s32 xbitmap::GetHeight( s32 Mip ) const
{
    ASSERT( Mip >= 0 );

    if( m_NMips == 0 )
    {
        ASSERT( Mip == 0 );
        return( m_Height );
    }
    else
    {
        ASSERT( Mip <= m_NMips );
        return( m_Data.pMip[Mip].Height );
    }
}

//==============================================================================

inline s32 xbitmap::GetPWidth( s32 Mip ) const
{
    ASSERT( Mip >= 0 );

    if( m_NMips == 0 )
    {
        ASSERT( Mip == 0 );
        return( m_PW );
    }
    else
    {
        ASSERT( Mip <= m_NMips );
        return( m_Data.pMip[Mip].Width );
    }
}

//==============================================================================

inline xbool xbitmap::IsPowerOf2( void ) const
{
    s32 W = GetWidth();
    s32 H = GetHeight();
    if( ((W&(W-1))==0) && ((H&(H-1))==0) )
        return TRUE;
    return FALSE;
}

//==============================================================================

inline s32 xbitmap::GetVRAMID( void ) const
{
    return( m_VRAMID );
}

//==============================================================================

inline void xbitmap::SetVRAMID( s32 VRAMID ) const
{
    m_VRAMID = VRAMID;
}

//==============================================================================

inline u32 xbitmap::GetFlags( void ) const
{
    return( m_Flags );
}

//==============================================================================

inline s32 xbitmap::GetNMips( void ) const
{
    return( m_NMips );
}

//==============================================================================

inline xbitmap::format xbitmap::GetFormat( void ) const
{
    return( (format)m_Format );
}

//==============================================================================

inline const xbitmap::format_info& xbitmap::GetFormatInfo( void ) const
{
    ASSERT( (m_Format > FMT_NULL) && (m_Format < FMT_END_OF_LIST) );
    return( m_FormatInfo[ m_Format ] );
}

//==============================================================================

inline const xbitmap::format_info& xbitmap::GetFormatInfo( xbitmap::format Format )
{
    ASSERT( (Format > FMT_NULL) && (Format < FMT_END_OF_LIST) );
    return( m_FormatInfo[ Format ] );
}

//==============================================================================

