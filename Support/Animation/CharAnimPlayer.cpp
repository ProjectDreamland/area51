//=========================================================================
//
//  CHARANIMPLAYER.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"
#include "charanimplayer.hpp"
#include "e_ScratchMem.hpp"
#include "Entropy.hpp"


//=========================================================================
//=========================================================================
//=========================================================================
// CHAR_ANIM_PLAYER
//=========================================================================
//=========================================================================
//=========================================================================

char_anim_player::char_anim_player     ( void )
{
    s32 i;

    // Clear track controller ptrs
    for( i=0; i<MAX_CHAR_ANIM_PLAYER_TRACKS; i++ )
        m_Track[i] = NULL;

    // Setup basic controller for track0
    SetTrackController( 0, &m_AnimTrack );
    m_AnimTrack.SetManualYaw( FALSE );
    
    // Clear all tracks
    ClearTracks();

    // Clear out other variables
    m_WorldPos.Zero();
    m_WorldRot.Zero();
    m_WorldScale = 1.0f;
    m_RenderOffset.Zero();
    m_SlideDelta.Zero();
    m_YawDelta = 0.0f;
    m_AnimHandleYaw = 0;
    m_BasisM.Identity();
    m_PreviousBasisM.Identity();
    m_pCachedL2W        = NULL;
    m_CachedL2WIsDirty  = TRUE;
    m_nCachedL2WUpdates = 0;
    m_nCachedL2WDirties = 0;

    m_pCachedBonePos    = NULL;
    m_CachedBonePosIsDirty  = TRUE;
    m_nCachedBonePosUpdates = 0;
    m_nCachedBonePosDirties = 0;
    m_bManualYaw        = FALSE;
    m_bRemoveTurnYaw    = FALSE ;
    m_iMirrorBone = -1;
}

//=========================================================================

char_anim_player::~char_anim_player     ( void )
{
    x_free(m_pCachedL2W);
    x_free(m_pCachedBonePos);
}

//=========================================================================

#if !defined(X_RETAIL)
const anim_group& char_anim_player::GetAnimGroup( void )
{
    anim_group* pG = (anim_group*)m_hAnimGroup.GetPointer();
    ASSERTS( pG, xfs("(%s) was not able to be found",m_hAnimGroup.GetName()) );
    //    m_hAnimGroup.Unlock();
    return *pG;
}
#endif

//=========================================================================

void char_anim_player::SetAnimGroup( const anim_group::handle& hAnimGroup )
{
    MEMORY_OWNER( "ANIM DATA" );

    m_hAnimGroup = hAnimGroup;

    ASSERT( GetAnimGroup().GetNBones() <= MAX_ANIM_BONES );

    // Allocate cached L2W arrays
    x_free( m_pCachedL2W );
    m_pCachedL2W = (matrix4*)x_malloc(sizeof(matrix4)*GetAnimGroup().GetNBones());
    ASSERT( m_pCachedL2W );
    DirtyCachedL2W();

    // Allocate cached bone position arrays
    x_free( m_pCachedBonePos );
    m_pCachedBonePos = (vector3*)x_malloc(sizeof(vector3)*GetAnimGroup().GetNBones());
    ASSERT( m_pCachedBonePos );
    DirtyCachedBonePos();

    // Notify track controllers of anim group
    for( s32 i=0; i<MAX_CHAR_ANIM_PLAYER_TRACKS; i++ )
    if( m_Track[i] )
        m_Track[i]->SetAnimGroup( hAnimGroup );
}


//=========================================================================

void char_anim_player::SetAnimHoriz( s32 iAnim, f32 BlendTime )
{
    SetAnim( iAnim, FALSE, TRUE, BlendTime );
}

//=========================================================================

void char_anim_player::SetAnimVert( s32 iAnim, f32 BlendTime )
{
    SetAnim( iAnim, TRUE, FALSE, BlendTime );
}

//=========================================================================

void char_anim_player::SetAnim( s32 iAnim, xbool ManualVert, xbool ManualHoriz, f32 BlendTime , xbool ResetFrameCount )
{
    // Lookup group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if (!pAnimGroup)
        return;
    
    // Select random anim
    iAnim = pAnimGroup->GetRandomAnimIndex( iAnim, m_AnimTrack.GetAnimIndex() );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_AnimTrack.SetAnim( iAnim, BlendTime , ResetFrameCount );
    m_AnimHandleYaw = m_AnimTrack.GetAnimInfo().GetHandleAngle();

    m_bManualVert = ManualVert;
    m_bManualHoriz = ManualHoriz;

    m_RenderOffset.Zero();
    m_SlideDelta.Zero();
    m_YawDelta = 0.0f;
}

//=========================================================================

void char_anim_player::SetAnimHoriz( const char* pAnimName, f32 BlendTime )
{
    SetAnim( GetAnimIndex(pAnimName), FALSE, TRUE, BlendTime );
}

//=========================================================================

void char_anim_player::SetAnimVert( const char* pAnimName, f32 BlendTime )
{
    SetAnim( GetAnimIndex(pAnimName), TRUE, FALSE, BlendTime );
}

//=========================================================================

void char_anim_player::SetManualYawControl ( xbool bIsManual )
{
    m_bManualYaw = bIsManual;

    m_AnimTrack.SetManualYaw( bIsManual );
}

//=========================================================================

void char_anim_player::SetOverrideRootBlend( xbool bOverrideRootBlend )
{
    m_AnimTrack.SetOverrideRootBlend( bOverrideRootBlend );
}

//=========================================================================

void char_anim_player::SetRemoveTurnYaw( xbool bRemoveTurnYaw )
{
    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    // Keep
    m_bRemoveTurnYaw = bRemoveTurnYaw ;

    // Tell all tracks
    for ( s32 i = 0 ; i < MAX_CHAR_ANIM_PLAYER_TRACKS ; i++ )
    {
        if( m_Track[i] )
            m_Track[i]->SetRemoveTurnYaw(bRemoveTurnYaw) ;
    }
}

//=========================================================================

void char_anim_player::Advance( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw )
{
    CONTEXT( "char_anim_player::Advance" );

    // If completely frozen just return
    if( nSeconds==0 )
    {
        // but make sure the return data is valid!
        DeltaPos.Zero();
        DeltaYaw = 0;
        return;
    }

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    s32 i;
    const anim_info& AnimData = m_AnimTrack.GetAnimInfo();

    //
    // Get first and last keys of Track0
    //
    anim_key Key0;
    anim_key Key1;
    AnimData.GetRawKey(0,0,Key0);
    AnimData.GetRawKey(AnimData.GetNFrames()-1,0,Key1);

    //
    // Get root key from old frame
    //
    anim_key OldKey;
    AnimData.GetInterpKey( m_AnimTrack.GetFrame(), 0, OldKey );

    //
    // Advance all tracks
    //
    for( i=0; i<MAX_CHAR_ANIM_PLAYER_TRACKS; i++ )
    if( m_Track[i] )
        m_Track[i]->Advance( nSeconds );

    //
    // Begin accumulating deltas
    //
    DeltaPos.Zero();
    DeltaYaw = 0;

    // Are we not in the same cycle?
    s32 OldCycle = m_AnimTrack.GetPrevCycle();
    s32 NewCycle = m_AnimTrack.GetCycle();

    // Catch up to current cycle
    while( OldCycle != NewCycle )
    {
        // Advance to end of this cycle
        DeltaPos += Key1.Translation - OldKey.Translation;
        DeltaYaw += x_MinAngleDiff( Key1.Rotation.GetRotation().Yaw, OldKey.Rotation.GetRotation().Yaw );
        OldKey = Key0;
        OldCycle++;
    }

    //
    // Get new key
    //
    anim_key NewKey;
    AnimData.GetInterpKey( m_AnimTrack.GetFrame(), 0, NewKey );

    //
    // Compute deltas in animation space
    //
    DeltaPos += NewKey.Translation - OldKey.Translation;
    DeltaYaw += x_MinAngleDiff( NewKey.Rotation.GetRotation().Yaw, OldKey.Rotation.GetRotation().Yaw );

    // Scale deltapos
    DeltaPos *= m_WorldScale;
    
    // Rotate DeltaPos into world
    matrix4 AnimToWorldM;
    ComputeAnimToWorldM( AnimToWorldM );
    DeltaPos = AnimToWorldM * DeltaPos;

    // Apply slide delta in world space
    DeltaPos += m_SlideDelta * nSeconds;

//    DeltaYaw += m_YawDelta * nSeconds;

    vector3 DeltaI;
    vector3 DeltaJ;
    vector3 DeltaK;
    CalcBasisVectors(DeltaPos, DeltaI, DeltaJ, DeltaK);
    DeltaPos.Zero();
    if (m_bManualVert)
    {
        DeltaPos += DeltaJ;
    }
    if (m_bManualHoriz)
    {
        DeltaPos += DeltaI;
        DeltaPos += DeltaK;
    }

    if (m_bManualYaw && 
        m_AnimTrack.GetNFrames() &&
        m_AnimTrack.GetFrame() > m_YawStartFrame &&
        m_AnimTrack.GetFrame() < m_YawEndFrame )
    {
        // ok, m_YawDelta is being used to specify how far we SHOULD turn per frame
        // calc the difference between desired and actual
//        radian fTotalYaw = AnimData.GetTotalYaw();

        f32 fVal = (m_YawDelta) * nSeconds;

        DeltaYaw += fVal;
    }

    // There should be no delta yaw if we are removing turn yaw and it's not a turn anim playing
    if ( (m_bRemoveTurnYaw) && (!m_AnimTrack.IsPlayingTurnAnim()) )
        DeltaYaw = 0 ;
}

//=========================================================================

void char_anim_player::SetRotationAndPosition( const matrix4& L2W )
{
    ASSERT( L2W.IsValid() );

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_WorldRot = L2W.GetRotation();
    m_WorldPos = L2W.GetTranslation();

    ASSERT( x_abs( m_WorldRot.Pitch ) < 1000.0f );
    ASSERT( x_abs( m_WorldRot.Yaw   ) < 1000.0f );
    ASSERT( x_abs( m_WorldRot.Roll  ) < 1000.0f );

}

//=========================================================================

xbool char_anim_player::GetPropL2W      ( const char* pPropName, matrix4& L2W ) 
{

    const anim_info& AnimData = m_AnimTrack.GetAnimInfo();

    // Is this prop channel present?
    s32 PropChannel = AnimData.GetPropChannel(pPropName);
    if( PropChannel == -1 )
    {
        L2W.Identity();
        return FALSE;
    }

    // Get key from raw prop animation
    anim_key PropKey;
    AnimData.GetPropInterpKey( PropChannel, m_AnimTrack.GetFrame(), PropKey);

    // Build final L2W for prop
    PropKey.Setup(L2W) ;

    // Put into world space if attached to a parent bone
    s32 iParent = AnimData.GetPropParentBoneIndex( PropChannel );
    if (iParent != -1)
    {
        const matrix4& ParentM = GetCachedL2W(iParent);
        L2W = ParentM * L2W;
    }

    // return
    return TRUE;
}


//=========================================================================

vector3 char_anim_player::GetWorldAnimTranslation( void )
{
    vector3 TotalTrans = m_AnimTrack.GetAnimInfo().GetTotalTranslation();
    TotalTrans *= m_WorldScale;

    // Rotate DeltaPos into world
    matrix4 AnimToWorldM;
    ComputeAnimToWorldM( AnimToWorldM );
    TotalTrans = AnimToWorldM * TotalTrans;

    return TotalTrans;
}

//=========================================================================

void char_anim_player::PrepareRootKey( anim_key& Key )
{
    // Get AnimToWorldM
    matrix4 AnimToWorldM;
    ComputeAnimToWorldM(AnimToWorldM);

    // Copy root key info into temporary variables
    vector3     RootTrans = Key.Translation;
    quaternion  RootRot   = Key.Rotation;
    f32         RootScale = m_WorldScale;

    // Scale root translation
    RootTrans *= RootScale;

    // Transform root translation and rotation into world
    RootTrans = AnimToWorldM * RootTrans;
    RootRot   = (AnimToWorldM * (matrix4)RootRot).GetQuaternion();

    vector3 RootTransI;
    vector3 RootTransJ;
    vector3 RootTransK;
    CalcBasisVectors( RootTrans, RootTransI, RootTransJ, RootTransK );
    RootTrans = m_WorldPos;

//    f32 test1 = RootTrans.Length();
//    f32 test2 = RootTransI.Length() + RootTransJ.Length() + RootTransK.Length();

    // Override root position if being manually controlled
    if( !m_bManualHoriz )
    {
        RootTrans += RootTransI;
        RootTrans += RootTransK;
    }

    if( !m_bManualVert ) 
    {
        RootTrans += RootTransJ;
    }

    // Transform render offset into world and add to root translation
    RootTrans += AnimToWorldM * m_RenderOffset;

    // Put data back into Key[0]
    Key.Rotation = RootRot;
    Key.Translation = RootTrans;
#if USE_SCALE_KEYS
    Key.Scale = vector3(RootScale,RootScale,RootScale);
#endif

}

//=========================================================================

void char_anim_player::GetInterpKeys( anim_key* pKey ) 
{
    s32 i;

    // Get original keys from track0
    m_AnimTrack.GetInterpKeys( pKey );

    // Modify root key before other tracks so they can respond
    PrepareRootKey( pKey[0] );

    // Loop through tracks and mix in
    for( i=1; i<MAX_CHAR_ANIM_PLAYER_TRACKS; i++ )
    if( m_Track[i] )
        m_Track[i]->MixKeys( pKey );
}

//=========================================================================
/*
void char_anim_player::GetInterpKey( s32 iBone, anim_key& Key ) 
{
    s32 i;

    // Get original keys from track0
    m_AnimTrack.GetInterpKey( iBone, Key );

    // Modify root key before other tracks so they can respond
    if( iBone == 0 )
        PrepareRootKey( Key );

    // Loop through tracks and mix in
    for( i=1; i<MAX_CHAR_ANIM_PLAYER_TRACKS; i++ )
    if( m_Track[i] )
        m_Track[i]->MixKey( iBone, Key );

}
*/

//=========================================================================
//=========================================================================
//=========================================================================
// EVENTS
//=========================================================================
//=========================================================================
//=========================================================================

vector3 char_anim_player::GetEventPosition( s32 iEvent ) 
{
    const anim_event& EV = m_AnimTrack.GetEvent(iEvent);
    
	//event_data eventData = EV.GetData();  // For debugging!

	const matrix4& BoneM = GetCachedL2W( EV.GetInt( anim_event::INT_IDX_BONE ) );
    vector3 P = BoneM * EV.GetPoint( anim_event::POINT_IDX_OFFSET );
    return P;
}

//=========================================================================

radian3 char_anim_player::GetEventRotation( s32 iEvent ) 
{
    const anim_event& EV = m_AnimTrack.GetEvent(iEvent);
    
	//event_data eventData = EV.GetData();

	const matrix4& BoneM = GetCachedL2W( EV.GetInt( anim_event::INT_IDX_BONE ) );

    vector3 ERot( EV.GetPoint( anim_event::POINT_IDX_ROTATION ) );
    radian3 Rot( ERot.GetX(), ERot.GetY(), ERot.GetZ() );

    matrix4 EventRot(Rot);
    matrix4 WorldRot = BoneM * EventRot;
    Rot = WorldRot.GetRotation();

    return Rot;
}

//=========================================================================

vector3 char_anim_player::GetEventPosition( const anim_event& Event )
{
	// Get the world position.
    const matrix4& BoneM = GetCachedL2W( Event.GetInt( anim_event::INT_IDX_BONE ) );
    vector3 P = BoneM * Event.GetPoint( anim_event::POINT_IDX_OFFSET );
    return P;
}

//=========================================================================

radian3 char_anim_player::GetEventRotation( const anim_event& Event )
{
	// Get Rotation.
    const matrix4& BoneM = GetCachedL2W( Event.GetInt( anim_event::INT_IDX_BONE ) );

    vector3 ERot( Event.GetPoint( anim_event::POINT_IDX_ROTATION ) );
    radian3 Rot( ERot.GetX(), ERot.GetY(), ERot.GetZ() );

    matrix4 EventRot(Rot);
    matrix4 WorldRot = BoneM * EventRot;
    Rot = WorldRot.GetRotation();

    return Rot;
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

void char_anim_player::RenderSkeleton( xbool LabelBones )
{
    s32 i;
    s32 nBones = GetNBones();
    
    // Render bones and joints
    for( i=0; i<nBones; i++ )
    {
        vector3 BP = GetBonePosition( i );
        vector3 PP = BP;

        if( GetBone(i).iParent != -1 )
            PP = GetBonePosition( GetBone(i).iParent );

        draw_Line( BP, PP, XCOLOR_GREEN );
        draw_Marker( BP, XCOLOR_RED );
    }

    // Label bones
    if( LabelBones )
    {
        for( i=0; i<nBones; i++ )
        {
            draw_Label( GetBonePosition( i ), XCOLOR_WHITE, GetBone(i).Name );
        }
    }
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================
//=========================================================================
//=========================================================================
// TRACK MANAGEMENT
//=========================================================================
//=========================================================================
//=========================================================================

void char_anim_player::ClearTracks( void )
{
    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    for( s32 i=1; i<MAX_CHAR_ANIM_PLAYER_TRACKS; i++ )
    {
        if( m_Track[i] )
            m_Track[i]->Clear();
    }
}

//=========================================================================

void char_anim_player::ClearTrack( s32 iTrack )
{
    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    ASSERT( (iTrack>=0) && (iTrack<MAX_CHAR_ANIM_PLAYER_TRACKS) );
    if( m_Track[iTrack] )
        m_Track[iTrack]->Clear();
}

//=========================================================================

void char_anim_player::SetTrackController( s32 iTrack, track_controller* pTrackController )
{
    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    ASSERT( (iTrack>=0) && (iTrack<MAX_CHAR_ANIM_PLAYER_TRACKS) );
    m_Track[iTrack] = pTrackController;
    if( m_Track[iTrack] )
    {
        m_Track[iTrack]->Clear();
    }
}

//=========================================================================

void char_anim_player::SetSlideDelta( const vector3& SlideDelta )
{
    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    f32 nSeconds = (f32)m_AnimTrack.GetNFrames() / (f32)m_AnimTrack.GetAnimInfo().GetFPS();
    m_SlideDelta = SlideDelta / nSeconds;
}

//=========================================================================

void char_anim_player::SetYawDelta( const radian& YawDelta  )
{
    // Specify exactly how far we WANT the animation to turn over a given frame

    // Mark cached L2W matrices as unusable
    DirtyCachedL2W(); 

    m_YawStartFrame = 0;
    m_YawEndFrame = (f32)m_AnimTrack.GetNFrames();
    // look for a start and or end event. 
    s32 c;
    for(c=0;c<m_AnimTrack.GetNEvents();c++)
    {
        anim_event animEvent = m_AnimTrack.GetEvent(c);
        if( !x_strcmp(animEvent.GetType(), "Generic") )
        {
            if( !x_strcmp(animEvent.GetString( anim_event::STRING_IDX_GENERIC_TYPE ), "Yaw Begin") )
            {
                m_YawStartFrame = (f32)animEvent.StartFrame();
            }
            else if( !x_strcmp(animEvent.GetString( anim_event::STRING_IDX_GENERIC_TYPE ), "Yaw End") )
            {
                m_YawEndFrame = (f32)animEvent.EndFrame();
            }        
        }
    }
    f32 nSeconds = (m_YawEndFrame-m_YawStartFrame) / (f32)m_AnimTrack.GetAnimInfo().GetFPS();
    m_YawDelta = YawDelta / nSeconds;
}

//=========================================================================

void char_anim_player::ComputeAnimToWorldM ( matrix4& M )
{
    radian3 Rot = m_WorldRot;
    Rot.Yaw = m_WorldRot.Yaw - m_AnimHandleYaw;
    if (m_bManualYaw)
    {
        const anim_info& AnimData = m_AnimTrack.GetAnimInfo();

        //
        // Get root key from old frame
        //
        anim_key OldKey;
        AnimData.GetInterpKey( m_AnimTrack.GetFrame(), 0, OldKey );

        Rot.Yaw -= OldKey.Rotation.GetRotation().Yaw;
    }
    M.Identity();
    M.SetRotation( Rot );
    M = m_BasisM * M;
}

//=========================================================================

void char_anim_player::CalcBasisVectors( const vector3& world, vector3& i, vector3& j, vector3& k)
{
    vector3 I,J,K;
    matrix4 TB;
    TB = m_BasisM;
    TB.GetColumns(I,J,K);
    TB.Transpose();

    vector3 BasisSpace = TB * world;

    i =  I * BasisSpace.GetX();
    j =  J * BasisSpace.GetY();
    k =  K * BasisSpace.GetZ();
}

//=========================================================================
//=========================================================================
//=========================================================================
// CACHED L2W
//=========================================================================
//=========================================================================
//=========================================================================

void char_anim_player::DirtyCachedL2W( void )
{
    m_CachedL2WIsDirty = TRUE;
    m_nCachedL2WDirties++;
    DirtyCachedBonePos();
}

//=========================================================================

const matrix4* char_anim_player::GetCachedL2Ws( void )
{
    if( m_CachedL2WIsDirty )
        UpdateCachedL2W();

    return m_pCachedL2W;
}

//=========================================================================

const matrix4& char_anim_player::GetCachedL2W( s32 iBone )
{
    ASSERT( (iBone>=0) && (iBone<GetAnimGroup().GetNBones()) );

    if( m_CachedL2WIsDirty )
        UpdateCachedL2W();

    return m_pCachedL2W[iBone];
}

//=========================================================================

void char_anim_player::ComputeBonesL2W( const matrix4& L2W, anim_key* pKey, matrix4* pBoneL2W )
{
    s32 i;

    // Lookup anim group
    const anim_group& AnimGroup = GetAnimGroup() ;

    // Convert all keys to matrices and put into world space
    s32 nBones = AnimGroup.GetNBones();
    for( i=0; i< nBones; i++ )
    {
        // Lookup bone
        const anim_bone& Bone = AnimGroup.GetBone(i);

        // Setup L2W
        pKey[i].Setup(pBoneL2W[i]) ;

        // Concatenate with parent or L2W
        const matrix4* PM = (Bone.iParent == -1) ? (&L2W):(&pBoneL2W[Bone.iParent]);
        pBoneL2W[i] = (*PM) * pBoneL2W[i] ;

        // Mirror bone in local space?
        if( i == m_iMirrorBone )
        {
            // Compute local -> world matrix
            matrix4 L2W;
            L2W.Setup( vector3( m_WorldScale, m_WorldScale, m_WorldScale ), m_WorldRot, m_WorldPos );

            // Compute world -> local matrix
            matrix4 W2L = L2W;
            if( m_WorldScale != 1.0f )
                W2L.InvertSRT();
            else
                W2L.InvertRT();

            // Compute flip matrix
            matrix4 F;
            F.Identity();
            F.SetScale( vector3( -1, 1, 1 ) );

            // Mirror bone in local space, then put back into world space
            pBoneL2W[i] = L2W * F * W2L * pBoneL2W[i];
        }
    }

    // Apply bind matrices
    for( i=0; i< nBones; i++ )
    {
        const anim_bone& Bone = AnimGroup.GetBone(i);
        pBoneL2W[i] = pBoneL2W[i] * Bone.BindMatrixInv;
    }
}

//=========================================================================

void char_anim_player::UpdateCachedL2W( void )
{
    if( !m_CachedL2WIsDirty )
        return;

    m_CachedL2WIsDirty = FALSE;
    m_nCachedL2WUpdates++;

    // Allocate mix buffer
    anim_key* MixBuffer = base_player::GetMixBuffer( base_player::MIX_BUFFER_PLAYER );
    ASSERT(MixBuffer);

    // Get keys
    matrix4 L2W;
    L2W.Identity();
    
    // Get keys
    GetInterpKeys( MixBuffer );

    // Build matrices
    ComputeBonesL2W( L2W, MixBuffer, m_pCachedL2W );
}

//=========================================================================
//=========================================================================
//=========================================================================
// CACHED BONE POSITIONS
//=========================================================================
//=========================================================================
//=========================================================================

void char_anim_player::DirtyCachedBonePos( void )
{
    m_CachedBonePosIsDirty = TRUE;
    m_nCachedBonePosDirties++;
}

//=========================================================================

const vector3* char_anim_player::GetCachedBonePoss   ( void )
{
    if( m_CachedBonePosIsDirty )
        UpdateCachedBonePos();

    return m_pCachedBonePos;
}

//=========================================================================

const vector3& char_anim_player::GetCachedBonePos    ( s32 iBone )
{
    ASSERT( (iBone>=0) && (iBone<GetAnimGroup().GetNBones()) );

    if( m_CachedBonePosIsDirty )
        UpdateCachedBonePos();

    return m_pCachedBonePos[iBone];
}

//=========================================================================

void char_anim_player::UpdateCachedBonePos ( void )
{
    if( !m_CachedBonePosIsDirty )
        return;

    m_CachedBonePosIsDirty = FALSE;
    m_nCachedBonePosUpdates++;

    const matrix4* pL2W = GetCachedL2Ws();
    const anim_group& AnimGroup = GetAnimGroup() ;
    s32 nBones = AnimGroup.GetNBones() ;

    for( s32 i=0; i<nBones; i++ )
    {
        m_pCachedBonePos[i] = pL2W[i] * AnimGroup.GetBone(i).BindTranslation;
    }
}

//=========================================================================
