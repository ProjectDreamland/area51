//==============================================================================
//
//  File:           ViewerObject.hpp
//
//  Description:    Viewer object class
//
//  Author:         Stephen Broumley
//
//  Date:           Started July29th, 2003 (C) Inevitable Entertainment Inc.
//
//==============================================================================

#ifndef __VIEWER_OBJECT_HPP__
#define __VIEWER_OBJECT_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================
#include "Entropy.hpp"
#include "Render\Render.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"
#include "Loco\Loco.hpp"
#include "Animation\AnimPlayer.hpp"
#include "Config.hpp"
#include "ViewerLoco.hpp"


//==============================================================================
//  CLASSES
//==============================================================================

// Contains everything you could ever want for an object...
class viewer_object
{
//==========================================================================
// DEFINES
//==========================================================================
public:

    // Render type
    enum render_type
    {
        RENDER_NULL,
        
        RENDER_RIGID,
        RENDER_SKIN,
    } ;

//==========================================================================
// STRUCTURES
//==========================================================================
public:
    
    // LOD
    struct lod
    {
        // Data
        f32     m_ScreenSize;
        u64     m_Mask;

        // Functions
        lod()
        {
            m_ScreenSize = -1;
            m_Mask       = 0;
        }
    };

    // Used for storing state when reload objects
    struct state
    {
        // Data
        char                m_CompiledGeom[X_MAX_PATH] ;// Name of compiled geometry
        matrix4             m_L2W ;                     // Local -> world matrix
        char                m_SelectedAnimName[34];     // Current anim selected
        char                m_AnimName[34] ;            // Current anim name
        f32                 m_AnimFrame ;               // Current anim frame
        f32                 m_AnimYaw ;                 // Current anim yaw
        vector3             m_Position ;                // Loco position
        vector3             m_MoveAt ;                  // Loco move at
        vector3             m_LookAt ;                  // Loco look at
        loco::move_style    m_MoveStyle ;               // Loco move style
        view                m_View ;                    // Current view

        // Functions
        state()
        {
            // Clear fields
            m_CompiledGeom[0] = 0 ;
            m_L2W.Identity() ;
            m_SelectedAnimName[0] = 0;
            m_AnimName[0]  = 0 ;
            m_AnimFrame    = 0 ;
            m_AnimYaw      = 0 ;
            m_Position.Zero() ;
            m_MoveAt.Zero() ;
            m_LookAt.Zero() ;
            m_MoveStyle = loco::MOVE_STYLE_WALK ;
        }
    } ;

//==========================================================================
// DATA
//==========================================================================

public:

    // Type
    char                    m_CompiledGeom[X_MAX_PATH] ;// Name of compiled geometry
    config_options::object*         m_pConfigObject;            // Owner config object
    render_type             m_RenderType ;              // Render type - skin or rigid
    config_options::type            m_Type ;                    // Type of object
                                                        
    // Resources                                        
    rhandle<skin_geom>      m_hSkinGeom ;               // Compiled skinned geometry
    rhandle<rigid_geom>     m_hRigidGeom ;              // Compiled rigid geometry
    anim_group::handle      m_hAnimGroup ;              // Compiled animation group
    rhandle<char>           m_hAudioPackage ;           // Compiled audio package
    xbool                   m_bAudioSet ;               // TRUE if audio is in config file
                            
    // Render                      
    view                    m_View ;                    // Current view
    geom*                   m_pGeom ;                   // Geometry
    render::hgeom_inst      m_hInst ;                   // Registered instance handle
    s32                     m_nColors ;                 // # of colors
    u16*                    m_pColors ;                 // List of rigid colors
    xarray<lod>             m_LODs;                     // List of LODs

    // Position/Animation                       
    s32                     m_iLastAnim ;               // Index of last anim
    matrix4                 m_L2W ;                     // Local to world
    viewer_loco             m_Loco ;                    // Locomotion
    simple_anim_player      m_AnimPlayer ;              // Animation player
    xbool                   m_bAnimPaused;              // Anim paused flag

    // Link info
    xarray<viewer_object*>  m_pChildren ;               // List of children
    viewer_object*          m_pParentObject ;           // Parent object (or NULL)
    s32                     m_iParentBone ;             // Parent bone attached to (or -1)
                           
    // Runtime                                          
    s32                     m_iAnim ;                   // Index of animation to play
    s32                     m_nActiveBones ;            // # of active bones in view
    s32                     m_iLOD ;                    // LOD in view
    s32                     m_iMaterials ;              // # of materials in view
    s32                     m_nVerts ;                  // Verts in view
    s32                     m_nTris ;                   // Tris in view

//==========================================================================
// FUNCTIONS
//==========================================================================

public:
    // Constructor/destructor
    viewer_object() ;
    ~viewer_object() ;

private:

    // Utility functions used to auto-generate LODs
    s32     FindMesh    ( const char* pName );
    void    GetLODInfo  ( const char* pGeomMesh, s32& iLOD, char* pMesh );
    xbool   IsMeshInLOD ( s32 iLOD, const char* pMesh );

    // Builds lods automatically or from config object
    void    BuildLODs   ( config_options::object& Object ) ;

public:
    // Initialize
    xbool Init          ( config_options::object& Object ) ;

    // Destroy
    void Kill           ( void ) ;

    // Resets view so object is fully on screen
    void ResetView      ( void ) ;

    // Returns type of anim (masked, lip sync, full body etc)
    const char* GetAnimType( const char* pAnimName ) const;    

    // Lookup up info for the current anim
    void GetAnimInfo( const char*&           pAnimName,
                      const char*&           pAnimType,
                      loco::bone_masks_type& MaskType,
                      u32&                   MaskFlags );

    // Plays the currently selected anim
    void PlayCurrentAnim( xbool bMovieStart );

    // Returns TRUE if current anim has finished playing
    xbool HasCurrentAnimFinished( void );

    // Returns current anim name    
    const char* GetCurrentAnimName( void );
    
    // Returns current frame
    s32 GetCurrentAnimFrame( void );
    
    // Writes out .tga of current frame
    void DoCurrentAnimFrameScreenShot( void );
    
    // Searches for bone index
    s32 FindBoneIndex   ( const char* pName ) ;

    // Returns bone local to world matrix
    void GetBoneL2W     ( s32 iBone, matrix4& L2W ) ;

    // Returns TRUE if object contains animation a camera bone
    xbool HasCameraBone     ( void );

    // Returns TRUE and sets up L2W if object animation contains a camera bone
    xbool GetCameraBoneL2W  ( matrix4& L2W );

    // Advance logic
    void Advance        ( f32 DeltaTime ) ;

    // Sets L2W for object and update loco/anim players
    void    SetL2W ( const matrix4& L2W, xbool bFlip180 = FALSE );
    
    // Computes matrices from animation (if present)
    const matrix4*      ComputeMatrices( s32 nActiveBones ) ;

    // Render
    void Render         ( const view& View, xtimer& LogicCPU, xtimer& RenderCPU ) ;

    // Returns local bbox
    const bbox&         GetLocalBBox( void ) ;

    // Returns bbox of bind pose
    bbox GetBindWorldBBox   ( void );

    // Returns local to world matrix
    matrix4 GetL2W( void );

    // Returns world bbox
    bbox GetWorldBBox       ( void ) ;

    // Light vert using config omni light
    xcolor LightVert( const vector3& Position, const vector3& Normal ) ;

    // Lights the object using the config file lighting
    void Light          ( void ) ;

    // Process input
    void HandleInput    ( f32 DeltaTime ) ;

    // Returns animation name
    const char* GetAnimName( s32 iAnim ) const ;

    // Shows current geometry and animation at top of screen
    void ShowInfo( s32& x, s32& y ) ;

    // Shows current bones and geometry info at the bottom of the screen
    void ShowStats( s32 x, s32 y ) ;

    // Returns current state
    void GetState( state& State ) ;

    // Sets current state
    void SetState( const viewer_object::state& State ) ;
} ;

//==============================================================================

// UTIL FUNCTIONS

// Draws marker with z buffering
void Util_DrawMarker( const vector3& Pos,
                     xcolor   Color,
                     s32      Size = 8 );



#endif  //#ifndef __VIEWER_OBJECT_HPP__
