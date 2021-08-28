//==============================================================================
//
//  RenderContext.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "RenderContext.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

render_context g_RenderContext;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void render_context::Set( s32   aLocalPlayerIndex, 
                          s32   aNetPlayerSlot,
                          u32   aTeamBits, 
                          xbool bIsMutated,
                          xbool bIsPipRender )
{
    LocalPlayerIndex = aLocalPlayerIndex;
    NetPlayerSlot    = aNetPlayerSlot;
    TeamBits         = aTeamBits;
    m_bIsMutated     = bIsMutated;
    m_bIsPipRender   = bIsPipRender;
}

//==============================================================================

void render_context::SetPipRender( xbool bIsPipRender )
{
    m_bIsPipRender = bIsPipRender;
}

//==============================================================================
