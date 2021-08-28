#ifndef XBOX_PRIVATE_HPP
#define XBOX_PRIVATE_HPP

#ifndef TARGET_XBOX
#   error This is not for this target platform. Check dependancy rules.
#endif

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

// Included this header only one time
#pragma once

#ifndef STRICT
#define STRICT
#endif

#define D3DCOMPILE_PUREDEVICE 1

#include <xtl.h>
#include <xonline.h>
#include <xgraphics.h>

#include "x_files.hpp"
#include "e_singleton.hpp"

#ifndef _TEXTUREMGR_HPP_
#include"texturemgr.hpp"
#endif

#ifndef _VERTEXMGR_HPP_
#include"vertexmgr.hpp"
#endif

#ifndef _INDEXMGR_HPP_
#include"indexmgr.hpp"
#endif

#ifndef _PUSHMGR_HPP_
#include"pushmgr.hpp"
#endif

#ifndef _DEFERRED_HPP_
#include"deferred.hpp"
#endif

#ifdef X_DEBUG
#include <d3d8perf.h>
#include <XbDm.h>
#endif

///////////////////////////////////////////////////////////////////////////
// DEFINE AND ENUMS
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void  input_LockCurrent(xbool);
xbool input_IsLocked   (s32* );

extern s32 g_ControllerCount;
extern HANDLE s_hPads[4];

void xbox_IoSetErrorCallback( void(*)(s32) );
void xbox_EntryPoint ( void );
s32  xbox_ExitPoint  ( void );

///////////////////////////////////////////////////////////////////////////
// HACK FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void xbox_EngInit( s32 maxXRes, s32 maxYRes, s32 XRes, s32 YRes );
void xbox_PageFlip();
void xbox_BuildXBitmapFromScreen( xbitmap& Dst, s32 W, s32 H );
void xbox_EnableFrameLock(xbool,s32 );

///////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////////////////////////////////////////////////

// This is not part of D3D but a multi-threaded alternative.
#if SWITCH_USE_DEFERRED_RENDERING
extern IDeferred3DDevice8* g_pd3dDevice;
#else
extern   IDirect3DDevice8* g_pd3dDevice;
#endif

///////////////////////////////////////////////////////////////////////////
// VRAM CONSTANTS
///////////////////////////////////////////////////////////////////////////

extern u32 g_PhysW;
extern u32 g_PhysH;

#define VRAM_FRAME_BUFFER_WIDTH     g_PhysW
#define VRAM_FRAME_BUFFER_HEIGHT    g_PhysH
#define VRAM_FRAME_BUFFER_BPP       4

#define VRAM_ZBUFFER_WIDTH          VRAM_FRAME_BUFFER_WIDTH
#define VRAM_ZBUFFER_HEIGHT         VRAM_FRAME_BUFFER_HEIGHT
#define VRAM_ZBUFFER_BPP            4

#define VRAM_FRAME_BUFFER_SIZE      (VRAM_FRAME_BUFFER_WIDTH * VRAM_FRAME_BUFFER_HEIGHT * VRAM_FRAME_BUFFER_BPP)
#define VRAM_ZBUFFER_SIZE           (VRAM_ZBUFFER_WIDTH      * VRAM_ZBUFFER_HEIGHT      * VRAM_ZBUFFER_BPP     )

///////////////////////////////////////////////////////////////////////////
// RENDER/TEXTURE-STAGE STATES
///////////////////////////////////////////////////////////////////////////

namespace render
{
    //! Render state object.
    /** This structure defines possible render states.
        */

    struct texture_stage_state
    {
        //  -----------------------------------------------------------------
        //                                                          Accessors
        //  -----------------------------------------------------------------

        /** Set individual state.
            This routine sets an individual render state.
            */

        u32 Set( u32 iStage,u32 iState,u32 iValue );

        /** Return render state.
            This routine returns the value of a specific
            render state.
            */

        u32 Get( u32 iStage,u32 iIndex );

        //  -----------------------------------------------------------------
        //                                                       Construction
        //  -----------------------------------------------------------------

        /** This constructor ensures that all states are set to their
            inital values. I do this by first setting the array to
            all 0xFFFFFFF and then calling SetState( ) on each.
            */

        texture_stage_state( void )
        {
            x_memset( m_iData,-1,D3DTSS_MAXSTAGES*D3DTSS_MAX*sizeof( u32 ));
        }

        /** The destructor doesn't need to do anything at this time
            and will ultimately evaporate.
            */

    ~   texture_stage_state( void )
        {
        }

        //  -----------------------------------------------------------------
        //                                                 Private properties
        //  -----------------------------------------------------------------

    private:

        //! Render state values.
        /** This member is an array of all possible
            texture stage states and their values.
            */

    	u32 m_iData[ D3DTSS_MAXSTAGES ][ D3DTSS_MAX ];
    };

    //! Render state object.
    /** This structure defines possible render states.
        */

    struct render_state
    {
        //  -----------------------------------------------------------------
        //                                                          Accessors
        //  -----------------------------------------------------------------

        void Push( void );
        void Pop ( void );
        u32  Set ( u32 iIndex,u32 iValue );
        u32  Get ( u32 iIndex );

        //  -----------------------------------------------------------------
        //                                                       Construction
        //  -----------------------------------------------------------------

        /** This constructor ensures that all states are set to their
            inital values. I do this by first setting the array to
            all 0xFFFFFFF and then calling SetState( ) on each.
            */

        render_state( void );

        /** The destructor doesn't need to do anything at this time
            and will ultimately evaporate.
            */

    ~   render_state( void );

        //  -----------------------------------------------------------------
        //                                                 Private properties
        //  -----------------------------------------------------------------

    private:

        struct push_record
        {
            u32 Index;
            u32 Value;
        };

        #define TOTAL_RECALL 256

        push_record m_Recall[TOTAL_RECALL];
        u32 m_RecallCount;

        u32 m_iData[ D3DRS_MAX ];
        u32 m_bPush;
    };
}


extern render::texture_stage_state g_TextureStageState;
extern render::render_state        g_RenderState;

///////////////////////////////////////////////////////////////////////////
// Magical macro which defines the entry point of the app. Make sure that 
// the use has the entry point: void AppMain( s32 argc, char* argv[] ) 
// define somewhere. MFC apps don't need the user entry point.
// ------------------------------------------------------------------------
// Xbox version also activates all the rendering singletons
///////////////////////////////////////////////////////////////////////////
#define AppMain AppMain( s32 argc, char* argv[] );                        \
int main( void )                                                          \
{                                                                         \
    xbox_EntryPoint( );                                                   \
    x_StartMain( AppMain, 0, NULL );                                      \
    return xbox_ExitPoint( );                                             \
}                                                                         \
void AppMain

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////
#endif
