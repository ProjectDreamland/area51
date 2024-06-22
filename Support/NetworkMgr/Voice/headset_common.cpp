//==============================================================================
//
//  Headset_Common.cpp
//
//==============================================================================
#include "x_types.hpp"
#include "x_log.hpp"
#include "x_string.hpp"

#include "headset.hpp"

//==============================================================================
void headset::ProvideUpdate( netstream& BitStream, s32 MaxLength )
{
    m_ReadFifo.ProvideUpdate( BitStream, MaxLength, m_EncodeBlockSize );
}

//==============================================================================
void headset::AcceptUpdate( netstream& BitStream )
{
    m_WriteFifo.AcceptUpdate( BitStream, m_EncodeBlockSize );
}

//==============================================================================

void headset::UpdateLoopBack( void )
{
    if( m_LoopbackEnabled == TRUE )
    {
        char Buffer[ 256 ];

        if( m_ReadFifo.Remove ( Buffer, m_EncodeBlockSize, m_EncodeBlockSize ) )
        {
            m_WriteFifo.Insert( Buffer, m_EncodeBlockSize, m_EncodeBlockSize );
        }
    }
}

//==============================================================================
s32 headset::Read( void* pBuffer, s32 Length )
{
    s32     Available;

    Available = m_ReadFifo.GetBytesUsed();
    if( Available > Length )
    {
        Available = Length;
    }

    m_ReadFifo.Remove( pBuffer, Available, m_EncodeBlockSize );
    return Available;
}

//==============================================================================
s32 headset::Write( const void* pBuffer, s32 Length )
{
    m_WriteFifo.Insert( pBuffer, Length, m_EncodeBlockSize );
    return 0;
}

//==============================================================================
void headset::SetTalking( xbool IsTalking )
{
    if( IsTalking && (m_HeadsetCount > 0) )
    {
        m_IsTalking = TRUE;
    }
    else
    {
        m_IsTalking = FALSE;
    }
}

//==============================================================================
void headset::SetVolume( f32 Headset, f32 MicrophoneSensitivity )
{
    m_HeadsetVolume = Headset;
    m_MicrophoneSensitivity = MicrophoneSensitivity;
    m_VolumeChanged = TRUE;

}

//==============================================================================
void headset::GetVolume( f32& Headset, f32& Microphone )
{
    Headset = m_HeadsetVolume;
    Microphone = m_MicrophoneSensitivity;
}

//==============================================================================

s32 headset::GetNumBytesInWriteFifo( void )
{
    return( m_WriteFifo.GetBytesUsed() );
}

//==============================================================================
/*
void headset::SetActiveHeadset( s32 HeadsetIndex )
{
    if( HeadsetIndex != m_ActiveHeadset )
    {
        //
        // If the old headset was there, then we tell the system it has been removed
        //
        if( (m_ActiveHeadset != -1) && (m_HeadsetMask & (1<<m_ActiveHeadset)) )
        {
            OnHeadsetRemove();
        }

        m_ActiveHeadset = HeadsetIndex;
        //
        // If the new headset is present, insert it.
        //
        if( (m_ActiveHeadset != -1) && (m_HeadsetMask & (1<<m_ActiveHeadset)) )
        {
            OnHeadsetInsert();
        }
    }
}
*/

#if defined ( TARGET_PC )
void headset::OnHeadsetInsert( void )
{
}

//==============================================================================

void headset::OnHeadsetRemove( void )
{
}

//==============================================================================
#endif