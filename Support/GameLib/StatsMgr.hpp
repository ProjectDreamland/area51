///////////////////////////////////////////////////////////////////////////////
//
//  stats_mgr
//
//      Keeps up with all performance stats for the game
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(X_RETAIL) || defined(CONFIG_PROFILE)
#define ENABLE_STATS_MGR 1
#else
#define ENABLE_STATS_MGR 0
#endif

///////////////////////////////////////////////////////////////////////////////

#if ENABLE_STATS_MGR

#ifndef STATS_MGR
#define STATS_MGR

#define ENABLE_LOGGING

#include "Obj_Mgr\Obj_Mgr.hpp"

const s32   k_SAMPLE_HISTORY = 120;

enum stat_fields
{
    //  Timer stats
    k_stats_OnAdvance = 0      ,
    k_stats_Debris             ,
    k_stats_AI_Think           ,
    k_stats_AI_Advance         ,
	k_stats_Animation          ,
    k_stats_Collision          ,
    k_stats_Sound              ,
    k_stats_ParticleSystem     ,
    k_stats_DecalSystem        ,
    k_stats_TriggerSystem      ,
    k_stats_HighLevelRender    ,
    k_stats_ObjectRender       ,
    k_stats_OtherRender        ,
    k_stats_Projectiles        ,
    k_stats_Portal             ,
    k_stats_Turret             ,
    k_stats_Physics            ,
    k_stats_Player             ,
    k_stats_CPU_Time           ,
    k_stats_NumTimerStats      ,

    // Non logging GPU stats
    k_stats_GSReset,
    k_stats_GSSettings0,
    k_stats_ShadowMap,
    k_stats_3dObjects,
    k_stats_SpecialObjects,
    k_stats_PolyCache,
    k_stats_2dObjects,
    k_stats_CollisionMgr,
    k_stats_GSSettings1,
    k_stats_StatsMgr,
    k_stats_UI,
    k_stats_DebugText,
    k_stats_GPUUnknown,

    //  Non logging game stats
    k_stats_GS                 ,
    k_stats_VSync              ,
    k_stats_DList              ,
    k_stats_SMem               ,
    k_stats_Memory             ,
    k_stats_VisibleObjectCount ,
    k_stats_TotalObjectCount   ,
    k_stats_Vertices           ,
    k_stats_Polygons           ,
    k_stats_TextureMemory      ,
    k_stats_AudioChannel       ,
    
    k_stats_Last,
};

enum stat_measurement
{
    k_stats_Instant = 0,
    k_stats_Average,
    k_stats_MaxRecent,
    k_stats_MinRecent,
    k_stats_MaxEver,
    k_stats_MinEver,
    k_stats_Measurement_Last,
    k_stats_All
};

#ifdef ENABLE_LOGGING
   
#define LOG_STAT( x )  stat_logger TempStatLoggerVar(x)

#else

#define LOG_STAT( x )  void

#endif

#define STAT_LOGGER(_a_,_b_) stat_logger _a_(_b_)

class stat_logger
{
public:
    enum { MAX_STAT_LEVELS = 8 };
    stat_logger( stat_fields fieldToTrack );
    ~stat_logger();
protected:    
    xtimer        m_Timer;
    stat_fields   m_FieldToTrack;

    static  s32          s_StatLevel;
    static  stat_logger* s_StatStack[MAX_STAT_LEVELS];
};

struct warning_holder
{
    stat_fields m_StatField;
    f32         m_WarningLevel;
    u32         X1,
                Y1;

};


class stats_mgr
{
public:
                        stats_mgr(void);
                       ~stats_mgr();

    static stats_mgr*   GetStatsMgr(void) { if(!s_This) s_This = new stats_mgr; return s_This; }

    void                Reset (void);
    void                OnGameUpdate( f32 deltaTime );


    void                DrawBar(    stat_fields thisStat, 
                                    f32 X1, f32 Y1, f32 X2, f32 Y2,
                                    xcolor BarColor );
    void                PrintStats( stat_fields thisStat, 
                                    stat_measurement thisMeasurement, 
                                    u32 X1, u32 Y1, 
                                    f32 lengthOfTimeToAverage = 1.0f );

    void                RegisterStat(   stat_fields thisField,
                                        f32 thisValue );

    f32                 GetStat(    stat_fields thisStat, 
                                    stat_measurement thisMeasurement,
                                    f32 lengthOfTimeToAverage = 1.0f );

    
    void                DrawFPS( void );
    void                DrawSmallBars( void );
    void                DrawCPULegend( void );
    void                DrawGPULegend( void );
    void                DrawSmallBarLegend( void );

    void                AddWarning( stat_fields StatField, f32 WarningLevel, u32 X1, u32 Y1 );

    void                DisableAllWarnings( void );
    void                EnableAllWarning  ( void );

    static xbool        m_bShowNumbers;
    static xbool        m_bShowHorizontalBars;

    static xbool        m_bShowCPUTimeLegend;
    static xbool        m_bShowGPUTimeLegend;
    static xbool        m_bShowVerticalLegend;


protected:
    static stats_mgr*   s_This;

    s32                 m_FrameCount;
    
    f32                 m_Stats[k_stats_Last][k_SAMPLE_HISTORY];
    
    f32                 m_StatsMin[k_stats_Last];
    f32                 m_StatsMax[k_stats_Last];
    f32                 m_StatsAverage[k_stats_Last];

    xbool               m_WarningsEnabled;

    xarray<warning_holder> m_Warnings;
};

#endif// STATS_MGR

#else //ENABLE_STATS_MGR

// Stub out the stat_mgr macros to make it all go away in the code
#define LOG_STAT(x)
#define STAT_LOGGER(_a_,_b_)

#endif //ENABLE_STATS_MGR
