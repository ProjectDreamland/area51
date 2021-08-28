
#ifndef SKIN_GEOM_HPP
#define SKIN_GEOM_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Geom.hpp"
#include "CollisionVolume.hpp"

//=========================================================================
// SKIN GEOMETRY
//=========================================================================

struct skin_geom : public geom
{
    //  -------------------------------------------------------------------

    struct vertex_xbox
    {
        vector3p Pos;
        u32      PackedNormal;
        vector2  UV;

        vector2 Weights;
        vector2 Bones;

        void FileIO( fileio& File );
    };

    enum command_types_xbox
    {
        XBOX_CMD_NULL,
        XBOX_CMD_UPLOAD_MATRIX,       // Arg1 = BoneID, Arg2 = CacheID
        XBOX_CMD_DRAW_SECTION,        // Arg1 = Start,  Arg2 = End
        XBOX_CMD_END = 0xffffffff
    };

    struct command_xbox
    {
        command_types_xbox  Cmd;
        u32                 Arg1;
        u32                 Arg2;

        void FileIO( fileio& File );
    };

    struct dlist_xbox
    {
        s32           nIndices;
        u16*          pIndex;

        u32           nPushSize;
        u8*           pPushBuffer;
        void*         hPushBuffer;

        s32           nVerts;
        vertex_xbox*  pVert;
        void*         hVert;

        s32           nCommands;            
        command_xbox* pCmd;
        
        void*         pOpt;

        void FileIO( fileio& File );
    };

    //  -------------------------------------------------------------------

    struct uv_ps2
    {
        s16 U;
        s16 V;
        
        void FileIO( fileio& File );
    };

    struct pos_ps2
    {
        s16 Pos[3];
        u16 ADC;

        void FileIO( fileio& File );
    };

    struct boneindex_ps2
    {
        u8  B0;             // is actually a vector offset
        u8  B1;             // is actually a vector offset
        u8  W0;             // 0.8 fixed point weight
        u8  W1;             // 0.8 fixed point weight

        void FileIO( fileio& File );
    };

    struct normal_ps2
    {
        u8  Normal[3];
        u8  Pad;            // pad is necessary for alignment...need to get rid of that

        void FileIO( fileio& File );
    };

    enum command_types_ps2
    {
        PS2_CMD_UPLOAD_MATRIX = 0,      // Arg1 = BoneID,     Arg2 = CacheIndex
        PS2_CMD_RENDER_VERTS_RIGID,     // Arg1 = VertOffset, Arg2 = nVerts
        PS2_CMD_RENDER_VERTS_SOFT,
        PS2_CMD_END,
    };

    struct command_ps2
    {
        command_types_ps2   Cmd;
        s16                 Arg1;
        s16                 Arg2;

        void FileIO( fileio& File );
    };

    struct dlist_ps2
    {
        s32                 nCmds;
        command_ps2*        pCmd;

        s32                 nUVs;
        uv_ps2*             pUV;

        s32                 nPos;
        pos_ps2*            pPos;

        s32                 nBoneIndices;
        boneindex_ps2*      pBoneIndex;

        s32                 nNormals;
        normal_ps2*         pNormal;

        void                FileIO( fileio& File );
    };

    //  -------------------------------------------------------------------

    struct vertex_pc
    {
        vector4 Position;   // W=B1
        vector4 Normal;     // W=B2
        vector4 UVWeights;  // XY=UV ZW=W1,W2

        void FileIO( fileio& File );
    };

    enum command_types_pc
    {
        PC_CMD_NULL,
        PC_CMD_UPLOAD_MATRIX,       // Arg1 = BoneID, Arg2 = CacheID
        PC_CMD_DRAW_SECTION,        // Arg1 = Start,  Arg2 = End
        PC_CMD_END = 0xffffffff
    };

    struct command_pc
    {
        command_types_pc    Cmd;
        s16                 Arg1;
        s16                 Arg2;

        void FileIO( fileio& File );
    };

    struct dlist_pc
    {
        s32         nIndices;
        s16*        pIndex;

        s32         nVertices;
        vertex_pc*  pVertex;

        s32         nCommands;            
        command_pc* pCmd;
        
        xhandle     hDList;

        void FileIO( fileio& File );
    };

    //  -------------------------------------------------------------------

    union system_ptr
    {
        dlist_xbox* pXbox;
        dlist_ps2*  pPS2;
        dlist_pc*   pPC;
    };

    //  -------------------------------------------------------------------

    skin_geom       ( void );
    skin_geom       ( fileio& File );
    void FileIO     ( fileio& File );

    s32                 m_nDList;
    system_ptr          m_System;
};

//=========================================================================
// END
//=========================================================================
#endif
