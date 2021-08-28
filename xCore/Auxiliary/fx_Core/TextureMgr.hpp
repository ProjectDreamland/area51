#ifndef __TEXTURE_MGR_HPP
#define __TEXTURE_MGR_HPP

#include "x_files.hpp"
#include "entropy.hpp"
#include "x_bytestream.hpp"
#include "dlinkedlist.hpp"

namespace fx_core
{

enum export_target
{
    EXPORT_TARGET_PC,
    EXPORT_TARGET_PS2,
    EXPORT_TARGET_GCN,
    EXPORT_TARGET_XBOX,
};

//============================================================================
// A class to manage loaded textures
class texture_mgr
{

    struct record
    {
        char        Path[ X_MAX_PATH ];
        xbool       IsLoaded;
        xbitmap     Bitmap;
        s32         ID;
        xbool       UsedFlag;           // for use during export...is this bitmap referenced?
        FILETIME    CreationTime;

        record()    { x_memset(this, 0, sizeof(record) ); ID = -1; }
        ~record()   { if ( ID != -1 ) vram_Unregister( Bitmap ); }
    };

    // a collection of strings and the ID's they point to (Ptr is actually used as s32)
    // CMapStringToPtr         m_Mapping;
    
    // de-coupling MFC from elements/effect -- use array instead
    DLinkedList<record*>        m_Records;

    xbool   m_UseVRAM;
    xbool   m_DontLoad;

public:
            texture_mgr();
           ~texture_mgr(); 

    xbool   ActivateBitmap      ( const char* pFileName );
    xbool   DeActivateBitmap    ( const char* pFileName );
    s32     GetBitmapID         ( const char* pFileName );
    s32     GetTextureIndex     ( const char* pFileName );
    void    MarkAsUsed          ( const char* pFileName );
    s32     GetBitmapCount      ( void );
    void    ExportList          ( const char* pFileName, s32 ExportTarget );
    void    ExportNames         ( xbytestream&   Stream, s32 ExportTarget );
    void    ExportXBMPs         ( const char* pFileName, s32 ExportTarget );

    record* Lookup              ( const char* pFileName );
    void    Remove              ( const char* pFileName );

    void    DontUseVRAM         ( void );
    void    DontLoad            ( void );
};

extern texture_mgr* g_pTextureMgr;

} // namespace fx_core

#endif
