//=========================================================================
//
//  ANIMPLAYER.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"
#include "animplayer.hpp"
#include "e_ScratchMem.hpp"


//=========================================================================
//=========================================================================
//=========================================================================
// SIMPLE_ANIM_PLAYER
//=========================================================================
//=========================================================================
//=========================================================================

simple_anim_player::simple_anim_player     ( void )
{
    m_hGroup.SetName("");
    m_iAnim         = -1;
    m_Frame         = 0;
    m_Cycle         = 0;
    m_PrevFrame     = 0;
    m_PrevCycle     = 0;
    m_Rate          = 1.0f;
    m_RootNodePos.Set(0, 0, 0);
    m_L2W.Identity();
    m_StopAtEnd     = FALSE;
    m_bAtEnd        = FALSE;
    m_LoopDelay     = 0;
    
    m_pCachedGroup      = NULL ;
    m_iCachedAnim       = -1 ;
    m_CachedFrame       = -1 ;
    m_bCachedApplyBind  = -1 ;

    m_pTrackController[0] = NULL;
    m_pTrackController[1] = NULL;
    m_pTrackController[2] = NULL;
    m_pTrackController[3] = NULL;
}

//=========================================================================

simple_anim_player::~simple_anim_player     ( void )
{
}

//=========================================================================
void simple_anim_player::SetStopAtEnd(xbool StopAtEnd)
{
    m_IsLooping = !StopAtEnd;
    m_StopAtEnd = StopAtEnd;
}

//=========================================================================

void simple_anim_player::SetAnimGroup( const anim_group::handle& hGroup )
{
    m_hGroup = hGroup;
    
    const anim_group* pGroup = GetAnimGroup();
    if( pGroup == NULL )
        return ;

    m_iAnim = -1;
    m_nFrames = 0;
    m_Frame = 0;
    m_Cycle = 0;
    m_bAtEnd = FALSE;
}


//=========================================================================

void simple_anim_player::SetLooping( xbool Looping )
{
    m_IsLooping = Looping;
    m_StopAtEnd = !Looping;
}

//=========================================================================

void simple_anim_player::SetLoopDelay( f32 LoopDelay )
{
    m_LoopDelay = LoopDelay;
}

//=========================================================================

void simple_anim_player::SetSimpleAnim( s32 iAnim )
{
    const anim_group* pGroup = GetAnimGroup();   
    ASSERT( pGroup );
    ASSERT( iAnim != -1 );
    SetAnim( iAnim, pGroup->GetAnimInfo(iAnim).DoesLoop());
}

//=========================================================================

void simple_anim_player::SetSimpleAnim( const char* pAnimName )
{
    ASSERT( pAnimName );
    SetSimpleAnim( GetAnimIndex( pAnimName ) );
}

//=========================================================================

void simple_anim_player::SetAnim( s32 iAnim, xbool IsLooping )
{
    m_IsLooping = IsLooping;
    m_LoopDelay = 0;

    if( iAnim == -1 )
    {
        m_iAnim = -1;
        m_nFrames = 2;
        return;
    }

    // Lookup group
    const anim_group* pGroup = m_hGroup.GetPointer();
    if (!pGroup)
        return;
    
    // Select random anim
    ASSERT( pGroup );
    iAnim = pGroup->GetRandomAnimIndex( iAnim, m_iAnim );
    ASSERT( (iAnim >= 0) && (iAnim < pGroup->GetNAnims()) );

    m_iAnim     = iAnim;
    m_nFrames   = pGroup->GetAnimInfo(m_iAnim).GetNFrames();
    m_Frame     = 0;
    m_Cycle     = 0;
    m_bAtEnd    = FALSE;
}

//=========================================================================

void simple_anim_player::SetAnim( const char* pAnimName,
                                        xbool IsLooping /*=TRUE*/ )
{
    ASSERT( pAnimName );
    SetAnim( GetAnimIndex( pAnimName ), IsLooping );
}

//=========================================================================

void simple_anim_player::SetTrackController  ( s32 iTrackController, track_controller* pTrackController )
{
    if ((iTrackController < 0) || (iTrackController >= 4))
        return;

    m_pTrackController[ iTrackController ] = pTrackController;

    DirtyCachedL2W();
}

//=========================================================================

track_controller* simple_anim_player::GetTrackController  ( s32 iTrackController )
{
    if ((iTrackController < 0) || (iTrackController >= 4))
        return NULL;

    return m_pTrackController[ iTrackController ];
}

//=========================================================================

void simple_anim_player::Advance( f32 nSeconds )
{
    // No animation?
    if( m_iAnim ==-1 )
        return;

    // No time?
    if( nSeconds == 0.0f )
        return;
        
    // No rate?
    if( m_Rate == 0.0f )
        return;
            
    CONTEXT( "simple_anim_player::Advance" );

    //
    // Remember previous frame and cycle
    //
    m_PrevFrame = m_Frame;
    m_PrevCycle = m_Cycle;

    m_bAtEnd = FALSE;

    //
    // Advance frame
    //
    const anim_group* pGroup = GetAnimGroup();
    ASSERT( pGroup );
    const anim_info& AnimInfo = pGroup->GetAnimInfo( m_iAnim );

    f32 nFramesToStep = m_Rate * nSeconds * (f32)AnimInfo.GetFPS();
    f32 nFramesWithDelay = m_nFrames + m_LoopDelay*(f32)AnimInfo.GetFPS();

    if( m_StopAtEnd && (m_Frame + nFramesToStep > (nFramesWithDelay-2)))
    {    
        m_Frame  = nFramesWithDelay-2;
        m_bAtEnd = TRUE;
    }
    else
    if( m_StopAtEnd && ((m_Frame + nFramesToStep) < 0))
    {    
        m_Frame  = 0;
        m_bAtEnd = TRUE;
    }
    else
    {
        m_Frame += nFramesToStep;

        while( m_Frame > (nFramesWithDelay-1) )
        {
            if( !m_IsLooping )
            {
                m_Frame = (f32)(nFramesWithDelay-1);
                m_Cycle = 0;
            }
            else
            {
                m_Frame -= (nFramesWithDelay-1);
                m_Cycle++;
            }
        }
    
        while( m_Frame < 0 )
        {
            if( !m_IsLooping )
            {
                m_Frame = 0;
                m_Cycle = 0;
            }
            else
            {
                m_Frame += (nFramesWithDelay-1);
                m_Cycle--;
            }
        }
    }

    ASSERT( (m_Frame>=0) && (m_Frame<nFramesWithDelay) );
    
    // Disable loop blending?
    if( ( m_Cycle != m_PrevCycle ) && ( AnimInfo.BlendLoop() == FALSE ) )
    {
        // Round frame to nearest integer frame
        s32 iFrame = (s32)m_Frame;
        m_Frame = (f32)iFrame;
    }
}

//=========================================================================

void simple_anim_player::SetL2W( const matrix4& L2W )
{
    m_L2W = L2W;

    // Invalidate the cache
    DirtyCachedL2W() ;
}

//=========================================================================

f32 simple_anim_player::ComputeInterpFrame( void )
{
    // No anim?
    if (m_iAnim == -1)
        return 0.0f ;

    // Get group
    const anim_group* pGroup = GetAnimGroup();
    if (!pGroup)
        return 0.0f ;

    // Get anim info
    const anim_info& AnimInfo = pGroup->GetAnimInfo(m_iAnim);
    
    // Compute valid frame
    f32 Frame = m_Frame;
    if( Frame < 0 ) 
        Frame = 0;
    if( Frame > (AnimInfo.GetNFrames()-2) )
        Frame = (f32)(AnimInfo.GetNFrames()-2) ;

    // This is what anim_info::GetKeys does!
    Frame = x_fmod(Frame,(f32)(AnimInfo.GetNFrames()-1));

    // Disable frame blending?
    if( AnimInfo.BlendFrames() == FALSE )
    {
        // Truncate to nearest integer frame
        s32 iFrame = (s32)Frame;
        Frame = (f32)iFrame;
    }

    return Frame ;
}

//=========================================================================

void simple_anim_player::GetInterpKeys( anim_key* pKey ) 
{
    s32 i;

    // Get group
    const anim_group* pGroup = GetAnimGroup();
    ASSERT( pGroup );
    
    // Get bone bount
    s32 nBones = pGroup->GetNBones();

    // Clear keys
    if( m_iAnim == -1 )
    {
        for( i=0; i<nBones; i++ )
            pKey[i].Identity() ;

        return;
    }

    // Get anim info
    const anim_info& AnimInfo = pGroup->GetAnimInfo(m_iAnim);

    // Get keys
    f32 Frame = ComputeInterpFrame() ;
    AnimInfo.GetInterpKeys( Frame, pKey );
}

//=========================================================================

s32 simple_anim_player::GetNAnims( void ) 
{
    const anim_group* pGroup = GetAnimGroup();
#ifdef X_EDITOR
    if( !pGroup )
        return 0;
#else
    ASSERT( pGroup );
#endif
    s32 nAnims = pGroup->GetNAnims();
    return nAnims;
}

//=========================================================================

s32 simple_anim_player::GetBoneIndex( const char* pBoneName, xbool bFindAnywhere )
{
    // No anim group loaded?
    const anim_group* pGroup = GetAnimGroup();
    if( !pGroup )
        return -1;
        
    // Found bone?
    s32 iBone = pGroup->GetBoneIndex( pBoneName, bFindAnywhere );
    return iBone;
}

//=========================================================================

s32 simple_anim_player::GetNBones( void ) 
{
    const anim_group* pGroup = GetAnimGroup();
#ifdef X_EDITOR
    if( !pGroup )
        return 0;
#else
    ASSERT( pGroup );
#endif
    s32 nBones = pGroup->GetNBones();
    return nBones;
}

//=========================================================================

s32 simple_anim_player::GetAnimIndex    ( const char* pAnimName ) 
{
    const anim_group* pGroup = GetAnimGroup();
#ifdef X_EDITOR
    if( !pGroup )
        return 0;
#else
    ASSERT( pGroup );
#endif
    s32 iAnim = pGroup->GetAnimIndex( pAnimName );
    return iAnim;
}

//=========================================================================

s32 simple_anim_player::GetCollisionBone( void )
{
    const anim_group* pGroup = GetAnimGroup();
    if(!pGroup)
        return -1;
    s32 iBone = pGroup->GetBoneIndex("_coll", TRUE);
    if( iBone == -1 )
        iBone = 0;
    return iBone;
}

//=========================================================================

// Returns TRUE if cache is valid
xbool simple_anim_player::IsCachedL2WValid( xbool bApplyTheBindPose )
{   
    // Check if the actual cashe is dirty
    if (m_CachedL2W.IsDirty())
        return FALSE;

    // Lookup anim group
    const anim_group* pAnimGroup = GetAnimGroup();
    if (!pAnimGroup)
        return FALSE ;

    // Anim group mis-match?
    if (pAnimGroup != m_pCachedGroup)
        return FALSE ;

    // Matrices dirty?
    if (m_CachedL2W.IsValid(pAnimGroup->GetNBones()) == FALSE)
        return FALSE ;

    // Animation index mis-match?
    if (m_iCachedAnim != m_iAnim)
        return FALSE ;

    // Frame mis-match?
    if (m_CachedFrame != ComputeInterpFrame())
        return FALSE ;

    // Apply bind mis-match?
    if (m_bCachedApplyBind != bApplyTheBindPose)
        return FALSE ;

    // Cache is valid!
    return TRUE ;
}

//=========================================================================

// Flags cached matrices as dirty
void simple_anim_player::DirtyCachedL2W( void )
{
    // Clear cache vars
    m_CachedL2W.SetDirty(TRUE) ;
    m_pCachedGroup              = NULL ;
    m_iCachedAnim               = -1 ;
    m_CachedFrame               = -1 ;
    m_bCachedApplyBind          = -1 ;    
}

//=========================================================================

// Updates the cache if it's out of date
const matrix4* simple_anim_player::UpdateCachedL2W( xbool bApplyTheBindPose )
{
    // If valid, just use the cache
    if (IsCachedL2WValid(bApplyTheBindPose))
        return m_CachedL2W.GetMatrices();

    // Clear incase of fail
    DirtyCachedL2W() ;

    // No anim?
    if (m_iAnim == -1)
        return NULL ;

    // Lookup group
    const anim_group* pGroup = GetAnimGroup();
    if (!pGroup)
        return NULL ;

    // Get cache matrices
    matrix4* pMatrices = m_CachedL2W.GetMatrices(pGroup->GetNBones()) ;
    if (!pMatrices)
        return NULL ;

    // Record cache values
    m_pCachedGroup              = pGroup ;
    m_iCachedAnim               = m_iAnim ;
    m_CachedFrame               = ComputeInterpFrame() ;
    m_bCachedApplyBind          = bApplyTheBindPose ;    

    // Allocate mix buffer
    anim_key* MixBuffer = base_player::GetMixBuffer( base_player::MIX_BUFFER_PLAYER );
    ASSERT(MixBuffer);

    // Grab all keys   
    GetInterpKeys( MixBuffer );

    // Mix in any additional track controllers
    if (m_pTrackController[0])
        m_pTrackController[0]->MixKeys( MixBuffer );
    if (m_pTrackController[1])
        m_pTrackController[1]->MixKeys( MixBuffer );
    if (m_pTrackController[2])
        m_pTrackController[2]->MixKeys( MixBuffer );
    if (m_pTrackController[3])
        m_pTrackController[4]->MixKeys( MixBuffer );

    // Setup matrices
    pGroup->ComputeBonesL2W( m_L2W, MixBuffer, pGroup->GetNBones(), pMatrices , bApplyTheBindPose);

    // Keep root node pos
    m_RootNodePos = pMatrices[0].GetTranslation();
    
    // Flag cache is valid
    m_CachedL2W.SetDirty(FALSE) ;

    // Return new matrices
    return pMatrices ;
}

//=========================================================================

// Returns the cached L2W matrix for that bone
const matrix4* simple_anim_player::GetBoneL2Ws( xbool bApplyTheBindPose /*= TRUE*/ )
{
    // Compute bones
    return UpdateCachedL2W(bApplyTheBindPose) ;
}

//=========================================================================


// Returns cached L2W for particular bone
const matrix4* simple_anim_player::GetBoneL2W( s32 iBone, xbool bApplyTheBindPose /*= TRUE*/  )
{
    // Compute bones
    const matrix4* pL2Ws = UpdateCachedL2W(bApplyTheBindPose) ;
    if (!pL2Ws)
        return NULL ;

    // Return specific bone
    return &pL2Ws[iBone] ;
}

//=========================================================================
xbool simple_anim_player::AnimDone( void ) const
{
//    ASSERT( !m_IsLooping ); // looping anims are never done
    //return ((m_nFrames - 1) - m_Frame) < 0.001f;
    return (!m_IsLooping) && ( (m_bAtEnd) || (m_Frame >= (m_nFrames - 1)) );  // Handles forward or backward playing
}


//=========================================================================

f32 simple_anim_player::GetTimeLeft( void )
{
    const f32 FramesLeft = (m_nFrames-1) - m_Frame;
    const anim_group* pGroup = GetAnimGroup();
    ASSERT( pGroup );
    const anim_info& AnimInfo = pGroup->GetAnimInfo( m_iAnim );

    return FramesLeft / (f32)AnimInfo.GetFPS();
}

//==============================================================================

f32 simple_anim_player::GetStartSpeed( f32 Time /*= 1.0f*/ )
{
    ASSERT( Time > 0 );

    s32 CurFrame = (s32)m_Frame;
    const anim_group* pGroup = GetAnimGroup();
    const anim_info& AnimInfo = pGroup->GetAnimInfo( m_iAnim );

    // Make sure we're not looking too far ahead
    Time = MIN( Time, (m_nFrames / AnimInfo.GetFPS()) );

    // find start position
    SetFrame( 0 );
    anim_key AnimKey;
    GetInterpKeys( &AnimKey );
    vector3 Start = AnimKey.Translation;

    // find end position
    s32 EndFrame = (s32)(Time * AnimInfo.GetFPS());
    if ( (EndFrame == 0) && (m_nFrames > 0) )
    {
        // not enough time to get a frame
        EndFrame = 1;
        Time = (1.0f / AnimInfo.GetFPS());
    }

    SetFrame( EndFrame );
    GetInterpKeys( &AnimKey );
    vector3 End = AnimKey.Translation;

    SetFrame( CurFrame );
    
    return (Start-End).Length() / Time;
}

//==============================================================================

f32 simple_anim_player::GetEndSpeed( f32 Time /*= 1.0f*/ )
{
    ASSERT( Time > 0 );

    s32 CurFrame = (s32)m_Frame;
    const anim_group* pGroup = GetAnimGroup();
    const anim_info& AnimInfo = pGroup->GetAnimInfo( m_iAnim );

    // Make sure we're not looking too far ahead
    Time = MIN( Time, (m_nFrames / AnimInfo.GetFPS()) );

    // find end position
    const s32 LastFrame = m_nFrames-2;
    SetFrame( LastFrame );
    anim_key AnimKey;
    GetInterpKeys( &AnimKey );
    vector3 End = AnimKey.Translation;

    // find start position
    s32 StartFrame = (s32)(LastFrame - (Time * AnimInfo.GetFPS()));

    if ( (StartFrame == (LastFrame) ) && (m_nFrames > 0) )
    {
        // not enough time to get a frame
        StartFrame = LastFrame-1;
        Time = (1.0f / AnimInfo.GetFPS());
    }

    SetFrame( StartFrame );
    GetInterpKeys( &AnimKey );
    vector3 Start = AnimKey.Translation;

    SetFrame( CurFrame );
    
    return (Start-End).Length() / Time;
}

//=========================================================================

s32 simple_anim_player::GetEventIndex( const char* pEventName )
{
    anim_info AnimInfo  = GetAnimInfo();
    s32 i;
    for (i=0; i<AnimInfo.GetNEvents(); ++i )
    {
        const anim_event&   Event = AnimInfo.GetEvent( i );

        if( x_strcmp( pEventName, Event.GetName() ) == 0 )
        {
            return i;
        }
    }
    return -1;
}

//=========================================================================

vector3 simple_anim_player::GetEventPosition( const anim_event& Event )
{
    // Compute matrices
    const matrix4* pL2Ws = GetBoneL2Ws() ;
    if (!pL2Ws)
        return vector3(0,0,0) ;

    // Compute world pos
	const matrix4& BoneL2W = pL2Ws[ Event.GetInt( anim_event::INT_IDX_BONE ) ];
    vector3 P = BoneL2W * Event.GetPoint( anim_event::POINT_IDX_OFFSET );
    return P;
}

//=========================================================================

radian3 simple_anim_player::GetEventRotation( const anim_event& Event )
{
    // Compute matrices
    const matrix4* pL2Ws = GetBoneL2Ws() ;
    if (!pL2Ws)
        return radian3(0,0,0) ;

	const matrix4& BoneM = pL2Ws[ Event.GetInt( anim_event::INT_IDX_BONE ) ];

    vector3 ERot( Event.GetPoint( anim_event::POINT_IDX_ROTATION ) );
    radian3 Rot( ERot.GetX(), ERot.GetY(), ERot.GetZ() );

    matrix4 EventRot( Rot );
    matrix4 WorldRot = BoneM * EventRot;
    Rot = WorldRot.GetRotation();

    return Rot;
}

//=========================================================================

vector3 simple_anim_player::GetEventPosition( s32 iEvent ) 
{
    // Compute matrices
    const matrix4* pL2Ws = GetBoneL2Ws() ;
    if (!pL2Ws)
        return vector3(0,0,0) ;

    // Lookup event
    const anim_group* pGroup = GetAnimGroup();
    ASSERT(pGroup);
    ASSERT(m_iAnim != -1) ;
    const anim_info& AnimInfo = pGroup->GetAnimInfo(m_iAnim);
    ASSERT( (iEvent>=0) && (iEvent<AnimInfo.GetNEvents()) );
    const anim_event& Event   = AnimInfo.GetEvent(iEvent);

    // Compute world pos
	const matrix4& BoneL2W = pL2Ws[ Event.GetInt( anim_event::INT_IDX_BONE ) ];
    vector3 P = BoneL2W * Event.GetPoint( anim_event::POINT_IDX_OFFSET );
    return P;
}

//=========================================================================

radian3 simple_anim_player::GetEventRotation( s32 iEvent ) 
{
    // Compute matrices
    const matrix4* pL2Ws = GetBoneL2Ws() ;
    if (!pL2Ws)
        return radian3(0,0,0) ;

    // Lookup event
    const anim_group* pGroup = GetAnimGroup();
    ASSERT(pGroup);
    ASSERT(m_iAnim != -1) ;
    const anim_info& AnimInfo = pGroup->GetAnimInfo(m_iAnim) ;
    ASSERT( (iEvent>=0) && (iEvent<AnimInfo.GetNEvents()) );
    const anim_event& EV   = AnimInfo.GetEvent(iEvent);


	const matrix4& BoneM = pL2Ws[ EV.GetInt( anim_event::INT_IDX_BONE ) ];

    vector3 ERot( EV.GetPoint( anim_event::POINT_IDX_ROTATION ) );
    radian3 Rot( ERot.GetX(), ERot.GetY(), ERot.GetZ() );

    matrix4 EventRot(Rot);
    matrix4 WorldRot = BoneM * EventRot;
    Rot = WorldRot.GetRotation();

    return Rot;
/*
    const anim_group* pGroup = GetAnimGroup();
    ASSERT( pGroup );
    const anim_info& AD = pGroup->GetAnimInfo(m_iAnim);
    anim_key Key[256];

    GetInterpKeys( Key );

    ASSERT( (iEvent>=0) && (iEvent<AD.GetNEvents()) );

    const anim_event&   EV      = AD.GetEvent(iEvent);
    //f32                 Radius  = EV.m_Radius;

    radian3 Rot = pGroup->GetEventRot( 
		EV.GetInt( anim_event::INT_IDX_BONE ), 
		EV.GetPoint( anim_event::POINT_IDX_ROTATION ), 
		Key );

    matrix4 RotM;
    RotM.Identity();
    RotM.Rotate(Rot);
    RotM = m_L2W * RotM;
    Rot = RotM.GetRotation();

    return Rot;
*/
}

//=========================================================================

const vector3 simple_anim_player::GetBoneBindPosition( s32 iBone )
{
    const anim_group* pAnimGroup = GetAnimGroup();
    if ( NULL == pAnimGroup )
        return vector3(0,0,0);

    const anim_bone& AnimBone = pAnimGroup->GetBone(iBone);

    return AnimBone.BindTranslation ;
}

//=============================================================================

// Returns the combined bbox of all the current playing animations
bbox simple_anim_player::ComputeBBox( void ) const
{
    // Lookup anim group and return default if not loaded
    const anim_group* pAnimGroup = m_hGroup.GetPointer();
    if( pAnimGroup == NULL )
    {
        // Use default bbox
        bbox BBox;
        BBox.Set( vector3( 0.0f, 0.0f, 0.0f ), 100.0f );
        return BBox;
    }

    // If no anim playing, then use bind pose (always present) bbox
    if( m_iAnim == -1 )
    {
        const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( 0 );
        return AnimInfo.GetBBox();
    }

    // Use bbox of current animation
    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( m_iAnim );
    return AnimInfo.GetBBox();
}

//=============================================================================
