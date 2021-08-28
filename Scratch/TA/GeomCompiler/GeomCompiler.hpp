
#ifndef GEOM_COMPILER_HPP
#define GEOM_COMPILER_HPP

#include "RawMesh.hpp"
#include "x_array.hpp"

class geom_compiler
{
public:
        
    void    AddPlatform    ( platform Platform, const char* pDirName );
    void    AddLOD         ( const char* pFileName, f32 MinDistance, xbool BuildCollision );
    void    Export         ( const char* pFileName );

protected:

    struct info
    {
        char    FileName[256];
        f32     MinDistance;
        xbool   BuildCollision;
    };

    struct plat_info
    {
        platform Platform;
        char     DirName[256];
    };

    xbool               m_bCompilePC;
    xbool               m_bCompilePS2;
    xarray< info >      m_InfoList;
    xarray< plat_info > m_PlatInfo;
};

#endif