#ifndef __IO_CACHE_HPP__
#define __IO_CACHE_HPP__

#include "x_types.hpp"
#include "x_threads.hpp"
#include "io_device.hpp"

//==============================================================================

struct io_open_file;

//==============================================================================

class io_cache
{  

friend class io_fs;

//------------------------------------------------------------------------------
//  Private data.

private:

                xsema           m_Semaphore;        // Cache can be in use by a single thread only.
                s64             m_Ticks;            // Time (for LRU calculation).             
                s64             m_FirstByte;        // Offset of the cached data
                io_open_file*   m_pFile;            // File this cache is associated with.
                char            Filename[ IO_DEVICE_FILENAME_LIMIT ];
                s32             m_LastThreadID;     // Last thread to access this cache.
                s32             m_BytesCached;      // Number of "valid" bytes in the cache.
                s32             m_CacheSize;        // Physical size of the cache.
                xbool           m_IsCacheValid;     // Is the cache valid?
                void*           m_pCacheMemory;     // Buffer allocation.
                u8*             m_pCacheData;       // Aligned buffer.

//------------------------------------------------------------------------------
//  Public functions.

public:

                        io_cache                    ( void );
                       ~io_cache                    ( void );
void                    Init                        ( void );
void                    Kill                        ( void );
inline xsema*           GetSemaphore                ( void )                { return &m_Semaphore; }
inline io_open_file*    GetFile                     ( void )                { return m_pFile; }
inline void             SetFile                     ( io_open_file* pFile ) { m_pFile = pFile; }
inline s64              GetTicks                    ( void )                { return m_Ticks; }
inline void             SetTicks                    ( s64 Ticks )           { m_Ticks = Ticks; }
inline s64              GetFirstByte                ( void )                { return m_FirstByte; }
inline void             SetFirstByte                ( s64 FirstByte )       { m_FirstByte = FirstByte; }
inline s32              GetThreadID                 ( void )                { return m_LastThreadID; }
inline void             SetThreadID                 ( s32 ThreadID )        { m_LastThreadID = ThreadID; }
inline s32              GetBytesCached              ( void )                { return m_IsCacheValid ? m_BytesCached : 0; }
inline s32              GetCacheSize                ( void )                { return m_CacheSize; }
inline xbool            IsCacheValid                ( void )                { return m_IsCacheValid; }
inline void             Invalidate                  ( void )                { m_BytesCached = 0; m_FirstByte = (s64)(((u64)(-1)) >> 1); m_IsCacheValid = FALSE; }
inline void             Validate                    ( s32 Bytes )           { m_BytesCached = Bytes; m_IsCacheValid = TRUE; }
inline u8*              GetBuffer                   ( void )                { return m_pCacheData; }
inline s32              GetSize                     ( void )                { return m_CacheSize; }

};

#endif // __IO_CACHE_HPP__