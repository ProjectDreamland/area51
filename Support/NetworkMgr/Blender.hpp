//==============================================================================
//
//  Blender.hpp
//
//==============================================================================

#ifndef BLENDER_HPP
#define BLENDER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_plus.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class blender
{
    f32     m_DeltaToRestart;
    f32     m_DeltaToPop;
    f32     m_Current;
    f32     m_Target;
    f32     m_Delta;
    s32     m_Frames;
    s32     m_FramesToBlend;
    xbool   m_IsAngular;
    xbool   m_Log;

public:
    blender()
    {
        m_DeltaToRestart = 0.0f;
        m_DeltaToPop     = 1.0f;
        m_Current        = 0.0f;
        m_Target         = 0.0f;
        m_Delta          = 0.0f;
        m_Frames         = 0;
        m_FramesToBlend  = 6;
        m_IsAngular      = FALSE;
        m_Log            = FALSE;
    }

    f32 GetValue( void )
    {
        return( m_Current );
    }

    void Init( f32 DeltaToRestart, f32 DeltaToPop, s32 FramesToBlend, xbool IsAngular )
    {
        m_DeltaToRestart = DeltaToRestart;
        m_DeltaToPop     = DeltaToPop;
        m_FramesToBlend  = FramesToBlend;
        m_IsAngular      = IsAngular;
    }

    void SetBlendFrames( s32 Frames )
    {
        m_FramesToBlend = Frames;
    }

    void SetTarget( f32 Target )
    {
        #if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)
        f32 Old      = m_Target;
        #endif
        f32 Delta    = Target - m_Target;
        f32 AbsDelta = (Delta >= 0.0f) ? Delta : -Delta;

        // Check for wrapping on angles
        if( m_IsAngular )
        {
            if( AbsDelta > PI )
            {
                if( Delta >= 0.0f )
                {
                    m_Target  += PI*2;
                    m_Current += PI*2;
                }
                else
                {
                    m_Target  -= PI*2;
                    m_Current -= PI*2;
                }

                Delta    = Target - m_Target;
                AbsDelta = (Delta > 0.0f) ? Delta : -Delta;
            }
        }

        // Accept the new target but don't reset the number of blend frames
        if( AbsDelta < m_DeltaToRestart )
        {
            m_Frames = MAX(1,m_Frames);
            m_Target = Target;
            m_Delta  = (m_Target - m_Current) / m_Frames;
        }

        // Accept the new target and reset the number of blend frames
        else if( AbsDelta < m_DeltaToPop )
        {
            m_Frames = m_FramesToBlend;
            m_Target = Target;
            m_Delta  = (m_Target - m_Current) / m_Frames;
        }

        // Just pop to the new location and reset the number of blend frames
        else
        {
            Teleport( Target );
        }

#if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)
        CLOG_MESSAGE( m_Log, "blender::SetTarget",
                      "%08X - Value:%d - Target:%d - Delta:%d - Frames:%d - PreviousTarget:%d",
                      (u32)this, (s32)m_Current, (s32)m_Target, (s32)m_Delta, 
                      m_Frames, (s32)Old ); 
#endif
    }

    void Teleport( f32 Target )
    {
        #if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)
        f32 Old   = m_Target;
        #endif
        m_Frames  = m_FramesToBlend;
        m_Target  = Target;
        m_Current = Target;
        m_Delta   = 0.0f;
#if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)
        CLOG_MESSAGE( m_Log, "blender::Teleport",
                      "%08X - Value:%d - Target:%d - Delta:%d - Frames:%d - PreviousTarget:%d",
                      (u32)this, (s32)m_Current, (s32)m_Target, (s32)m_Delta, 
                      m_Frames, (s32)Old ); 
#endif
    }       

    f32 BlendLogic( void )
    {
        if( m_Frames == 0 )
            return m_Current;

        m_Frames--;
        m_Current += m_Delta;

        CLOG_MESSAGE( m_Log, "blender::Logic",
                      "%08X - Value:%d - Target:%d - Delta:%d - Frames:%d - PreviousValue:%d",
                      (u32)this, (s32)m_Current, (s32)m_Target, (s32)m_Delta, 
                      m_Frames, (s32)(m_Current-m_Delta) ); 

        return( m_Current );
    }
};

//==============================================================================
#endif // BLENDER_HPP
//==============================================================================
