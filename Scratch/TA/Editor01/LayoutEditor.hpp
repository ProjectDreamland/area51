#ifndef LAYOUT_EDITOR_HPP
#define LAYOUT_EDITOR_HPP

#include "Geometry\WallGeom.hpp"
#include "Auxiliary\MiscUtils\slist.hpp"
#include "Auxiliary\MiscUtils\GUID.HPP"
#include "Geometry\GeomMgr.hpp"

#define SOUND_SPHERE_RADIUS     50
#define LIGHT_SPHERE_RADIUS     50
#define GLOW_SPHERE_RADIUS     50
#define PFX_SPHERE_RADIUS       50
#define PICKUP_SPHERE_RADIUS    50

//=========================================================================
// CLASSA
//=========================================================================

class layout_editor
{
public:

    enum inst_flags
    {
        INST_FLAGS_NULL     = 0,
        INST_FLAGS_SELECTED = (1<<0),
        INST_FLAGS_WALL     = (1<<1),
        INST_FLAGS_AMMO     = (1<<2),
        INST_FLAGS_HEALTH   = (1<<3),
        INST_FLAGS_ITEM     = (1<<4),
        INST_FLAGS_OBJECT   = (1<<5),
    };

    enum material_type
    {
        MAT_TYPE_NULL,
        MAT_TYPE_WOOD,
        MAT_TYPE_METAL,
        MAT_TYPE_FLESH,
        MAT_TYPE_STONE,
        MAT_TYPE_LAST
    };
    
    enum PICKUP_TYPE
    {
        PICKUP_START,
        PICKUP_HEALTH = PICKUP_START,
        PICKUP_AMMO,
        PICKUP_ITEMS,
        
        PICKUP_END
    };

    struct piece_desc
    {
        char            FileName[256];
        f32             SurfaceArea;
        hgeom           hGeom;              // Use for rendering
        wall_geom*      pWallGeom;          // Use for quick reference
        material_type   MatType;            // Material Type
        
        piece_desc(){ MatType = MAT_TYPE_NULL; }
    };

    struct piece_inst
    {
        guid        Guid;
        xhandle     hDesc;
        //xhandle     hObjectDesc;
        matrix4     L2W;
        matrix4     W2L;
        u32         Flags;
        hgeom       hGeom;              // Use for rendering
        char        ObjectName[64];
    };

    struct light
    {
        guid        Guid;
        sphere      Sphere;
        xcolor      Color;
        f32         Intensity;
        u32         Flags;
    };

    struct sound_desc
    {
        char        Name[256];
        s32         ID;
        xbool       Looped;
        xbool       Clipped;
        sound_desc(){ Looped = FALSE; }
    };

    struct sound_inst
    {
        guid        Guid;
        sphere      Sphere;
        xhandle     hDesc;
        u32         Flags;
    };

    struct pfx_desc
    {
        char        Name[256];
        s32         ID;
    };

    struct pfx_inst
    {
        guid        Guid;
        sphere      Sphere;
        vector3     Normal;
        xhandle     hDesc;
        u32         Flags;
    };

    struct glow
    {
        guid        Guid;
        sphere      Sphere;
        xcolor      Color;
        f32         Intensity;
        u32         Flags;
    };
    
    struct pickup_desc
    {
        char        Name[256];
        s32         Type;
    };
    
    struct pickup_inst
    {
        guid        Guid;
        xhandle     hGeomDesc;  
        matrix4     L2W;    
        sphere      Sphere;
        xhandle     hDesc;
        s32         Flags;
    };

    struct texture
    {
        xbitmap     Bitmap;
        char        FileName[256];
    };

public:
                layout_editor       ( void );

    void        NewProject          ( void );
    void        SetWorkingDir       ( const char* pFileName );
    void        Save                ( const char* pFileName );
    void        Load                ( const char* pFileName );
    void        Export              ( const char* pFileName );

    void        RefreshAllFiles     ( void );
    void        AddPieceDescFile    ( const char* pFileName );
    s32         GetNDescs           ( void );
    piece_desc& GetDesc             ( xhandle hDesc );
    piece_desc& GetDesc             ( s32 Index ); 
    piece_desc& GetDesc             ( const char* pFileName ); 

    void        AddSoundDescFile    ( const char* pFileName );
    void        InitParticleFXList  ( void );

    s32         GetSoundNDescs           ( void );
    sound_desc& GetSoundDesc        ( xhandle hDesc );
    sound_desc& GetSoundDesc        ( s32 Index ); 
    sound_desc& GetSoundDesc        ( const char* pFileName ); 

    s32         GetPfxNDescs           ( void );
    pfx_desc&   GetPfxDesc          ( xhandle hDesc );
    pfx_desc&   GetPfxDesc          ( s32 Index ); 
    pfx_desc&   GetPfxDesc          ( const char* pFileName ); 

    s32             GetPickupNDescs           ( void );
    pickup_desc&    GetPickupDesc          ( xhandle hDesc );
    pickup_desc&    GetPickupDesc          ( s32 Index ); 
    pickup_desc&    GetPickupDesc          ( const char* pFileName ); 


    s32         GetNInsts           ( void );
    piece_inst& GetInst             ( s32 Index );
    void        InstUpdateLight     ( s32 Index );

    void        AddInst             ( const char* pDescName, matrix4& Matrix, u32 Flag, char* pObjectName = NULL );
    void        DeleteInst          ( s32     Index );
    void        DeleteInst          ( xhandle hInst );
    xbool       IsModified          ( void );
    void        Undo                ( void );
    void        Redo                ( void );
    void        Select              ( vector3& P0, vector3& P1, xbool bSelect );
    f32         GetHitPos           ( const vector3& P0, const vector3& P1, vector3& Normal );
    f32         GetHitPos           ( const vector3& P0, const vector3& P1 );
    void        UnselectAll         ( void );
    void        DeleteSelection     ( void );

    void        AddLight            ( sphere& Light, xcolor Color, f32 Intensity );
    s32         GetLightCount       ( void ) const;
    light&      GetLight            ( s32 I );
    void        UpdateLighting      ( void );
    void        GetAttenConstants   ( f32 Intensity, f32 AttenEnd, f32& k0, f32& k1, f32& k2 ) const;
    f32         ComputeAttenuation  ( const light& Light, const vector3& Pos ) const ;
    void        InstUpdateFinalLight( xhandle hInst );
    xbool       CanSeeLight         ( const light& Light, const vector3& P ) const;

    void        AddSound            ( sphere& Sound, s32 Type );
    s32         GetSoundCount       ( void ) const;
    sound_inst& GetSound            ( s32 I );

    void        AddEffect           ( vector3& Point, vector3& Normal, s32 Type );
    s32         GetParticleCount    ( void ) const;
    pfx_inst&   GetParticle         ( s32 I );

    s32         GetNumFXTypes       ( void );
    void        GetFXType           ( int Idx, char* pName, s32& ID );

    void        AddPickup           ( sphere& Sphere, s32 Type );
    s32         GetPickupCount      ( void ) const;
    pickup_inst&GetPickup           ( s32 I );

    s32         GetNumPickupTypes   ( void );
    void        GetPickupType       ( int Idx, char* pName, s32& ID );
    void        InitPickupList      ( void );

    void        AddGlow            ( sphere& Light, xcolor Color, f32 Intensity );
    s32         GetGlowCount       ( void ) const;
    glow&       GetGlow            ( s32 I );

protected:

    enum command_type
    {
        COMMAND_NONE,
        COMMAND_ADD_INST,
        COMMAND_DEL_INST,
    };

    struct command
    {
        command_type        UndoCommand;
        command_type        RedoCommand;
    };

    struct undo_data
    {
        enum object_type
        {
            TYPE_NONE,
            TYPE_INSTANCE,
            TYPE_LIGHT,
            TYPE_SOUND,
            TYPE_PFX,
            TYPE_PICKUP,
            TYPE_GLOW,
        };

        object_type Type;
        u8          Data[ MAX( sizeof(sound_inst), MAX( (MAX( sizeof(piece_inst), sizeof(light) ) ), (MAX( sizeof(pfx_inst), sizeof(pickup_inst) ))) ) ];


        piece_inst& GetInstace( void ){ Type = TYPE_INSTANCE; return *(piece_inst*)Data; }
        light&      GetLight  ( void ){ Type = TYPE_LIGHT;    return *(light*)Data;      }
        sound_inst& GetSound  ( void ){ Type = TYPE_SOUND;    return *(sound_inst*)Data; }
        pfx_inst&   GetPfx    ( void ){ Type = TYPE_PFX;      return *(pfx_inst*)Data;   }
        pickup_inst&GetPickup ( void ){ Type = TYPE_PICKUP;   return *(pickup_inst*)Data;}
        glow&       GetGlow   ( void ){ Type = TYPE_GLOW;     return *(glow*)Data;       }
    };

protected:

    s32         GetDescIndexByName  ( const char* pName );
    xhandle     GetDescHandleByName ( const char* pName );
    s32         GetSoundDescIndexByName  ( const char* pName );
    xhandle     GetSoundDescHandleByName ( const char* pName );

    s32         GetPfxDescIndexByName  ( const char* pName );
    xhandle     GetPfxDescHandleByName ( const char* pName );

    s32         GetPickupDescIndexByName  ( const char* pName );
    xhandle     GetPickupDescHandleByName ( const char* pName );

    xbool       UpdateUndo          ( void );
    void        HandleCommand       ( command_type Command );
    void        AddInst             ( xhandle hDesc, matrix4& L2W, u32 Flag, char* pObjectName = NULL );
    void        InstUpdateLight     ( xhandle hInst );

protected:

    guid_lookup                     m_lGuid2Handle;

    xharray< piece_desc >           m_lPieceDesc;
    xharray< piece_inst >           m_lPieceInst;

    xharray< sound_desc >           m_lSoundDesc;
    xharray< sound_inst >           m_lSoundInst;

    xharray< pfx_desc >             m_lPfxDesc;
    xharray< pfx_inst >             m_lPfxInst;

    xharray< pickup_desc >          m_lPickupDesc;
    xharray< pickup_inst >          m_lPickupInst;

    xharray< light >                m_lLight;
    xarray<texture>                 m_lTexture;
    
    xharray< glow >                 m_lGlow;

    xarray< xhandle >               m_lSelect;
    slist< command, undo_data >     m_lUndo;

    char                            m_WorkingDir[256];
    xhandle                         m_hUndoCursor;
    xbool                           m_bUpdateUndo;
    xcolor                          m_AmbientLight;
};

//=========================================================================
// END
//=========================================================================
#endif