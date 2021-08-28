//==============================================================================
//
//  Archive.hpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#if !defined(ARCHIVE_HPP)
#define ARCHIVE_HPP

#include "NetworkMgr/downloader/zlib/zlib.h"

struct archive_member
{
        char            Filename[128];
        byte*           pData;
        s32             Length;
};

class archive
{
public:
                        archive             ( void );
                       ~archive             ( void );
        // Initializes the archive. Decompresses the data as necessary and sets up the m_Members list
        // depending on the content of the archive.
        xbool           Init                ( const byte* pArchive, s32 Length );
        void            Kill                ( void );
        s32             GetMemberCount      ( void );
        const byte*     GetMemberData       ( s32 MemberIndex );
        s32             GetMemberLength     ( s32 MemberIndex );
        const char*     GetMemberFilename   ( s32 MemberIndex );
private:
        xarray<archive_member>  m_Members;
        xbool           m_Initialized;
        z_stream        m_Stream;
        byte            m_Buffer[32768];
};

#endif