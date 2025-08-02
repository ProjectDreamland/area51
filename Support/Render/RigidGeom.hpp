
#ifndef RIGID_GEOM_HPP
#define RIGID_GEOM_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Geom.hpp"
#include "CollisionVolume.hpp"

//=============================================================================
// RIGID GEOMETRY
//=============================================================================

struct rigid_geom : public geom
{
    struct vertex_xbox
    {
        vector3p Pos;
        u32      PackedNormal;
        vector2  UV;

        void FileIO( fileio& File );
    };

    struct dlist_xbox
    {
        s32          nIndices;
        u16*         pIndices;

        u32          nPushSize;
        u8*          pPushBuffer;
        void*        hPushBuffer;

        s32          nVerts;
        vertex_xbox* pVert;
        void*        hVert;

        s32          iBone;
        s32         iColor;                 // Index into color table

        void FileIO( fileio& File );
    };

    struct dlist_ps2
    {
        s32         nVerts;
        s16*        pUV;                    // 2*VertIndex
        s8*         pNormal;                // 3*VertIndex
        vector4*    pPosition;              // 1:1

        s32         iBone;
        s32         iColor;                 // Index into color table

        void FileIO( fileio& File );
    };

    struct vertex_pc
    {
        vector3p Pos;
        vector3p Normal;
        xcolor   Color;
        vector2  UV;

        void FileIO( fileio& File );
    };

    struct dlist_pc
    {
        s32         nIndices;
        u16*        pIndices;
        
        s32         nVerts;
        vertex_pc*  pVert;
        
        s32         iBone;
        
        u32         Pad; // GS: Unknown padding in V41.

        void FileIO( fileio& File );
    };

    union system_ptr
    {
        dlist_xbox* pXbox;
        dlist_ps2*  pPS2;
        dlist_pc*   pPC;
    };

    //-------------------------------------------------------------------------

    rigid_geom      ( void ) ;
    rigid_geom      ( fileio& File );
    void FileIO     ( fileio& File );

    collision_data      m_Collision;
    s32                 m_nDList;
    system_ptr          m_System;       // System dependent information

    //-------------------------------------------------------------------------

    xbool       GetGeoTri( s32 TriKey, vector3& V0, vector3& V1, vector3& V2 ) const;

};

//=============================================================================
#endif
//=============================================================================
