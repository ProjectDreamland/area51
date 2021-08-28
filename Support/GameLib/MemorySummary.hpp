#ifndef MEMORYSUMMARY_HPP
#define MEMORYSUMMARY_HPP

struct mem_resource_summary
{
    s32     TotalTextures;
    s32     LevelTextures;
    s32     EffectTextures;
    s32     OtherTextures;
    s32     TotalGeoms;
    s32     RigidGeoms;
    s32     SkinGeoms;
    s32     RigidColors;
    s32     Decals;
    s32     Animations;
};

struct mem_playsurface_summary
{
    s32     Total;
    s32     Geometry;
    s32     ColorData;
    s32     SpatialDatabase;
    s32     Other;
};

struct mem_object_summary
{
    s32     Total;
    s32     Triggers;
    s32     SuperDestructibles;
    s32     DeadBodies;
    s32     AnimSurfaces;
    s32     SoundEmitters;
    s32     NPCS;
};

struct mem_level_summary
{
    s32     Zones;
    s32     Templates;
    s32     Strings;
    s32     Decals;
    s32     NavMaps;
    s32     EffectDefinitions;
    s32     EffectInstances;
    s32     Dictionary;
    s32     AnimationCache;
    s32     AnimData;
    s32     SpatialDatabase;
    s32     AudioPackageData;
    s32     GlobalVariables;
};

struct mem_base_summary
{
    s32     Total;
    s32     Constructor;
    s32     Startup;
    s32     LoadLevel;
    s32     CodeSize;
    s32     BSSSize;
    s32     HeapSize;
    s32     StackSize;
    s32     Free;
};

struct mem_summary
{
    mem_base_summary        Base;
    mem_resource_summary    Resources;
    mem_playsurface_summary PlaySurfaces;
    mem_object_summary      Objects;
    mem_level_summary       Level;
};

void A51_MemorySummary( void );

#endif // MEMORYSUMMARY_HPP
