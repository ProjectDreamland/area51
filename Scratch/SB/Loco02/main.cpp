//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "MeshUtil\RawMesh2.hpp"
#include "Render\Render.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"
#include "Render\LightMgr.hpp"
#include "Objects\Render\SkinInst.hpp"

#include "Characters\Grunt\GruntLoco.hpp"
#include "Characters\Scientist\FriendlyScientistLoco.hpp"
#include "Characters\MutantTank\Mutant_Tank_Loco.hpp"
#include "Characters\Soldiers\SoldierLoco.hpp"
#include "Characters\Gray\GrayLoco.hpp"
#include "Characters\GenericNPC\GenericNPCLoco.hpp"
#include "Objects\PlayerLoco.hpp"

#include "Cloth\Cloth.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "IOManager\io_mgr.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "PhysicsMgr\PhysicsInst.hpp"


//==============================================================================
//  DATA
//==============================================================================

static s32   s_LOD       = -1;
static xbool s_bLocoTest = TRUE;//FALSE;
static xbool s_bDrawLoco = TRUE;
static xbool s_bDrawSkin = TRUE;
static xbool s_bClipped  = TRUE;
static xbool s_bExact    = TRUE;//FALSE;
static xbool s_bDrawPhysics = FALSE;//TRUE;//FALSE;//TRUE;
static xbool s_bShowAnimInfo = TRUE;

static f32   s_SlowMo     = 1;
static f32   s_Pause      = 1;
static f32   s_Step       = 1.0f / 30.0f;
static f32   s_SingleStep = 0;

// NEEDED FOR PS2 GAME
xbool       g_GameLogicDebug     = FALSE;
xbool       g_game_running       = TRUE;
xbool       g_first_person       = FALSE;
xbool       g_InvertYAxis        = FALSE;
xbool       g_UnlimitedAmmo      = TRUE;
xbool       g_MirrorWeapon       = FALSE;
xbool       g_LevelLoadRequested = FALSE;
xbool       g_Pause              = FALSE;
xbool       g_bBloodEnabled      = TRUE;
xbool       g_bRagdollsEnabled   = TRUE;
u32         g_nFramesAfterLoad   = 0;
xtimer      g_GameTimer;
char        g_LevelToLoad[ 256 ];
char        g_FullPath [ 256 ];
xbool       g_bInsideRTF        = FALSE;
xbool       g_FreeCam           = FALSE;
view        g_View;
xbool       g_RenderBoneBBoxes = FALSE;

xbool       g_AimAssist_Render_Reticle      = FALSE;
xbool       g_AimAssist_Render_Bullet       = FALSE;
xbool       g_AimAssist_Render_Turn         = FALSE;
xbool       g_AimAssist_Render_Bullet_Angle = FALSE;
xbool       g_AimAssist_Render_Player_Pills = FALSE;

s32         g_Difficulty = 1;  // Start out on Medium difficulty
const char* DifficultyText[] = { "Easy", "Medium", "Hard" };

s32         g_Changelist = 60944;
const char* g_pBuildDate = "2004-06-24 09:34:51";

char        g_LevelName[256] = {0};

void     UnloadLevel     ( void ) {}
void     LoadLevel       ( void ) {}



//==============================================================================
//  DEFINES
//==============================================================================

// Leaper
//#define LOCO_CLASS      grunt_loco
//#define SKIN_FILE       "AITest_Leaper_BIND.skingeom"
//#define ANIM_FILE       "NPC_Leaper.anim"

// THETA
//#define LOCO_CLASS      mutant_tank_loco
//#define SKIN_FILE       "AITest_Tank_BIND.skingeom"
//#define ANIM_FILE       "NPC_Strain_Tank.anim"

// Spec ops
//#define LOCO_CLASS      soldier_loco
//#define SKIN_FILE       "AITest_MIL_Spec4_BIND.skingeom"
//#define ANIM_FILE       "NPC_Military_SMP.anim"
//#define MOVE_STYLE      loco::MOVE_STYLE_RUN


// Avatar
//#define LOCO_CLASS      player_loco
//#define SKIN_FILE       "MP_AVATAR_BIND.skingeom"
//#define ANIM_FILE       "MP_AVATAR.anim"
//#define MOVE_STYLE      loco::MOVE_STYLE_WALK


// Hazmat
//#define LOCO_CLASS      soldier_loco
//#define SKIN_FILE       "AITest_MIL_HazMat_BIND.skingeom"
//#define ANIM_FILE       "NPC_Military_SMP.anim"
//#define MOVE_STYLE      loco::MOVE_STYLE_WALK

// Blackops
#define LOCO_CLASS      soldier_loco
#define SKIN_FILE       "AITest_MIL_BlackOps_BIND.skingeom"
#define ANIM_FILE       "NPC_BlackOps_SMP.anim"
#define MOVE_STYLE      loco::MOVE_STYLE_RUNAIM

// Grunt-military
//#define LOCO_CLASS      grunt_loco
//#define SKIN_FILE       "AITest_Grunt_Military_BIND.skingeom"
//#define SKIN_FILE       "NPC_Grunt_Military_01_LevelBlue_SL2.skingeom"
//#define ANIM_FILE       "NPC_Grunt_SMP.anim"

// Grunt-civilian
//#define LOCO_CLASS      grunt_loco
//#define SKIN_FILE       "AITest_Grunt_Civilian_BIND.skingeom"
//#define ANIM_FILE       "NPC_Grunt_Civilian_Unarmed.anim"

// Frienldy civilian
//#define LOCO_CLASS      friendly_scientist_loco
//#define SKIN_FILE       "AITest_CIV_Friendly_BIND.skingeom"
//#define ANIM_FILE       "NPC_Civilian.anim"

// Illuminati civilian
//#define LOCO_CLASS      friendly_scientist_loco
//#define SKIN_FILE       "AITest_CIV_Illuminati_BIND.skingeom"
//#define ANIM_FILE       "NPC_Civilian.anim"

// Gray
//#define LOCO_CLASS      gray_loco
//#define SKIN_FILE       "AITest_Gray_BIND.skingeom"
//#define ANIM_FILE       "NPC_Gray.anim"
//#define MOVE_STYLE      loco::MOVE_STYLE_RUN



// Mech robot
//#define LOCO_CLASS      generic_loco
//#define SKIN_FILE       "MECH_Gray_BIND.skingeom"
//#define ANIM_FILE       "NPC_Mech_Gray.anim"

// DR CRAY
//#define LOCO_CLASS      friendly_scientist_loco
//#define SKIN_FILE       "NPC_DrCray_03_Illuminati_Load0-3.skingeom"
//#define ANIM_FILE       "NPC_DrCray.anim"

// Scientist in game
//#define LOCO_CLASS      friendly_scientist_loco
//#define SKIN_FILE       "NPC_CIV_Friendly_03_Illuminati_Load0-3.skingeom"
//#define ANIM_FILE       "NPC_Civilian.anim"


// Scientist lip sync test
//#define LOCO_CLASS      friendly_scientist_loco
//#define SKIN_FILE       "sci_facetest.skingeom"
//#define ANIM_FILE       "sci_facetest.anim"

//#define LOD_01          (BODY_HIGH | HEAD_HIGH_TALK)
//#define LOD_02          (BODY_HIGH | HEAD_HIGH)
//#define LOD_03          (BODY_MEDIUM)
//#define LOD_04          (BODY_LOW)

//#define BODY_HIGH       (1<<0)
//#define BODY_MEDIUM     (1<<1)
//#define HEAD_HIGH_TALK  (1<<2)
//#define HEAD_HIGH       (1<<2)
//#define BODY_LOW        (1<<3)



// Mutant scientist
//#define LOCO_CLASS      mutant_sci_loco
//#define SKIN_FILE       "SCIMUT.skingeom"
//#define ANIM_FILE       "SCIMUT.anim"

// Gray
//#define LOCO_CLASS      gray_loco
//#define SKIN_FILE       "Gray_Edgar_BIND.skingeom"
//#define ANIM_FILE       "NPC_Gray_MHG.anim"
//#define MOVE_STYLE      loco::MOVE_STYLE_RUN

// Stage5
//#define LOCO_CLASS      stage5_loco
//#define SKIN_FILE       "HM_5_bindpose.skingeom"
//#define ANIM_FILE       "HM_5_bindpose.skingeom"


// Default move style
#ifndef MOVE_STYLE
#define MOVE_STYLE      loco::MOVE_STYLE_WALK
#endif

// ALL LODS ON
#ifndef LOD_01
#define LOD_01          (0xFFFFFFFFFFFFFFFF)
#endif

#ifndef LOD_02
#define LOD_02          (0xFFFFFFFFFFFFFFFF)
#endif

#ifndef LOD_03
#define LOD_03          (0xFFFFFFFFFFFFFFFF)
#endif

#ifndef LOD_04
#define LOD_04          (0xFFFFFFFFFFFFFFFF)
#endif




//==============================================================================
// LOADERS FOR PC
//==============================================================================

#ifdef TARGET_PC

static struct texture_loader : public rsc_loader
{
    texture_loader( void ) : rsc_loader( "TEXTURE", ".xbmp" ) {}

	//-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        texture* pTexture = new texture;
        ASSERT( pTexture );

        pTexture->m_Bitmap.Load( FP );
        
        return( pTexture );
    }

	//-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        texture* pTexture = (texture*)pData;

        vram_Register( pTexture->m_Bitmap );

        return( pTexture );
    }

	//-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        texture* pTexture = (texture*)pData;

        vram_Unregister( pTexture->m_Bitmap );

        delete pTexture;
    }

} s_Texture_Loader;


static struct cubemap_loader : public rsc_loader
{
    cubemap_loader( void ) : rsc_loader( "CUBEMAP", ".envmap" ) {}

	//-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        cubemap* pCubemap = new cubemap;
        ASSERT( pCubemap );

        for ( s32 i = 0; i < 6; i++ )
        {
            pCubemap->m_Bitmap[i].Load( FP );
        }

        return( pCubemap );
    }

	//-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        cubemap* pCubemap = (cubemap*)pData;

        for ( s32 i = 0; i < 6; i++ )
            vram_Register( pCubemap->m_Bitmap[i] );

        return( pCubemap );
    }

	//-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        cubemap* pCubemap = (cubemap*)pData;

        for ( s32 i = 0; i < 6; i++ )
        {
            vram_Unregister( pCubemap->m_Bitmap[i] );
        }

        delete pCubemap;
    }

} s_Cubemap_Loader;


static struct skin_loader : public rsc_loader
{
    //-------------------------------------------------------------------------
    
    skin_loader( void ) : rsc_loader( "SKIN GEOM", ".skingeom" ) {}

    //-------------------------------------------------------------------------
    
    virtual void* PreLoad ( X_FILE* FP )
    {
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------
    
    virtual void* Resolve ( void* pData ) 
    {
        fileio      File;
        skin_geom* pSkinGeom = NULL;

        File.Resolved( (fileio::resolve*)pData, pSkinGeom );

        return( pSkinGeom );
    }

    //-------------------------------------------------------------------------
    
    virtual void Unload( void* pData )
    {
        skin_geom* pSkinGeom = (skin_geom*)pData;
        ASSERT( pSkinGeom );
    
        delete pSkinGeom;
    }

} s_Skin_Geom_Loader;

#endif



//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
// MAIN
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================


//static cloth*       s_pCloth = NULL;


struct loco_inst
{
    // Defines
    enum counts
    {
        PHYSICS_INST_COUNT = 6,
    };
    
    // Data
    loco*               m_pLoco;
    s32                 m_MoveStyle;
    s32                 m_BlendMoveStyle;
    f32                 m_BlendMoveStyleAmount;
    skin_inst           m_SkinInst;
    skin_geom*          m_pGeom;
    
    s32                 m_iPhysicsInst;
    physics_inst        m_PhysicsInst[PHYSICS_INST_COUNT];
    
    u64                 m_LODMask;
    s32                 m_nActiveBones;
    const matrix4*      m_pMatrices;

public:
    loco_inst();
    ~loco_inst();
    void Init( loco* pLoco, const char* pSkinGeom, const char* pAnim );
    void Reset( xbool bClear = FALSE );
    void Kill( void );
    u64 GetLODMask( const matrix4& L2W );
    s32 GetNActiveBones( const u64& LODMask ) const;
    void Render( void );
    void Advance( f32 DeltaTime );
};

//==========================================================================

// Constructor
loco_inst::loco_inst()
{
    m_pLoco        = NULL;
    m_MoveStyle    = MOVE_STYLE;
    m_BlendMoveStyle = MOVE_STYLE;
    m_BlendMoveStyleAmount = 0.0f;
    m_pGeom        = NULL;
    m_LODMask       = 0xFFFFFFFFFFFFFFFF;
    m_nActiveBones  = -1;
    m_pMatrices     = NULL;
    m_iPhysicsInst  = 0;
}

//==========================================================================

loco_inst::~loco_inst()
{
    Kill();
}

//==========================================================================

// Initialize
void loco_inst::Init( loco* pLoco, const char* pSkinGeom, const char* pAnim )
{
    // Load and register geometry
    m_SkinInst.SetUpSkinGeom( pSkinGeom );
    m_pGeom = m_SkinInst.GetSkinGeom();

    // Init loco
    m_pLoco = pLoco;
    m_pLoco->OnInit( m_pGeom, pAnim);
    m_pLoco->SetYaw(R_180);
    m_pLoco->SetMoveStyle(loco::MOVE_STYLE_WALK);
    
    //m_pLoco->SetArriveDistSqr(25*25);

    //TEST!!
    m_pLoco->GetAimController().SetHorizLimits(-R_90, R_90);

    m_pLoco->SetStateAnimRate(loco::STATE_MOVE, 1.0f);
    m_pLoco->SetStateAnimRate(loco::STATE_IDLE, 1.0f);
    m_pLoco->SetStateAnimRate(loco::STATE_PLAY_ANIM, 1.0f);

    // Init physics
    //for( s32 i = 0; i < PHYSICS_INST_COUNT; i++ )
        //m_PhysicsInst[i].Init( m_hGeom.GetName() );

    // Reset loco
    Reset();
}

//==========================================================================

// Reset
void loco_inst::Reset( xbool bClear )
{
    s32 i;
    
    //m_pLoco->SetState( loco::STATE_IDLE );

    // Clear all the physics?
    if( bClear )
    {
        for( i = 0; i < PHYSICS_INST_COUNT; i++ )
            m_PhysicsInst[i].Kill();
            
        m_iPhysicsInst = 0;
    }

    // Init physics
    physics_inst& PhysicsInst = m_PhysicsInst[ m_iPhysicsInst ];
    PhysicsInst.Init( m_SkinInst.GetSkinGeomName(), TRUE );
    PhysicsInst.SetMatrices( m_pLoco->m_Player, m_pLoco->GetDeltaPos() );
    PhysicsInst.Activate();
    
    // Fling in the air?
    if( bClear == FALSE )
    {
        for( i = 0; i < PhysicsInst.GetNRigidBodies(); i++ )    
        {
            rigid_body& RigidBody = PhysicsInst.GetRigidBody( i );    
            RigidBody.GetLinearVelocity() +=  vector3( x_frand( -100.0f, 100.0f ),
                                                    x_frand( 600.0f, 900.0f ),
                                                    x_frand( -100.0f, 100.0f ) );
        }
    }
            
    // Next physics inst...
    if( ++m_iPhysicsInst >= PHYSICS_INST_COUNT )
        m_iPhysicsInst = 0;
        
        
    // Good test case
    //pLoco->m_Player.SetYaw(R_45);
    //pLoco->m_Player.ApplyCurrAnimDeltaYaw(R_27);


    // Setup ragdoll
//        pLoco->m_RagdollController.m_Ragdoll.Init(pSkinGeom, pAnim, HumanRagdoll);

    // Setup pos
    //m_pLoco->SetPosition(vector3(0,12*100,0));
}

//==========================================================================

// Cleanup
void loco_inst::Kill( void )
{
    // Cleanup physics
    for( s32 i = 0; i < PHYSICS_INST_COUNT; i++ )
        m_PhysicsInst[i].Kill();

    // Geom
    m_pGeom = NULL;

    // Loco
    if( m_pLoco )
    {
        delete m_pLoco;
        m_pLoco = NULL;
    }
}

//==========================================================================

static u64 LODMasks     [4] = {LOD_01, LOD_02, LOD_03, LOD_04};
static f32 LODScreenSize[4] = {10000,250,150,80};

// Returns LOD mask to use
u64 loco_inst::GetLODMask( const matrix4& L2W )
{
#if 0
    // TEMP FOR MP AVATAR
    m_SkinInst.SetVMeshMask( 1 );
    return m_SkinInst.GetLODMask( L2W );

#else
    s32 i;

    // Compute screen size of object
    const view* pView = eng_GetView();
    f32 ScreenSize = pView->CalcScreenSize( L2W.GetTranslation(), m_pGeom->m_BBox.GetRadius() );

    // Get correct LOD Mask based on screen size
    u64 Mask = (u64)-1;
    s32 iLOD = 0;
    for (i = 0; i < 4; i++ )
    {
        if( ScreenSize < LODScreenSize[i] )
            iLOD = i;
    }

    if(s_LOD != -1)
        iLOD = s_LOD;

    Mask = LODMasks[iLOD];

    // Compute verts and faces
    s32 nVerts = 0;
    s32 nFaces = 0;
    for (i = 0; i < m_pGeom->m_nMeshes; i++)
    {
        if(Mask & (1<<i))
        {
            nVerts += m_pGeom->m_pMesh[i].nVertices;
            nFaces += m_pGeom->m_pMesh[i].nFaces;
        }
    }

    //x_printfxy(1,5, "LOD:%d",   iLOD);
    //x_printfxy(1,6, "Verts:%d", nVerts);
    //x_printfxy(1,7, "Faces:%d", nFaces);

    return Mask;
#endif
}

//==========================================================================

// Returns active bones to use for player
s32 loco_inst::GetNActiveBones( const u64& LODMask ) const
{
    // Count bones used by geometry and tell the animation player
    // (bones in sorted into heirarchical order so we can just keep the max)
    s32 nActiveBones = 0;
    for (s32 i = 0; i < m_pGeom->m_nMeshes; i++)
    {
        // Is this mesh being used?
        if(LODMask & (1<<i))
        {
            // Update max count
            nActiveBones = MAX(nActiveBones, m_pGeom->m_pMesh[i].nBones);
        }
    }

    return nActiveBones;
}

//==========================================================================

// Renders
void loco_inst::Render( void )
{
    // Compute LOD mask
    matrix4 L2W;
    L2W.Identity();
    L2W = m_pLoco->GetL2W();

    m_LODMask      = GetLODMask(L2W);
    m_nActiveBones = GetNActiveBones(m_LODMask);

    // Get matrices
    m_pLoco->m_Player.SetNActiveBones(m_nActiveBones);
    if(m_nActiveBones)
        m_pMatrices = m_pLoco->m_Player.GetBoneL2Ws();

    // Call render system
    if((s_bDrawSkin) && (m_pMatrices) && (m_nActiveBones))
    {
        u32 Flags = 0;
        if(s_bClipped) 
            Flags |= render::CLIPPED;

        //temp
        m_SkinInst.Render( m_pMatrices,
                           m_pMatrices,
                           m_nActiveBones,
                           render::CLIPPED,
                           m_LODMask,
                           XCOLOR_WHITE );


        // Render physics skin inst
        if( !s_bDrawPhysics )
        {                                        
            for( s32 i = 0; i < loco_inst::PHYSICS_INST_COUNT; i++ )
                m_PhysicsInst[i].Render( xcolor(128,128,128,255 ) );
        }
    }

    /*
    // Render bone bboxes
    ASSERT(m_pGeom->m_nBones == m_pLoco->m_Player.GetNBones());
    for (s32 i = 0; i < m_pGeom->m_nBones; i++)
    {
        geom::bone& Bone = m_pGeom->m_pBone[i];

        draw_SetL2W(pMatrices[i]);
        draw_BBox(Bone.BBox, XCOLOR_WHITE);
    }
    */
}

//==========================================================================

// Advances
void loco_inst::Advance( f32 DeltaTime )
{
    ASSERT(m_pLoco);
    m_pLoco->OnAdvance(DeltaTime);

    // Move the cloth
    vector3 Bottom = m_pLoco->GetPosition();
    vector3 Top    = Bottom;
    Top.GetY() += 2.0f * 100;

    //s_pCloth->ApplyCappedCylinderColl(Bottom, Top, 0.75f * 100);
}


//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;

f32    g_BlurOffset;
xcolor g_BlurColor;

#ifdef TARGET_PC
#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list
    const char*     DataPath = "";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif

static loco_inst*   s_LocoInsts = NULL;
static s32          s_NLocoInsts = 0;
                    
static s32          s_NActiveLocoInst = 0;
static loco_inst*   s_pActiveLocoInst = NULL;

static vector3      s_MoveAtPoint(0,0,-400);
static radian       s_DesYaw = R_180;
static vector3      s_LookAtPoint(0,0,-100);
static xbool        s_bRandDest = FALSE;//TRUE;
int g_right_stick_swap_xy = FALSE;
static xbool        s_bShowHeirarchy = FALSE;


//==============================================================================

static void Initialize( void )
{
    eng_Init();
    g_IoMgr.Init();
    
    render::Init();

    // Init animation system
    anim_event::Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(0, 500, -1000) );
    View.LookAtPoint( vector3( 0,  200,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );


    // Lip sync test
    if(!s_bLocoTest)
    {
        View.SetPosition( vector3(0, 160, -80) );
        View.LookAtPoint( vector3( 0,  160,  0) );
    }

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );

    //eng_SetBackColor( XCOLOR_WHITE);


    if(!s_bLocoTest)
        s_MoveAtPoint = s_LookAtPoint;


    g_RscMgr.Init();
    g_PhysicsMgr.Init();
    g_PhysicsMgr.m_Settings.m_bUsePolycache = FALSE;    // Use ground plane

#ifdef TARGET_PC    
#define ROOT_DIR        "C:\\GameData\\A51\\Release\\PC"
#endif

#ifdef TARGET_PS2    
#define ROOT_DIR        "C:\\GameData\\A51\\Release\\PS2"
#endif


    // Setup resource manager
    g_RscMgr.SetRootDirectory( ROOT_DIR );
    g_RscMgr.SetOnDemandLoading( FALSE );//TRUE );//FALSE );

    // Create loco
    LOCO_CLASS *pLoco = new LOCO_CLASS();
    ASSERT(pLoco);

    // Init loco
    s_LocoInsts = new loco_inst[16];
    ASSERT( s_LocoInsts );
    //s_LocoInsts[s_NLocoInsts++].Init(pLoco,
        //xfs("%s\\%s", ROOT_DIR, SKIN_FILE),
        //ANIM_FILE);
    s_LocoInsts[s_NLocoInsts++].Init( pLoco, SKIN_FILE, ANIM_FILE );

    // Disable strafing
    //pLoco->SetAllowMotion( loco::MOTION_LEFT,  FALSE );
    //pLoco->SetAllowMotion( loco::MOTION_RIGHT, FALSE );
    //pLoco->SetAllowMotion( loco::MOTION_BACK, FALSE );
    
    // Init look at
    s_LookAtPoint += pLoco->GetEyeOffset();
    pLoco->SetLookAt(s_LookAtPoint);

    // Create cloth
    matrix4 L2W;
    L2W.Identity();
    L2W.SetTranslation(vector3(0,3*100,0));
    //s_pCloth = new cloth();
    //ASSERT(s_pCloth);

    // Initialize cloth
    //s_pCloth->Init(xfs("%s\\M_flag_000.rigidgeom", ROOT_DIR), L2W);

    // Set starting instance
    s_NActiveLocoInst = 0;

    // Lookup instance
    s_pActiveLocoInst = &s_LocoInsts[s_NActiveLocoInst];

    // Init test audio
    //g_AudioManager.Init();
    //g_AudioManager.LoadPackage( xfs("%s\\%s", ROOT_DIR, "CF_DrYates.audiopkg") );

}

//=========================================================================

static void Shutdown( void )
{
    //g_AudioManager.Kill();
    //g_IoMgr.Kill();   // Hangs on the pc!!!

    // Kill loco instances
    delete [] s_LocoInsts;
    s_LocoInsts = NULL;

    //s_pCloth->Kill();
    //delete s_pCloth;
    render::Kill();

    g_PhysicsMgr.Kill();
    g_RscMgr.UnloadAll();
    g_RscMgr.Kill();

}

//=========================================================================

#define JOY_OBJECT_PITCH        INPUT_PS2_STICK_RIGHT_Y
#define JOY_OBJECT_YAW          INPUT_PS2_STICK_RIGHT_X
#define JOY_OBJECT_SPEEDUP      INPUT_PS2_BTN_L2
#define JOY_OBJECT_RESET        INPUT_PS2_BTN_CIRCLE
#define JOY_OBJECT_SHIFT        INPUT_PS2_BTN_L1

#define JOY_OBJECT_PLAY_ANIM    INPUT_PS2_BTN_CROSS
#define JOY_OBJECT_PREV_ANIM    INPUT_PS2_BTN_L_LEFT
#define JOY_OBJECT_NEXT_ANIM    INPUT_PS2_BTN_L_RIGHT

#define JOY_OBJECT_PREV         INPUT_PS2_BTN_L_LEFT
#define JOY_OBJECT_NEXT         INPUT_PS2_BTN_L_RIGHT

#define JOY_OBJECT_MOVE_X       INPUT_PS2_STICK_LEFT_X
#define JOY_OBJECT_MOVE_Z       INPUT_PS2_STICK_LEFT_Y
#define JOY_OBJECT_LOOK_X       INPUT_PS2_STICK_RIGHT_X
#define JOY_OBJECT_LOOK_Z       INPUT_PS2_STICK_RIGHT_Y
#define JOY_OBJECT_LOOK_UP      INPUT_PS2_BTN_R1
#define JOY_OBJECT_LOOK_DOWN    INPUT_PS2_BTN_R2
#define JOY_OBJECT_MOVE_STYLE   INPUT_PS2_BTN_SQUARE
#define KEY_OBJECT_MOVE_STYLE   INPUT_KBD_M

#define JOY_OBJECT_MOVE_UP      INPUT_PS2_BTN_R1
#define JOY_OBJECT_MOVE_DOWN    INPUT_PS2_BTN_R2

#define MSE_OBJECT_SPEEDUP      INPUT_MOUSE_BTN_R   
#define MSE_OBJECT_MOVE         INPUT_MOUSE_BTN_C   
#define MSE_OBJECT_MOVE_VERT    INPUT_MOUSE_Y_REL
#define MSE_OBJECT_MOVE_HORIZ   INPUT_MOUSE_X_REL
#define MSE_OBJECT_PITCH        INPUT_MOUSE_Y_REL
#define MSE_OBJECT_PITCH        INPUT_MOUSE_Y_REL
#define MSE_OBJECT_YAW          INPUT_MOUSE_X_REL
#define MSE_OBJECT_ZOOM         INPUT_MOUSE_WHEEL_REL

#define KEY_OBJECT_SPEEDUP      INPUT_KBD_LSHIFT   
#define KEY_OBJECT_FORWARD      INPUT_KBD_W   
#define KEY_OBJECT_BACK         INPUT_KBD_S   
#define KEY_OBJECT_LEFT         INPUT_KBD_A   
#define KEY_OBJECT_RIGHT        INPUT_KBD_D   
#define KEY_OBJECT_UP           INPUT_KBD_R   
#define KEY_OBJECT_DOWN         INPUT_KBD_F   

#define KEY_OBJECT_SET_MOVE_AT  INPUT_KBD_SPACE   
#define KEY_OBJECT_SET_LOOK_AT  INPUT_KBD_C


//=========================================================================

static xbool HandleInput( f32 DeltaTime )
{
    //
    // First lets go throw all the queue.
    //
#ifndef TARGET_PS2
    while( input_UpdateState() )
    {
        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return FALSE;
    };
#else
    input_UpdateState();
#endif

    // Move camera on PS2?
    if( input_IsPressed(JOY_OBJECT_SHIFT) )
    {
        f32    T;

        // Joy controller input
        f32 S = 5.0f * DeltaTime;

        //if(!m_Loco.HasMoveAnims())                    S *= 0.33f;
        if( input_IsPressed( JOY_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        T = S*200;
        if( input_IsPressed( JOY_OBJECT_MOVE_UP  ) )  View.Translate( vector3( 0, T,0 ), view::VIEW );
        if( input_IsPressed( JOY_OBJECT_MOVE_DOWN ) )  View.Translate( vector3( 0,-T,0 ), view::VIEW );

        T = S*500;
#ifdef TARGET_PC    
        View.Translate( vector3(0, 0, T * -input_GetValue( JOY_OBJECT_MOVE_Z ) ), view::VIEW );
        View.Translate( vector3(T * input_GetValue( JOY_OBJECT_MOVE_X ),0,0), view::VIEW );
#else
        View.Translate( vector3(0, 0, T * input_GetValue( JOY_OBJECT_MOVE_Z ) ), view::VIEW );
        View.Translate( vector3(T * -input_GetValue( JOY_OBJECT_MOVE_X ), 0, 0 ), view::VIEW );
        
        View.RotateY( -T * 0.001f * input_GetValue( JOY_OBJECT_LOOK_X ), view::WORLD );
        View.RotateX( T * 0.001f * input_GetValue( JOY_OBJECT_LOOK_Z ), view::VIEW );
        
#endif

        // Keyboard input
        T = S * 350;
        if( input_IsPressed( KEY_OBJECT_FORWARD ) )  View.Translate( vector3( 0, 0, T), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_BACK   ) )  View.Translate( vector3( 0, 0,-T), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_LEFT   ) )  View.Translate( vector3( T, 0, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_RIGHT  ) )  View.Translate( vector3(-T, 0, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_UP     ) )  View.Translate( vector3( 0, T, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_DOWN   ) )  View.Translate( vector3( 0,-T, 0), view::VIEW );

        // Mouse input
        if( input_IsPressed( MSE_OBJECT_SPEEDUP ) )  S *= 2.0f;

        T = S * 200.0f;
        if( input_IsPressed( MSE_OBJECT_MOVE ) )
        {
            View.Translate( vector3( input_GetValue(MSE_OBJECT_MOVE_HORIZ)   * T, 0, 0), view::VIEW );
            View.Translate( vector3( 0, input_GetValue(MSE_OBJECT_MOVE_VERT) * T, 0), view::VIEW );
        }
        View.Translate( vector3( 0, 0, input_GetValue(MSE_OBJECT_ZOOM) * S * 10000.0f), view::VIEW );
    }
    else
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
    #ifdef TARGET_PC
        f32    S = 1.6125f * 60 * 5 * DeltaTime;
        f32    R = 0.0005f  * 120 * 10 * DeltaTime;
    #else
        f32    S = 1.6125f * 60 * DeltaTime * 20.0f;
        f32    R = 0.0005f  * 120 * DeltaTime;
    #endif

        if( input_IsPressed( INPUT_KBD_LSHIFT ) ) S *= 2.0f;

        if( input_IsPressed( INPUT_KBD_ESCAPE ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_KBD_W      ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_S      ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_A      ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_D      ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_R      ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_F      ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );     
        if( input_IsPressed( INPUT_MOUSE_BTN_R ) )
        {          
            Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
            Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
        }            
        View.SetRotation( radian3(Pitch,Yaw,0) );

        //if( input_IsPressed( INPUT_PS2_BTN_L_UP   ) )  View.Translate( vector3( 0, 0, S*0.2f), view::VIEW );
        //if( input_IsPressed( INPUT_PS2_BTN_L_DOWN ) )  View.Translate( vector3( 0, 0,-S*0.2f), view::VIEW );
        //if( input_IsPressed( INPUT_PS2_BTN_L_LEFT ) )  View.Translate( vector3( S*0.2f, 0, 0), view::VIEW );
        //if( input_IsPressed( INPUT_PS2_BTN_L_RIGHT ) )  View.Translate( vector3(-S*0.2f, 0, 0), view::VIEW );


        //if( input_IsPressed( INPUT_PS2_BTN_L1 ) )  View.Translate( vector3( 0,  S*0.1f, 0), view::VIEW );
        //if( input_IsPressed( INPUT_PS2_BTN_L2 ) )  View.Translate( vector3( 0, -S*0.1f,  0), view::VIEW );


    #ifdef TARGET_PC
        // Update loco?
        if(s_bLocoTest)
        {
            //
            // Compute the new fnial destination
            //
            if( input_IsPressed( INPUT_KBD_SPACE ) )
            {
                vector3 RayDir = View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() ) * 100000.0f;
                vector3 RayPos = View.GetPosition();
                plane   Plane( vector3(0,1,0), 0 );
                f32     t;
                if( Plane.Intersect( t, RayPos, RayDir ) )
                {
                    s_MoveAtPoint =  RayPos + RayDir*t;
                }
            }
        }

        if( input_IsPressed( INPUT_KBD_C ) )
        {
            vector3 RayDir = View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() ) * 100000.0f;
            vector3 RayPos = View.GetPosition();
            plane   Plane( vector3(0,1,0), 0 );
            f32     t;
            if( Plane.Intersect( t, RayPos, RayDir ) )
            {
                vector3 ColPos = RayPos + RayDir*t;
                s_LookAtPoint.Set( ColPos.GetX(), s_LookAtPoint.GetY(), ColPos.GetZ() );
            }
        }
    #endif

        if( input_WasPressed( INPUT_KBD_H      ) )  s_bShowHeirarchy ^= TRUE;

        // Update loco?
        if(s_bLocoTest)
        {
            if( input_IsPressed( INPUT_KBD_T      ) )  s_LookAtPoint.GetY() += 800*DeltaTime;
            if( input_IsPressed( INPUT_KBD_G      ) )  s_LookAtPoint.GetY() -= 800*DeltaTime;
            if( input_WasPressed( INPUT_KBD_U     ) )
            {
                s_MoveAtPoint.GetX() = s_LookAtPoint.GetX() + x_frand(-100, 100);
                s_MoveAtPoint.GetZ() = s_LookAtPoint.GetZ() + x_frand(-100, 100);
            }        

            if( input_WasPressed( INPUT_KBD_Q ) )
                s_bRandDest ^= TRUE;
    
            if( input_WasPressed( INPUT_PS2_BTN_CROSS ) )
                s_bRandDest ^= TRUE;
        }
        
        if( input_WasPressed( INPUT_PS2_BTN_SQUARE ) )
        {
            if(++s_LOD == 4)
                s_LOD = -1;
        }

        // Update loco on ps2?
        if(s_bLocoTest)
        {
            // Toggle move style
            if( (input_WasPressed( INPUT_PS2_BTN_CIRCLE)) || ( input_WasPressed( INPUT_KBD_M )) )
            {
                // Goto next valid move style
                s32 StartMoveStyle = s_pActiveLocoInst->m_MoveStyle;
                s32 MoveStyle      = s_pActiveLocoInst->m_MoveStyle;
                do
                {
                    if(++MoveStyle >= loco::MOVE_STYLE_COUNT)
                        MoveStyle = 0;
                }
                while(     (MoveStyle != StartMoveStyle) &&
                            (s_pActiveLocoInst->m_pLoco->IsValidMoveStyle((loco::move_style)MoveStyle) == FALSE) );

                // Allow/disable aim styles so that we can toggle through them!
                if(       (MoveStyle == loco::MOVE_STYLE_RUNAIM)
                        ||  (MoveStyle == loco::MOVE_STYLE_CROUCHAIM) )
                {
                    s_pActiveLocoInst->m_pLoco->SetUseAimMoveStyles(TRUE);
                }
                else
                {
                    s_pActiveLocoInst->m_pLoco->SetUseAimMoveStyles(FALSE);
                }

                // Set it
                s_pActiveLocoInst->m_MoveStyle = MoveStyle;
                s_pActiveLocoInst->m_pLoco->SetMoveStyle((loco::move_style)MoveStyle);

                // Init look at
                s_LookAtPoint.GetY() = s_pActiveLocoInst->m_pLoco->GetEyeOffset().GetY();
            }
            
            // Toggle blend move style
            if( (input_WasPressed( INPUT_PS2_BTN_SQUARE )) || ( input_WasPressed( INPUT_KBD_N )) )
            {
                // Goto next valid move style
                s32 StartMoveStyle = s_pActiveLocoInst->m_BlendMoveStyle;
                s32 MoveStyle      = s_pActiveLocoInst->m_BlendMoveStyle;
                do
                {
                    if(++MoveStyle >= loco::MOVE_STYLE_COUNT)
                        MoveStyle = 0;
                }
                while(     (MoveStyle != StartMoveStyle) &&
                    (s_pActiveLocoInst->m_pLoco->IsValidMoveStyle((loco::move_style)MoveStyle) == FALSE) );

                // Set it
                s_pActiveLocoInst->m_BlendMoveStyle = MoveStyle;
                s_pActiveLocoInst->m_pLoco->SetBlendMoveStyle((loco::move_style)MoveStyle);
            }
            
            // Inc blend move style amount
            if( (input_WasPressed( INPUT_PS2_BTN_TRIANGLE )) || ( input_WasPressed( INPUT_KBD_B )) )
            {
                s_pActiveLocoInst->m_BlendMoveStyleAmount += 0.1f;
                if( s_pActiveLocoInst->m_BlendMoveStyleAmount >= 1.1f )
                    s_pActiveLocoInst->m_BlendMoveStyleAmount = 0.0f;
                    
                s_pActiveLocoInst->m_pLoco->SetBlendMoveStyleAmount( x_max( 0.0f, x_min( 1.0f, s_pActiveLocoInst->m_BlendMoveStyleAmount) ) );
            }
        }



        // Exact?
        if( input_WasPressed( INPUT_KBD_E ) )
        {
            s_bExact ^= TRUE;
        }
        
        // Update exact look/move
        s_pActiveLocoInst->m_pLoco->SetExactMove( s_bExact );
        s_pActiveLocoInst->m_pLoco->SetExactLook( s_bExact );

        // Reset?
        if( ( input_WasPressed( INPUT_KBD_Z ) ) || ( input_WasPressed( INPUT_PS2_BTN_START ) ) )
            s_pActiveLocoInst->Reset();
        
        // Reset with clear?
        if( ( input_WasPressed( INPUT_KBD_X ) ) || ( input_WasPressed( INPUT_PS2_BTN_SELECT ) ) )
            s_pActiveLocoInst->Reset( TRUE );
    
        // Toggle creature
        if( input_WasPressed( INPUT_KBD_N ) )
        {
            // Next instance
            if(++s_NActiveLocoInst >= s_NLocoInsts)
                s_NActiveLocoInst = 0;

            // Setup active pointer
            s_pActiveLocoInst = &s_LocoInsts[s_NActiveLocoInst];
        }
    
        loco* pLoco = s_pActiveLocoInst->m_pLoco;

        if( input_WasPressed( INPUT_KBD_1 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_HEAD_BACK);
        if( input_WasPressed( INPUT_KBD_2 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_SHOULDER_LEFT_BACK);
        if( input_WasPressed( INPUT_KBD_3 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_SHOULDER_RIGHT_BACK);
        if( input_WasPressed( INPUT_KBD_4 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_TORSO_BACK);
        if( input_WasPressed( INPUT_KBD_5 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_TORSO_FRONT);
        if( input_WasPressed( INPUT_KBD_6 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_SHOULDER_LEFT_FRONT);
        if( input_WasPressed( INPUT_KBD_7 ) )
            pLoco->PlayAdditiveAnim(loco::ANIM_ADD_IMPACT_SHOULDER_RIGHT_FRONT);

        if( input_WasPressed( INPUT_KBD_B ) )
            pLoco->PlayLipSyncAnim(loco::ANIM_LIP_SYNC_TEST, "CF_DrYates_07", loco::ANIM_FLAG_MASK_TYPE_FACE);
    
        if( input_WasPressed( INPUT_PS2_BTN_CROSS ) )
            pLoco->PlayLipSyncAnim(loco::ANIM_LIP_SYNC_TEST, "CF_DrYates_07", loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY);

        if( input_WasPressed( INPUT_PS2_BTN_TRIANGLE ) )
            pLoco->PlayLipSyncAnim(loco::ANIM_LIP_SYNC_TEST, "CF_DrYates_07", loco::ANIM_FLAG_MASK_TYPE_FULL_BODY);

    #define RELOAD_SHOOT_MASKS      LOCO_CLASS ## ::s_ReloadShootBoneMasks

    /*
        // Play reload/shoot anims
        if( input_WasPressed( INPUT_KBD_NUMPAD0 ) )
        {
            if(pLoco->GetState() == loco::STATE_IDLE)
                pLoco->PlayMaskedAnim(loco::ANIM_SHOOT_SMP, RELOAD_SHOOT_MASKS, 0.1f);
            else
                pLoco->PlayMaskedAnim(loco::ANIM_LOCO_SHOOT_SMP, RELOAD_SHOOT_MASKS, 0.1f);
        }
    */

        if( input_WasPressed( INPUT_KBD_NUMPAD1 ) )
            pLoco->PlayAnim(loco::ANIM_MELEE_SHORT, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER);
        if( input_WasPressed( INPUT_KBD_NUMPAD2 ) )
            pLoco->PlayAnim(loco::ANIM_MELEE_LEAP, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER);

    /*
        if( input_WasPressed( INPUT_KBD_NUMPAD1 ) )
            pLoco->PlayMaskedAnim(loco::ANIM_RELOAD_SMP,     RELOAD_SHOOT_MASKS, 0.1f);
        if( input_WasPressed( INPUT_KBD_NUMPAD2 ) )
            pLoco->PlayMaskedAnim(loco::ANIM_SHOOT_SHOTGUN,  RELOAD_SHOOT_MASKS, 0.1f);
        if( input_WasPressed( INPUT_KBD_NUMPAD3 ) )
            pLoco->PlayMaskedAnim(loco::ANIM_RELOAD_SHOTGUN, RELOAD_SHOOT_MASKS, 0.1f);
        if( input_WasPressed( INPUT_KBD_NUMPAD4 ) )
            pLoco->PlayMaskedAnim(loco::ANIM_SHOOT_SNIPER,   RELOAD_SHOOT_MASKS, 0.1f);
        if( input_WasPressed( INPUT_KBD_NUMPAD5 ) )
            pLoco->PlayMaskedAnim(loco::ANIM_RELOAD_SNIPER,  RELOAD_SHOOT_MASKS, 0.1f);
    */
        // Play an impact anim?
        if( input_WasPressed( INPUT_KBD_I ) )
            pLoco->PlayAnim("ATTACK_CHARGE_MISS", 0.0f, loco::ANIM_FLAG_TURN_OFF_AIMER);

    #ifdef TARGET_PS2
        // Update loco on ps2?
        if(s_bLocoTest)
        {
            // Is input coming from controller?
            vector3 Input;
            Input.GetX() = -input_GetValue(INPUT_PS2_STICK_LEFT_X);
            Input.GetY() = 0;
            Input.GetZ() = input_GetValue(INPUT_PS2_STICK_LEFT_Y);
            if(Input.LengthSquared() > 0)
            {
                Input *= 200.0f;
                vector3 Move = s_pActiveLocoInst->m_pLoco->GetPosition() + Input;

                s_LookAtPoint = Move;
                s_MoveAtPoint = Move;

                s_pActiveLocoInst->m_pLoco->SetState(loco::STATE_MOVE);
            }
        }
    #endif

    #ifdef TARGET_PC
        if( input_WasPressed( INPUT_KBD_P ) )
        {
            if( s_Pause == 1 ) s_Pause = 0;
            else s_Pause = 1;
        }

        if( input_WasPressed( INPUT_KBD_O ) )
        {
            if( s_SlowMo == 1 ) s_SlowMo = 0.25f;
            else s_SlowMo = 1;
        }

        if( input_WasPressed( INPUT_KBD_L ) )
        {
            s_SingleStep += s_Step;
        }
    #endif


    }
    
    return( TRUE );
}

//==============================================================================

static void AppRender( void )
{
    CONTEXT("Render");

#ifdef TARGET_PS2    
    // split vram into banks for the different texture types
    vram_Flush();
    vram_AllocateBank( 24 );     // detail maps
    vram_AllocateBank( 24 );     // specular maps
    vram_AllocateBank( 24 );     // spotlight/shadow maps
#endif

    // set up the environment map
    cubemap::handle Handle;
    Handle.SetName( "DefaultEnvMap.envmap" );
    render::SetAreaCubeMap( Handle );


    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    //if(s_bLocoTest)
    {
        eng_Begin( "GridBoxMarker" );
        {
            draw_ClearL2W();
            draw_Grid( vector3( -5000,   0,    -5000), 
                       vector3(10000,  0,    0), 
                       vector3( 0,   0, 10000), 
                       xcolor ( 0,128,  0), 32 );
        }
        eng_End();
    }

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    if(s_bDrawLoco)
    {
        eng_Begin( "Skeleton" );

        s_pActiveLocoInst->m_pLoco->RenderInfo( TRUE,       // bRenderLoco
                                                FALSE,      // bLabelLoco
                                                FALSE,      // bRenderSkeleton
                                                FALSE );   // bLabelSkeleton

        eng_End();
    }

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    if( 1 )
    {
        // Setup skinned lighting
        eng_Begin("skin_inst");
        g_LightMgr.ClearLights();
        g_LightMgr.BeginLightCollection();
        g_LightMgr.AddDynamicLight( vector3( 0,1000,00 ), // Pos
            xcolor(128,128,128,128),    // Color
            100*10000.0f,       // Radius
            0.5f,               // Intensity
            TRUE);             // CharOnly
        g_LightMgr.AddDynamicLight( vector3( 1000,1000,00 ), // Pos
            xcolor(128,128,128,128),    // Color
            100*10000.0f,       // Radius
            0.5f,               // Intensity
            TRUE);             // CharOnly
        g_LightMgr.AddDynamicLight( vector3( 00,1000,1000 ), // Pos
            xcolor(128,128,128,128),    // Color
            100*10000.0f,       // Radius
            0.5f,               // Intensity
            TRUE);             // CharOnly

        render::BeginNormalRender();
    
        s_pActiveLocoInst->Render();
        
        g_LightMgr.EndLightCollection();
        render::EndNormalRender();
        eng_End();
    }


    //==---------------------------------------------------
    //  Render cloth
    //==---------------------------------------------------
    //eng_Begin("Cloth");
    //s_pCloth->RenderGeometry();
    //s_pCloth->RenderSkeleton();
    //eng_End();


    //==---------------------------------------------------
    //  Render physics
    //==---------------------------------------------------
#ifndef X_RETAIL
    {
        eng_Begin( "Physics" );
        //if( s_bDrawPhysics )
        {
            //g_PhysicsMgr.m_Settings.m_bDebugRender = TRUE;
            //g_PhysicsMgr.DebugRender();
            //g_PhysicsMgr.DebugRenderCollisions();
            //g_PhysicsMgr.DebugRenderInstances( TRUE );
        }
                    
        eng_End();
    }
#endif
    
    x_printfxy( 2, 20, "ActiveConstraints:%d", g_PhysicsMgr.m_Profile.m_nActiveConstraints );
    x_printfxy( 2, 21, "ActiveCollisions :%d", g_PhysicsMgr.m_Profile.m_nCollisions );
    
    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    //if(s_bLocoTest)
    {
        eng_Begin( "Rect" );

        //rect Rect ( d3deng_GetABSMouseX()-32, d3deng_GetABSMouseY()-32, d3deng_GetABSMouseX()+32, d3deng_GetABSMouseY()+32);
        //draw_Rect( Rect );

        bbox BBox;

        BBox.Min = s_MoveAtPoint - vector3(50,0,50);
        BBox.Max = s_MoveAtPoint + vector3(50,0,50);

        draw_BBox       ( BBox );
        draw_Marker( s_MoveAtPoint );

        BBox.Min = s_LookAtPoint - vector3(50,0,50);
        BBox.Max = s_LookAtPoint + vector3(50,0,50);

        draw_BBox       ( BBox, xcolor(0,0,255,255));
        draw_Marker( s_LookAtPoint, xcolor(0,0,255,255) );
        draw_Marker( s_LookAtPoint*vector3(1,0,1), xcolor(0,255,0,255) );

    
        eng_End();    
    }
}

//==============================================================================

static void AppAdvance( f32 DeltaTime )
{
    CONTEXT("Advance");

    // Lookup loco
    loco* pLoco = s_pActiveLocoInst->m_pLoco;
    ASSERT(pLoco);

    // Cap incase we are in the debugger
    if(DeltaTime > (1.0f / 5.0f))
        DeltaTime = (1.0f / 5.0f);

    if(s_bLocoTest)
        pLoco->SetLookAt(s_LookAtPoint);
    else
    {
        //pLoco->SetLookAt(View.GetPosition(), FALSE);
        pLoco->SetEyesLookAt(View.GetPosition());
    }

    pLoco->SetMoveAt(s_MoveAtPoint);
    pLoco->SetMoveStyle( (loco::move_style)s_pActiveLocoInst->m_MoveStyle );
    pLoco->SetBlendMoveStyle( (loco::move_style)s_pActiveLocoInst->m_BlendMoveStyle);
    pLoco->SetBlendMoveStyleAmount( s_pActiveLocoInst->m_BlendMoveStyleAmount );

    //
    // Advance the Logic 
    //

    DeltaTime = DeltaTime * s_Pause * s_SlowMo + s_SingleStep;
    s_SingleStep = 0;

    // Update audio
    //g_AudioManager.Update(DeltaTime);

    // Run at PS2 speed
    static f32 AccumTime = 0;
    AccumTime += DeltaTime;
    if(AccumTime >= s_Step)
    {
        AccumTime -= s_Step;
        DeltaTime = s_Step;
    }
    else
        DeltaTime = 0;


//    x_printfxy( 50, 0, "Pause : %f ", Pause );
//    x_printfxy( 50, 1, "Pause : %f ", SlowMo );

    s_pActiveLocoInst->Advance(DeltaTime);

    //TEMP
    g_PhysicsMgr.Advance( DeltaTime );
    
    //s_pCloth->Advance(DeltaTime);
}


//==============================================================================

void ShowHierarchyTree( s32& x, s32& y, const anim_group* pAnimGroup, s32 iBone )
{ 
    // Compute indent
    s32 Indent  = 0;
    s32 iParent = pAnimGroup->GetBone(iBone).iParent;
    while(iParent != -1)
    {
        Indent++;
        iParent = pAnimGroup->GetBone(iParent).iParent;
        x_DebugMsg(" ");
    }

    // Show bone
    x_printfxy(x + Indent,y++, "%s\n", pAnimGroup->GetBone(iBone).Name);

    // Recurse on children
    for (s32 i = 0; i < pAnimGroup->GetNBones(); i++)
    {
        iParent = pAnimGroup->GetBoneParent(i);
        ASSERT(iParent < i);
        if(iParent == iBone)
        {
            ShowHierarchyTree(x,y, pAnimGroup, i);
        }
    }
}

#ifdef TARGET_PS2
void    eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &INT, f32 &FPS);
#endif


void AppMain( s32, char** )
{
    Initialize();
    xtimer Timer;
    Timer.Start();

    xtimer LogicTimer;
    xtimer RenderTimer;

    //s32 EarID = g_AudioManager.CreateEar();

    while( TRUE )
    {
        LogicTimer.Reset();
        RenderTimer.Reset();
        
        f32 Near, Far;
        View.GetZLimits(Near, Far);
        //g_AudioManager.SetEar(EarID, View.GetW2V(), Near, Far, 1.0f);

        eng_MaximizeViewport( View );
        eng_SetView         ( View );

        f32 DeltaTime = Timer.TripSec();


        if( !HandleInput( DeltaTime ) )
            break;
/*
        if( 0 )
        {
            irect Rect( 128,0, 128+128,128 );

            View2.SetPosition( s_LookAtPoint + vector3( 0,100,0) );//s_HazmatLoco.GetPosition() + vector3( 0, 100, 300 ) );
            View2.LookAtPoint( s_pActiveLocoInst->m_pLoco->GetPosition() + vector3( 0, 100,   0 ) );
            View2.SetViewport( Rect.l, Rect.t, Rect.r, Rect.b );
            eng_SetView         ( View2, 0 );
            eng_ActivateView    ( 0 );

            eng_Begin( "Clear");
            draw_GouraudRect( Rect,XCOLOR_BLACK,XCOLOR_BLACK,XCOLOR_BLACK,XCOLOR_BLACK, FALSE );
            draw_ClearZBuffer( Rect );
            eng_End();

            RenderTimer.Start();
            Render();
            RenderTimer.Stop();

        }
*/

        // Lookup loco
        loco* pLoco = s_pActiveLocoInst->m_pLoco;

        if( (s_bRandDest) && (pLoco->IsAtDestination()) )
        {
            f32    Dist = x_frand(500,600);
            //s_DesYaw += x_frand(-R_120, R_120);
            s_DesYaw += x_frand(R_180 - R_20, R_180 + R_20);
            s_MoveAtPoint.GetX() = s_LookAtPoint.GetX() + (Dist * x_sin(s_DesYaw));
            s_MoveAtPoint.GetZ() = s_LookAtPoint.GetZ() + (Dist * x_cos(s_DesYaw));
        }

        //draw_Label(pLoco->GetPosition(), XCOLOR_WHITE, "nBones:%d", pLoco->m_Player.GetNActiveBones() );

        // Show times
        s32 x=1, y=1;

#ifdef TARGET_PS2
        //s32 Count;
        //f32 CPU, GS, INT, FPS;
        //eng_GetStats(Count, CPU, GS, INT, FPS);
        //x_printfxy(x,y++, "CPU_LOGIC:%.2f",  LogicTimer.ReadMs());
        //x_printfxy(x,y++, "CPU_RENDER:%.2f", RenderTimer.ReadMs());
        //CPU = LogicTimer.ReadMs() + RenderTimer.ReadMs();
        //x_printfxy(x,y++, "CPU:%.2f", CPU);
        //x_printfxy(x,y++, "GS :%.2f", GS);
#endif


        // Show useful debug info
        if( s_bShowAnimInfo )
        {
            x=1;
            y=0;

            loco_motion_controller& CurrAnim    = pLoco->m_Player.GetCurrAnim();
            loco_motion_controller& BlendAnim   = pLoco->m_Player.GetBlendAnim();
            xbool                 bIsBlending = pLoco->m_Player.IsBlending();

            const char*     pCurrAnim  = CurrAnim.GetAnimName();
            const char*     pBlendAnim = BlendAnim.GetAnimName();
            const vector3&  Pos        = pLoco->GetPosition();
            const vector3&  DeltaPos   = pLoco->GetDeltaPos();

            x_printfxy(x,y++, "LocoState:       %s",   pLoco->GetStateName());
            x_printfxy(x,y++, "MoveStyle:       %s",   loco::GetMoveStyleName(pLoco->GetMoveStyle()));
            x_printfxy(x,y++, "BlendMoveStyle:  %s",   loco::GetMoveStyleName(pLoco->GetBlendMoveStyle()));
            x_printfxy(x,y++, "BlendMoveStyleA: %.2f", pLoco->GetBlendMoveStyleAmount() );
            x_printfxy(x,y++, "CurrAnim:        %s (Yaw=%.2f)", pCurrAnim, RAD_TO_DEG(CurrAnim.GetYaw()) );
            if(bIsBlending)
                x_printfxy(x,y++, "BlendAnim:       %s (Yaw=%.2f)", pBlendAnim, RAD_TO_DEG(BlendAnim.GetYaw()) );
            x_printfxy(x,y++, "AdditiveAnim:    %s", pLoco->GetAdditiveController().GetAnimName());
            x_printfxy(x,y++, "MaskAnim:        %s", pLoco->GetMaskController().GetAnimName());
            x_printfxy(x,y++, "LipSyncAnim:     %s", pLoco->GetLipSyncController().GetAnimName());
            x_printfxy(x,y++, "AimerWeight:     %f", pLoco->GetAimController().GetWeight());
            x_printfxy(x,y++, "Pos:             (%.2f, %.2f, %.2f)", Pos.GetX(), Pos.GetY(), Pos.GetZ());
            x_printfxy(x,y++, "DeltaPos:        (%.2f, %.2f, %.2f)", DeltaPos.GetX(), DeltaPos.GetY(), DeltaPos.GetZ());
            x_printfxy(x,y++, "Speed:%.2f bMotionProp:%d bAccumHoriz:%d bAccumVert:%d bAccumYaw:%d bGrav:%d bColl:%d",
                CurrAnim.GetMovementSpeed(),
                CurrAnim.IsUsingMotionProp() != 0,
                CurrAnim.GetAccumHorizMotion() != 0,
                CurrAnim.GetAccumVertMotion() != 0,
                CurrAnim.GetAccumYawMotion() != 0,
                CurrAnim.GetGravity() != 0,
                CurrAnim.GetWorldCollision() != 0 );
            
            x_printfxy(x,y++, "bExactMove:%d bExactLook:%d bExactMoveBlending:%d bExactLookComplete:%d",
                pLoco->IsExactMove(),
                pLoco->IsExactLook(),
                pLoco->IsExactMoveBlending(),
                pLoco->IsExactLookComplete() );
                
            vector3 DeltaMoveAt = pLoco->GetPosition() - pLoco->GetMoveAt();
            DeltaMoveAt.GetY() = 0;
            f32     DistSqr     = DeltaMoveAt.LengthSquared();
            f32     Dist        = 0;
            if(DistSqr > 0.000001f)
                Dist = x_sqrt(DistSqr);
            x_printfxy(x,y++, "DeltaMoveDist: %.2f", Dist);


            x_printfxy(x,y++, "HorizAim: %.2f", RAD_TO_DEG( pLoco->GetAimController().GetHorizAim() ) );

            radian H,V;
            pLoco->ComputeHeadAim(0, H, V);
            x_printfxy(x,y++, "LookDeltaYaw: %.2f", RAD_TO_DEG(H));
            
            pLoco->ComputeHeadAim( pLoco->GetAimController().GetHorizAim(), H, V);
            x_printfxy(x,y++, "LookDeltaAimYaw: %.2f", RAD_TO_DEG(H));

            // Show heirarchy?
            if(s_bShowHeirarchy)
            {
                x = 30;
                y = 1;

                const anim_group::handle hAnimGroup = pLoco->GetAnimGroupHandle();
                const anim_group* pAnimGroup = hAnimGroup.GetPointer();
                if(pAnimGroup)
                {
                    //ShowHierarchyTree(x,y, pAnimGroup, 0);

                    x_printfxy(x,y++,"\"%s\"   NBones:%d   NAnims:%d", 
                            ANIM_FILE, 
                            pAnimGroup->GetNBones(),
                            pAnimGroup->GetNAnims());

                    x_DebugMsg("\n\nHierarchy: %d Bones\n", pAnimGroup->GetNBones());

                    for (s32 i = 0; i < pAnimGroup->GetNBones(); i++)
                    {
                        // Compute indent
                        s32 Indent  = 0;
                        s32 iParent = pAnimGroup->GetBone(i).iParent;
                        while(iParent != -1)
                        {
                            Indent++;
                            iParent = pAnimGroup->GetBone(iParent).iParent;

                            x_DebugMsg(" ");
                        }

                        // Show bone
                        x_printfxy(x + Indent,y++, "%s", pAnimGroup->GetBone(i).Name);

                        iParent = pAnimGroup->GetBoneParent(i);
                        if(iParent != -1)
                            x_DebugMsg("%s  (Parent=%s)\n", 
                                    pAnimGroup->GetBone(i).Name,
                                    pAnimGroup->GetBone(iParent).Name);
                        else
                            x_DebugMsg("%s\n", pAnimGroup->GetBone(i).Name);
                    }
                    x_DebugMsg("\n\n");

                }
            }
        }
        else
        {   
#ifdef TARGET_PC        
            // Show energy
            x = 0;
            y = 20;
            physics_inst& Inst = s_pActiveLocoInst->m_PhysicsInst[0];
            f32 TotalLinVel = 0.0f;
            f32 TotalAngVel = 0.0f;
            for( s32 b = 0; b < Inst.GetNRigidBodies(); b++ )
            {
                rigid_body& Body = Inst.GetRigidBody( b );
                f32 LinVel = Body.GetLinearVelocity().Length();
                f32 AngVel = Body.GetAngularVelocity().Length();
                x_printfxy( x,y++, "LinVel:%f AngVel:%f", LinVel, AngVel );

                TotalLinVel += LinVel;
                TotalAngVel += AngVel;                
            }
            x_printfxy( x,y++, "TotalLinVel:%f TotalAngVel:%f", TotalLinVel, TotalAngVel );
#endif            
        }
        
        RenderTimer.Start();
        AppRender();
        RenderTimer.Stop();

        LogicTimer.Start();
        AppAdvance( DeltaTime );
        LogicTimer.Stop();

        if( s_bDrawPhysics )
            g_PhysicsMgr.DebugShowStats();
        
        // DONE!
        eng_PageFlip();

        // Profile
        x_ContextPrintProfile();
        x_ContextResetProfile();
    }

    Shutdown();
}

//==============================================================================
