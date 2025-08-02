#ifndef SOFT_VERTEX_MANAGER_HPP
#define SOFT_VERTEX_MANAGER_HPP

#if !defined(TARGET_PC)
#error "This is only for the PC target platform. Please check build exclusion rules"
#endif

//=========================================================================
// INCLUDES
//=========================================================================
#include "VertexMgr.hpp"
#include "Render/SkinGeom.hpp"
#include "Shaders/SkinShader.h"

//=========================================================================
// CLASS
//=========================================================================

class soft_vertex_mgr : protected vertex_mgr
{
public:

    void        Init                ( void );
    void        Kill                ( void );
    xhandle     AddDList            ( void*                   pVertex, 
                                      s32                     nVertices, 
                                      u16*                    pIndex, 
                                      s32                     nIndices, 
                                      s32                     nPrims, 
                                      s32                     nCmds, 
                                      skin_geom::command_pc*  pCmd );


    void        DelDList            ( xhandle hDList );
    void        BeginRender         ( void );
    void        DrawDList           ( xhandle hDList, const matrix4* pBone, const d3d_skin_lighting* pLighting );
    void        InvalidateCache     ( void );

protected:

    struct soft_dlist
    {   
        xhandle                 hDList;                         // Display List
        s32                     nCommands;                      // Commands            
        skin_geom::command_pc*  pCmd;                           // List of commands
    };

protected:

    xharray<soft_dlist> m_lSoftDList;
    static DWORD        s_hShader;
    static s32          s_InitCount;
};

//=========================================================================
// END
//=========================================================================
#endif