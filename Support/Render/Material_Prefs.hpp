#ifndef MATERIAL_PREFS_HPP
#define MATERIAL_PREFS_HPP
    
enum material_type
{
    Material_Not_Used,

    Material_Diff,
    Material_Alpha,
    Material_Diff_PerPixelEnv,
    Material_Diff_PerPixelIllum,
    Material_Alpha_PerPolyEnv,
    Material_Alpha_PerPixelIllum,
    Material_Alpha_PerPolyIllum,
    Material_Distortion,
    Material_Distortion_PerPolyEnv,

    Material_NumTypes,
};

#endif

