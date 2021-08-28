//=========================================================================
//
//  ANIMTRACK.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animtrack.hpp"

#define INTERP_TEST InterpolateSRT
//=========================================================================


//=========================================================================
//=========================================================================
//=========================================================================
// track_controller
//=========================================================================
//=========================================================================
//=========================================================================

track_controller::track_controller     ( void )
{
}

//=========================================================================

track_controller::~track_controller     ( void )
{
}

//=========================================================================
//=========================================================================
//=========================================================================
// anim_track_controller
//=========================================================================
//=========================================================================
//=========================================================================

anim_track_controller::anim_track_controller     ( void )
{
    m_pBlendKey = NULL;
    Clear();
}

//=========================================================================

anim_track_controller::~anim_track_controller     ( void )
{
    Clear();
    delete[] m_pBlendKey;
}

//=========================================================================

void anim_track_controller::Clear( void )
{
    m_iAnim                 = -1;
    m_nFrames               = 0;
    m_Frame                 = 0;
    m_Cycle                 = 0;
    m_Weight                = 0;
    m_PrevFrame             = 0;
    m_PrevCycle             = 0;
    m_Rate                  = 1.0f;
    m_BlendLength           = 0;
    m_BlendFrame            = 0;
	m_bPreviousManualYaw    = FALSE;
    m_bRemoveTurnYaw        = FALSE ;
    m_iRefFrame             = 0;
    m_MixMode               = MIX_BLENDED;
}

//=========================================================================

void anim_track_controller::SetOverrideRootBlend( xbool bOverrideRootBlend )
{
    m_bOverrideRootBlend = bOverrideRootBlend;
}

//=========================================================================

void anim_track_controller::SetRemoveTurnYaw( xbool bRemove )
{
    m_bRemoveTurnYaw = bRemove ;
}

//=========================================================================

void anim_track_controller::SetAnimGroup( const anim_group::handle& AnimGroup )
{
    MEMORY_OWNER_DETAIL( "anim_track_controller::SetAnimGroup()" );
    delete[] m_pBlendKey;

    m_hAnimGroup = AnimGroup;
    const anim_group& AG = GetAnimGroup();

    ASSERT( AG.GetNBones() <= MAX_ANIM_BONES );

    m_pBlendKey = new anim_key[ AG.GetNBones() ];
    ASSERT( m_pBlendKey );

    Clear();
}

//=========================================================================

void anim_track_controller::GetInterpKeys( f32  Frame, anim_key* pKey )
{
    // Lookup anim data
    const anim_info& AnimData = GetAnimInfo() ;

    // Grab the keys
    AnimData.GetInterpKeys(Frame, pKey) ;

    // Remove yaw from root anim?
    if ( (m_bRemoveTurnYaw) && (IsPlayingTurnAnim()) )
    {
        // Remove yaw from root node key
        radian RootYaw = pKey[0].Rotation.GetRotation().Yaw ;
        pKey[0].Rotation *= quaternion(vector3(0,1,0), -RootYaw) ;
    }
}

//=========================================================================

void anim_track_controller::SetAnim( s32 iAnim, f32 BlendTime , xbool ResetFrameCount )
{
    ASSERT( (iAnim>=0) && (iAnim<GetAnimGroup().GetNAnims()) );

	// If we are already playing this anim, don't reset
    if( iAnim==m_iAnim )
	{
        // If the animation is looping and we are pegged at the end then restart
        if( !(GetAnimInfo().DoesLoop() && IsAtEnd()) )
		{
			if ( ResetFrameCount )
			{
				m_Frame = 0;
				m_Cycle = 0;
			}
	        return;
		}
	}

    // Check for using a specified blend time
    if( iAnim != -1 )
    {
        // Lookup anim info
        const anim_group& AnimGroup = GetAnimGroup();
        const anim_info&  AnimInfo  = AnimGroup.GetAnimInfo( iAnim );

        // Use blend from animation if it's specified
        if( AnimInfo.GetBlendTime() >= 0.0f )
            BlendTime = AnimInfo.GetBlendTime();
    }

    // Allocate mix buffer
    anim_key* MixBuffer = base_player::GetMixBuffer( base_player::MIX_BUFFER_CONTROLLER );
    ASSERT(MixBuffer);

    // Setup blend information
    if( (m_iAnim!=-1) && (iAnim != m_iAnim) )
    {
        // If we care about blending then prepare blend keys
        if( BlendTime > 0 )
        {

            // If blending has not finished from previous animation then we
            // need to combine the blend keys and the current keys back into
            // the blend keys and use that as our 'previous' animation
            if( m_BlendLength>0 )
            {
                f32 T = m_BlendFrame / m_BlendLength;
                s32 nBones = GetAnimGroup().GetNBones();
                GetInterpKeys( m_Frame, MixBuffer );
                for( s32 i=0; i<nBones; i++ )
                    m_pBlendKey[i].Interpolate( m_pBlendKey[i], MixBuffer[i], T );
            }
            else
            {
                GetInterpKeys( m_Frame, m_pBlendKey );
            }
            if (m_bPreviousManualYaw || m_bOverrideRootBlend)
            {
                anim_key Key;
                const anim_info AnimData = GetAnimGroup().GetAnimInfo( iAnim );
                AnimData.GetRawKey(0, 0, Key);
                m_pBlendKey[0].Rotation = Key.Rotation;
            }
        }

        m_BlendLength   = BlendTime;
        m_BlendFrame    = 0.0f;
    }

    m_iAnim     = iAnim;
    m_nFrames   = GetAnimInfo().GetNFrames();
    m_Frame     = 0;
    m_Cycle     = 0;
    m_PrevFrame = 0;
    m_PrevCycle = 0;
	m_bPreviousManualYaw = m_bManualYaw;
    m_bOverrideRootBlend = FALSE;
}

//=========================================================================

void anim_track_controller::SetAnim( const char* pAnimName, f32 BlendTime )
{
    SetAnim( GetAnimIndex(pAnimName), BlendTime );
}

//=========================================================================

void anim_track_controller::Advance( f32 nSeconds )
{
    if( m_iAnim == -1 )
        return;

    //
    // Remember previous frame and cycle
    //
    m_PrevFrame = m_Frame;
    m_PrevCycle = m_Cycle;

    //
    // Count down blend time
    //
    m_BlendFrame += x_abs(nSeconds);
    if( m_BlendFrame >= m_BlendLength )
    {
        m_BlendFrame = 0.0f;
        m_BlendLength= 0.0f;
    }

    //
    // Advance frame 
    //
    f32 nFrames = nSeconds * (f32)GetAnimInfo().GetFPS() * m_Rate;
    m_Frame += nFrames;

    // Update which cycle we are in and modulate the frame
    while( m_Frame >= (m_nFrames-1) )
    {
        m_Frame -= (m_nFrames-1);
        m_Cycle++;
    }

    // Lookup anim data
    const anim_info& AnimData = GetAnimInfo() ;

    // Get looping flag
    xbool DoesLoop = AnimData.DoesLoop() ;

    // If the anim doesn't loop and we are past the end then peg at the end
    if( (!DoesLoop) && ((m_Cycle>0) || (m_Frame >= (m_nFrames-2))) )
    {
        m_Cycle = 0;
        m_Frame = (f32)(m_nFrames-2);
    }
}

//=========================================================================

void anim_track_controller::GetInterpKeys( anim_key* pKey ) 
{
    s32 i;
    s32 nBones = GetAnimGroup().GetNBones();

    // Clear keys if no animation
    if( m_iAnim == -1 )
    {
        for( i=0; i<nBones; i++ )
            pKey[i].Identity() ;

        return;
    }

    // Read interpolated keys from the animation
    GetInterpKeys( m_Frame, pKey );

    // Blend with previous anim exit keyframes
    if( m_BlendLength > 0.0f )
    {
        f32 T = m_BlendFrame / m_BlendLength;
        for( i=0; i<nBones; i++ )
            pKey[i].Interpolate( m_pBlendKey[i], pKey[i], T );
    }
}

//=========================================================================

void anim_track_controller::GetInterpKey( s32 iBone, anim_key& Key )
{
    // Clear keys if no animation
    if( m_iAnim == -1 )
    {
        Key.Identity();
        return;
    }

    // Read interpolated keys from the animation
    GetAnimInfo().GetInterpKey( m_Frame, iBone, Key );

    // Blend with previous anim exit keyframes
    if( m_BlendLength > 0.0f )
    {
        f32 T = m_BlendFrame / m_BlendLength;
        Key.Interpolate( m_pBlendKey[iBone], Key, T );
    }
}


//=========================================================================
//=========================================================================
//=========================================================================
// SUPPORT ROUTINES
//=========================================================================
//=========================================================================
//=========================================================================

const anim_group& anim_track_controller::GetAnimGroup( void )
{
    anim_group* pGroup = (anim_group*)m_hAnimGroup.GetPointer();
    ASSERT( pGroup );
    return *pGroup;
}

//=========================================================================

xbool anim_track_controller::IsAtEnd( void )
{
    return ( (m_Cycle>0) || (m_Frame >= (m_nFrames-2)) );
}

//=========================================================================

void anim_track_controller::SetFrame( f32 Frame )
{
    m_Frame = x_fmod(Frame, (f32)m_nFrames-1);
}

//=========================================================================

void anim_track_controller::SetCycle( s32 Cycle )
{
    m_Cycle = Cycle;
}

//=========================================================================

void anim_track_controller::SetRate( f32 Rate )
{
    m_Rate = Rate;
}

//=========================================================================

f32 anim_track_controller::GetRate( void )
{
    return m_Rate;
}

//=========================================================================

void anim_track_controller::SetWeight( f32 Weight )
{
    m_Weight = Weight;
    m_Weight = MIN(1.0f,m_Weight);
    m_Weight = MAX(0.0f,m_Weight);
}

//=========================================================================

f32 anim_track_controller::GetWeight( void )
{
    return m_Weight;
}

//=========================================================================

f32 anim_track_controller::GetFrame( void ) 
{
    return m_Frame;
}

//=========================================================================

s32 anim_track_controller::GetCycle( void ) 
{
    return m_Cycle;
}

//=========================================================================

f32 anim_track_controller::GetFrameParametric( void )
{
    // NOTE: The -2 is because the last 2 frames are the same - they are used
    //       to calculate a correct delta for the positions eg. looping run anim
    //       DO NOT CHANGE THIS. If you use -1, then an incorrect delta gets calculated
    //       upon the loop of the animation, resulting in a position pop!

    f32 Frame = m_Frame / (m_nFrames-2);

    // Range check
    if (Frame < 0)
        Frame = 0;
    else
    if (Frame > 1)
        Frame = 1;

    return Frame;
}

//=========================================================================

void anim_track_controller::SetFrameParametric( f32 Frame )
{
    // Range check
    if (Frame < 0)
        Frame = 0;
    else
    if (Frame > 1)
        Frame = 1;

    // NOTE: The -2 is because the last 2 frames are the same - they are used
    //       to calculate a correct delta for the positions eg. looping run anim
    //       DO NOT CHANGE THIS. If you use -1, then an incorrect delta gets calculated
    //       upon the loop of the animation, resulting in a position pop!

    // Set
    m_Frame = Frame * ((f32)m_nFrames-2);
}

//=========================================================================

f32 anim_track_controller::GetPrevFrame( void ) 
{
    return m_PrevFrame;
}

//=========================================================================

s32 anim_track_controller::GetPrevCycle( void ) 
{
    return m_PrevCycle;
}

//=========================================================================

s32 anim_track_controller::GetNFrames( void ) 
{
    return m_nFrames;
}

//=========================================================================

s32 anim_track_controller::GetAnimIndex( void ) 
{
    return m_iAnim;
}

//=========================================================================

s32 anim_track_controller::GetNAnims( void ) 
{
    return GetAnimGroup().GetNAnims();
}

//=========================================================================

s32 anim_track_controller::GetNBones( void ) 
{
    return GetAnimGroup().GetNBones();
}

//=========================================================================

s32 anim_track_controller::GetAnimIndex    ( const char* pAnimName ) 
{
    return GetAnimGroup().GetAnimIndex( pAnimName );
}

//=========================================================================

xbool anim_track_controller::IsPlaying( const char* pAnimName )
{
    if( m_iAnim == -1 )
        return FALSE;

    if( m_iAnim == GetAnimIndex(pAnimName) )
        return TRUE;

    return FALSE;
}

//=========================================================================

xbool anim_track_controller::IsPlayingTurnAnim( void )
{
    if( m_iAnim == -1 )
        return FALSE;

    // Lookup anim data
    const anim_info& AnimData = GetAnimInfo() ;

    // Is this a turn animation?
    xbool bIsTurn =     (x_stristr(AnimData.GetName(), "turn")  != NULL) 
                     || (x_stristr(AnimData.GetName(), "blend") != NULL) ;


    return bIsTurn ;
}

//=========================================================================

// Returns true if the current animations is a transition
xbool anim_track_controller::IsPlayingTransitionAnim( void )
{
    if( m_iAnim == -1 )
        return FALSE;

    // Lookup anim data
    const anim_info& AnimData = GetAnimInfo() ;

    // Is this a transition animation?
    xbool bIsTransition = (x_stristr(AnimData.GetName(), "blend") != NULL) ;

    return bIsTransition ;
}

//=========================================================================

vector3 anim_track_controller::GetTotalTranslation( void ) 
{
    return GetAnimInfo().GetTotalTranslation();
}

//=========================================================================

const anim_info& anim_track_controller::GetAnimInfo( void ) 
{
    return GetAnimGroup().GetAnimInfo(m_iAnim);
}

//=========================================================================

s32 anim_track_controller::GetBoneIndex( const char* pBoneName ) 
{
    return GetAnimGroup().GetBoneIndex( pBoneName );
}

//=========================================================================

const anim_bone& anim_track_controller::GetBone( s32 iBone ) 
{
    return GetAnimGroup().GetBone(iBone);
}

//=========================================================================

void anim_track_controller::SetAdditveRefFrame  ( s32 iRefFrame )
{
    m_iRefFrame = iRefFrame;
}

//=========================================================================

s32 anim_track_controller::GetAdditiveRefFrame ( void )
{
    return m_iRefFrame;
}

//=========================================================================

void anim_track_controller::SetMixMode( mix_mode Mode )
{
    m_MixMode = Mode;
}

//=========================================================================
//=========================================================================
//=========================================================================
// EVENTS
//=========================================================================
//=========================================================================
//=========================================================================

s32 anim_track_controller::GetNEvents( void ) 
{
    if( m_iAnim==-1 )
        return 0;

    return GetAnimInfo().GetNEvents();
}

//=========================================================================

const anim_event& anim_track_controller::GetEvent( s32 iEvent ) 
{
    return GetAnimInfo().GetEvent( iEvent );
}

//=========================================================================

xbool anim_track_controller::IsEventActive( s32 iEvent ) 
{
    return GetAnimInfo().IsEventActive( iEvent, m_Frame, m_PrevFrame);
}

//=========================================================================

xbool anim_track_controller::IsEventTypeActive( s32 Type ) 
{
    return GetAnimInfo().IsEventTypeActive( Type, m_Frame, m_PrevFrame);
}

//=========================================================================
//=========================================================================
//=========================================================================
// MIXING
//=========================================================================
//=========================================================================
//=========================================================================

void anim_track_controller::MixKeys( anim_key* pDestKey )
{
    switch (m_MixMode )
    {
    case MIX_BLENDED:
        BlendedMixKeys( pDestKey );
        break;
    case MIX_ADDITIVE:
        AdditiveMixKeys( pDestKey );
        break;
    }
}
    
//=========================================================================

void anim_track_controller::BlendedMixKeys( anim_key* pDestKey )
{
    CONTEXT("anim_track_controller::MixKeys") ;

    // If we aren't playing anything then just return
    if( (m_iAnim == -1) || (m_Weight==0.0f) )
        return;

    s32 i;
    s32 nBones = GetAnimGroup().GetNBones();

    // Allocate mix buffer
    anim_key* MixBuffer = base_player::GetMixBuffer( base_player::MIX_BUFFER_CONTROLLER );
    ASSERT(MixBuffer);

    // Read interpolated keys from the animation
    GetInterpKeys( m_Frame, MixBuffer );

    // Blend with previous anim exit keyframes
    if( m_BlendLength > 0.0f )
    {
        f32 T = m_BlendFrame / m_BlendLength;
        for( i=0; i<nBones; i++ )
            MixBuffer[i].Interpolate( m_pBlendKey[i], MixBuffer[i], T );
    }

    // Check if this animation has bone masks
    if( GetAnimInfo().HasMasks() )
    {
        // Blend destination into track keys by weight amount
        for( i=0; i<nBones; i++ )
        if( !GetAnimInfo().IsBoneMasked(i) )
            pDestKey[i].Interpolate( pDestKey[i], MixBuffer[i], m_Weight );
    }
    else
    {
        // Blend destination into track keys by weight amount
        for( i=0; i<nBones; i++ )
            pDestKey[i].Interpolate( pDestKey[i], MixBuffer[i], m_Weight );
    }
}

//=========================================================================

void anim_track_controller::AdditiveMixKeys( anim_key* pDestKey )
{
    CONTEXT(" anim_track_controller::AdditiveMixKeys");

    // If we aren't playing anything then just return
    if( (m_iAnim == -1) || (m_Weight==0.0f) )
        return;

    // Bail on bad param
    if ( NULL == pDestKey )
        return;

    // Get anim group
    const anim_group& AnimGroup = GetAnimGroup();

    // Lookup anim info
    const anim_info& AnimInfo = AnimGroup.GetAnimInfo(m_iAnim);

    // Loop over all bones and mix keys
    s32 NBones = AnimGroup.GetNBones();

    for (s32 i = 0; i < NBones; i++)
    {
        // Skip if masked
        if (AnimInfo.IsBoneMasked(i))
            continue;

        // Read reference key
        anim_key InvRefKey;
        AnimInfo.GetRawKey(m_iRefFrame, i, InvRefKey);

        // Lookup current key
        anim_key CurrKey;
        AnimInfo.GetInterpKey(m_Frame, i, CurrKey);
/*
        // Skip if keys are the same
        if (InvRefKey.Translation == CurrKey.Translation)
        {
            // Get cosine of angle between rotations
            f32 CosAngle = (InvRefKey.Rotation.X * CurrKey.Rotation.X) +
                           (InvRefKey.Rotation.Y * CurrKey.Rotation.Y) +
                           (InvRefKey.Rotation.Z * CurrKey.Rotation.Z) +
                           (InvRefKey.Rotation.W * CurrKey.Rotation.W);
            
            // Skip if rotation is almost the same
            if (CosAngle > 0.99999999f)
                continue;
        }
*/
        // Eyely applies the difference between reference frame and the current frame as follows:
        //
        // Variables:   CurrRot,   CurrPos   = Current key frame
        //              AnimRot,   AnimPos   = Animation key frame
        //              InvRefRot, InvRefPos = Inverse key frame of reference frame
        //
        //  Apply delta to current keys
        //          CurrRot = AnimRot * InvRefRot * CurrRot;
        //          CurrPos = AnimPos + InvRefPos + CurrPos;
        //
        //

        // Get inverse of reference key
        InvRefKey.Rotation.Invert();
        InvRefKey.Translation = -InvRefKey.Translation;

        // Compute eye key
        anim_key AddKey;
        AddKey.Rotation    = CurrKey.Rotation    * InvRefKey.Rotation;
        AddKey.Translation = CurrKey.Translation + InvRefKey.Translation;

        // Weighted?
        if (m_Weight != 1.0f)
        {
            AddKey.Rotation = BlendToIdentity(AddKey.Rotation, 1.0f - m_Weight);
            AddKey.Translation *= m_Weight;
        }

        // Add delta on top of current keys
        pDestKey[i].Rotation    = AddKey.Rotation    * pDestKey[i].Rotation;
        pDestKey[i].Translation = AddKey.Translation + pDestKey[i].Translation;

    }
}

//=========================================================================
