//==============================================================================
//  
//  Entropy.hpp
//
//==============================================================================

#ifndef ENTROPY_HPP
#define ENTROPY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "e_View.hpp"
#include "e_Draw.hpp"
#include "e_Text.hpp"
#include "e_VRAM.hpp"
#include "e_Input.hpp"
#include "e_ScratchMem.hpp"
#include "e_Audio.hpp"
#include "e_Profile.hpp"

#include "x_files.hpp"

#ifdef TARGET_PC
#include "D3DEngine\d3deng_Private.hpp"
#endif

#ifdef TARGET_XBOX
#include "xbox\xbox_Private.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

typedef u64 datestamp;

struct split_date
{
    u16     Year;
    u8      Month;
    u8      Day;
    u8      Hour;
    u8      Minute;
    u8      Second;
    u8      CentiSecond;
};

#define ENG_MAX_VIEWS   8

//==============================================================================
//  FUNCTIONS
//==============================================================================

void            eng_Init                ( void );
void            eng_Kill                ( void );

void            eng_GetRes              ( s32& XRes, s32& YRes );
void            eng_GetPALMode          ( xbool& PALMode );
void            eng_SetBackColor        ( xcolor Color );

void            eng_MaximizeViewport    ( view& View );

void            eng_SetView             ( const view& View );
const view*     eng_GetView             ( void );

#if !defined(X_RETAIL) || defined(X_QA)
void            eng_ScreenShot          ( const char* pFileName = NULL, s32 Size = 1 );
xbool           eng_ScreenShotActive    ( void );
s32             eng_ScreenShotSize      ( void );
s32             eng_ScreenShotX         ( void );
s32             eng_ScreenShotY         ( void );
#endif  // !defined( X_RETAIL ) || defined( X_QA )

xbool           eng_Begin               ( const char* pTaskName=NULL );
void            eng_End                 ( void );
xbool           eng_InBeginEnd          ( void );
void            eng_ResetAfterException ( void );
void            eng_PageFlip            ( void );
void            eng_Sync                ( void );

void            eng_SetViewport         ( const view& View );

f32             eng_GetFPS              ( void );
void            eng_PrintStats          ( void );
//
// System date functions
//
datestamp       eng_GetDate             ( void );
split_date      eng_SplitDate           ( datestamp DateStamp );
#ifdef TARGET_PS2
split_date      eng_SplitJSTDate        ( u64 JSTStamp );
#endif
datestamp       eng_JoinDate            ( const split_date& DateStamp );

#ifdef TARGET_XBOX
void eng_ShowSafeArea( xbool bEnable );
#else
#define eng_ShowSafeArea(bEnable)
#endif

enum reboot_reason
{
    REBOOT_HALT,
    REBOOT_QUIT,
    REBOOT_MANAGE,
    REBOOT_NEWUSER,
    REBOOT_MESSAGE,
    REBOOT_UPDATE,
};

void        eng_Reboot( reboot_reason ExitCode );
s32         eng_GetProductCode( void );
const char *eng_GetProductKey( void );

//==============================================================================
#endif // ENTROPY_HPP
//==============================================================================
