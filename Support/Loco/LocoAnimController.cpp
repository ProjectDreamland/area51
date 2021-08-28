//=========================================================================
//
//  LocoAnimController.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Loco.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================
loco_anim_controller::loco_anim_controller ( void ) :
    m_iAnim                 ( -1    ),  // Index of current anim
    m_iAnimType             ( -1    ),  // Index that was passed into "SetAnim" call
    m_AnimFlags             ( 0     ),  // Associated anim flags for playback
    m_nFrames               ( 0     ),  // nFrames in animation
    m_EndFrameOffset        ( 0     ),  // # of frames from end of anim to trigger it's finished
    m_Frame                 ( 0     ),  // Current modulated frame
    m_Cycle                 ( 0     ),  // Current Cycle, 0,1,2,3
    m_Weight                ( 1     ),  // influence at mixing time
    m_PrevFrame             ( 0     ),  // frame before Advance()
    m_PrevCycle             ( 0     ),  // cycle before Advance()
    m_Rate                  ( 1     ),  // playback rate in frames per second
    m_bLooping              ( FALSE ),  // TRUE if playing a looping anim
    
    m_bAccumYawMotion       ( FALSE ),  // TRUE if delta Yaw motion should be extracted
    m_bAccumHorizMotion     ( FALSE ),  // TRUE if delta XZ motion should be extracted
    m_bAccumVertMotion      ( FALSE ),  // TRUE if delta Y motion should be extracted
    
    m_bRemoveYawMotion      ( FALSE ),  // TRUE if yaw motion should be removed from motion bone
    m_bRemoveHorizMotion    ( FALSE ),  // TRUE if horiz motion should be removed from motion bone
    m_bRemoveVertMotion     ( FALSE ),  // TRUE if vert motion should be removed from motion bone
    
    m_bGravity              ( TRUE  ),  // TRUE if gravity should be applied
    m_bWorldCollision       ( TRUE  ),  // TRUE if world collision should happen
    
    m_bStartedOnMainTrack   ( FALSE ),  // TRUE if anim has been started on the main track
    m_bIsBlendingOut        ( FALSE ),  // TRUE if anim is blending out
    m_bIsBlendingIn         ( FALSE )   // TRUE if anim if blending in
{
}

//=========================================================================

loco_anim_controller::~loco_anim_controller     ( void )
{
}

//=========================================================================

void loco_anim_controller::SetAnimGroup( const anim_group::handle& hAnimGroup )
{
    // Set group
    m_hAnimGroup = hAnimGroup ;

    // Clear vars
    Clear();
}

//=========================================================================

void loco_anim_controller::SetAnimGroup( const char* pAnimGroup )
{
    // Set group
    m_hAnimGroup.SetName( pAnimGroup );

    // Clear vars
    Clear();
}

//=========================================================================

void loco_anim_controller::Clear( void )
{
    // Clear vars
    m_iAnim                 = -1;
    m_iAnimType             = -1;
    m_AnimFlags             = 0;
    m_nFrames               = 0;
    m_EndFrameOffset        = 0;
    m_Frame                 = 0;
    m_Cycle                 = 0;
    m_Weight                = 0;
    m_PrevFrame             = 0;
    m_PrevCycle             = 0;
    m_Rate                  = 1.0f;
    m_bLooping              = FALSE;
    
    m_bAccumYawMotion       = FALSE;
    m_bAccumHorizMotion     = TRUE;
    m_bAccumVertMotion      = FALSE;

    m_bRemoveYawMotion      = FALSE;
    m_bRemoveHorizMotion    = TRUE;
    m_bRemoveVertMotion     = FALSE;

    m_bWorldCollision       = TRUE;
    m_bGravity              = TRUE;
}

//=========================================================================
// Anim group functions - returns info about animation package
//=========================================================================

const anim_group& loco_anim_controller::GetAnimGroup( void ) const
{
    anim_group* pGroup = (anim_group*)m_hAnimGroup.GetPointer();
    ASSERT( pGroup );
    return *pGroup;
}

//=========================================================================

const anim_group::handle& loco_anim_controller::GetAnimGroupHandle( void ) const
{
    return m_hAnimGroup ;
}

//=========================================================================

const anim_info& loco_anim_controller::GetAnimInfo( s32 iAnim ) const
{
    ASSERT(iAnim != -1) ;
    const anim_group& AnimGroup = GetAnimGroup() ;
    return AnimGroup.GetAnimInfo(iAnim) ;
}

//=========================================================================

s32 loco_anim_controller::GetNAnims( void ) const
{
    return GetAnimGroup().GetNAnims();
}
//=========================================================================

s32 loco_anim_controller::GetAnimIndex    ( const char* pAnimName ) const
{
    return GetAnimGroup().GetAnimIndex( pAnimName );
}


//=========================================================================

s32 loco_anim_controller::GetNBones( void ) const
{
    return GetAnimGroup().GetNBones();
}

//=========================================================================

const anim_bone& loco_anim_controller::GetBone( s32 iBone ) const
{
    return GetAnimGroup().GetBone(iBone);
}

//=========================================================================

s32 loco_anim_controller::GetBoneIndex( const char* pBoneName ) const
{
    return GetAnimGroup().GetBoneIndex( pBoneName );
}

//=========================================================================
// Animation settings functions
//=========================================================================

void loco_anim_controller::SetAnim( const anim_group::handle& hAnimGroup, s32 iAnim, u32 Flags )
{
    // Update group?
    // NOTE: This clears all anim vars to -1, 0, or NULL
    if (m_hAnimGroup.GetPointer() != hAnimGroup.GetPointer())
        SetAnimGroup(hAnimGroup) ;

    // Keep flags
    m_AnimFlags = Flags;

    // Lookup group
    const anim_group& AnimGroup = GetAnimGroup() ;
    ASSERT( (iAnim>=0) && (iAnim < AnimGroup.GetNAnims()) ) ;
    if( (iAnim<0) || (iAnim >= AnimGroup.GetNAnims()) )
        return;

    // If we are already playing this anim type index?
    if ( (m_Weight != 0.0f) && (iAnim == m_iAnimType) && (m_iAnim != -1) && ((Flags & loco::ANIM_FLAG_RESTART_IF_SAME_ANIM) == 0) )
	{
        // If the animation is non looping and we are pegged at the end then restart
        if( (!m_bLooping) && (IsAtEnd()) )
        {
            // Reset frame
            m_Frame     = 0;
            m_Cycle     = 0;

            // Set previous frame/cycle also
            m_PrevFrame = 0;
            m_PrevCycle = 0;
        }

        // Do not reset!
        return;
	}

    // Clear main track flag
    m_bStartedOnMainTrack = FALSE;

    // Store the type index that was requested
    m_iAnimType = iAnim ;

    // Select a new animation, skipping the current one?
    if ((Flags & loco::ANIM_FLAG_TURN_OFF_RANDOM_SELECTION) == 0)
        iAnim = AnimGroup.GetRandomAnimIndex( iAnim, m_iAnim );

    // Lookup anim info
    const anim_info& AnimInfo = AnimGroup.GetAnimInfo( iAnim );
    
    // Keep useful info
    m_iAnim             = iAnim;
    m_nFrames           = AnimInfo.GetNFrames() ;
    m_EndFrameOffset    = AnimInfo.GetEndFrameOffset();
    m_bLooping          = AnimInfo.DoesLoop() ;
    
    m_bAccumHorizMotion = m_bRemoveHorizMotion = AnimInfo.AccumHorizMotion() ;
    m_bAccumVertMotion  = m_bRemoveVertMotion  = AnimInfo.AccumVertMotion() ;
    m_bAccumYawMotion   = m_bRemoveYawMotion   = AnimInfo.AccumYawMotion() ;

    m_bGravity          = AnimInfo.Gravity() ;
    m_bWorldCollision   = AnimInfo.WorldCollision() ;

    // Clear blending status
    m_bIsBlendingIn     = FALSE;
    m_bIsBlendingOut    = FALSE;

    // Reset frame and cycle info
    if (Flags & loco::ANIM_FLAG_START_ON_SAME_FRAME)
        m_Frame     = (f32)((s32)m_Frame % (m_nFrames-1)) ;
    else
        m_Frame     = 0;
    m_Cycle         = 0;
    m_PrevFrame     = m_Frame;
    m_PrevCycle     = 0;
}

//=========================================================================

void loco_anim_controller::SetAnim( const anim_group::handle& hAnimGroup, const char* pAnimName, u32 Flags )
{
    // Update group?
    if (m_hAnimGroup.GetPointer() != hAnimGroup.GetPointer())
        SetAnimGroup(hAnimGroup) ;

    SetAnim( hAnimGroup, GetAnimIndex(pAnimName), Flags ) ;
}

//=========================================================================
// Logic functions - advances animation and returns delta pos and delta yaw
//=========================================================================

void loco_anim_controller::Advance( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw )
{
    ASSERT( m_Rate>=0 );

    // Clear deltas
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;

    // No anim playing?
    if( m_iAnim == -1 )
        return;

    // Skip if no rate (cinema audio driven) so that PrevCycle, PrevFrame are not touched (which would stop events from firing)
    if( m_Rate == 0.0f )
        return;
        
    // Get anim info
    const anim_info& AnimInfo = GetAnimInfo();

    // Remember previous frame and cycle
    m_PrevFrame = m_Frame;
    m_PrevCycle = m_Cycle;

    // Advance frame 
    f32 nFrames = nSeconds * (f32)AnimInfo.GetFPS() * m_Rate;
    m_Frame += nFrames;

    // Update which cycle we are in and modulate the frame
    while( m_Frame >= (m_nFrames-1) )
    {
        m_Frame -= (m_nFrames-1);
        m_Frame += AnimInfo.GetLoopFrame();
        m_Cycle++;
    }

    // If the anim doesn't loop and we are past the end then peg at the end
    if( (!m_bLooping) && ((m_Cycle>0) || (m_Frame >= (m_nFrames-2))) )
    {
        m_Cycle = 0;
        m_Frame = (f32)(m_nFrames-2);
    }
}

//=========================================================================
// Animation query functions - returns info on animation being played
//=========================================================================

const char* loco_anim_controller::GetAnimName( void ) const
{
    // None playing?
    if (m_iAnim == -1)
        return "NULL" ;

    // Lookup name
    const anim_info& Info = GetAnimInfo() ;
    return Info.GetName() ;
}

//=========================================================================

s32 loco_anim_controller::GetLoopFrame( void ) const
{
    // No anim playing?
    if( m_iAnim == -1 )
        return 0;
        
    // Lookup loop to frame
    const anim_info& Info = GetAnimInfo() ;
    return Info.GetLoopFrame();
}

//=========================================================================

xbool loco_anim_controller::IsAtEnd( void ) const
{
    // -2 is because the last and first frames are the same so loops work.
    
    // Also take the end frame offset into account - this can be used to
    // flag the anim has ended early so that non-looping animations do not
    // "freeze" and look static when blending to the next animation
    s32 EndFrame = x_max( 0, m_nFrames - 2 - m_EndFrameOffset);
    return ( ( m_Cycle > 0 ) || ( m_Frame >= EndFrame ) );
}

//=========================================================================

xbool loco_anim_controller::IsPlaying( const char* pAnimName ) const
{
    if( m_iAnim == -1 )
        return FALSE;

    if( m_iAnimType == GetAnimIndex(pAnimName) )
        return TRUE;

    return FALSE;
}

//=========================================================================

xbool loco_anim_controller::IsCinemaRelativeMode( void ) const
{ 
    return ( m_AnimFlags & loco::ANIM_FLAG_CINEMA_RELATIVE_MODE ) != 0;
}

//=========================================================================

void loco_anim_controller::SetCinemaRelativeMode( xbool bFlag )
{
    // Set or clear the relative flag?
    if( bFlag )
    {
        m_AnimFlags |= loco::ANIM_FLAG_CINEMA_RELATIVE_MODE; 
    }        
    else        
    {    
        m_AnimFlags &= ~loco::ANIM_FLAG_CINEMA_RELATIVE_MODE; 
    }        
}

//=========================================================================

xbool loco_anim_controller::IsCoverRelativeMode( void ) const
{ 
    return ( m_AnimFlags & loco::ANIM_FLAG_COVER_RELATIVE_MODE ) != 0;
}

//=========================================================================

void loco_anim_controller::SetCoverRelativeMode( xbool bFlag )
{
    // Set or clear the relative flag?
    if( bFlag )
    {
        m_AnimFlags |= loco::ANIM_FLAG_COVER_RELATIVE_MODE; 
    }        
    else        
    {    
        m_AnimFlags &= ~loco::ANIM_FLAG_COVER_RELATIVE_MODE; 
    }        
}

//=========================================================================

void loco_anim_controller::SetFrame( f32 Frame )
{
    // Make sure frame is valid
    if (Frame < 0)
        Frame = 0 ;
    else
    {
        f32 MaxFrame = (f32)m_nFrames-2 ;
        if (Frame > MaxFrame)
            Frame = MaxFrame ;
    }

    // Set it
    m_Frame = Frame ;
}

//=========================================================================

void loco_anim_controller::SetTime( f32 Time )
{
    // No anim?
    if( m_iAnim == -1 )
        return;
        
    // Convert time into frame and set
    s32 FPS = GetFPS();
    f32 Frame = (f32)FPS * Time;
    
    // Past the end of the anim?
    f32 NFrames = (f32)GetNFrames() - 2.0f;
    if( Frame > NFrames )
    {
        // Fixup for looping?
        if( IsLooping() )
        {
            f32 LoopFrames = NFrames - (f32)GetLoopFrame();
            m_PrevCycle = m_Cycle;
            m_Cycle     = 0;
            while( Frame > NFrames )
            {
                Frame -= LoopFrames;
                m_Cycle++;
            }                
        }
        else
        {
            // Clamp at end of anim
            Frame = NFrames;
        }
    }
        
    // Record so events fire off correctly
    m_PrevFrame = m_Frame;
            
    // Finally, set the frame    
    SetFrame( Frame );
}

//=========================================================================

f32 loco_anim_controller::GetFrameParametric( void ) const
{
    // NOTE: The -2 is because the last 2 frames are the same - they are used
    //       to calculate a correct delta for the positions eg. looping run anim
    //       DO NOT CHANGE THIS. If you use -1, then an incorrect delta gets calculated
    //       upon the loop of the animation, resulting in a position pop!

    f32 Frame   = m_Frame / MAX(1, (m_nFrames-2)) ;

    // Range check
    if (Frame < 0)
        Frame = 0 ;
    else
    if (Frame > 1)
        Frame = 1 ;

    return Frame ;
}

//=========================================================================

void loco_anim_controller::SetFrameParametric( f32 Frame )
{
    // Range check
    if (Frame < 0)
        Frame = 0 ;
    else
    if (Frame > 1)
        Frame = 1 ;

    // NOTE: The -2 is because the last 2 frames are the same - they are used
    //       to calculate a correct delta for the positions eg. looping run anim
    //       DO NOT CHANGE THIS. If you use -1, then an incorrect delta gets calculated
    //       upon the loop of the animation, resulting in a position pop!

    // Set
    m_Frame = Frame * ((f32)m_nFrames-2) ;
}

//=========================================================================

s32 loco_anim_controller::GetFPS( void ) const
{
    if( m_iAnim == -1 )
        return 30;
        
    const anim_info& AnimInfo = GetAnimGroup().GetAnimInfo(m_iAnim) ;
    return AnimInfo.GetFPS(); ;
}

//=========================================================================

// Returns TRUE if animation is upper body
xbool loco_anim_controller::IsUpperBody( void ) const
{
    // No anim?
    if ( m_iAnim == -1 )
        return FALSE;

    // Declare so we can debug easier
    xbool bUpperBody = ((m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY) != 0) ;
    return bUpperBody ;
}

//=========================================================================

// Returns TRUE if animation is full body
xbool loco_anim_controller::IsFullBody( void ) const
{
    // No anim?
    if ( m_iAnim == -1 )
        return FALSE;

    // Declare so we can debug easier
    xbool bFullBody = ((m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY) != 0) ;
    return bFullBody ;
}

//=========================================================================

xbool loco_anim_controller::IsPlaying( void ) const
{
    // Any anim?
    if ( m_iAnim == -1 )
        return FALSE;

    // Are we blending in?
    if( m_bIsBlendingIn )
        return TRUE;

    // Any weight?
    if ( m_Weight <= 0.0f )
        return FALSE;

    // Must be playing
    return TRUE;
}


//=========================================================================
// Event functions
//=========================================================================

s32 loco_anim_controller::GetNEvents( void ) 
{
    if( m_iAnim==-1 )
        return 0;

    if (m_Weight == 0)
        return 0 ;

    return GetAnimInfo().GetNEvents();
}

//=========================================================================
// Weight functions - controls the influence during the mixing process
//=========================================================================

void loco_anim_controller::SetWeight( f32 Weight )
{
    m_Weight = x_max(0.0f, x_min(1.0f, Weight)) ;
}

//=========================================================================
// Key mixing functions
//=========================================================================

void loco_anim_controller::GetInterpKeys( const info& Info, anim_key* pKey )
{
    s32 i ;

    // Clear keys if no animation
    if( m_iAnim == -1 )
    {
        for( i=0; i< Info.m_nActiveBones; i++ )
            pKey[i].Identity();

        return;
    }

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (Info.m_nActiveBones <= GetAnimGroup().GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Lookup anim data
    const anim_info& AnimInfo = GetAnimInfo() ;

    // Grab the keys
    AnimInfo.GetInterpKeys(m_Frame, pKey, Info.m_nActiveBones) ;
}

//=========================================================================

void loco_anim_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    CONTEXT("loco_anim_controller::MixKeys") ;

    // If we aren't playing anything then just return
    if( (m_iAnim == -1) || (m_Weight==0.0f) )
        return;

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (Info.m_nActiveBones <= GetAnimGroup().GetNBones()), "Incompatible anim group with bone lods!" ) ;

    s32 i;
    s32 nBones = Info.m_nActiveBones ;

    // Allocate mix buffer
    anim_key* MixBuffer = base_player::GetMixBuffer( base_player::MIX_BUFFER_CONTROLLER );
    ASSERT(MixBuffer);

    // Read interpolated keys from the animation
    GetInterpKeys(Info, MixBuffer );

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

void loco_anim_controller::AdditiveMixKeys( const info& Info, s32 iAnim, f32 Frame, s32 iRefFrame, anim_key* pDestKey )
{
    CONTEXT(" loco_anim_controller::AdditiveMixKeys") ;

    // If we aren't playing anything then just return
    if ( (iAnim == -1) || (m_Weight == 0.0f) )
        return;

    // Get anim group
    const anim_group& AnimGroup = GetAnimGroup() ;

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (Info.m_nActiveBones <= AnimGroup.GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Lookup anim info
    const anim_info& AnimInfo = AnimGroup.GetAnimInfo(iAnim) ;

    // Lookup min and max bones used
    s32 iAnimBoneMin = AnimInfo.GetAnimBoneMinIndex();
    s32 iAnimBoneMax = AnimInfo.GetAnimBoneMaxIndex();
    
    // No additive animation present?
    if( iAnimBoneMin == -1 )
    {
        ASSERT( iAnimBoneMax == -1 );
        return;
    }
    
    // Make sure anim compiler is working correctly
    ASSERT( iAnimBoneMin >= 0 );
    ASSERT( iAnimBoneMax >= 0 );
    ASSERT( iAnimBoneMax >= iAnimBoneMin );
            
    // Bones not displayed?
    if( Info.m_nActiveBones <= iAnimBoneMin )
        return;

    // Take LOD into account
    iAnimBoneMax = x_min( Info.m_nActiveBones-1, iAnimBoneMax );
    
    // Debug count
    #ifdef X_DEBUG
        s32 NBonesUsed = 0 ;
    #endif

    // Loop over all animated bones and mix keys
    for (s32 i = iAnimBoneMin ; i <= iAnimBoneMax ; i++)
    {
        // Skip if masked
        if (AnimInfo.IsBoneMasked(i))
            continue ;

        // Read reference key
        anim_key InvRefKey ;
        AnimInfo.GetRawKey(iRefFrame, i, InvRefKey) ;

        // Lookup current key
        anim_key CurrKey ;
        AnimInfo.GetInterpKey(Frame, i, CurrKey) ;

        // Skip if keys are the same
        if (InvRefKey.Translation == CurrKey.Translation)
        {
            // Get cosine of angle between rotations
            f32 CosAngle = (InvRefKey.Rotation.X * CurrKey.Rotation.X) +
                           (InvRefKey.Rotation.Y * CurrKey.Rotation.Y) +
                           (InvRefKey.Rotation.Z * CurrKey.Rotation.Z) +
                           (InvRefKey.Rotation.W * CurrKey.Rotation.W) ;
            
            // Skip if rotation is almost the same
            if (CosAngle > 0.99999999f)
                continue ;
        }

        // Eyely applies the difference between reference frame and the current frame as follows:
        //
        // Variables:   CurrRot,   CurrPos   = Current key frame
        //              AnimRot,   AnimPos   = Animation key frame
        //              InvRefRot, InvRefPos = Inverse key frame of reference frame
        //
        //  Apply delta to current keys
        //          CurrRot = AnimRot * InvRefRot * CurrRot ;
        //          CurrPos = AnimPos + InvRefPos + CurrPos ;
        //
        //

        // Get inverse of reference key
        InvRefKey.Rotation.Invert() ;
        InvRefKey.Translation = -InvRefKey.Translation ;

        // Compute eye key
        anim_key AddKey ;
        AddKey.Rotation    = CurrKey.Rotation    * InvRefKey.Rotation ;
        AddKey.Translation = CurrKey.Translation + InvRefKey.Translation ;

        // Weighted?
        if (m_Weight != 1.0f)
        {
            AddKey.Rotation = BlendToIdentity(AddKey.Rotation, 1.0f - m_Weight) ;
            AddKey.Translation *= m_Weight ;
        }

        // Add delta on top of current keys
        pDestKey[i].Rotation    = AddKey.Rotation    * pDestKey[i].Rotation ;
        pDestKey[i].Translation = AddKey.Translation + pDestKey[i].Translation ;

        #ifdef X_DEBUG
            // Update debug count
            NBonesUsed++ ;
        #endif
    }
}

//=========================================================================

void loco_anim_controller::MaskedMixKeys( const info&               Info, 
                                                s32                 iAnim, 
                                                f32                 Frame, 
                                          const geom::bone_masks&   BoneMasks,
                                                anim_key*           pDestKey )
{
    CONTEXT(" loco_anim_controller::MaskedMixKeys") ;

    // If we aren't playing anything then just return
    if ( (iAnim == -1) || (m_Weight == 0.0f) )
        return;
        
    // Get anim group
    const anim_group& AnimGroup = GetAnimGroup() ;

    // Get bone counts from masks and take into account current LOD
    s32 NBones = x_min( BoneMasks.nBones, Info.m_nActiveBones );
    
    // Exit if nothing to do
    if( NBones == 0 )
        return;

    // If this fires, the above bone count logic in incorrect
    ASSERT( NBones <= Info.m_nActiveBones );

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (NBones <= AnimGroup.GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Lookup anim info
    const anim_info& AnimInfo = AnimGroup.GetAnimInfo(iAnim) ;

    // Loop over all bones and mix keys
    for (s32 i = 0 ; i < NBones ; i++)
    {
        // Skip if masked
        if (AnimInfo.IsBoneMasked(i))
            continue ;

        // Compute bone weight and skip if it has no influence
        f32 Weight = BoneMasks.Weights[ i ] * m_Weight ;
        if (Weight == 0)
            continue ;

        // Lookup key
        anim_key Key ;
        AnimInfo.GetInterpKey(Frame, i, Key) ;

        // Mix key
		pDestKey[i].Interpolate( pDestKey[i], Key, Weight ) ;
    }
}

//=========================================================================

void loco_anim_controller::MaskedMixKeys( const info&               Info, 
                                                s32                 iAnim, 
                                                f32                 Frame, 
                                          const geom::bone_masks&   CurrentBoneMasks,
                                          const geom::bone_masks&   BlendBoneMasks,
                                                f32                 BoneBlend,
                                                anim_key*           pDestKey )
{
    CONTEXT(" loco_anim_controller::MaskedMixKeys") ;

    // If we aren't playing anything then just return
    if ( (iAnim == -1) || (m_Weight == 0.0f) )
        return;
        
    // Get anim group
    const anim_group& AnimGroup = GetAnimGroup() ;
    
    // Get bone counts from masks
    s32 NBones = x_max( CurrentBoneMasks.nBones, BlendBoneMasks.nBones );

    // Take into account current LOD
    NBones = x_min( NBones, Info.m_nActiveBones );

    // Exit if nothing to do
    if( NBones == 0 )
        return;

    // If this fires, the above bone count logic in incorrect
    ASSERT( NBones <= Info.m_nActiveBones );

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (NBones <= AnimGroup.GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Lookup anim info
    const anim_info& AnimInfo = AnimGroup.GetAnimInfo(iAnim) ;

    // Loop over all bones and mix keys
    for (s32 i = 0 ; i < NBones ; i++)
    {
        // Skip if masked
        if (AnimInfo.IsBoneMasked(i))
            continue ;

        // Compute blended bone weight and skip if it has no influence
        f32 Weight = CurrentBoneMasks.Weights[i];
        Weight += BoneBlend * ( BlendBoneMasks.Weights[i] - Weight );
        if (Weight == 0.0f)
            continue ;

        // Lookup key
        anim_key Key ;
        AnimInfo.GetInterpKey(Frame, i, Key) ;

        // Mix key
		pDestKey[i].Interpolate( pDestKey[i], Key, Weight * m_Weight ) ;
    }
}

//=========================================================================

