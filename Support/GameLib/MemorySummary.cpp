#include "x_types.hpp"
#include "x_memory.hpp"
#include "x_files.hpp"
#include "MemorySummary.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

mem_summary a51_mem_summary;

void A51_MemReport( const char* pFileName )
{
    (void)pFileName;

#if defined(USE_OWNER_STACK) && !defined(TARGET_XBOX) && !defined(TARGET_PC)

    // ====== MEMORY ======

    extern void* __heap_start;
    extern void* __heap_end;
    extern byte _stack_size[];
    extern byte __text_objend[];

    // total allocations.
    x_MemQueryIncludeAll();
    a51_mem_summary.Base.Total = x_MemQuery( TRUE );

    x_MemQueryExcludeAll();
    x_MemQueryInclude("STARTUP");
    x_MemQueryInclude("RunFrontEnd");
    x_MemQueryRequire("DYNAMIC");
    a51_mem_summary.Base.Startup = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("LOADLEVEL");
    a51_mem_summary.Base.LoadLevel = x_MemQuery();

    // constructor allocations.
    x_MemQueryIncludeAll();
    x_MemQueryExclude("DYNAMIC");
    a51_mem_summary.Base.Constructor = x_MemQuery( TRUE );

    a51_mem_summary.Base.CodeSize  = (u32)__text_objend;
    a51_mem_summary.Base.BSSSize   = (u32)__heap_start-(u32)__text_objend;
    a51_mem_summary.Base.HeapSize  = (u32)__heap_end - (u32)__heap_start;
    a51_mem_summary.Base.StackSize = (u32)_stack_size;
    a51_mem_summary.Base.Free      = x_MemGetFree();

    // ====== RESOURCES ======

    // total textures.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("TEXTURE DATA");
    x_MemQueryInclude("XBMP DATA");
    a51_mem_summary.Resources.TotalTextures = x_MemQuery();

    // effects textures.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("TEXTURE DATA");
    x_MemQueryInclude("XBMP DATA");
    x_MemQueryRequire("EFFECT DATA");
    a51_mem_summary.Resources.EffectTextures = x_MemQuery();

    // Now lets find the level textures.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("TEXTURE DATA");
    x_MemQueryInclude("XBMP DATA");
    x_MemQueryRequire("LOADLEVEL");
    x_MemQueryExclude("EFFECT DATA");
    a51_mem_summary.Resources.LevelTextures = x_MemQuery();

    // other textures (hud, front end, etc...).
    a51_mem_summary.Resources.OtherTextures = a51_mem_summary.Resources.TotalTextures  - 
        a51_mem_summary.Resources.EffectTextures - 
        a51_mem_summary.Resources.LevelTextures;

    // skinned geometry.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("SKIN GEOM");
    a51_mem_summary.Resources.SkinGeoms = x_MemQuery();

    // rigid geometry.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("RIGID GEOM");
    a51_mem_summary.Resources.RigidGeoms = x_MemQuery();

    // total geoms.
    a51_mem_summary.Resources.TotalGeoms = a51_mem_summary.Resources.SkinGeoms +
        a51_mem_summary.Resources.RigidGeoms;

    // rigid color.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("RIGID COLOR");
    x_MemQueryRequire("LOADLEVEL");
    x_MemQueryRequire("PRELOAD");
    a51_mem_summary.Resources.RigidColors = x_MemQuery();

    // decal data
    x_MemQueryExcludeAll();
    x_MemQueryInclude("DECAL DATA");
    x_MemQueryRequire("LOADLEVEL");
    x_MemQueryRequire("PRELOAD");
    a51_mem_summary.Resources.Decals = x_MemQuery();

    // animations.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("ANIMATION DATA");
    x_MemQueryRequire("LOADLEVEL");
    x_MemQueryRequire("PRELOAD");
    a51_mem_summary.Resources.Animations = x_MemQuery();

    // ====== PLAYSURFACES ======

    // total.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("PLAYSURFACE DATA");
    a51_mem_summary.PlaySurfaces.Total = x_MemQuery();

    // playsurfaces.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("PLAYSURFACES");
    x_MemQueryRequire("PLAYSURFACE DATA");
    a51_mem_summary.PlaySurfaces.Geometry = x_MemQuery();

    // color data.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("COLOR DATA");
    x_MemQueryRequire("PLAYSURFACE DATA");
    a51_mem_summary.PlaySurfaces.ColorData = x_MemQuery();

    // spatial database.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("SPATIAL DBASE");
    x_MemQueryRequire("PLAYSURFACE DATA");
    a51_mem_summary.PlaySurfaces.SpatialDatabase = x_MemQuery();

    // other.
    a51_mem_summary.PlaySurfaces.Other = a51_mem_summary.PlaySurfaces.Total - 
        a51_mem_summary.PlaySurfaces.Geometry -
        a51_mem_summary.PlaySurfaces.ColorData -
        a51_mem_summary.PlaySurfaces.SpatialDatabase;

    // ====== OBJECTS ======

    // all objects.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("OBJECT DATA");
    x_MemQueryExclude("DICTIONARY");
    x_MemQueryExclude("AUDIO PACKAGE DATA");
    x_MemQueryExclude("EFFECT INSTANCE");
    a51_mem_summary.Objects.Total = x_MemQuery();

    // triggers.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("OBJECT DATA");
    x_MemQueryRequire("TriggerEx");
    x_MemQueryExclude("DICTIONARY");
    a51_mem_summary.Objects.Triggers = x_MemQuery();

    // super destructibles.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("OBJECT DATA");
    x_MemQueryRequire("Super Destructible");
    x_MemQueryExclude("DICTIONARY");
    a51_mem_summary.Objects.SuperDestructibles = x_MemQuery();

    // dead bodies.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("OBJECT DATA");
    x_MemQueryRequire("Dead Body");
    x_MemQueryExclude("DICTIONARY");
    a51_mem_summary.Objects.DeadBodies = x_MemQuery();

    // animated surfaces.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("OBJECT DATA");
    x_MemQueryRequire("Anim Surface");
    x_MemQueryExclude("DICTIONARY");
    a51_mem_summary.Objects.AnimSurfaces = x_MemQuery();

    // sound emitters.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("Sound Emitter"); // <== 2 types of sound emitters! do it this way!
    x_MemQueryExclude("AUDIO PACKAGE DATA");
    x_MemQueryExclude("DICTIONARY");
    a51_mem_summary.Objects.SoundEmitters = x_MemQuery();

    // NPCS.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("NPC");
    x_MemQueryInclude("Friendly Scientist");
    x_MemQueryExclude("DICTIONARY");
    x_MemQueryExclude("ANIM DATA");
    a51_mem_summary.Objects.NPCS = x_MemQuery();

    // ====== LEVEL DATA ======

    // zones.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("ZONE DATA");
    a51_mem_summary.Level.Zones = x_MemQuery();

    // templates.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("TEMPLATE DATA");
    a51_mem_summary.Level.Templates = x_MemQuery();

    // string data.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("STRING DATA");
    a51_mem_summary.Level.Strings = x_MemQuery();

    // decals.
    x_MemQueryExcludeAll();
    x_MemQueryInclude("DECAL DATA");
    x_MemQueryInclude("DECAL VERTLIST");
    x_MemQueryExclude("PRELOAD");
    a51_mem_summary.Level.Decals = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("NAVMAP DATA");
    a51_mem_summary.Level.NavMaps = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("EFFECT DEFINITION");
    x_MemQueryInclude("EFFECT NAME");
    a51_mem_summary.Level.EffectDefinitions = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("EFFECT INSTANCE");
    a51_mem_summary.Level.EffectInstances = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("DICTIONARY");
    a51_mem_summary.Level.Dictionary = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("ANIMATION CACHE");
    a51_mem_summary.Level.AnimationCache = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("ANIM DATA");
    a51_mem_summary.Level.AnimData = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("SPATIAL DBASE");
    x_MemQueryExclude("PLAYSURFACE DATA");
    a51_mem_summary.Level.SpatialDatabase = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("AUDIO PACKAGE DATA");
    a51_mem_summary.Level.AudioPackageData = x_MemQuery();

    x_MemQueryExcludeAll();
    x_MemQueryInclude("GLOBAL VARIABLE DATA");
    a51_mem_summary.Level.GlobalVariables = x_MemQuery();

    X_FILE* f = x_fopen( pFileName, "w" );
    if( f )
    {
        s32 BreathingRoom = 768*1024;
        s32 AnimCacheSize = 300*1024;
        s32 EstimatedFree = a51_mem_summary.Base.Free
                            - ( AnimCacheSize - a51_mem_summary.Level.AnimationCache ) 
                            - BreathingRoom;

        s32 TotalArt = 0;
        TotalArt += a51_mem_summary.Resources.TotalTextures;
        TotalArt += a51_mem_summary.Resources.SkinGeoms;
        TotalArt += a51_mem_summary.Resources.RigidGeoms;
        TotalArt += a51_mem_summary.Resources.Animations;
        TotalArt += a51_mem_summary.PlaySurfaces.Total;
        TotalArt += a51_mem_summary.Resources.Decals;
        TotalArt += a51_mem_summary.Level.EffectDefinitions;
        TotalArt += a51_mem_summary.Level.EffectInstances;
        TotalArt += a51_mem_summary.Resources.RigidColors;

        s32 MiscLevelData = 0;
        //MiscLevelData += a51_mem_summary.Level.AnimationCache;
        //MiscLevelData += a51_mem_summary.Level.AnimData;
        MiscLevelData += a51_mem_summary.Level.AudioPackageData;
        MiscLevelData += a51_mem_summary.Level.Decals;
        MiscLevelData += a51_mem_summary.Level.Dictionary;
        //MiscLevelData += a51_mem_summary.Level.EffectDefinitions;
        //MiscLevelData += a51_mem_summary.Level.EffectInstances;
        MiscLevelData += a51_mem_summary.Level.GlobalVariables;
        MiscLevelData += a51_mem_summary.Level.NavMaps;
        MiscLevelData += a51_mem_summary.Level.SpatialDatabase;
        MiscLevelData += a51_mem_summary.Level.Strings;
        MiscLevelData += a51_mem_summary.Level.Templates;
        MiscLevelData += a51_mem_summary.Level.Zones;

        x_fprintf( f, "%32s, %d\n", "Estimated Free",        EstimatedFree );
        x_fprintf( f, "%32s, %d\n", "Total Art",             TotalArt );
        x_fprintf( f, "%32s, %d\n", "Total Objects",         a51_mem_summary.Objects.Total );
        x_fprintf( f, "%32s, %d\n", "Total Buffered",        BreathingRoom );
        x_fprintf( f, "%32s, %d\n", "Total Misc Level Data", MiscLevelData );
        x_fprintf( f, "%32s, %d\n", "Total Engine Allocs",   a51_mem_summary.Base.Constructor+a51_mem_summary.Base.Startup );
        
        x_fprintf( f, "\nArt Summary\n" );
        x_fprintf( f, "%32s, %d\n", "Textures",             a51_mem_summary.Resources.TotalTextures );  
        x_fprintf( f, "%32s, %d\n", "Skin Geoms",           a51_mem_summary.Resources.SkinGeoms);       
        x_fprintf( f, "%32s, %d\n", "Rigid Geoms",          a51_mem_summary.Resources.RigidGeoms);      
        x_fprintf( f, "%32s, %d\n", "Animation Data",       a51_mem_summary.Resources.Animations);      
        x_fprintf( f, "%32s, %d\n", "Play Surface Total",   a51_mem_summary.PlaySurfaces.Total );       
        x_fprintf( f, "%32s, %d\n", "Play Surface Color",   a51_mem_summary.PlaySurfaces.ColorData );       
        x_fprintf( f, "%32s, %d\n", "Rigid Color",          a51_mem_summary.Resources.RigidColors);
        x_fprintf( f, "%32s, %d\n", "Decals",               a51_mem_summary.Resources.Decals);          
        x_fprintf( f, "%32s, %d\n", "Effect Definitions",   a51_mem_summary.Level.EffectDefinitions );  
        x_fprintf( f, "%32s, %d\n", "Effect Instances",     a51_mem_summary.Level.EffectInstances );    

        x_fprintf( f, "\nObject Summary\n" );
        s32 ObjectAccountedFor = 0;
        x_fprintf( f, "%32s, %d\n", "Dead Bodies",          a51_mem_summary.Objects.DeadBodies );           ObjectAccountedFor += a51_mem_summary.Objects.DeadBodies;
        x_fprintf( f, "%32s, %d\n", "Super Destructibles",  a51_mem_summary.Objects.SuperDestructibles );   ObjectAccountedFor += a51_mem_summary.Objects.SuperDestructibles;
        x_fprintf( f, "%32s, %d\n", "Triggers",             a51_mem_summary.Objects.Triggers );             ObjectAccountedFor += a51_mem_summary.Objects.Triggers;
        x_fprintf( f, "%32s, %d\n", "NPCS",                 a51_mem_summary.Objects.NPCS );                 ObjectAccountedFor += a51_mem_summary.Objects.NPCS;
        x_fprintf( f, "%32s, %d\n", "Objects Unlisted",     a51_mem_summary.Objects.Total - ObjectAccountedFor );

/*
        x_fprintf( f, "\nResource Summary\n" );
        x_fprintf( f, "%32s, %d\n", "Total Textures",       a51_mem_summary.Resources.TotalTextures );
        x_fprintf( f, "%32s, %d\n", "Level Textures",       a51_mem_summary.Resources.LevelTextures );
        //x_fprintf( f, "%32s, %d\n", "Effect Textures",      a51_mem_summary.Resources.EffectTextures );
        x_fprintf( f, "%32s, %d\n", "Other Textures",       a51_mem_summary.Resources.OtherTextures );
        x_fprintf( f, "%32s, %d\n", "Total Geoms",          a51_mem_summary.Resources.TotalGeoms);
        x_fprintf( f, "%32s, %d\n", "Skin Geoms",           a51_mem_summary.Resources.SkinGeoms);
        x_fprintf( f, "%32s, %d\n", "Rigid Geoms",          a51_mem_summary.Resources.RigidGeoms);
        x_fprintf( f, "%32s, %d\n", "Rigid Color",          a51_mem_summary.Resources.RigidColors);
        x_fprintf( f, "%32s, %d\n", "Animations",           a51_mem_summary.Resources.Animations);
        x_fprintf( f, "%32s, %d\n", "Decals",               a51_mem_summary.Resources.Decals);
*/
        x_fprintf( f, "\nMisc Level Data Summary\n" );
        //x_fprintf( f, "%32s, %d\n", "Animation Cache",      a51_mem_summary.Level.AnimationCache );
        //x_fprintf( f, "%32s, %d\n", "Anim Data",            a51_mem_summary.Level.AnimData );
        x_fprintf( f, "%32s, %d\n", "Audio Package Data",   a51_mem_summary.Level.AudioPackageData );
        x_fprintf( f, "%32s, %d\n", "Decals",               a51_mem_summary.Level.Decals );
        x_fprintf( f, "%32s, %d\n", "Dictionary",           a51_mem_summary.Level.Dictionary );
        //x_fprintf( f, "%32s, %d\n", "Effect Definitions",   a51_mem_summary.Level.EffectDefinitions );
        //x_fprintf( f, "%32s, %d\n", "Effect Instances",     a51_mem_summary.Level.EffectInstances );
        x_fprintf( f, "%32s, %d\n", "Global Variables",     a51_mem_summary.Level.GlobalVariables );
        x_fprintf( f, "%32s, %d\n", "Nav Maps",             a51_mem_summary.Level.NavMaps );
        x_fprintf( f, "%32s, %d\n", "Spatial Database",     a51_mem_summary.Level.SpatialDatabase );
        x_fprintf( f, "%32s, %d\n", "Strings",              a51_mem_summary.Level.Strings );
        x_fprintf( f, "%32s, %d\n", "Templates",            a51_mem_summary.Level.Templates );
        x_fprintf( f, "%32s, %d\n", "Zones",                a51_mem_summary.Level.Zones );

        x_fprintf( f, "\nPlay Surface Summary\n" );
        x_fprintf( f, "%32s, %d\n", "Total Play Surface",   a51_mem_summary.PlaySurfaces.Total );
        x_fprintf( f, "%32s, %d\n", "Geometry",             a51_mem_summary.PlaySurfaces.Geometry );
        x_fprintf( f, "%32s, %d\n", "ColorData",            a51_mem_summary.PlaySurfaces.ColorData );
        x_fprintf( f, "%32s, %d\n", "SpatialDatabase",      a51_mem_summary.PlaySurfaces.SpatialDatabase );
        x_fprintf( f, "%32s, %d\n", "Other",                a51_mem_summary.PlaySurfaces.Other );

/*
        x_fprintf( f, "\nObject Summary\n" );
        x_fprintf( f, "%32s, %d\n", "Total",                a51_mem_summary.Objects.Total );
        x_fprintf( f, "%32s, %d\n", "Dead Bodies",          a51_mem_summary.Objects.DeadBodies );
        x_fprintf( f, "%32s, %d\n", "Super Destructibles",  a51_mem_summary.Objects.SuperDestructibles );
        x_fprintf( f, "%32s, %d\n", "Triggers",             a51_mem_summary.Objects.Triggers );
        x_fprintf( f, "%32s, %d\n", "NPCS",                 a51_mem_summary.Objects.NPCS );
        x_fprintf( f, "%32s, %d\n", "Sound Emitters",       a51_mem_summary.Objects.SoundEmitters );
        x_fprintf( f, "%32s, %d\n", "Anim Surfaces",        a51_mem_summary.Objects.AnimSurfaces );
*/
        x_fprintf( f, "\nRAM Summary\n" );
        //x_fprintf( f, "%32s, %d\n", "Total Allocs",         a51_mem_summary.Base.Total );
        //x_fprintf( f, "%32s, %d\n", "Constructor Allocs",   a51_mem_summary.Base.Constructor );
        //x_fprintf( f, "%32s, %d\n", "Startup Allocs",       a51_mem_summary.Base.Startup );
        //x_fprintf( f, "%32s, %d\n", "LoadLevel Allocs",     a51_mem_summary.Base.LoadLevel );
        x_fprintf( f, "%32s, %d\n", "Code Size",            a51_mem_summary.Base.CodeSize );
        x_fprintf( f, "%32s, %d\n", "BSS Size",             a51_mem_summary.Base.BSSSize );
        x_fprintf( f, "%32s, %d\n", "Heap Size",            a51_mem_summary.Base.HeapSize );
        x_fprintf( f, "%32s, %d\n", "Stack Size",           a51_mem_summary.Base.StackSize );
        x_fprintf( f, "%32s, %d\n", "Free RAM",             a51_mem_summary.Base.Free );

        x_fclose( f );
    }

#endif // USE_OWNER_STACK
}

void A51_MemorySummary( void )
{
#ifdef RSC_MGR_COLLECT_STATS
    g_RscMgr.DumpStatsToFile( "c:\\ResourceSummary.csv" );
    x_MemDump( "c:\\MemoryDump.csv", TRUE );
    A51_MemReport("c:\\MemorySummary.csv");
#endif
}

