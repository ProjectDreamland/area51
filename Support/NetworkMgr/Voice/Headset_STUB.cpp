//==============================================================================
//
//  Headset_PC.cpp
//
//==============================================================================
#include "x_types.hpp"

#if !defined(TARGET_PC)
#error This should only be getting compiled for the PC platform. Check your dependancies.
#endif

#include "headset.hpp"
//
// Even though the xbox voice code will be significantly different than PS2 voice code,
// I am still including the Provide and Accept update functions as they will allow us
// to maintain compatibility between PS2 & XBOX. XBOX live should take care of headset
// support.
//

//==============================================================================
void headset::Init( xbool )
{
    m_pEncodeBuffer = new u8[512];
    m_pDecodeBuffer = m_pEncodeBuffer+256;
    m_ReadFifo.Init(m_pEncodeBuffer,256);
    m_WriteFifo.Init(m_pDecodeBuffer,256);
}

//==============================================================================
void headset::Kill( void )
{
    m_WriteFifo.Kill();
    m_ReadFifo.Kill();
    delete[] m_pEncodeBuffer;
}

//==============================================================================
void headset::PeriodicUpdate( f32 )
{
}

//==============================================================================
void headset::Update( f32 DeltaTime )
{
    (void)DeltaTime;
}

//==============================================================================
void headset::OnHeadsetInsert(void)
{
}

//==============================================================================
void headset::OnHeadsetRemove(void)
{
}

