
#ifndef MATX_MISC_HPP
#define MATX_MISC_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"

//=========================================================================
// CLASS
//=========================================================================
struct matx_misc
{
//=========================================================================

     matx_misc( void );
    ~matx_misc( void );

    xbool   Load                ( const char* pFileName );
    
//=========================================================================

    struct xref
    {
        char                ObjectName[256];
        char                FileName[256];
        vector3             Scale;
        quaternion          Rotation;
        vector3             Position;
        bbox                BBox;
        xbool               IsChild;
    };

    struct light
    {
        char                Name[32];
        s32                 Type;
        vector3             Position;
        xcolor              Color;
        f32                 Intensity;
        f32                 AttenuationStart;
        f32                 AttenuationEnd;
    };

//=========================================================================

    s32         m_nXRefs;
    xref*       m_pXRef;

    s32         m_nLights;
    light*      m_pLight;
};

xbool Matx_HasHeader(const char *pFileName, const char *pHeader);
//=========================================================================
// END
//=========================================================================
#endif










































