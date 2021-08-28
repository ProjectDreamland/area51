//=========================================================================
//
// ps2_spad.hpp
//
//  This singleton class is intended to provide some VERY minimal
//  management for the PS2 scratchpad area. What we're mostly interested
//  in is having an ASSERT fire off if multiple parts of code are trying
//  to use scratchpad simultaneously. We'll also throw in some DMA routines
//  to/from scratchpad. Note that the ability to use the channel for
//  dma'ing from scratchpad will be limited if MFIFO is in use.
//
//=========================================================================

#ifndef PS2_SPAD_HPP
#define PS2_SPAD_HPP

#include "x_types.hpp"
#include "x_debug.hpp"
#include <eeregs.h>

//=========================================================================
// CLASSES
//=========================================================================

class scratchpad
{
public:
     scratchpad( void );
    ~scratchpad( void );

    // Functions for the game/entropy to use for managing scratchpad.
    // Since scratchpad is generally used for very specialized code, this
    // is just intended to be used as a safety mechanism. Use of MFIFO
    // will greatly reduce the scratchpad size, so query the available
    // amount of spad first. Lock scratchpad when you're about to use it,
    // and unlock it as soon as you're done.
    void*   Lock                ( void );
    void    Unlock              ( void );
    void*   GetUsableStartAddr  ( void );
    s32     GetUsableSize       ( void );

    // DMA helper functions for moving data into and out of scratchpad
    void    DmaTo       ( u32         SpadOffset,
                          const void* pSrc,
                          s32         NBytes );
    void    DmaFrom     ( u32         SpadOffset,
                          const void* pDst,
                          s32         NBytes );
    void    DmaSyncTo   ( void );
    void    DmaSyncFrom ( void );
    void    DmaSyncAll  ( void );

protected:
#ifdef X_ASSERT
    xbool   m_bLocked;    
#endif
};

//=========================================================================

extern scratchpad SPAD;

//=========================================================================
// PRIVATE DEFINES
//=========================================================================

//#if USE_MFIFO
#define SPAD_START  0x70002000
#define SPAD_SIZE   0x00002000
#define SPAD_END    (SPAD_START+SPAD_SIZE)
//#else
//#define SPAD_START  0x70000000
//#define SPAD_SIZE   0x00004000
//#define SPAD_END    (SPAD_START+SPAD_SIZE)
//#endif

//=========================================================================
// INLINES
//=========================================================================

inline void* scratchpad::Lock( void )
{
    ASSERT( TRUE == (m_bLocked = !m_bLocked) );
    
    return (void*)SPAD_START;
}

//=========================================================================

inline void scratchpad::Unlock( void )
{
    ASSERT( FALSE == (m_bLocked = !m_bLocked) );
}

//=========================================================================

inline void* scratchpad::GetUsableStartAddr( void )
{
    return (void*)SPAD_START;
}

//=========================================================================

inline s32 scratchpad::GetUsableSize( void )
{
    return SPAD_SIZE;
}

//=========================================================================

inline void scratchpad::DmaTo( u32 SpadOffset, const void* pSrc, s32 NBytes )
{
    ASSERT( m_bLocked );
    ASSERT( (NBytes & 0xf) == 0 );

    *D9_MADR = ((u32)pSrc)&0x0fffffff;
    *D9_SADR = (SPAD_START-0x70000000) + SpadOffset;
    *D9_QWC  = NBytes/16;
    *D9_CHCR = (1<<8);
    asm __volatile__ ( "sync.l" );
}

//=========================================================================

inline void scratchpad::DmaFrom( u32 SpadOffset, const void* pDst, s32 NBytes )
{
    ASSERT( m_bLocked );
    ASSERT( (NBytes & 0xf) == 0 );

    *D8_MADR = ((u32)pDst)&0x0fffffff;
    *D8_SADR = (SPAD_START-0x70000000) + SpadOffset;
    *D8_QWC  = NBytes/16;
    *D8_CHCR = (1<<8);
    asm __volatile__ ( "sync.l" );
}

//=========================================================================

inline void scratchpad::DmaSyncTo( void )
{
    ASSERT( m_bLocked );

    while( *D9_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }
}

//=========================================================================

inline void scratchpad::DmaSyncFrom( void )
{
    ASSERT( m_bLocked );
    while( *D8_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }
}

//=========================================================================

inline void scratchpad::DmaSyncAll( void )
{
    DmaSyncFrom();
    DmaSyncTo();
    while ( (*D8_CHCR & (1<<8)) ||
            (*D9_CHCR & (1<<8)) )
    {
        // intentionally empty loop
    }
}

//=========================================================================

#endif  // PS2_SPAD_HPP
