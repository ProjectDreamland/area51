//==============================================================================
//
//  fx_Ctrl.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================

void fx_ctrl::Initialize( fx_ctrl_def* pCtrlDef, f32* pOutput )
{
    f32 CycleSize = pCtrlDef->DataEnd - pCtrlDef->DataBegin;
    f32 StartAge  = -pCtrlDef->DataBegin / CycleSize;

    m_pCtrlDef = pCtrlDef;
    m_AgeRate  = 1.0f / CycleSize;
    m_Cycle    = (s32)x_floor( StartAge );
    m_CycleAge = StartAge - m_Cycle;
    m_pOutput  = pOutput;
}

//==============================================================================

void fx_ctrl::AdvanceLogic( f32 DeltaTime )
{
    m_CycleAge += m_AgeRate * DeltaTime;

    while( m_CycleAge >= 1.0f )
    {
        m_CycleAge -= 1.0f;
        m_Cycle    += 1;
    }
    while( m_CycleAge < 0.0f )
    {
        m_CycleAge += 1.0f;
        m_Cycle    -= 1;
    }

    Evaluate( ComputeLogicalTime() );
}

//==============================================================================

f32 fx_ctrl::ComputeLogicalTime( void ) const
{
    s32 LoopCode = FX_TILE;

    if( m_Cycle < 0 )  LoopCode = m_pCtrlDef->LeadIn;
    if( m_Cycle > 0 )  LoopCode = m_pCtrlDef->LeadOut;

    switch( LoopCode )
    {
    default:
    case FX_CLAMP:      
        return( (m_Cycle < 0) ? 0.0f : 1.0f );    
        break;

    case FX_TILE:       
        return( m_CycleAge );  
        break;

    case FX_MIRROR:     
        return( (m_Cycle & 0x01) 
                ? (1.0f - m_CycleAge)
                : (       m_CycleAge) );     
        break;
    }

    // Can't get here.
    ASSERT( FALSE );
}

//==============================================================================
