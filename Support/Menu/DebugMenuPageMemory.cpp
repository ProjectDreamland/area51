//==============================================================================
//  DebugMenuPageMemory.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu memory page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "Configuration/GameConfig.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

extern xbool DISPLAY_SMEM_STATS;
extern stats g_Stats;

//==============================================================================

debug_menu_page_memory::debug_menu_page_memory( ) : debug_menu_page()
{
    m_pTitle = "Memory";

#if defined( TARGET_PS2 ) && !defined( X_RETAIL )
                                 AddItemBool     ( "Display memory stats" , DISPLAY_SMEM_STATS );
                                 AddItemBool     ( "PS2 memory stats"     , g_Stats.PS2_MemStats );
                                 AddItemBool     ( "Vertical memory bars" , g_Stats.MemVertBars );
                                 AddItemSeperator( );

    m_pItemMemorySummaryDump   = AddItemButton   ( "Dump memory summary to file" );
    m_pItemResourceSummaryDump = AddItemButton   ( "Dump resource summary file" );
    m_pItemObjMgrSummaryDump   = AddItemButton   ( "Dump Object Manager summary file" );
#endif
    m_pItemMemoryDump          = AddItemButton   ( "COMPLETE Memory dump to file" );
}

//==============================================================================

bool FileExists( const xstring PathName )
{
    X_FILE* pFile = x_fopen( PathName, "rb" );
    if( pFile )
        x_fclose( pFile );
    return( pFile != NULL );
}

xstring FindNextFileInSequence( const char* pFile, const char* pExtension )
{
    s32 Index = 0;

    while( 1 )
    {
        xstring Name = pFile;
        if( Index > 0 )
            Name.AddFormat( "_%04d", Index );
        Name.AddFormat( ".%s", pExtension );

        if( !FileExists( Name ) )
            return Name;

        Index++;
    }
}

void debug_menu_page_memory::OnChangeItem( debug_menu_item* pItem )
{
    char LevelName[32];
    x_splitpath( g_ActiveConfig.GetLevelPath(),NULL,NULL,LevelName,NULL);

    if( pItem == m_pItemMemoryDump )
    {
        xstring PathName = FindNextFileInSequence( xfs("c:\\MemoryDump_%s",LevelName), "csv" );
        x_MemDump( PathName, TRUE );
    }
#if defined( TARGET_PS2 ) && !defined( X_RETAIL )
    else if( pItem == m_pItemMemorySummaryDump ) 
    {
        void A51_MemReport( const char* pFileName );
        xstring PathName = FindNextFileInSequence( xfs("c:\\MemorySummary_%s",LevelName), "csv" );
        A51_MemReport( PathName );
    }
    else if( pItem == m_pItemResourceSummaryDump ) 
    {
        #ifdef RSC_MGR_COLLECT_STATS
        xstring PathName = FindNextFileInSequence( xfs("c:\\ResourceSummary_%s",LevelName), "csv" );
        g_RscMgr.DumpStatsToFile( PathName );
        #endif
    }
    else if( pItem == m_pItemObjMgrSummaryDump ) 
    {
        xstring PathName = FindNextFileInSequence( xfs("c:\\ObjMgrSummary_%s",LevelName), "csv" );
        g_ObjMgr.DumpStatsToFile( PathName );
    }
#endif
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
