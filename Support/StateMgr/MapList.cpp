//=========================================================================
//
//  MapList.cpp
//
//=========================================================================

#include "x_files/x_types.hpp"

#include "mapList.hpp"
#include "Parsing/textin.hpp"
#include "stringmgr/stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "StateMgr/StateMgr.hpp"

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs                                                                
//=========================================================================

//=========================================================================
//  Globals                                                                
//=========================================================================

map_list g_MapList;

//=========================================================================
//  Functions
//=========================================================================
map_list::map_list( void )
{
    Clear();
    m_Version = 0;
}

//=========================================================================
map_list::~map_list( void )
{
    // we can unlock the arrays now
    m_Maps.SetLocked( FALSE );
    m_GameTypes.SetLocked( FALSE );
    m_MapList.SetLocked( FALSE );

    m_Maps.Clear();
    m_GameTypes.Clear();
    m_MapList.Clear();
}

//=========================================================================
void map_list::Init( void )
{
    Clear();

    // Lock the array sizes. This will keep any calls to clear from shifting
    // memory around and causing fragmentation issues.
    m_Maps.SetCapacity( MAX_MAPS );
    m_GameTypes.SetCapacity( MAX_GAME_TYPES );
    m_MapList.SetCapacity( MAP_TABLE_SIZE );

    m_Maps.SetLocked( TRUE );
    m_GameTypes.SetLocked( TRUE );
    m_MapList.SetLocked( TRUE );
}

//=========================================================================
void map_list::LoadDefault( void )
{
    xstring Manifest;

    // TODO: Add preload statements for foreign language map lists
    (void)PRELOAD_FILE("ENG_DiskMaps.txt");
    (void)PRELOAD_FILE("FRE_DiskMaps.txt");
    (void)PRELOAD_FILE("GER_DiskMaps.txt");
    (void)PRELOAD_FILE("ITA_DiskMaps.txt");
    (void)PRELOAD_FILE("SPA_DiskMaps.txt");

    Clear();
    VERIFY( Manifest.LoadFile( xfs("%s\\%s_DiskMaps.txt", g_RscMgr.GetRootDirectory(), x_GetLocaleString()) ) );
    Parse( (const char*)Manifest, MF_NOT_PRESENT, -1 );
    // Note: We do NOT add the HDD maps at the moment as the likelihood of the hard drive
    // having completed initialization at this point is zero. This should be done
    // just prior to going in to the map select portion of the online component of the game.


    // Now append all those maps that appear in the MapList.txt file. Just in case they
    // are not already present.

    // determine what levels we can load
    text_in     TextIn;


    if( TextIn.OpenFile( xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "MapList.txt") ) == FALSE )
    {
        ASSERTS( FALSE, "Could not open MapList.txt file\n" );
        return;
    }

    TextIn.ReadHeader();

    s32             i;
    s32             nLevel = TextIn.GetHeaderCount();
    char            Filename[128];

    for( i=0; i < nLevel; i++ )
    {
        if( TextIn.ReadFields() == FALSE )
        {
            ASSERTS( FALSE, "The MapList.txt file seems to be malformed.");
            break;
        }
        TextIn.GetString( "Level", Filename );
        ASSERT( x_strlen(Filename) < sizeof(Filename)-1 );

        // If this particular map is not currently in the maplist, add it to the end. It's
        // a test level or a level that hasn't yet been put in the ENG_DiskMaps.txt
        if( IsPresent( Filename ) == FALSE )
        {
            #ifndef X_RETAIL
            //
            // Allocate some space for the auto defined maps.

            // 
            // Add this map to the list of maps.
            //
            map_info& MapInfo = m_Maps.Append();

            MapInfo.Filename    = Filename;
            MapInfo.DisplayName = Filename;
            MapInfo.Flags       = MF_DVD_MAP;
            MapInfo.MapID       = 8000+i;
            // Add a map_entry for deathmatch and campaign types for this map

            map_entry& Campaign = m_MapList.Append();

            Campaign.m_GameTypeID       = GAME_CAMPAIGN;
            Campaign.m_MapID            = 8000+i;
            Campaign.m_MinPlayers       = 1;
            Campaign.m_MaxPlayers       = 16;

            map_entry& Deathmatch = m_MapList.Append();

            Deathmatch.m_GameTypeID     = GAME_DM;
            Deathmatch.m_MapID          = 8000+i;
            Deathmatch.m_MinPlayers     = 1;
            Deathmatch.m_MaxPlayers     = 16;
            
            #endif
        }
        else
        {
            s32 j;
            for( j=0; j<m_Maps.GetCount(); j++ )
            {
                if( x_stricmp( Filename, m_Maps[j].Filename ) == 0 )
                {
                    // Mark this map as existing
                    m_Maps[j].Flags = MF_DVD_MAP;
                }
            }
        }
    }
    TextIn.CloseFile();

#if (defined(bwatson)||defined(Biscuit)) && defined(X_DEBUG)
    for( i=0; i<m_GameTypes.GetCount(); i++ )
    {
        game_type_info& Current = m_GameTypes[i];

        LOG_MESSAGE( "map_list::Init", "game_type(%d): Type:%d, ShortType:%s, LongType:%s, Rules:%s", 
            i, Current.Type, (const char*)Current.ShortTypeName, (const char*)Current.TypeName, (const char*)Current.Rules );
    }

    for( i=0; i<m_Maps.GetCount(); i++ )
    {
        map_info& Current = m_Maps[i];
        if( Current.IsAvailable() )
        {
            LOG_MESSAGE( "map_list::Init", "map_info(%d): MapID:%d, Filename:%s, DisplayName:%s, Description:%s",
                i, Current.MapID, (const char*)Current.Filename, (const char*)Current.DisplayName, (const char*)Current.Description );
        }
    }

    for( i=0; i<m_MapList.GetCount(); i++ )
    {
        map_entry& Current = m_MapList[i];

        LOG_MESSAGE( "map_list::Init", "map_entry(%d): MapID:%d, GameID:%d, MinPlayers:%d, MaxPlayers:%d", 
                        i, Current.m_MapID, Current.m_GameTypeID, Current.m_MinPlayers, Current.m_MaxPlayers );
    }
#endif
    ASSERTS( m_GameTypes.GetCount(), "No gametypes have been defined in ENG_DiskMaps.txt" );
    ASSERTS( m_Maps.GetCount(), "No maps have been defined in ENG_DiskMaps.txt" );
    ASSERTS( m_MapList.GetCount(), "No map bindings have been defined in ENG_DiskMaps.txt" );
}

//=========================================================================

void map_list::Kill( void )
{
    Clear();
}

//=========================================================================

void map_list::Clear( void )
{
    //
    // Clear out any previous map data. Since the arrays are locked, this
    // won't actually delete memory, but will clear the data out.
    //

    // First, walk the arrays and make sure any allocated memory is freed
    s32 i;

    for ( i = 0; i < m_GameTypes.GetCount(); ++i )
    {
        m_GameTypes[i].ShortTypeName.Clear();
        m_GameTypes[i].ShortTypeName.FreeExtra();
        m_GameTypes[i].TypeName.Clear();
        m_GameTypes[i].TypeName.FreeExtra();
        m_GameTypes[i].Rules.Clear();
        m_GameTypes[i].Rules.FreeExtra();
    }

    for ( i = 0; i < m_Maps.GetCount(); ++i )
    {
        m_Maps[i].Filename.Clear();
        m_Maps[i].Filename.FreeExtra();
        m_Maps[i].DisplayName.Clear();
        m_Maps[i].DisplayName.FreeExtra();
        m_Maps[i].Description.Clear();
        m_Maps[i].Description.FreeExtra();
    }

    // m_MapList elements have no dynamic data...

    // Now do the clears
    m_GameTypes.Clear();
    m_Maps.Clear();
    m_MapList.Clear();
}

//=========================================================================
// This will append an entry from one maplist to another. If the game type info is not
// within the new maplist, then all required fields will be copied. This will make the
// current manifest totally self-contained.
xbool map_list::Append( const map_entry& Entry, const map_list* pSourceMapList )
{
    if( m_MapList.Find( Entry ) != -1 )
    {
        return FALSE;
    }

    if( pSourceMapList == NULL )
    {
        pSourceMapList = &g_MapList;
    }

    // Ok, so this entry was not in the maplist. So, we duplicate it and append it to the maplist.

    map_entry& NewEntry = m_MapList.Append( );
    NewEntry = Entry;

    // We need to search through our GameType field in the new maplist to see if it's there. If so,
    // then we set up the index within this entry or we append it to our list of gametypes.
    game_type_info TempType;

    TempType.Type = Entry.m_GameTypeID;
    if( m_GameTypes.Find( TempType ) == -1 )
    {
        s32 GameTypeIndex = pSourceMapList->m_GameTypes.Find( TempType );
        if( GameTypeIndex != -1 )
        {
            m_GameTypes.Append( pSourceMapList->m_GameTypes[GameTypeIndex] );
        }
        else
        {
            // Must have found it!
            GameTypeIndex = g_MapList.m_GameTypes.Find( TempType );
            ASSERTS( GameTypeIndex != -1, "Could not find game type definition" );
            m_GameTypes.Append( g_MapList.m_GameTypes[GameTypeIndex] );
        }
    }

    map_info TempMap;
    TempMap.MapID = Entry.m_MapID;
    if( m_Maps.Find( TempMap ) == -1 )
    {
        s32 MapIndex = pSourceMapList->m_Maps.Find( TempMap );
        if( MapIndex != -1 )
        {
            m_Maps.Append( pSourceMapList->m_Maps[MapIndex] );
        }
        else
        {
            MapIndex = g_MapList.m_Maps.Find( TempMap );
            ASSERTS( MapIndex != -1, "Could not find map definition" );
            m_Maps.Append( g_MapList.m_Maps[MapIndex] );
        }

    }

    return TRUE;
}

//=========================================================================

const char* map_list::GetDisplayName( s32 MapID )
{
    const map_entry* pEntry;

    pEntry = Find( MapID );
    if( pEntry==NULL )
    {
        return NULL;
    }
    return pEntry->GetDisplayName();
}

//=========================================================================

const char* map_list::GetFileName( s32 MapID )
{
    const map_entry* pEntry;

    pEntry = Find( MapID );
    if( pEntry==NULL )
    {
        return NULL;
    }
    return pEntry->GetFilename();
}

//=========================================================================

const map_entry* map_list::Find( s32 MapID, s32 GameType )
{
    s32 i;
    for( i=0; i<m_MapList.GetCount(); i++ )
    {
        if( MapID == -1 )
        {
            if( (m_MapList[i].GetGameType() == GameType) && m_MapList[i].IsAvailable() )
            {
                return &m_MapList[i];
            }
        }
        else if( m_MapList[i].GetMapID() == MapID )
        {
            if( (GameType == -1) || (m_MapList[i].GetGameType() == GameType) )
            {
                return &m_MapList[i];
            }
        }
    }
    return NULL;
}

//=========================================================================

map_entry* map_list::GetByIndex( s32 MapIndex )
{
    return &m_MapList[MapIndex];
}

//=========================================================================
// Search for a specific entry using filename and gametype as the search
// parameters.

xbool map_list::IsPresent( const char* pFilename )
{
    s32 i;

    for( i=0; i< m_MapList.GetCount(); i++ )
    {
        if( x_stricmp( pFilename, m_MapList[i].GetFilename() )==0 )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

map_entry* map_list::GetNextMap( const map_entry* pCurr )
{
    game_type   GameType = pCurr->GetGameType();
    s32         MinID    = S32_MAX;
    s32         MaxID    = S32_MIN;
    s32         MinIndex = -1;
    s32         MaxIndex = -1;
    s32         ResultIndex = -1;
    s32         i;
    s32         CurrentIndex = m_MapList.Find( *pCurr );

    ASSERT( CurrentIndex != -1 );

    // Search the entire map list looking for the 'next' mapid
    for( i=0; i<m_MapList.GetCount(); i++ )
    {
        if( (m_MapList[i].GetGameType() == GameType) && m_MapList[i].IsAvailable() )
        {
            if( m_MapList[i].GetMapID() < MinID )
            {
                MinID    = m_MapList[i].GetMapID();
                MinIndex = i;
            }

            if( m_MapList[i].GetMapID() > MaxID )
            {
                MaxID = m_MapList[i].GetMapID();
                MaxIndex = i;
            }
        }
    }

    // Wrap?
    if( CurrentIndex == MaxIndex )
    {
        ResultIndex = MinIndex;
    }
    else
    {
        ResultIndex = MaxIndex;
        for( i=0; i< m_MapList.GetCount(); i++ )
        {
            if( (m_MapList[i].GetGameType() == GameType) && m_MapList[i].IsAvailable() )
            {
                if( (m_MapList[i].GetMapID() > m_MapList[ResultIndex].GetMapID()) && (m_MapList[i].GetMapID() < m_MapList[CurrentIndex].GetMapID()) )
                {
                    ResultIndex = i;
                }
            }
        }
    }

    if( ResultIndex==-1 )
    {
        return NULL;
    }
    else
    {
        return &m_MapList[ResultIndex];
    }
}

//=========================================================================

void map_list::RemoveByFlags( map_flags Flags )
{
    s32 i,j;

    //
    // Go through this maplist, remove any bindings that were added with the download flag set
    //
    for( i=0; i<m_MapList.GetCount(); i++ )
    {
        if( (m_MapList[i].GetFlags() & Flags) == Flags )
        {
            m_MapList.Delete(i);
            i--;
        }
    }

    // 
    // All flags have been removed. Now go through the gametype list and remove
    // any entries that have no references to them.
    //
    for( i=0; i<m_GameTypes.GetCount(); i++ )
    {
        // For each game type, search map bindings for a reference to it.
        xbool Found = FALSE;
        for( j=0; j<m_MapList.GetCount(); j++ )
        {
            if( m_MapList[j].GetGameType() == m_GameTypes[i].Type )
            {
                Found = TRUE;
                break;
            }
        }
        if( !Found )
        {
            m_GameTypes.Delete(i);
            i--;
        }
    }

    //
    // Now go through the maps list and remove any no longer referenced maps
    //
    for( i=0; i<m_Maps.GetCount(); i++ )
    {
        // For each game type, search map bindings for a reference to it.
        xbool Found = FALSE;
        for( j=0; j<m_MapList.GetCount(); j++ )
        {
            if( m_MapList[j].GetMapID() == m_Maps[i].MapID )
            {
                Found = TRUE;
                break;
            }
        }
        if( !Found )
        {
            m_Maps.Delete(i);
            i--;
        }
    }
}

//=========================================================================

void map_list::RemoveByMapID( s32 MapID )
{
    s32 i,j;

    //
    // Go through this maplist, remove any bindings for this specific mapid.
    //
    for( i=0; i<m_MapList.GetCount(); i++ )
    {
        if( m_MapList[i].GetMapID() == MapID )
        {
            m_MapList.Delete(i);
            i--;
        }
    }

    // 
    // All flags have been removed. Now go through the gametype list and remove
    // any entries that have no references to them.
    //
    for( i=0; i<m_GameTypes.GetCount(); i++ )
    {
        // For each game type, search map bindings for a reference to it.
        xbool Found = FALSE;
        for( j=0; j<m_MapList.GetCount(); j++ )
        {
            if( m_MapList[j].GetGameType() == m_GameTypes[i].Type )
            {
                Found = TRUE;
                break;
            }
        }
        if( !Found )
        {
            m_GameTypes.Delete(i);
            i--;
        }
    }

    //
    // Now go through the maps list and remove any no longer referenced maps
    //
    for( i=0; i<m_Maps.GetCount(); i++ )
    {
        // For each game type, search map bindings for a reference to it.
        xbool Found = FALSE;
        for( j=0; j<m_MapList.GetCount(); j++ )
        {
            if( m_MapList[j].GetMapID() == m_Maps[i].MapID )
            {
                Found = TRUE;
                break;
            }
        }
        if( !Found )
        {
            m_Maps.Delete(i);
            i--;
        }
    }
}

//==============================================================================
#if !defined(X_RETAIL) || defined(X_QA)
// function for string tests. to flush out hard coded text in game. 
// Substitutes X's for all printable text.
void FillString( const char* pString )
{
    unsigned char* pText = (unsigned char*)pString;

    while( *pText )
    {
        // skip over any control codes.
        if(( *pText > 0x10 ) && (*pText != 0x20))
        {
            // check for button code pairs
            if( *pText == 0xAB)
            {
                while(*pText && *pText != 0xBB)
                    pText++;
            }
            // check for bracket pairs
            if( *pText == '<')
            {
                while(*pText && *pText != '>')
                    pText++;
            }
            if( *pText == 0 ) break;
            if( (*pText != 0xBB) && (*pText != '>') )
                *pText = 'x';
        }
        pText++;
    }
}
#endif

//=========================================================================
enum file_section
{
    SECTION_UNDEFINED = 0,
    SECTION_VERSION,
    SECTION_GAMETYPE,
    SECTION_MAPTYPE,
    SECTION_PLAY,
};

//=========================================================================
void map_list::Parse( const char* pMapFile, map_flags Flags, s32 Location )
{
#if !defined(X_RETAIL) || defined(X_QA)
    // text debug - used to locate hard-coded strings
    extern xbool    g_bStringTest;
#endif

    token_stream    Stream;
    file_section    FileSection = SECTION_UNDEFINED;
    s32             Cursor;

    Stream.OpenText( pMapFile );

    while( TRUE )
    {
        Cursor = Stream.GetCursor();
        Stream.Read();
        if( Stream.IsEOF() )
        {
            break;
        }
        if( x_stricmp( Stream.String(), "[VERSION]" ) == 0 )
        {
            FileSection = SECTION_VERSION;
        }
        else if( x_stricmp( Stream.String(), "[GAMETYPE]" ) == 0 )
        {
            FileSection = SECTION_GAMETYPE;
        }
        else if( x_stricmp( Stream.String(), "[MAP]" ) == 0 )
        {
            FileSection = SECTION_MAPTYPE;
        }
        else if( x_stricmp( Stream.String(), "[PLAY]" ) == 0 )
        {
            FileSection = SECTION_PLAY;
        }
        else if( FileSection == SECTION_VERSION )
        {
            Stream.SetCursor( Cursor );
            m_Version = Stream.ReadInt();
            Stream.Read();
            ASSERT( x_strcmp( Stream.String(), ";" ) == 0 );
        }
        else if( FileSection == SECTION_GAMETYPE )
        {
            //
            // Entries within this section of the file take the following format:
            // <GameTypeID>, <ShortTypeName>, <LongTypeName>, <Description>
            // e.g. -2, "SOLO", "Campaign", "This is a campaign"
            //
            // It is used to describe a game type
            //
            game_type_info& TypeEntry = m_GameTypes.Append();

            Stream.SetCursor( Cursor );
            TypeEntry.Type          = (game_type)Stream.ReadInt();  Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
            TypeEntry.ShortTypeName = Stream.ReadString();          Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
            TypeEntry.TypeName      = Stream.ReadString();          Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
            TypeEntry.Rules         = Stream.ReadString();          Stream.Read();  
            while( x_strcmp( Stream.String(), ";")!=0 )
            {
                ASSERTS( Stream.Type() == token_stream::TOKEN_STRING, "Expected quoted string or ;" );
                TypeEntry.Rules += "\n";
                TypeEntry.Rules += Stream.String();
                Stream.Read();
            }

#if !defined(X_RETAIL) || defined(X_QA)
            if(g_bStringTest)
            {
                FillString((const char*)TypeEntry.TypeName);
                FillString((const char*)TypeEntry.Rules);
            }
#endif
        }
        else if( FileSection == SECTION_MAPTYPE )
        {
            //
            // Entries within this section of the file take the following format:
            // <mapid>, <path-to-dfs-file>, <display-name>, <description>
            // e.g. 3000, "mp_00\blaze", "Blaze", "This is the blaze map"
            //
            // It is used to describe an individual map
            //
            map_info&   MapInfo = m_Maps.Append();


            Stream.SetCursor( Cursor );
            MapInfo.MapID       = Stream.ReadInt();     Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
            MapInfo.Filename    = Stream.ReadString();  Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
            Stream.Read();
            if( Stream.Type()==token_stream::TOKEN_NUMBER )
            {
                MapInfo.Length  = Stream.Int();         Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
                Stream.Read();
            }
            else
            {
                MapInfo.Length  = 0;
            }

            MapInfo.DisplayName = Stream.String();      Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
            MapInfo.Description = Stream.ReadString();  Stream.Read();  
            MapInfo.Flags       = Flags;
            while( x_strcmp( Stream.String(), ";")!=0 )
            {
                ASSERTS( Stream.Type() == token_stream::TOKEN_STRING, "Expected quoted string or ;" );
                MapInfo.Description += "\n";
                MapInfo.Description += Stream.String(); 
                Stream.Read();
            };

#if !defined(X_RETAIL) || defined(X_QA)
            if(g_bStringTest)
            {
                FillString((const char*)MapInfo.DisplayName);
                FillString((const char*)MapInfo.Description);
            }
#endif

        }
        else if( FileSection == SECTION_PLAY )
        {
            //
            // Entries within this section of the file take the following format:
            // <gametypeid>, <mapid>, <minplayers>, <maxplayers>
            // e.g. -2, 3000, 1, 16
            //
            // It is used to bind a game type to a map
            //
            Stream.SetCursor( Cursor );

            token_stream::type TokenType = Stream.Read();

            if( TokenType == token_stream::TOKEN_NUMBER )
            {
                map_entry MapEntry;

                MapEntry.m_GameTypeID   = (game_type)Stream.Int();      Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
                MapEntry.m_MapID        =            Stream.ReadInt();  Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
                MapEntry.m_MinPlayers   =            Stream.ReadInt();  Stream.Read();  ASSERT( x_strcmp( Stream.String(), ",")==0 );
                MapEntry.m_MaxPlayers   =            Stream.ReadInt();  Stream.Read();  ASSERT( x_strcmp( Stream.String(), ";")==0 );
                MapEntry.m_Location     =            Location;

                // Verify that we have the game type and map already defined for this play binding.
                ASSERTS( GetMapInfo(MapEntry.m_MapID), "Map needs to be defined before bind." );
                ASSERTS( GetGameTypeInfo(MapEntry.m_GameTypeID), "GameType needs to be defined before bind." );

                m_MapList.Append() = MapEntry;
            }
            else
            {
                Stream.SkipToNextLine();
            }
        }
        else
        {
            ASSERTS( FALSE, "Expected [VERSION]|[GAMETYPE]|[MAP]|[PLAY]" );
        }
    }
    Stream.CloseText();
}

//=========================================================================

void map_list::SetVersion( s32 Version )
{
    m_Version = Version;
}

//=========================================================================

s32 map_list::GetVersion( void )
{
    return m_Version;
}

//=========================================================================

xstring map_list::Serialize( void ) const
{
    xstring Manifest;
    s32     i;

    //
    // Start off with version information
    //
    Manifest = xfs( "[Version]\n%d;\n\n", m_Version );

    // 
    // Now build the GAMETYPE section
    //
    if( m_GameTypes.GetCount() )
    {
        Manifest += "\n//========\n[GameType]\n";
        for( i=0; i<m_GameTypes.GetCount(); i++ )
        {
            const game_type_info& GameInfo = m_GameTypes[i];

            Manifest += xfs( "%d, \"%s\", \"%s\", ", GameInfo.Type, (const char*)GameInfo.ShortTypeName, (const char*)GameInfo.TypeName );
            // Each rule line is seperated by a '\n' character, so we need to pad that out to a quoted string as in the original format.
            const char* pString = (const char*)GameInfo.Rules;

            while( x_strlen( pString ) )
            {
                xstring TempString;
                while( *pString == '\n' )
                {
                    pString++;
                }
                while( *pString && (*pString!='\n') )
                {
                    TempString += *pString++;
                }
                Manifest += xfs( "\n%20c\"%s\"", ' ', (const char*)TempString );
            }
            Manifest += ";\n";

        }
    }

    if( m_Maps.GetCount() )
    {
        Manifest += "\n//========\n[Map]\n";
        for( i=0; i<m_Maps.GetCount(); i++ )
        {
            const map_info& MapInfo = m_Maps[i];

            Manifest += xfs( "%d, \"%s\", %d, \"%s\", ", MapInfo.MapID, (const char*)MapInfo.Filename, MapInfo.Length, (const char*)MapInfo.DisplayName );

            const char* pString = (const char*)MapInfo.Description;

            while( x_strlen( pString ) )
            {
                xstring TempString;
                while( *pString == '\n' )
                {
                    pString++;
                }
                while( *pString && (*pString!='\n') )
                {
                    TempString += *pString++;
                }
                Manifest += xfs( "\n%20c\"%s\"", ' ', (const char*)TempString );
            }
            Manifest += ";\n";
        }
    }

    if( m_MapList.GetCount() )
    {
        Manifest += "\n//========\n[Play]\n";
        for( i=0; i<m_MapList.GetCount(); i++ )
        {
            const map_entry& MapEntry = m_MapList[i];

            Manifest += xfs( "%d, %d, %d, %d;\n", MapEntry.m_GameTypeID, MapEntry.m_MapID, MapEntry.m_MinPlayers, MapEntry.m_MaxPlayers );
        }
    }

    x_DebugMsg( "%s", (const char*)Manifest );
    Manifest.SaveFile( "c:\\temp\\manifest.txt" );
    return Manifest;
}

//=========================================================================

const game_type_info* map_list::GetGameTypeInfo( s32 TypeID ) const
{
    s32 i;
    game_type_info Dummy;

    Dummy.Type = (game_type)TypeID;

    i=m_GameTypes.Find( Dummy );
    if( i!= -1 )
    {
        return &m_GameTypes[i];
    }
    if( this != &g_MapList )
    {
        return g_MapList.GetGameTypeInfo( TypeID );
    }
    return NULL;
}

//=========================================================================
const map_info* map_list::GetMapInfo( s32 MapID ) const
{
    s32 i;
    map_info Dummy;

    Dummy.MapID = MapID;
    i = m_Maps.Find( Dummy );
    if( i!=-1 )
    {
        return &m_Maps[i];
    }

    if( this != &g_MapList )
    {
        return g_MapList.GetMapInfo( MapID );
    }
    return NULL;
}

//=========================================================================
s32 map_entry::GetMapID ( void ) const
{ 
    return m_MapID;
}

//=========================================================================
const char* map_entry::GetFilename ( void ) const 
{ 
    return g_MapList.GetMapInfo( m_MapID )->Filename; 
}

//=========================================================================
const char* map_entry::GetDisplayName ( void ) const 
{ 
    return g_MapList.GetMapInfo( m_MapID )->DisplayName; 
}

//=========================================================================
const char* map_entry::GetDescription ( void ) const 
{ 
    return g_MapList.GetMapInfo( m_MapID )->Description; 
}

//=========================================================================
map_flags map_entry::GetFlags ( void ) const 
{ 
    return g_MapList.GetMapInfo( m_MapID )->Flags; 
}

//=========================================================================
game_type map_entry::GetGameType ( void ) const 
{ 
    return m_GameTypeID;
}

//=========================================================================
const char* map_entry::GetShortGameTypeName( void ) const 
{ 

    return g_MapList.GetGameTypeInfo( m_GameTypeID )->ShortTypeName; 
}

//=========================================================================
const char* map_entry::GetGameTypeName ( void ) const 
{ 
    return g_MapList.GetGameTypeInfo( m_GameTypeID )->TypeName; 
}

//=========================================================================
const char* map_entry::GetGameRules( void ) const 
{ 
    return g_MapList.GetGameTypeInfo( m_GameTypeID )->Rules; 
}

//=========================================================================
xbool map_entry::IsAvailable ( void ) const 
{ 
    return GetFlags() != MF_NOT_PRESENT;
}

//=========================================================================
xbool map_entry::operator== ( const map_entry& MapEntry ) const
{
    return (m_GameTypeID==MapEntry.m_GameTypeID) && (m_MapID==MapEntry.m_MapID);
}

//=========================================================================
xbool map_entry::operator!= ( const map_entry& MapEntry ) const
{
    return (m_GameTypeID!=MapEntry.m_GameTypeID) || (m_MapID!=MapEntry.m_MapID);
}

//=========================================================================
map_info::map_info( void )
{
    MapID           = -1;
    Flags           = MF_NOT_PRESENT;
}

//=========================================================================
map_info::~map_info( void )
{
}

//=========================================================================
xbool map_info::operator == ( const map_info& Right ) const
{
    return (Right.MapID == MapID);
}

//=========================================================================
game_type_info::game_type_info( void )
{
    Type            = GAME_DM;
}

//=========================================================================
game_type_info::~game_type_info( void )
{
}

//=========================================================================
xbool game_type_info::operator==( const game_type_info& Right ) const
{
    return (Right.Type == Type);
}

//=========================================================================
xbool map_info::IsAvailable( void )
{
    return (Flags != MF_NOT_PRESENT );
}
