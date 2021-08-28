//==============================================================================
//  
//  x_context.cpp
//
//==============================================================================

#include "..\x_files.hpp"
#include "..\x_context.hpp"

//==============================================================================
//  Make this whole file go away if CONTEXT teacking is off
//==============================================================================

#if defined(ENABLE_CONTEXT_TRACKING)

//==============================================================================
//  DATA
//==============================================================================

static xbool            s_Initialized       = FALSE;

static xtick            s_InitTicks;                                            // Ticks at xContext_Init
static xcontext_node*   s_pCurrentNode=NULL;                                    // Pointer to current context node
static s32              s_nNodes;                                               // Number of context nodes allocated
static xcontext_node    s_GlobalContextNode = { "GLOBAL", NULL, NULL, NULL, NULL, 0, 0, 0, 0 };// The global context node
       s32              s_DisplayFrameCount;
static xbool            s_bSpikeOccurred;

xcontext_debug          g_Context;

//==============================================================================
//  x_ContextInit
//==============================================================================

void x_ContextInit( void )
{
    ASSERT( !s_Initialized );

    // Setup current context node, enable profiling, set initialized
    s_InitTicks         = x_GetTime();
    s_pCurrentNode      = &s_GlobalContextNode;
    s_nNodes            = 1;

    //
    // Setup initial context setting
    //
    g_Context.bProfilingEnabled                 = FALSE;
    g_Context.bDisplayEnabled                   = FALSE;
    g_Context.bDisplayHierarchyEnabled          = TRUE;
    g_Context.bDisplaySummaryEnabled            = TRUE;

    g_Context.bShowAllOverrideEnabled           = FALSE;
    g_Context.bShowSubstringFilteredEnabled     = FALSE;
    g_Context.bHideSubstringFilteredEnabled     = TRUE;
    g_Context.bSubstringParentFilterEnabled     = TRUE;

    g_Context.bMSFilterEnabled                  = TRUE;
    g_Context.bCallsFilterEnabled               = TRUE;

    g_Context.TotalMSThreshold                  = 0.0125f;
    g_Context.TotalCallsThreshold               = 250;
    g_Context.DisplaySpikeMSThreshold           = F32_MAX;

    g_Context.bCommandSetAllSubstringToOn       = FALSE;
    g_Context.bCommandSetAllSubstringToOff      = FALSE;
    g_Context.bCommandFilterSubstring           = FALSE;
    x_strcpy(g_Context.SubstringBuffer,"");

    #ifdef TARGET_PC
    g_Context.nFramesBetweenDisplays            = 20;
    #else
    g_Context.nFramesBetweenDisplays            = 10;
    #endif

    s_DisplayFrameCount = g_Context.nFramesBetweenDisplays;

    g_Context.bBuildTree = FALSE;

    s_Initialized       = TRUE;
    s_bSpikeOccurred = FALSE;

}

//==============================================================================
//  x_ContextKill
//==============================================================================

void x_ContextKill( void )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrentNode == &s_GlobalContextNode );

    s_Initialized = FALSE;
}

//==============================================================================
//  x_ContextEnableProfiling
//==============================================================================

void x_ContextEnableProfiling( void )
{
    g_Context.bProfilingEnabled = TRUE;
}

//==============================================================================
//  x_ContextDisableProfiling
//==============================================================================

void x_ContextDisableProfiling( void )
{
    g_Context.bProfilingEnabled = FALSE;
}

//==============================================================================
//  x_ContextResetProfile
//==============================================================================

static void ResetNode( xcontext_node* pNode )
{
    // Reset profile data
    pNode->Hits  = 0;
    pNode->Ticks = 0;

    // Iterate through children resetting
    pNode = pNode->pChildren;
    while( pNode )
    {
        ResetNode( pNode );
        pNode = pNode->pNext;
    }
}

void x_ContextResetProfile( void )
{
    ResetNode( &s_GlobalContextNode );
    s_InitTicks = x_GetTime();

    // Flip bool on whether to build tree or not
    g_Context.bBuildTree = g_Context.bProfilingEnabled;
}

//==============================================================================
//  SolveVisibleNodes
//==============================================================================

static void SolveVisibleNodes( xcontext_node* pNode )
{

    // Assume initially that we will render it
    pNode->Flags |= XCONTEXT_FLAG_VISIBLE;

    //
    // Handle substring filtering
    //
    if( 
        (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT)
        ||
        (g_Context.bSubstringParentFilterEnabled && (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT))
      )
    {
        if( g_Context.bShowSubstringFilteredEnabled )
            pNode->Flags |= XCONTEXT_FLAG_VISIBLE;

        if( g_Context.bHideSubstringFilteredEnabled )
            pNode->Flags &= ~XCONTEXT_FLAG_VISIBLE;
    }
    else
    {
        if( g_Context.bShowSubstringFilteredEnabled )
            pNode->Flags &= ~XCONTEXT_FLAG_VISIBLE;

        if( g_Context.bHideSubstringFilteredEnabled )
            pNode->Flags |= XCONTEXT_FLAG_VISIBLE;
    }

    // Check if it should be hidden due to the number of MS
    if( g_Context.bMSFilterEnabled )
    {
        if( x_TicksToMs(pNode->Ticks) < g_Context.bMSFilterEnabled )
            pNode->Flags &= ~XCONTEXT_FLAG_VISIBLE;
    }

    // Check if it should be visible due to the number of calls
    if( g_Context.bCallsFilterEnabled )
    {
        if( pNode->Hits >= g_Context.TotalCallsThreshold )
            pNode->Flags |= XCONTEXT_FLAG_VISIBLE;
    }

    // Check if it should be visible from the ShowAllOverride
    if( g_Context.bShowAllOverrideEnabled )
        pNode->Flags |= XCONTEXT_FLAG_VISIBLE;

    // Hide node if it has ZERO hits
    if( pNode->Hits == 0 )
        pNode->Flags &= ~XCONTEXT_FLAG_VISIBLE;

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        SolveVisibleNodes( pNode );
        pNode = pNode->pNext;
    }
}

//==============================================================================

static void PushClearSubstringFilter( xcontext_node* pNode, xbool On )
{
    pNode->Flags &= ~XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT;
    pNode->Flags &= ~XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT;
    if( On )
    {
        pNode->Flags |= XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT;
        pNode->Flags |= XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT;
    }

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        PushClearSubstringFilter( pNode, On );
        pNode = pNode->pNext;
    }
}

void x_ContextClearSubstringFilter( xbool On )
{
    PushClearSubstringFilter( &s_GlobalContextNode, On );
}

//==============================================================================

static void PushSubstringFilter( xcontext_node* pNode, const char* pSubstring, xbool ParentFiltered )
{
    // If a parent was filtered then propogate the On state in the ParentFlag
    if( ParentFiltered )
    {
        pNode->Flags |= XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT;
    }

    // Check if name contains substring
    if( x_stristr( pNode->pName, pSubstring ) != NULL )
    {
        ParentFiltered = TRUE;
        pNode->Flags |= XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT;
    }

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        PushSubstringFilter( pNode, pSubstring, ParentFiltered );
        pNode = pNode->pNext;
    }
}

void x_ContextSubstringFilter( const char* pSubstring )
{
    PushSubstringFilter( &s_GlobalContextNode, pSubstring, FALSE );
}

//==============================================================================

static xcontext_node* PushFindDuplicate( xcontext_node* pNode, const char* pName )
{
    // Check if this is it
    if( pNode->pDuplicate == NULL )
    {
        if( x_stricmp( pName, pNode->pName ) == 0 )
        {
            return pNode;
        }
    }

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        xcontext_node* pDuplicate = PushFindDuplicate( pNode, pName );
        if( pDuplicate )
            return pDuplicate;
        pNode = pNode->pNext;
    }

    return NULL;
}

//==============================================================================

static void SolveSummaryNode( xcontext_node* pNode )
{
    // Move this node's data up to the duplicate
    if( pNode->pDuplicate  )
    {
        pNode->pDuplicate->Hits += pNode->Hits;
        pNode->pDuplicate->Ticks += pNode->Ticks;
        pNode->pDuplicate->ChildrenTicks += pNode->ChildrenTicks;
    }

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        SolveSummaryNode( pNode );
        pNode = pNode->pNext;
    }
}

//==============================================================================

static void PrintHierarchyNode( xcontext_node* pNode, s32 Depth, X_FILE* pFile );

static void PushPrintNode( xcontext_node* pNode, s32 Depth, X_FILE* pFile )
{
    if( pNode )
    {
        if( pNode->pNext )
            PushPrintNode( pNode->pNext, Depth, pFile );
        PrintHierarchyNode( pNode, Depth, pFile );
    }
}

//==============================================================================

static void SolveChildrenTicks( xcontext_node* pNode )
{
    // Compute measured execution time of children
    pNode->ChildrenTicks = 0;

    xcontext_node* pChild = pNode->pChildren;
    while( pChild )
    {
        pNode->ChildrenTicks += pChild->Ticks;
        pChild = pChild->pNext;
    }

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        SolveChildrenTicks( pNode );
        pNode = pNode->pNext;
    }
}

//==============================================================================

static void PrintHierarchyNode( xcontext_node* pNode, s32 Depth, X_FILE* pFile )
{

    // Compute spacing for text
    static char VertBars[] = "||||||||||||||||||||||||||||||||";
    static char HorizBar[] = "--------------------------------";
    static char NameTab [] = "................................";
    s32 Indent = Depth;
    s32 DistToName = 20 - Indent - 2;
    DistToName = MIN(DistToName,20);
    DistToName = MAX(DistToName,0);

    // Patch VertBars
    char VB = VertBars[Indent];
    VertBars[Indent] = 0;

    //
    // Is this a normal line or a parent to children?
    //
    if( pNode->pChildren )
    {
        if( pNode->Flags & XCONTEXT_FLAG_VISIBLE )
        {
            if( pFile )
            {
                x_fprintf( pFile,   "%s*-%s[%6d]-(%9.3f)-(%9.3f)-(%9.3f) [%1d%1d] %s\n", 
                                    VertBars,
                                    HorizBar + 32 - DistToName,
                                    pNode->Hits, 
                                    x_TicksToMs(pNode->Ticks),
                                    x_TicksToMs(pNode->Ticks)-x_TicksToMs(pNode->ChildrenTicks),
                                    x_TicksToMs(pNode->ChildrenTicks),
                                    (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT) ? (1) : (0),
                                    (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT) ? (1) : (0),
                                    pNode->pName
                         );
            }
            else
            {
                x_DebugMsg(7,"%s*-%s[%6d]-(%9.3f)-(%9.3f)-(%9.3f) [%1d%1d] %s\n", 
                            VertBars,
                            HorizBar + 32 - DistToName,
                            pNode->Hits, 
                            x_TicksToMs(pNode->Ticks),
                            x_TicksToMs(pNode->Ticks)-x_TicksToMs(pNode->ChildrenTicks),
                            x_TicksToMs(pNode->ChildrenTicks),
                            (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT) ? (1) : (0),
                            (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT) ? (1) : (0),
                            pNode->pName
                        );
            }
        }

        VertBars[Indent] = VB;

        // Iterate through children
        PushPrintNode( pNode->pChildren, Depth+1, pFile );
    }
    else
    {
        if( pNode->Flags & XCONTEXT_FLAG_VISIBLE )
        {
            if( pFile )
            {
                x_fprintf( pFile,   "%s..%s[%6d].(%9.3f)........................ [%1d%1d] %s\n",
                                    VertBars,
                                    NameTab + 32 - DistToName,
                                    pNode->Hits, 
                                    x_TicksToMs(pNode->Ticks),
                                    (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT) ? (1) : (0),
                                    (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT) ? (1) : (0),
                                    pNode->pName
                         );
            }
            else
            {
                x_DebugMsg(7, "%s..%s[%6d].(%9.3f)........................ [%1d%1d] %s\n",
                            VertBars,
                            NameTab + 32 - DistToName,
                            pNode->Hits, 
                            x_TicksToMs(pNode->Ticks),
                            (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT) ? (1) : (0),
                            (pNode->Flags & XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT) ? (1) : (0),
                            pNode->pName
                        );
            }
        }
    }

    VertBars[Indent] = VB;
}

//==============================================================================

static void PrintSummaryNode( xcontext_node* pNode, X_FILE* pFile )
{
    // Is this a single node?
    if( pNode->pDuplicate == NULL  )
    {
        if( pNode->Flags & XCONTEXT_FLAG_VISIBLE )
        {
            if( pFile )
            {
                x_fprintf( pFile,   "[%6d]..(%9.3f).(%9.3f).(%9.3f)..%s\n",
                                    pNode->Hits, 
                                    x_TicksToMs(pNode->Ticks),
                                    x_TicksToMs(pNode->Ticks)-x_TicksToMs(pNode->ChildrenTicks),
                                    x_TicksToMs(pNode->ChildrenTicks),
                                    pNode->pName
                         );
            }
            else
            {
                x_DebugMsg(7, "[%6d]..(%9.3f).(%9.3f).(%9.3f)..%s\n",
                            pNode->Hits, 
                            x_TicksToMs(pNode->Ticks),
                            x_TicksToMs(pNode->Ticks)-x_TicksToMs(pNode->ChildrenTicks),
                            x_TicksToMs(pNode->ChildrenTicks),
                            pNode->pName
                        );
            }
        }
    }

    // Iterate through children
    pNode = pNode->pChildren;
    while( pNode )
    {
        PrintSummaryNode( pNode, pFile );
        pNode = pNode->pNext;
    }
}

//==============================================================================

void x_ContextPrintProfile( void )
{
    // Provide a quick exit if context tracking has been disabled
#ifndef ENABLE_CONTEXT_TRACKING
    return;
#endif

    // Check commands for this turn
    {
        if( g_Context.bCommandSetAllSubstringToOn )
        {
            x_ContextClearSubstringFilter( TRUE );
            g_Context.bCommandSetAllSubstringToOn = FALSE;
        }

        if( g_Context.bCommandSetAllSubstringToOff )
        {
            x_ContextClearSubstringFilter( FALSE );
            g_Context.bCommandSetAllSubstringToOff = FALSE;
        }

        if( g_Context.bCommandFilterSubstring )
        {
            x_ContextSubstringFilter( g_Context.SubstringBuffer );
            g_Context.bCommandFilterSubstring = FALSE;
        }
    }

    xbool bDoDisplay = g_Context.bDisplayEnabled ;

    // Set global node profile data
    s_GlobalContextNode.Hits  = 1;
    s_GlobalContextNode.Ticks = x_GetTime() - s_InitTicks;

    // Check if we've spiked and need to do a dump
    if( (!s_bSpikeOccurred) && (x_TicksToMs(s_GlobalContextNode.Ticks) > g_Context.DisplaySpikeMSThreshold) )
    {
        s_DisplayFrameCount = 0;
        s_bSpikeOccurred = TRUE;
        bDoDisplay = TRUE;
    }
    else
    {
        // If we've reached the end of the frame count then display
        s_DisplayFrameCount--;
        if( s_DisplayFrameCount >= 0 )
            return;
        s_DisplayFrameCount = g_Context.nFramesBetweenDisplays;
        s_bSpikeOccurred = FALSE;
    }

    // If display is turned off then just reset the frame count
    if( !bDoDisplay )
    {
        s_DisplayFrameCount = g_Context.nFramesBetweenDisplays;
        return;
    }

    SolveChildrenTicks( &s_GlobalContextNode );

    // Print nodes recursively
    if( g_Context.bDisplayHierarchyEnabled )
    {
        SolveVisibleNodes( &s_GlobalContextNode );
        x_DebugMsg(7, "\n" );
        x_DebugMsg(7, "*-------------------nCalls---(TotalMs)---(InFunct)-(InChild)---------------*\n");
        PrintHierarchyNode( &s_GlobalContextNode, 0, NULL );
    }

    if( g_Context.bDisplaySummaryEnabled )
    {
        x_DebugMsg(7, "\n" );
        x_DebugMsg(7, "*---------(TotalMs)---(InFunct)-(InChild)----------------------------------*\n");
        SolveSummaryNode( &s_GlobalContextNode );
        SolveVisibleNodes( &s_GlobalContextNode );
        PrintSummaryNode( &s_GlobalContextNode, NULL );

        x_DebugMsg(7, "*--------------------------------------------------------------------------*\n");
        x_DebugMsg(7, "nNodes   : %d\n",s_nNodes);
        x_DebugMsg(7, "MemUsed  : %d\n",s_nNodes*(sizeof(xcontext_node)+32));
        x_DebugMsg(7, "*--------------------------------------------------------------------------*\n");
    }

}

//==============================================================================
//  x_ContextSaveProfile
//==============================================================================

void x_ContextSaveProfile( const char* pPathName )
{
    // Open the file
    X_FILE* pFile = x_fopen( pPathName, "w" );
    ASSERT( pFile );

    // Set global node profile data
    s_GlobalContextNode.Hits  = 1;
    s_GlobalContextNode.Ticks = x_GetTime() - s_InitTicks;

    g_Context.bShowAllOverrideEnabled = TRUE;

    SolveChildrenTicks( &s_GlobalContextNode );

    // Print nodes recursively
    SolveVisibleNodes( &s_GlobalContextNode );
    x_fprintf( pFile, "\n" );
    x_fprintf( pFile, "*-------------------nCalls---(TotalMs)---(InFunct)-(InChild)---------------*\n");
    PrintHierarchyNode( &s_GlobalContextNode, 0, pFile );

    x_fprintf( pFile, "\n" );
    x_fprintf( pFile, "*---------(TotalMs)---(InFunct)-(InChild)----------------------------------*\n");
    SolveSummaryNode( &s_GlobalContextNode );
    SolveVisibleNodes( &s_GlobalContextNode );
    PrintSummaryNode( &s_GlobalContextNode, pFile );

    x_fprintf( pFile, "*--------------------------------------------------------------------------*\n");
    x_fprintf( pFile, "nNodes   : %d\n",s_nNodes);
    x_fprintf( pFile, "MemUsed  : %d\n",s_nNodes*(sizeof(xcontext_node)+32));
    x_fprintf( pFile, "*--------------------------------------------------------------------------*\n");

    // Close the file
    x_fclose( pFile );
}

//==============================================================================

void DeleteTree( xcontext_node* pNode )
{
    if( pNode->Hits > 0 )
        return;

    xcontext_node* pChild = pNode->pChildren;
    while( pChild )
    {
        DeleteTree( pChild );
        pChild = pChild->pNext;
    }

    if( pNode != &s_GlobalContextNode )
        delete pNode;
}

//==============================================================================

void x_ContextClear( void )
{
    DeleteTree( &s_GlobalContextNode );
    x_ContextResetProfile();
}

//==============================================================================
//  xcontext
//==============================================================================

xcontext::xcontext( const char* pName )
{
    if( g_Context.bBuildTree == FALSE )
        return;

    if( s_Initialized == FALSE )
        return;

    // Search children of current node for a match
    xcontext_node* pNode = s_pCurrentNode->pChildren;
    while( pNode )
    {
        // Do the addresses of the names match?
        if( pNode->pName == pName )
        {
            // Found, set current node pointer
            s_pCurrentNode = pNode;

            // Set profiling data
            if( g_Context.bProfilingEnabled )
            {
                m_StartTicks = x_GetTime();
            }

            // Exit
            return;
        }
        pNode = pNode->pNext;
    }

    // Look through tree and see if we can find a duplicate
    xcontext_node* pDuplicate = PushFindDuplicate( &s_GlobalContextNode, pName );

    // Not found, allocate a new node and initialize
    pNode = (xcontext_node*)x_malloc( sizeof(xcontext_node) );
    pNode         ->pName     = pName;

    // Init profile data
    pNode         ->Hits      = 0;
    pNode         ->Ticks     = 0;
    pNode         ->Flags     = XCONTEXT_FLAG_VISIBLE;

    // Link node into tree
    pNode         ->pParent   = s_pCurrentNode;
    pNode         ->pNext     = s_pCurrentNode->pChildren;
    pNode         ->pChildren = NULL;
    s_pCurrentNode->pChildren = pNode;

    pNode         ->pDuplicate = pDuplicate;
    pNode         ->ChildrenTicks = 0;

    // Set this as the current node
    s_pCurrentNode            = pNode;

    // Increment number of nodes created
    s_nNodes++;

    // Set profling status
    if( g_Context.bProfilingEnabled )
    {
        m_StartTicks = x_GetTime();
    }
}

//==============================================================================

xcontext::~xcontext()
{
    if( s_Initialized == FALSE )
        return;

    if( g_Context.bBuildTree == FALSE )
        return;

    // Fill in profile stats
    if( g_Context.bProfilingEnabled )
    {
        s_pCurrentNode->Hits++;
        s_pCurrentNode->Ticks += x_GetTime() - m_StartTicks;
    }

    // Set parent context
    //if( s_pCurrentNode == (xcontext_node*)this )
        s_pCurrentNode = s_pCurrentNode->pParent;
}

//==============================================================================

void x_ContextDisplayStack( xbool DumpToScreen, xbool DumpToTTY )
{
    const xcontext_node* pCur = s_pCurrentNode;

    s32     Y = 1;
    s32     X = 1;

    if (DumpToTTY)
    {   
        x_DebugMsg("CONTEXT STACK DUMP\n");
        x_DebugMsg("------------------\n");
    }

    char        Name[ 25 ] = {0};
    
    while (pCur)
    {
        if (pCur->pName)
        {
            x_strncpy( Name, pCur->pName, 24 );            
        }
        else
        {
            x_strcpy( Name, "-UNNAMED-" );
        }

        if (DumpToScreen)
            x_printfxy(X,Y,Name);
        if (DumpToTTY)
            x_DebugMsg("%s\n",Name);

        Y++;
        if (Y >= 22)
        {
            Y = 1;
            X += 25;
        }

        pCur = pCur->pParent;
    }    
    
    if (DumpToTTY)
    {   
        x_DebugMsg("-------------------------\n");
        x_DebugMsg("END OF CONTEXT STACK DUMP\n");        
    }
}

//==============================================================================

#endif //defined(ENABLE_CONTEXT_TRACKING)
