//=========================================================================
//
//  ANIMINFO.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"

//=========================================================================

anim_info::anim_info( void )
{
}

//=========================================================================

anim_info::~anim_info( void )
{
}

//=========================================================================

void anim_info::GetRawKey( s32 iFrame, s32 iBone, anim_key& Key ) const
{
    iFrame = iFrame % m_nFrames;
    m_AnimKeys.GetRawKey( *m_pAnimGroup, iFrame, iBone, Key );
}

//=========================================================================

void anim_info::GetInterpKey( f32  Frame, s32 iBone, anim_key& Key ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    m_AnimKeys.GetInterpKey( *m_pAnimGroup, Frame, iBone, Key );
}

//=========================================================================

void anim_info::GetRawKeys( s32 iFrame, anim_key* pKey ) const
{
    iFrame = iFrame % m_nFrames;
    m_AnimKeys.GetRawKeys( *m_pAnimGroup, iFrame, pKey );
}

//=========================================================================

void anim_info::GetInterpKeys( f32  Frame, anim_key* pKey ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    m_AnimKeys.GetInterpKeys( *m_pAnimGroup, Frame, pKey );
}

//=========================================================================

void anim_info::GetInterpKeys( f32  Frame, anim_key* pKey, s32 nBones ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    m_AnimKeys.GetInterpKeys( *m_pAnimGroup, Frame, pKey, nBones );
}

//=========================================================================

s32 anim_info::GetPropChannel( const char *pChannelName  ) const
{
    for(s32 i = 0; i < m_nProps; i++)
    if( x_stricmp(m_pAnimGroup->m_pProp[m_iProp + i].m_Type, pChannelName) == 0 )
        return i;

    return -1;
}
 
//=========================================================================
   
void anim_info::GetPropRawKey( s32 iChannel, s32 iFrame, anim_key& Key ) const
{
    iFrame = iFrame % m_nFrames;
    ASSERT(iChannel >= 0);
    ASSERT(iChannel < m_nProps);
    m_AnimKeys.GetRawKey( *m_pAnimGroup, iFrame, m_pAnimGroup->m_nBones + iChannel, Key );
    return;
}

//=========================================================================

void anim_info::GetPropInterpKey( s32 iChannel, f32 Frame,  anim_key& Key ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    ASSERT(iChannel >= 0);
    ASSERT(iChannel < m_nProps);
    m_AnimKeys.GetInterpKey( *m_pAnimGroup, Frame, m_pAnimGroup->m_nBones + iChannel, Key );
    return;
}

//=========================================================================

f32 anim_info::GetSpeed( void ) const
{
    f32 Dist = m_TotalTranslation.Length();
    f32 Time = (f32)(m_nFrames) / (f32)m_FPS;
    return Dist / Time;
}

//=========================================================================

f32 anim_info::GetYawRate( void ) const
{
    radian Yaw  = GetTotalYaw();
    f32    Time = (f32)(m_nFrames) / (f32)m_FPS;
    return Yaw / Time;
}

//=========================================================================

xbool anim_info::IsEventActive( s32 iEvent, f32 Frame ) const
{
    ASSERT( (iEvent>=0) && (iEvent<m_nEvents) );
    const anim_event& E = m_pAnimGroup->m_pEvent[m_iEvent+iEvent];

    if( (Frame>=E.StartFrame()) && (Frame<=E.EndFrame()) )
        return TRUE;

    return FALSE;
}

//=========================================================================

xbool anim_info::IsEventTypeActive( s32 Type, f32 Frame ) const
{
    for( s32 i=0; i<m_nEvents; i++ )
    {
        if( m_pAnimGroup->m_pEvent[m_iEvent+i].GetInt( anim_event::INT_IDX_OLD_TYPE ) == Type )
        {
            const anim_event& E = m_pAnimGroup->m_pEvent[m_iEvent+i];

            if( (Frame>=E.StartFrame() ) && (Frame<=E.EndFrame()) )
                return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

// *INEV* *SB* - Added for A51
xbool anim_info::IsEventActive( s32 iEvent, f32 CurrFrame, f32 PrevFrame ) const
{
    ASSERT( (iEvent>=0) && (iEvent<m_nEvents) );
    const anim_event& E = m_pAnimGroup->m_pEvent[m_iEvent+iEvent];

    // For debugging
    s32 EndFrame = E.EndFrame();
    s32 StartFrame = E.StartFrame();
    
    if( EndFrame == (StartFrame+1) )
    {
        // This is considered a single-shot and should only be active
        // if we are crossing over the start frame
        if( PrevFrame <= CurrFrame )
        {
            if( (CurrFrame > StartFrame) && (PrevFrame <= StartFrame) )
                return TRUE;
        }
        else
        if( PrevFrame > CurrFrame )
        {
            f32 P,C;

            P = PrevFrame;
            C = (f32)m_nFrames-1.0f;
            if( (C >= StartFrame) && (P < StartFrame) )
                return TRUE;

            P = 0;
            C = CurrFrame;
            if( (C >= StartFrame) && (P < StartFrame) )
                return TRUE;
        }
        else
        {
            if( CurrFrame == StartFrame )
                return TRUE;
        }
    }
    else
    {
        // This is considered a duration event and should be active
        // if we are overlapping any part of the event time frame

        if( PrevFrame > CurrFrame )
        {
            f32 P,C;

            P = PrevFrame;
            C = (f32)m_nFrames-1.0f;
            if( !((C < StartFrame) || (P > EndFrame)) )
                return TRUE;

            P = 0;
            C = CurrFrame;
            if( !((C < StartFrame) || (P > EndFrame)) )
                return TRUE;
        }
        else
        {
            if( !((CurrFrame < StartFrame) || (PrevFrame > EndFrame)) )
                return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

xbool anim_info::IsEventTypeActive( s32 Type, f32 CurrFrame, f32 PrevFrame ) const
{
    for( s32 i=0; i<m_nEvents; i++ )
    {
        if( m_pAnimGroup->m_pEvent[m_iEvent+i].GetInt( anim_event::INT_IDX_OLD_TYPE ) == Type )
        {
            const anim_event& E = m_pAnimGroup->m_pEvent[m_iEvent+i];

            // For debugging
            s32 EndFrame = E.EndFrame()-1;
            s32 StartFrame = E.StartFrame();

            // Frames overlapping event?
            if (!((PrevFrame >= EndFrame) || (CurrFrame < StartFrame)))
                return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

f32 anim_info::FindLipSyncEventStartFrame ( void ) const
{
    // Check all events
    for( s32 i = 0; i < GetNEvents(); i++ )
    {
        // Lookup event info
        const anim_event& Event = GetEvent( i );  
        const char*       pType = Event.GetType();  
        
        // Is this a generic event?
        if( x_strcmp( pType,  "Generic" ) == 0 )
        {
            // Get generic type
            const char* pGenericType = Event.GetString( anim_event::STRING_IDX_GENERIC_TYPE );
            
            // Is this a lip sync start event?
            if( x_strcmp( pGenericType, "Lip-Sync Start" ) == 0 )
            {
                // Found!
                return (f32)Event.StartFrame();
            }
        }
    }
    
    // Not found
    return -1.0f;
}

//=========================================================================
