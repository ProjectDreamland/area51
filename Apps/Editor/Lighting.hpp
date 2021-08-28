#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "Obj_Mgr\Obj_Mgr.hpp"

enum
{
    LIGHTING_WHITE,
    LIGHTING_DIRECTIONAL,
    LIGHTING_DYNAMIC,
    LIGHTING_DISTANCE,
    LIGHTING_RAYCAST,
    LIGHTING_ZONE,
};

void lighting_Initialize                ( void );
void lighting_ExportTo3DMax             ( const xarray<guid>& lGuid, const char* pFileName );

void lighting_LightObject               ( platform            Platform,
                                          guid                Guid,
                                          const matrix4&      L2W,
                                          s32                 Mode );

void lighting_LightObjects              ( platform            Platform,
                                          const xarray<guid>& lGuid,
                                          s32                 Mode );

void lighting_CreateColorTable          ( platform            Platform,
                                          const xarray<guid>& lGuid,
                                          const char*         pFileName );

void lighting_CreatePlaySurfaceColors   ( platform            Platform,
                                          const xarray<guid>& lGuid );

void lighting_KillPlaySurfaceColors     ( platform            Platform,
                                          const xarray<guid>& lGuid );

#endif
