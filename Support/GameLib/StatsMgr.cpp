///////////////////////////////////////////////////////////////////////////////
//
//  stats_mgr
//
//      Keeps up with all performance stats for the game
//
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Includes
//=============================================================================

#include "render\Render.hpp"
#include "Entropy\Audio\audio_channel_mgr.hpp"
#include "e_ScratchMem.hpp"
#include "StatsMgr.hpp"

#ifndef TARGET_PC
#include "e_engine.hpp"
#endif

#ifdef TARGET_PS2
#include "PS2\PS2_misc.hpp"
#endif

//=============================================================================

#if ENABLE_STATS_MGR

//=============================================================================
// Externs
//=============================================================================

#if defined(TARGET_PS2)
extern s32   s_SMem;
extern s32   ENG_DLIST_MAX_USED;
#endif

#if ENABLE_RENDER_STATS
extern   render::stats s_RenderStats;
#endif

//=============================================================================
// Statics
//=============================================================================

stats_mgr*   stats_mgr::s_This = NULL;
xbool        stats_mgr::m_bShowNumbers = FALSE;
#if defined(CONFIG_QA) || defined(CONFIG_RETAIL) || defined(aharp)
xbool        stats_mgr::m_bShowHorizontalBars = FALSE;
#else
xbool        stats_mgr::m_bShowHorizontalBars = TRUE;
#endif

xbool stats_mgr::m_bShowCPUTimeLegend  = FALSE;
xbool stats_mgr::m_bShowGPUTimeLegend  = FALSE;
xbool stats_mgr::m_bShowVerticalLegend = FALSE;

//=============================================================================
// Text and color display info
//=============================================================================

struct stat_render_info
{
    stat_render_info( const char* pName_, xcolor BarColor_ ) { pName = pName_; BarColor = BarColor_; }
    const char* pName;
    xcolor      BarColor;
};

stat_render_info StatRenderInfo[k_stats_Last] =
{
    stat_render_info( "OnAdvance",      xcolor(128, 128, 0,   255) ),   // k_stats_OnAdvance
    stat_render_info( "DebrisLogic",    xcolor(255, 128, 0,   255) ),   // k_stats_Debris
    stat_render_info( "AIThink",        xcolor(0,   255, 0,   255) ),   // k_stats_AI_Think
    stat_render_info( "AIAdvance",      xcolor(128, 64,  0,   255) ),   // k_stats_AI_Advance
    stat_render_info( "Animation",      xcolor(0,   0,   100, 255) ),   // k_stats_Animation
    stat_render_info( "Collision",      xcolor(128, 128, 255, 255) ),   // k_stats_Collision
    stat_render_info( "Sound",          xcolor(100, 100, 255, 255) ),   // k_stats_Sound
    stat_render_info( "ParticleLogic",  xcolor(255, 255,  0,  255) ),   // k_stats_ParticleSystem
    stat_render_info( "Decals",         xcolor(0,   255, 255, 255) ),   // k_stats_DecalSystem
    stat_render_info( "TriggerLogic",   xcolor(255, 0,   255, 255) ),   // k_stats_TriggerSystem
    stat_render_info( "HiLevelRender",  xcolor(64,  64,  64,  255) ),   // k_stats_HighLevelRender
    stat_render_info( "ObjectRender",   xcolor(255, 255, 255, 255) ),   // k_stats_ObjectRender
    stat_render_info( "OtherRender",    xcolor(128, 128, 128, 255) ),   // k_stats_OtherRender
    stat_render_info( "ProjectileLogic",xcolor(255, 128, 0,   255) ),   // k_stats_Projectiles
    stat_render_info( "PortalLogic",    xcolor(255, 0,   0,   255) ),   // k_stats_Portal
    stat_render_info( "TurretLogic",    xcolor(0,   128, 0,   255) ),   // k_stats_Turret
    stat_render_info( "Physics",        xcolor(128, 0,   0,   255) ),   // k_stats_Physics
    stat_render_info( "Player",         xcolor(128, 0,   128, 255) ),   // k_stats_Player
    stat_render_info( "Other",          xcolor(128, 255, 128, 255) ),   // k_stats_CPU_Time
    stat_render_info( "NumTimers",      xcolor(0,   0,   0,   255) ),   // k_stats_NumTimerStats

    stat_render_info( "GS Reset",       xcolor(128, 128, 0,   255) ),   // k_stats_GSReset
    stat_render_info( "GS Settings 0",  xcolor(255, 128, 0,   255) ),   // k_stats_GSSettings0
    stat_render_info( "Shadow Create",  xcolor(0,   255, 0,   255) ),   // k_stats_ShadowMap
    stat_render_info( "3d Objects",     xcolor(128, 64,  0,   255) ),   // k_stats_3dObjects
    stat_render_info( "Special Objs",   xcolor(0,   0,   100, 255) ),   // k_stats_SpecialObjects
    stat_render_info( "Poly Cache",     xcolor(128, 128, 128, 255) ),   // k_stats_PolyCache
    stat_render_info( "2d Objects",     xcolor(100, 100, 255, 255) ),   // k_stats_2dObjects
    stat_render_info( "CollisionMgr",   xcolor(255, 255, 0,   255) ),   // k_stats_CollisionMgr
    stat_render_info( "GS Settings 1",  xcolor(0,   255, 255, 255) ),   // k_stats_GSSettings1
    stat_render_info( "Stats Mgr",      xcolor(255, 0,   255, 255) ),   // k_stats_StatsMgr
    stat_render_info( "UI",             xcolor(255, 128, 0,   255) ),   // k_stats_UI
    stat_render_info( "Debug Text",     xcolor(64,  64,  64,  255) ),   // k_stats_DebugText
    stat_render_info( "Unknown",        xcolor(255, 255, 255, 255) ),   // k_stats_unknown

    stat_render_info( "GS",             xcolor(0,   0,   0,   255) ),   // k_stats_GS
    stat_render_info( "VSync",          xcolor(255, 0,   128, 255) ),   // k_stats_VSync
    stat_render_info( "DList",          xcolor(0,   0,   0,   255) ),   // k_stats_DList
    stat_render_info( "SMem",           xcolor(0,   0,   0,   255) ),   // k_stats_SMem
    stat_render_info( "Memory",         xcolor(0,   0,   0,   255) ),   // k_stats_Memory
    stat_render_info( "VisibleObjects", xcolor(0,   0,   0,   255) ),   // k_stats_VisibleObjectCount
    stat_render_info( "TotalObjects",   xcolor(0,   0,   0,   255) ),   // k_stats_TotalObjectCount
    stat_render_info( "Vertices",       xcolor(0,   0,   0,   255) ),   // k_stats_Vertices
    stat_render_info( "Polys",          xcolor(0,   0,   0,   255) ),   // k_stats_Polygons
    stat_render_info( "TextureMemory",  xcolor(0,   0,   0,   255) ),   // k_stats_TextureMemory
    stat_render_info( "AudioChannels",  xcolor(0,   0,   0,   255) ),   // k_stats_AudioChannel
};

const char* Stat_Measurement_Name[k_stats_Measurement_Last] = 
{
    "Instant",
    "Average",
    "MaxRecent",
    "MinRecent",
    "MaxEver",
    "MinEver"
};


//=============================================================================
// Implentation of the stat_logger class
//=============================================================================

s32 stat_logger::s_StatLevel = 0;
stat_logger* stat_logger::s_StatStack[stat_logger::MAX_STAT_LEVELS];

//=============================================================================

stat_logger::stat_logger( stat_fields fieldToTrack ) :
    m_FieldToTrack(fieldToTrack)
{
    // pause the current hierarchy timer and start up the next one
    ASSERT( s_StatLevel < stat_logger::MAX_STAT_LEVELS );
    if ( s_StatLevel > 0 )
    {
        stats_mgr::GetStatsMgr()->RegisterStat( s_StatStack[s_StatLevel-1]->m_FieldToTrack,
                                                s_StatStack[s_StatLevel-1]->m_Timer.StopMs() );
    }
    
    s_StatStack[s_StatLevel] = this;
    s_StatLevel++;

    m_Timer.Reset();
    m_Timer.Start();
}

//=============================================================================

stat_logger::~stat_logger()
{
    // read out the timer, and restart the next one up in the hierarchy
    stats_mgr::GetStatsMgr()->RegisterStat(m_FieldToTrack, m_Timer.StopMs() );
    ASSERT( s_StatLevel > 0 );
    s_StatLevel--;
    if ( s_StatLevel > 0 )
    {
        s_StatStack[s_StatLevel-1]->m_Timer.Reset();
        s_StatStack[s_StatLevel-1]->m_Timer.Start();
    }
}

//=============================================================================
// implementation of the stats manager
//=============================================================================

stats_mgr::stats_mgr(void) :
    m_FrameCount(0)
{
    // should only ever be created once!
    ASSERT(!s_This);
    s_This = this;
}

//=============================================================================

stats_mgr::~stats_mgr()
{
    s_This = NULL;
}

//=============================================================================

void stats_mgr::Reset (void)
{
    m_FrameCount = 0;
    s32 outerCount, innerCount;
    for(outerCount = 0; outerCount < k_stats_Last; outerCount++)
    {
        for( innerCount = 0; innerCount < k_SAMPLE_HISTORY; innerCount++ )
        {
            m_Stats[outerCount][innerCount] = 0.0f;
        }
    
        m_StatsMin[outerCount]     = 0.0f;
        m_StatsMax[outerCount]     = 0.0f;
        m_StatsAverage[outerCount] = 0.0f;
    }
}

//=============================================================================

void stats_mgr::OnGameUpdate(f32 deltaTime)
{
    CONTEXT( "stats_mgr::OnGameUpdate" );

    (void)deltaTime;

    // update ps2-specific stats
#ifdef TARGET_PS2
    RegisterStat( k_stats_VSync, x_TicksToMs(DLIST.GetVBlankTime()) );
    RegisterStat( k_stats_DList, (f32)ENG_DLIST_MAX_USED);

    // get all of the gpu stats
    s32 DIRet = DI();
    f32 TotalTime = 0.0f;
    for ( s32 iTask = 0; iTask < DLIST.GetPrevNTasks(); iTask++ )
    {
        const char* pTaskName = DLIST.GetPrevTaskName( iTask );
        f32         TaskTime  = x_TicksToMs(DLIST.GetPrevTaskTime( iTask ));

        if ( !x_strcmp( "GSReset", pTaskName ) )                RegisterStat( k_stats_GSReset,        TaskTime );
        else if ( !x_strcmp( "GSSettings0",     pTaskName ) )   RegisterStat( k_stats_GSSettings0,    TaskTime );
        else if ( !x_strcmp( "Shadow Map",      pTaskName ) )   RegisterStat( k_stats_ShadowMap,      TaskTime );
        else if ( !x_strcmp( "3d Objects",      pTaskName ) )   RegisterStat( k_stats_3dObjects,      TaskTime );
        else if ( !x_strcmp( "Special Objects", pTaskName ) )   RegisterStat( k_stats_SpecialObjects, TaskTime );
        else if ( !x_strcmp( "PolyCache",       pTaskName ) )   RegisterStat( k_stats_PolyCache,      TaskTime );
        else if ( !x_strcmp( "2d Objects",      pTaskName ) )   RegisterStat( k_stats_2dObjects,      TaskTime );
        else if ( !x_strcmp( "CollisionMgr",    pTaskName ) )   RegisterStat( k_stats_CollisionMgr,   TaskTime );
        else if ( !x_strcmp( "GSSettings1",     pTaskName ) )   RegisterStat( k_stats_GSSettings1,    TaskTime );
        else if ( !x_strcmp( "StatsMgr",        pTaskName ) )   RegisterStat( k_stats_StatsMgr,       TaskTime );
        else if ( !x_strcmp( "UI",              pTaskName ) )   RegisterStat( k_stats_UI,             TaskTime );
        else if ( !x_strcmp( "Text",            pTaskName ) )   RegisterStat( k_stats_DebugText,      TaskTime );
        else                                                    RegisterStat( k_stats_GPUUnknown,     TaskTime );

        TotalTime += TaskTime;
    }
    if( DIRet )
        EI();

    RegisterStat( k_stats_GS, TotalTime );
#endif

    // update sats shared across all platforms
    s32 nTexture, TextureMemory;
    texture::GetStats( &nTexture, &TextureMemory );
    
    #if ENABLE_RENDER_STATS
    RegisterStat( k_stats_VisibleObjectCount, (f32)render::GetStats().m_nInstancesRendered );
    RegisterStat( k_stats_Vertices,           (f32)render::GetStats().m_nVerticesRendered  );
    RegisterStat( k_stats_Polygons,           (f32)render::GetStats().m_nTrisRendered      );
    #endif

    RegisterStat( k_stats_TextureMemory,      (f32)TextureMemory );
    RegisterStat( k_stats_Memory,             (f32)x_MemGetUsed() );
    RegisterStat( k_stats_SMem,               (f32)smem_GetBufferSize()-smem_GetMaxUsed() );

    s32 nCount = 0;
    for (u32 i = object::TYPE_NULL + 1; i < object::TYPE_END_OF_LIST; i++)
    {
        nCount += g_ObjMgr.GetNumInstances( (object::type)i );
    }
    RegisterStat( k_stats_TotalObjectCount,   (f32)nCount );

    // get set for the next frame of stats
    m_FrameCount++;
    m_FrameCount %= k_SAMPLE_HISTORY;

    s32 count;
    for(count = 0; count < k_stats_Last; count++ )
    {
        m_Stats[count][m_FrameCount] = 0.0f;
    }
}

//=============================================================================
void stats_mgr::DrawBar(    stat_fields thisStat, 
                            f32 X1, f32 Y1, f32 X2, f32 Y2,
                            xcolor BarColor )

{
#ifndef TARGET_PC

    CONTEXT( "stats_mgr::Render" );
    
    //
    //  First we draw the base bar that other bars will be built on.
    //  This bar is drawn to the length above and is assumed to be the base bar
    //  that all others are measured by
    //
    xcolor baseBar(128,128,128, 128 );
    xcolor midBar = xcolor( BarColor.R, MIN(BarColor.G + 84, 255), BarColor.B, 128);
    xcolor highBar = xcolor( MIN(BarColor.R + 84, 255), BarColor.G, BarColor.B, 128);

    vector2 corner1( X1, Y1 );
    vector2 corner2( X2, Y2 );

    f32 maxScale;
    switch(thisStat) 
    {
    default:
    case k_stats_VSync:
        maxScale = 1000.0f/60.0f;
        break;

    case k_stats_DList:
    #ifndef TARGET_XBOX
        maxScale = (f32)DLIST.GetMFIFOSize();
        break;
    #else
        return;
    #endif

    case k_stats_SMem:
    #ifndef TARGET_XBOX
        maxScale = (f32)s_SMem;
        break;
    #else
        return;
    #endif

    case k_stats_VisibleObjectCount:
        maxScale = 500.0f;
        break;
    case k_stats_Vertices:
        maxScale = 100000.0f;
        break;
    case k_stats_Polygons:
        maxScale = 65000.0f;
        break;
    case k_stats_TextureMemory:
        maxScale = 6000000.0f;
        break;
    case k_stats_Memory:
        maxScale = 26.0f*1024.0f*1024.0f;
        break;
    case k_stats_TotalObjectCount:
        maxScale = obj_mgr::MAX_OBJECTS;
        break;
    case k_stats_AudioChannel:
        maxScale = 48.0f;
        break;
    }

    // Draw.
    rect DrawRect;
    DrawRect.Set( corner1, corner2 );
    draw_Rect( DrawRect, baseBar, FALSE );
    
    if(thisStat == k_stats_Vertices ||
       thisStat == k_stats_Polygons   ||
       thisStat == k_stats_TextureMemory   ||
       thisStat == k_stats_SMem   ||
       thisStat == k_stats_DList   ||
       thisStat == k_stats_VisibleObjectCount   ||
       thisStat == k_stats_TotalObjectCount   ||
       thisStat == k_stats_Memory ||
       thisStat == k_stats_AudioChannel)
    {
        vector2 tempCorner1, tempCorner2;

        f32 calculatedScale;
        //////////////////////////////////////////////////////////////////////////
        //  Draw the average first
        //////////////////////////////////////////////////////////////////////////
        
        tempCorner1 = corner1;
        tempCorner2 = corner2;
    
        calculatedScale = ( (maxScale - GetStat(thisStat,k_stats_Average))/maxScale );
        
        tempCorner1.Y += (tempCorner2.Y -tempCorner1.Y) * calculatedScale;
            
        DrawRect.Set( tempCorner1, tempCorner2 );
        draw_Rect( DrawRect, midBar, FALSE );
        
        //////////////////////////////////////////////////////////////////////////
        //  Draw the Instant value
        //////////////////////////////////////////////////////////////////////////
        

        tempCorner1 = corner1;
        tempCorner2 = corner2;
        
        calculatedScale = ( (maxScale - GetStat(thisStat,k_stats_Instant))/maxScale );

        tempCorner1.Y += (tempCorner2.Y -tempCorner1.Y) * calculatedScale;

        DrawRect.Set( tempCorner1, tempCorner2 );
        draw_Rect( DrawRect, highBar, FALSE );        

        //////////////////////////////////////////////////////////////////////////
        //  Max Recent
        //////////////////////////////////////////////////////////////////////////

        tempCorner1 = corner1;
        tempCorner2 = corner2;

        calculatedScale =  ( (maxScale - GetStat(thisStat,k_stats_MaxRecent))/maxScale );

        tempCorner1.Y += (tempCorner2.Y -tempCorner1.Y) *calculatedScale;
        tempCorner2.Y = tempCorner1.Y + 3;
  
        DrawRect.Set( tempCorner1, tempCorner2 );
        draw_Rect( DrawRect, XCOLOR_BLACK, FALSE );      

        //////////////////////////////////////////////////////////////////////////
        //  Min Recent
        //////////////////////////////////////////////////////////////////////////


        tempCorner1 = corner1;
        tempCorner2 = corner2;

        calculatedScale =  ( (maxScale - GetStat(thisStat,k_stats_MinRecent))/maxScale );

        tempCorner1.Y += (tempCorner2.Y -tempCorner1.Y) *calculatedScale;
        tempCorner2.Y = tempCorner1.Y + 3;
  
        DrawRect.Set( tempCorner1, tempCorner2 );
        draw_Rect( DrawRect, XCOLOR_PURPLE, FALSE );      
    }
    
#endif
}

//=============================================================================

void stats_mgr::PrintStats(     stat_fields thisStat, 
                                stat_measurement thisMeasurement, 
                                u32 X1, u32 Y1, 
                                f32 lengthOfTimeToAverage  )
{
 
    f32 value = GetStat( thisStat, thisMeasurement, lengthOfTimeToAverage);
    x_printfxy( X1, Y1, "%s-%s: %4.1f", StatRenderInfo[thisStat].pName,
                                        Stat_Measurement_Name[thisMeasurement],
                                        value );
}

//=============================================================================

void stats_mgr::RegisterStat(   stat_fields thisField,
                                f32 thisValue )
{

    //  update the value at the proper frame slot
    m_Stats[thisField][m_FrameCount%k_SAMPLE_HISTORY] += thisValue;

    //  update min and max values if needed
    if( thisValue > m_StatsMax[thisField] )
        m_StatsMax[thisField] = thisValue;
    if( thisValue < m_StatsMin[thisField] )
        m_StatsMin[thisField] = thisValue;
}

//=============================================================================

f32 stats_mgr::GetStat(    stat_fields thisStat, stat_measurement thisMeasurement, f32 lengthOfTimeToAverage )
{
    f32 value = 0.0f;

    switch( thisMeasurement )
    {   
    case k_stats_Instant:
        {
            value = m_Stats[thisStat][(m_FrameCount+ k_SAMPLE_HISTORY -1)%k_SAMPLE_HISTORY];
        }
        break;

    case k_stats_Average:
        {
            s32 count  ;
            for(count = (s32)(30.0f*lengthOfTimeToAverage); count > 0; count-- )
            {
                s32 index = m_FrameCount - count;
                if(index < 0 )
                {
                    index += k_SAMPLE_HISTORY;
                }
                value += m_Stats[thisStat][index];
            }
            value /= (f32)((s32)(30.0f*lengthOfTimeToAverage));
        }
        break;
        
    case k_stats_MaxRecent:
        {
            s32 count;
            for(count = 0; count < k_SAMPLE_HISTORY; count++ )
            {
                if( m_Stats[thisStat][count] > value )
                {
                    value = m_Stats[thisStat][count];
                }
            }
        }
        break;
        
    case k_stats_MinRecent:
        {
            value   = 999999999.0f;
            s32 count;
            for(count = 0; count < k_SAMPLE_HISTORY; count++ )
            {
                if( m_Stats[thisStat][count] < value && m_Stats[thisStat][count] != 0.0f )
                {
                    value = m_Stats[thisStat][count];
                }
            }
        }
        break;
        
    case k_stats_MaxEver:
        {
            value = m_StatsMax[thisStat];

        }
        break;
        
    case k_stats_MinEver:
        {
            value = m_StatsMin[thisStat];
        }
        break;
    default:
        ASSERT(false);
        
    }

    return value;

}

//=============================================================================

void stats_mgr::DrawFPS(void)
{
    if (!m_bShowHorizontalBars)
        return;
    
#ifndef TARGET_PC   
    // Draw.
    vector2 CPUUpperLeft  ( 20.0f,  10.0f );
    vector2 CPULowerRight ( 480.0f, 16.0f );
    vector2 GSUpperLeft   ( 20.0f,  16.0f );
    vector2 GSLowerRight  ( 480.0f, 22.0f );
    vector2 temp1, temp2;

#ifdef X_DEBUG
    s32 notches = 10;
#else
    s32 notches = 4;
#endif
    f32 totalTime = ((f32)notches )* (1000.0f/60.0f);

    // Denominator
    f32 fSampleTime             = .25f;

    // get the stat timings
    f32 percents[k_stats_NumTimerStats];
    f32 fCPUStatTime = 0.0f;
    s32 i;
    for ( i = 0; i < k_stats_NumTimerStats; i++ )
    {
        percents[i] = GetStat( (stat_fields)i, k_stats_Average, fSampleTime );
        fCPUStatTime += percents[i];
    }

    // remove the vsync time from the render "other" time
    f32 fVSyncTime = GetStat( (stat_fields)k_stats_VSync, k_stats_Average, fSampleTime );
    percents[k_stats_CPU_Time] -= fVSyncTime;

    // convert the timings into percentages
    if ( fCPUStatTime <= 0.0f ) fCPUStatTime = 1.0f;    // this if should never get hit...just here for safety
    for ( i = 0; i < k_stats_NumTimerStats; i++ )
    {
        percents[i] /= fCPUStatTime;
    }

    // what is the total cpu percent of the bar?
    f32 percentTotalCPU = fCPUStatTime / totalTime;

    // get the gpu stat timings
    f32 GPUPercents[k_stats_GPUUnknown-k_stats_GSReset+1];
    f32 fGPUStatTime = 0.0f;
    for ( i = k_stats_GSReset; i <= k_stats_GPUUnknown; i++ )
    {
        GPUPercents[i-k_stats_GSReset]  = GetStat( (stat_fields)i, k_stats_Average, fSampleTime );
        fGPUStatTime                   += GPUPercents[i-k_stats_GSReset];
    }

    // convert the timings into percentages
    if ( fGPUStatTime <= 0.0f ) fGPUStatTime = 1.0f;    // this if should never get hit...just here for safety
    for ( i = k_stats_GSReset; i <= k_stats_GPUUnknown; i++ )
    {
        GPUPercents[i-k_stats_GSReset] /= fGPUStatTime;
    }

    // what is the total gpu percent of the bar?
    f32 percentTotalGS = fGPUStatTime / totalTime;

    //draw base gray bars
    rect DrawRect;
    DrawRect.Set( CPUUpperLeft.X, CPUUpperLeft.Y, GSLowerRight.X, GSLowerRight.Y );
    draw_Rect( DrawRect, xcolor(64,64,64,255), FALSE );

    // draw each of the timer stats
    temp1   = CPUUpperLeft;
    temp2   = CPULowerRight;
    temp2.X = temp1.X;
    for ( i = 0; i < k_stats_NumTimerStats; i++ )
    {
        temp1.X = temp2.X;
        temp2.X = temp2.X + (CPULowerRight.X - CPUUpperLeft.X) * (percentTotalCPU*percents[i]);
        DrawRect.Set( temp1, temp2 );
        draw_Rect( DrawRect, StatRenderInfo[i].BarColor, FALSE );
    }

    // draw the vsync time
    temp1.X = temp2.X;
    temp2.X = temp2.X + (CPULowerRight.X - CPUUpperLeft.X) * (percentTotalCPU*fVSyncTime/fCPUStatTime);
    DrawRect.Set( temp1, temp2 );
    draw_Rect( DrawRect, StatRenderInfo[k_stats_VSync].BarColor, FALSE );

    // draw each of the GPU time stats
    temp1   = GSUpperLeft;
    temp2   = GSLowerRight;
    temp2.X = temp1.X;
    for ( i = k_stats_GSReset; i <= k_stats_GPUUnknown; i++ )
    {
        temp1.X = temp2.X;
        temp2.X = temp2.X + (GSLowerRight.X - GSUpperLeft.X) * (percentTotalGS*GPUPercents[i-k_stats_GSReset]);
        DrawRect.Set( temp1, temp2 );
        draw_Rect( DrawRect, StatRenderInfo[i].BarColor, FALSE );
    }

/*
    //  draw the GS bar on it
    temp1   = GSUpperLeft;
    temp2   = GSLowerRight;
    temp2.X = GSUpperLeft.X + (GSLowerRight.X - GSUpperLeft.X) * percentTotalGS;
    DrawRect.Set( temp1, temp2 );
    draw_Rect( DrawRect, xcolor(180,180,180,255), FALSE );
    */

    // draw the notches across
    s32 count;
    for( count =0; count < notches; count++ )
    {
        temp1 = CPUUpperLeft;
        temp2 = GSLowerRight;
        temp1.X = CPUUpperLeft.X + ((CPULowerRight.X - CPUUpperLeft.X)*count)/(notches);
        temp1.X -=1;
        temp2.X = temp1.X +2;
        DrawRect.Set( temp1, temp2 );
        draw_Rect( DrawRect, xcolor(0,0,0,255), FALSE );
    }

    if(  m_bShowNumbers )
    {
        PrintStats( k_stats_Portal,k_stats_Average,         1,  1  );
        PrintStats( k_stats_CPU_Time,k_stats_Average,       1,  2  );
        PrintStats( k_stats_Animation,k_stats_Average,      1,  3  );
        PrintStats( k_stats_TriggerSystem,k_stats_Average,  1,  4  );
        PrintStats( k_stats_OtherRender,k_stats_Average,    1,  5  );
        PrintStats( k_stats_ObjectRender,k_stats_Average,   1,  6  );
        PrintStats( k_stats_Projectiles,k_stats_Average,    1,  7  );
        PrintStats( k_stats_DecalSystem,k_stats_Average,    1,  8  );
        PrintStats( k_stats_Portal,k_stats_Average,         1,  9  );
        PrintStats( k_stats_Turret,k_stats_Average,         1,  10 );
    }
#endif
}

//=============================================================================

void stats_mgr::DrawCPULegend( void )
{
    if ( !m_bShowCPUTimeLegend )
        return;

#ifdef TARGET_PS2
    static const s32 kCharHeight = 18;
    irect DrawRect(188, 80, 214, 80+kCharHeight);

    s32 YOffset = 4;
    for ( s32 i = 0; i < k_stats_NumTimerStats; i++ )
    {
        x_printfxy( 16, YOffset++, StatRenderInfo[i].pName );
        draw_Rect( DrawRect, StatRenderInfo[i].BarColor, FALSE );
        DrawRect.t += kCharHeight;
        DrawRect.b += kCharHeight;
    }

    x_printfxy( 16, YOffset++, StatRenderInfo[k_stats_VSync].pName );
    draw_Rect( DrawRect, StatRenderInfo[k_stats_VSync].BarColor, FALSE );
    DrawRect.t += kCharHeight;
    DrawRect.b += kCharHeight;
#endif
}

//=============================================================================

void stats_mgr::DrawGPULegend( void )
{
    if ( !m_bShowGPUTimeLegend )
        return;

#ifdef TARGET_PS2
    static const s32 kCharHeight = 18;
    irect DrawRect(188, 80, 214, 80+kCharHeight);

    s32 YOffset = 4;
    for ( s32 i = k_stats_GSReset; i <= k_stats_GPUUnknown; i++ )
    {
        x_printfxy( 16, YOffset++, StatRenderInfo[i].pName );
        draw_Rect( DrawRect, StatRenderInfo[i].BarColor, FALSE );
        DrawRect.t += kCharHeight;
        DrawRect.b += kCharHeight;
    }
#endif
}

//=============================================================================

void stats_mgr::DrawSmallBars( void )
{
#ifdef TARGET_PS2
    static const s32 kCharWidth = 13;
    irect DrawRect( 8+kCharWidth, 240, 8+kCharWidth+kCharWidth-1, 360 );
    
    stats_mgr::GetStatsMgr()->DrawBar(k_stats_Vertices,           DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(64, 64, 128) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_Polygons,           DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(64, 64, 128) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_Memory,             DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(64, 128, 64) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_DList,              DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(128, 128, 64) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_SMem,               DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(128, 128, 64) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_TextureMemory,      DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(64, 128, 64) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_VisibleObjectCount, DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(128, 128, 192) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_TotalObjectCount,   DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(128, 128, 192) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;

    stats_mgr::GetStatsMgr()->DrawBar(k_stats_AudioChannel,       DrawRect.l,DrawRect.t, DrawRect.r, DrawRect.b, xcolor(64, 128, 192) );
    DrawRect.l += kCharWidth;
    DrawRect.r += kCharWidth;
#endif
}

//=============================================================================

void stats_mgr::DrawSmallBarLegend( void )
{
    if ( !m_bShowVerticalLegend )
        return;

    x_printfxy( 1, 20, "1" );
    x_printfxy( 2, 20, "2" );
    x_printfxy( 3, 20, "3" );
    x_printfxy( 4, 20, "4" );
    x_printfxy( 5, 20, "5" );
    x_printfxy( 6, 20, "6" );
    x_printfxy( 7, 20, "7" );
    x_printfxy( 8, 20, "8" );
    x_printfxy( 9, 20, "9" );

    x_printfxy( 10, 4,  "1. Vertices" );
    x_printfxy( 10, 5,  "2. Polygons" );
    x_printfxy( 10, 6,  "3. Memory" );
    x_printfxy( 10, 7,  "4. DList" );
    x_printfxy( 10, 8,  "5. SMem" );
    x_printfxy( 10, 9,  "6. Texture Memory" );
    x_printfxy( 10, 10, "7. Visible Objects" );
    x_printfxy( 10, 11, "8. Total Objects" );
    x_printfxy( 10, 12, "9. Active Sound Channels" );
}

//=============================================================================

#endif //ENABLE_STATS_MGR
