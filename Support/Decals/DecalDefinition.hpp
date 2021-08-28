//==============================================================================
//  DecalDefinition.hpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class defines the behavior of a decal. Which bitmap does it use? How
//  big is it? Does it wrap around walls? What blending does it use? etc.
//==============================================================================

#ifndef DECALDEFINITION_HPP
#define DECALDEFINITION_HPP

#include "Auxiliary\MiscUtils\Fileio.hpp"

//==============================================================================
// The class
//==============================================================================

struct decal_definition
{
public:
    enum
    {
        DECAL_FLAG_USE_TRI          = 0x0001,   // decal can be a triangle (which is more efficient)
        DECAL_FLAG_NO_CLIP          = 0x0002,   // don't clip the decal to polygon edges
        DECAL_FLAG_USE_PROJECTION   = 0x0004,   // decal is projected along the incoming ray
        DECAL_FLAG_KEEP_SIZE_RATIO  = 0x0008,   // maintain the width/height ratio when doing random size
        DECAL_FLAG_PERMANENT        = 0x0010,   // this decal will never fade out or disappear
        DECAL_FLAG_FADE_OUT         = 0x0020,   // this decal fades out over time
        DECAL_FLAG_ADD_GLOW         = 0x0040,   // adds a bloom effect around the decal
        DECAL_FLAG_ENV_MAPPED       = 0x0080,   // makes the decal get env. mapped with the cube map
    };

    enum blend_mode
    {
        DECAL_BLEND_NORMAL    = 0,  // ((DecalColor*Alpha) + (DestColor*(1-Alpha))
        DECAL_BLEND_ADD,            // (DestColor + (DecalColor*Alpha))
        DECAL_BLEND_SUBTRACT,       // (DestColor - (DecalColor*Alpha))
        DECAL_BLEND_INTENSITY,      // (DestColor * Alpha) where alpha==128 means no change, <128 darkens, >128 brightens
    };

    //==========================================================================
    // Constructors/destructors
    //==========================================================================
            decal_definition    ( void );
            decal_definition    ( fileio& File );
            ~decal_definition   ( void );
    void    FileIO              ( fileio& File );


    //==========================================================================
    // Methods for generating random data based on the decal parameters
    //==========================================================================
    vector2 RandomSize  ( void ) const;
    radian  RandomRoll  ( void ) const;
    
public:
    // the decal data
    char                m_Name[32];
    vector2             m_MinSize;
    vector2             m_MaxSize;
    radian              m_MinRoll;
    radian              m_MaxRoll;
    xcolor              m_Color;
    char                m_BitmapName[256];
    u32                 m_MaxVisible;
    f32                 m_FadeTime;
    u32                 m_Flags;
    u16                 m_BlendMode;
    xhandle             m_Handle;
};

#endif // DECALDEFINITION_HPP
