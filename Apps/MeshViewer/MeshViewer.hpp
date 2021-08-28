
#ifndef MESHVIEWER_HPP
#define MESHVIEWER_HPP

//=========================================================================
#include "RawMesh.hpp"
#include "RawAnim.hpp"

//=========================================================================
#define MAX_MESHES  21
//=========================================================================
class mesh_viewer
{
public:
    mesh_viewer();

    vector3 GetObjectFocus  ( void ) { return m_BBox.GetCenter(); }
    bbox    GetBBox         ( void ) { return m_BBox; }
    void    Load            ( const char* pFileName );
    void    LoadAdditional  ( const char* pFileName, const vector3& Pos, const radian3& Rot );
    void    Render          ( void );
    void    PlayAnimation   ( xbool bPlayInPlace = FALSE );
    xbool   IsPause         ( void ) {return !m_bPlayAnim; }
    void    PauseAnimation  ( void );
    void    SetBackFacets   ( xbool bFaceFacets );
    void    SetAnimFrameRate( f32 Rate ){m_AnimFrameRate=Rate;}
    void    SetRenderToBind ( xbool bRenderToBind )  { m_bRenderToBind=bRenderToBind;}
    void    SetRenderSkel   ( xbool bRenderSkel )    { m_bRenderSkeleton=bRenderSkel;}
    void    SetRenderSkelLabels( xbool bSkelLabels ) { m_bRenderSkeletonLabels=bSkelLabels;}
           ~mesh_viewer     ( void );
    void    CleanUp         ( void );

protected:

    void    RenderSoftSkin  ( s32 nMesh );
    void    RenderSolid     ( s32 nMesh );

protected:

    s32         m_nCurrentMeshCount;

    rawmesh     m_Mesh[MAX_MESHES];
    rawanim     m_Anim[MAX_MESHES];
    matrix4     m_L2W[MAX_MESHES];
    xbitmap     m_Bitmap[MAX_MESHES][32];
    vector3     m_BonePos[256];

    f32         m_Frame;
    bbox        m_BBox;
    vector3     m_LightDir;
    vector3     m_Ambient;
    xbool       m_bPlayAnim;
    xtimer      m_Timer;
    f32         m_AnimFrameRate;
    xbool       m_bBackFacets;
    xbool       m_bPlayInPlace;
    xbool       m_bRenderToBind;
    xbool       m_bRenderSkeleton;
    xbool       m_bRenderSkeletonLabels;
};

//=========================================================================
// END
//=========================================================================
#endif