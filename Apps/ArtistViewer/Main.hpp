//==============================================================================
//
//  File:           Main.cpp
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================
#include "Entropy.hpp"
#include "Config.hpp"
#include "ViewerObject.hpp"

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class player;


//==============================================================================
//  DEFINES
//==============================================================================

// Edit modes
enum mode
{
    MODE_OBJECT,
    MODE_LIGHT,
    MODE_SCREEN_SHOT,
    MODE_FX,

    MODE_COUNT,
    MODE_START = MODE_OBJECT,
    MODE_END   = MODE_FX
} ;

// Background pic status
enum pic_status
{
    PIC_STATUS_NONE,
    PIC_STATUS_FILE_NOT_FOUND,
    PIC_STATUS_INVALID_FORMAT,
    PIC_STATUS_VALID
} ;

// Pic modes
enum pic_mode
{
    PIC_MODE_TOP_LEFT,
    PIC_MODE_CENTER,
    PIC_MODE_FIT,

    PIC_MODE_COUNT,
    PIC_MODE_DEFAULT = PIC_MODE_FIT
} ;

// Stats modes
enum stats_mode
{
    STATS_MODE_OFF,
    STATS_MODE_HIGH,
    STATS_MODE_MEDIUM,
    STATS_MODE_LOW,
} ;

// Viewer time step
#define TIME_STEP   (1.0f / 30.0f)

// Pic info
#define PIC_WIDTH   512
#define PIC_HEIGHT  512

// Physics
#define MAX_PHYSICS_INSTS   6

//==============================================================================
//  FORWARD DECLARATIONS
//==============================================================================

class skin_inst;
class loco_char_anim_player;
class object;


//==============================================================================
//  FUNCTIONS
//==============================================================================

// Lighting functions
xcolor ComputeAmbient( const vector3& Pos );

// Physics functions
void ClearPhysicsInsts  (  void );

void AddPhysicsInsts    ( const char*               pGeomName,
                          u64                       Mask,
                          loco_char_anim_player&    AnimPlayer,
                          const vector3&            Vel,
                          xbool                     bClearAll, 
                          xbool                     bBlast );


//==============================================================================
//  DATA
//==============================================================================

// Render
extern view                    g_View ;                         // Main view
extern xbitmap                 g_PicBitmap ;                    // Background pic
extern s32                     g_PicWidth ;                     // Actual width
extern s32                     g_PicHeight ;                    // Actual height
extern pic_status              g_PicStatus ;                    // Pic status
extern s32                     g_ScreenShotSize ;               // Size of screen shot
extern xbool                   g_bMovieShotActive;              // TRUE if active
                                                    
// Object                                           
extern viewer_object*          g_pObjects ;                     // List of pointers to objects
extern s32                     g_nObjects ;                     // # of objects
extern s32                     g_iObject ;                      // Active object to render/advance
extern s32                     g_iLoadedObject ;                // Index of loaded object in memory
extern player*                 g_pPlayer;                       // Ptr to dummy player

// Fx
extern s32                      g_iFx;                          // Current fx

// Editables
extern s32                     g_iConfig ;                      // Index of current config file
extern s32                     g_Mode ;                         // Edit mode
extern xbool                   g_bPause ;                       // Pause flag
extern s32                     g_ShowHelp ;                     // Shows help
extern stats_mode              g_StatsMode ;                    // Shows stats
extern config_options::light           g_Light ;                        // Current light
extern s32                     g_PicMode ;                      // Pic display mode


//==============================================================================
