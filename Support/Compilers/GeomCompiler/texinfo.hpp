//=========================================================================
//
//  TEXINFO.HPP
//
//=========================================================================
#ifndef TEXINFO_HPP
#define TEXINFO_HPP

#include "x_files.hpp"

//
// Sound material properties.
//

enum
{
    MAT_TYPE_NULL,
    MAT_TYPE_EARTH,
    MAT_TYPE_ROCK,
    MAT_TYPE_CONCRETE,
    MAT_TYPE_SOLID_METAL,
    MAT_TYPE_HOLLOW_METAL,    
    MAT_TYPE_METAL_GRATE,
    MAT_TYPE_PLASTIC,
    MAT_TYPE_WATER,
    MAT_TYPE_WOOD,
    MAT_TYPE_ENERGY_FIELD,
    MAT_TYPE_BULLET_PROOF_GLASS,
    MAT_TYPE_ICE,

    MAT_TYPE_LEATHER,
    MAT_TYPE_EXOSKELETON,
    MAT_TYPE_FLESH,
    MAT_TYPE_BLOB,

    MAT_TYPE_FIRE,
    MAT_TYPE_GHOST,
    MAT_TYPE_FABRIC,
    MAT_TYPE_CERAMIC,
    MAT_TYPE_WIRE_FENCE,
    MAT_TYPE_GLASS,
    MAT_TYPE_RUBBER,

    MAT_TYPE_CARPET,
    MAT_TYPE_CLOTH,
    MAT_TYPE_DRYWALL,
    MAT_TYPE_FLESHHEAD,
    MAT_TYPE_MARBLE,
    MAT_TYPE_TILE,

    MAT_TYPE_LAST,
};

enum pref_bpp
{
    PREF_BPP_DEFAULT = 0,
    PREF_BPP_32,
    PREF_BPP_16,
    PREF_BPP_8,
    PREF_BPP_4
};

struct tex_info
{
public:
    xbool    Load    ( const char* pFilename );

    u32         SoundMat;
    pref_bpp    PreferredBPP;
    s32         nMipsToBuild;
};

#endif // TEXINFO_HPP