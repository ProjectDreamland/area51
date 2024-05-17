//=========================================================================
//
//  ANIMDATA.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

//#define CRT_SANITY      ASSERT(_CrtCheckMemory())
#define CRT_SANITY      

#include "animdata.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Geometry\geom.hpp"

static s32 CHUNK_SIZE = 64*1024*1024;

//=========================================================================
//=========================================================================
//=========================================================================
// SUPPORT
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================

void SwapEndian( f32& V )
{
    u32 I = *((u32*)(&V));
    I = (I>>24) | (I<<24) | ((I&0x00FF0000) >> 8) | ((I&0x0000FF00) << 8);
    *((u32*)(&V)) = I;
}

void SwapEndian( u16& V )
{
    u16 I = *((u16*)(&V));
    I = (I>>8) | (I<<8);
    *((u16*)(&V)) = I;
}

void SwapEndian( s16& V )
{
    u16 I = *((u16*)(&V));
    I = (I>>8) | (I<<8);
    *((u16*)(&V)) = I;
}

void SwapEndian( s32& V )
{
    u32 I = *((u32*)(&V));
    I = (I>>24) | (I<<24) | ((I&0x00FF0000) >> 8) | ((I&0x0000FF00) << 8);
    *((u32*)(&V)) = I;
}

void SwapEndian( u32& V )
{
    u32 I = *((u32*)(&V));
    I = (I>>24) | (I<<24) | ((I&0x00FF0000) >> 8) | ((I&0x0000FF00) << 8);
    *((u32*)(&V)) = I;
}

void SwapEndian( u64& V )
{
    (void)V;
    //u64 I = *((u64*)(&V));
    //I = (I>>24) | (I<<24) | ((I&0x00FF0000) >> 8) | ((I&0x0000FF00) << 8);
    //*((u32*)(&V)) = I;
}

template< class T >
void SwapEndian( T*& V )
{
    u32 I = *((u32*)(&V));
    I = (I>>24) | (I<<24) | ((I&0x00FF0000) >> 8) | ((I&0x0000FF00) << 8);
    *((u32*)(&V)) = I;
}

void SwapEndian( f32* pF, s32 nFloats )
{
    s32 i;
    u32* pI = (u32*)pF;

    for( i=0; i<nFloats; i++ )
    {
        u32 I = pI[i];
        I = (I>>24) | (I<<24) | ((I&0x00FF0000) >> 8) | ((I&0x0000FF00) << 8);
        pI[i] = I;
    }
}

void SwapEndian( vector3& V )
{
    SwapEndian( (f32*)&V, 3 );
}

void SwapEndian( quaternion& V )
{
    SwapEndian( (f32*)&V, 4 );
}

void SwapEndian( matrix4& V )
{
    SwapEndian( (f32*)&V, 16 );
}


//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_PROP
//=========================================================================
//=========================================================================
//=========================================================================
anim_prop::anim_prop()
{
}

anim_prop::~anim_prop()
{
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_SKEL
//=========================================================================
//=========================================================================
//=========================================================================

anim_skel::anim_skel( void )
{
    m_pBone = NULL;
    m_nBones = 0;
}

//=========================================================================

anim_skel::~anim_skel( void )
{
    delete[] m_pBone;
    m_pBone = NULL;
    m_nBones = 0;
}

//=========================================================================
f32 g_SkelSkinScale = 1.0f;

void anim_skel::ComputeBonesL2W ( const matrix4& L2W, anim_key* pKey, matrix4* pBoneL2W ) const
{
    s32 i;
    //matrix4 SM;
    //SM.Identity();
    //SM.Scale(vector3(g_SkelSkinScale,g_SkelSkinScale,g_SkelSkinScale));

    for( i=0; i<m_nBones; i++ )
    {
        // Setup L2W
        pBoneL2W[i].Setup( pKey[i].Scale, pKey[i].Rotation, pKey[i].Translation );

        // Concatenate with parent or L2W
        matrix4* PM = (m_pBone[i].iParent == -1) ? ((matrix4*)&L2W):(&pBoneL2W[m_pBone[i].iParent]);
        pBoneL2W[i] = (*PM) * pBoneL2W[i];
    }

    // Apply bind matrices
    for( i=0; i<m_nBones; i++ )
    {
        pBoneL2W[i] = pBoneL2W[i] * m_pBone[i].BindMatrixInv;
    }
}

//=========================================================================

void anim_skel::ComputeBoneL2W ( s32 iBone, const matrix4& L2W, anim_key* pKey, matrix4& BoneL2W ) const
{
    // Clear bone matrix
    BoneL2W.Identity();

    // Run hierarchy from bone to root node
    s32 I = iBone;
    while( I != -1 )
    {
        matrix4 LM;
        LM.Setup( pKey[I].Scale, pKey[I].Rotation, pKey[I].Translation );
        BoneL2W = LM * BoneL2W;
        I = m_pBone[I].iParent;
    }

    // Concatenate L2W 
    BoneL2W = L2W * BoneL2W;

    // Apply bind matrix
    BoneL2W = BoneL2W * m_pBone[iBone].BindMatrixInv;
}

//=========================================================================

vector3 anim_skel::GetEventPos( s32 iBone, const vector3& Offset, anim_key* pKey ) const
{
    matrix4 BoneM;
    matrix4 IdentM;
    IdentM.Identity();

    ComputeBoneL2W( iBone, IdentM, pKey, BoneM );

    vector3 P = BoneM * Offset;
    return P;
}

//=========================================================================

s32 anim_skel::GetBoneParent   ( s32 iBone ) const
{
    return m_pBone[iBone].iParent;
}

//=========================================================================

const matrix4&  anim_skel::GetBoneBindMatrix ( s32 iBone ) const
{
    return m_pBone[iBone].BindMatrix;
}

//=========================================================================

s32	anim_skel::GetBoneIDFromName( const char* pName ) const
{
    s32 i;

    for( i=0; i<m_nBones; i++ )
    if( x_stricmp( m_pBone[i].Name,pName ) == 0 )
        return i;

    return -1;
}

//=========================================================================

const anim_bone& anim_skel::GetBone( s32 iBone ) const
{
    ASSERT( (iBone>=0) && (iBone<m_nBones) );
    return m_pBone[iBone];
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEY
//=========================================================================
//=========================================================================
//=========================================================================

void anim_key::InterpolateSRT( const anim_key& K0, const anim_key& K1, f32 T )
{
    Rotation    = Blend( K0.Rotation, K1.Rotation, T );
    Scale       = K0.Scale       + T*(K1.Scale       - K0.Scale);
    Translation = K0.Translation + T*(K1.Translation - K0.Translation);
}

void anim_key::InterpolateSR( const anim_key& K0, const anim_key& K1, f32 T )
{
    Rotation    = Blend( K0.Rotation, K1.Rotation, T );
    Scale       = K0.Scale       + T*(K1.Scale       - K0.Scale);
    Translation = K1.Translation;
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEY_SET
//=========================================================================
//=========================================================================
//=========================================================================

anim_key_set::anim_key_set( void )
{
}

//=========================================================================

anim_key_set::~anim_key_set( void )
{
}


//=========================================================================

void anim_key_set::GetInterpKey( f32  Frame, anim_key& Key ) const
{
    // Compute integer and fractional frame
    s32 iFrame0 = (s32)Frame;
    s32 iFrame1 = (iFrame0+1)%m_nFrames;
    ASSERT( iFrame0 < (m_nFrames-1) );
    ASSERT( iFrame1 > iFrame0 );

    //ASSERT( iFrame1 < m_nFrames );
    f32 fFrame  = Frame - iFrame0;

    anim_key K0;
    anim_key K1;

    GetRawKey( iFrame0, K0 );
    GetRawKey( iFrame1, K1 );

    Key.InterpolateSRT( K0, K1, fFrame );
}

//=========================================================================

s32 anim_key_set::GetNBytes( void )
{
    s32 NBytes = 0;

    //
    // Handle scale
    //
    if( m_ScaleFormat == KNOWN_CONSTANT ) NBytes += 0;
    else
    if( m_ScaleFormat == FULL_PRECISION ) NBytes += 12*m_nFrames;
    else
    if( m_ScaleFormat == SINGLE_VALUE ) NBytes += 12;


    //
    // Handle rotation
    //
    if( m_RotationFormat == QUAT_8_PACK ) {ASSERT(FALSE);}
    else
    if( m_RotationFormat == PRECISION_8 ) NBytes += 8*4 + 4*m_nFrames;
    else
    if( m_RotationFormat == PRECISION_16 ) NBytes += 8*m_nFrames;
    else
    if( m_RotationFormat == FULL_PRECISION ) NBytes += 4*4*m_nFrames;
    else
    if( m_RotationFormat == SINGLE_VALUE ) NBytes += 4*4;


    //
    // Handle translation
    //
    if( m_TranslationFormat == SINGLE_VALUE_16 ) NBytes += ALIGN_4(2*3);
    else
    if( m_TranslationFormat == PRECISION_16 ) NBytes += ALIGN_4(2*3*m_nFrames);
    else
    if( m_TranslationFormat == FULL_PRECISION ) NBytes += 12*m_nFrames;
    else
    if( m_TranslationFormat == SINGLE_VALUE ) NBytes += 12;

    return NBytes;
}

//=========================================================================

void anim_key_set::GetRawKey( s32 iFrame, anim_key& Key ) const
{
    //
    // Get Scale
    //
    {

        if( m_ScaleFormat == KNOWN_CONSTANT )
        {
            Key.Scale.X = 1.0f;
            Key.Scale.Y = 1.0f;
            Key.Scale.Z = 1.0f;
        }
        else
        if( m_ScaleFormat == FULL_PRECISION )
        {
            vector3* pD = (vector3*)m_pScale;
            Key.Scale = pD[iFrame];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Scale );  
            #endif
        }
        else
        if( m_ScaleFormat == SINGLE_VALUE )
        {
            vector3* pD = (vector3*)m_pScale;
            Key.Scale = pD[0];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Scale );  
            #endif
        }
    }

    //
    // Get Rotation
    //
    {
        if( m_RotationFormat == QUAT_8_PACK )
        {
            static f32 ITable[16] = {0,0.142857f,0.28571f,0.428571f,0.571428f,0.714285f,0.857143f,1.0f,
                                    0.1f,0.2f,0.3f,0.5f,0.6f,0.7f,0.8f,0.9f};

            s32 iPack = iFrame >> 3;
            s32 iQ    = iFrame & 7;
            u64 PA = *((u64*)(m_pRotation + sizeof(u64)*(iPack<<1) + 0));
            u64 PB = *((u64*)(m_pRotation + sizeof(u64)*(iPack<<1) + sizeof(u64)));

            #ifdef TARGET_GCN
            SwapEndian(PA);
            SwapEndian(PB);
            #endif

            // unpack boundary quaternions
            quaternion QA,QB;
            QA.X = (((s64)PA<<(13*0))>>(64-(13*1))) * (1.0f/4095.0f);
            QA.Y = (((s64)PA<<(13*1))>>(64-(13*1))) * (1.0f/4095.0f);
            QA.Z = (((s64)PA<<(13*2))>>(64-(13*1))) * (1.0f/4095.0f);
            QA.W = (((s64)PA<<(13*3))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.X = (((s64)PB<<(13*0))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.Y = (((s64)PB<<(13*1))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.Z = (((s64)PB<<(13*2))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.W = (((s64)PB<<(13*3))>>(64-(13*1))) * (1.0f/4095.0f);

            // Blend between the boundary quaternions
            f32 T;
            if( iQ==0 ) T = 0.0f;
            else
            if( iQ==7 ) T = 1.0f;
            else
            if( iQ<4 ) T = ITable[((PA>>((3-iQ)<<2)) & 0xF)];
            else       T = ITable[((PB>>((6-iQ)<<2)) & 0xF)];

            Key.Rotation = Blend(QA,QB,T);

        }
        else
        if( m_RotationFormat == PRECISION_8 )
        {
            f32 Min[4];
            f32 Max[4];
            Min[0] = ((f32*)m_pRotation)[0];
            Min[1] = ((f32*)m_pRotation)[1];
            Min[2] = ((f32*)m_pRotation)[2];
            Min[3] = ((f32*)m_pRotation)[3];
            Max[0] = ((f32*)m_pRotation)[4];
            Max[1] = ((f32*)m_pRotation)[5];
            Max[2] = ((f32*)m_pRotation)[6];
            Max[3] = ((f32*)m_pRotation)[7];

            #ifdef TARGET_GCN
            SwapEndian( Min[0] );
            SwapEndian( Min[1] );
            SwapEndian( Min[2] );
            SwapEndian( Min[3] );
            SwapEndian( Max[0] );
            SwapEndian( Max[1] );
            SwapEndian( Max[2] );
            SwapEndian( Max[3] );
            #endif

            byte* pI = m_pRotation + (sizeof(f32)*8) + (iFrame<<2);

            Key.Rotation.X = Min[0] + (pI[0]/255.0f)*(Max[0]-Min[0]);
            Key.Rotation.Y = Min[1] + (pI[1]/255.0f)*(Max[1]-Min[1]);
            Key.Rotation.Z = Min[2] + (pI[2]/255.0f)*(Max[2]-Min[2]);
            Key.Rotation.W = Min[3] + (pI[3]/255.0f)*(Max[3]-Min[3]);
        }
        else
        if( m_RotationFormat == PRECISION_16 )
        {
            s16* pD = (s16*)m_pRotation;
            pD += (iFrame<<2);
            s16 D[4];
            D[0] = pD[0];
            D[1] = pD[1];
            D[2] = pD[2];
            D[3] = pD[3];

            #ifdef TARGET_GCN
            SwapEndian(D[0]);
            SwapEndian(D[1]);
            SwapEndian(D[2]);
            SwapEndian(D[3]);
            #endif

            Key.Rotation.X = (f32)D[0] * (1.0f/16384.0f);
            Key.Rotation.Y = (f32)D[1] * (1.0f/16384.0f);
            Key.Rotation.Z = (f32)D[2] * (1.0f/16384.0f);
            Key.Rotation.W = (f32)D[3] * (1.0f/16384.0f);
        }
        else
        if( m_RotationFormat == FULL_PRECISION )
        {
            quaternion* pD = (quaternion*)m_pRotation;
            Key.Rotation = pD[iFrame];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Rotation );  
            #endif
        }
/*
        else
        if( m_RotationFormat == ROT_ON_AXIS )
        {
            s16* pD = (s16*)m_pRotation;

            vector3 Axis = ((vector3*)pD)[0];
            pD = (s16*)(&((vector3*)pD)[1]);

            radian Angle = (f32)pD[iFrame]*(R_360/16384.0f);
            Key.Rotation.Setup( Axis, Angle );
        }
*/
        else
        if( m_RotationFormat == SINGLE_VALUE )
        {
            quaternion* pD = (quaternion*)m_pRotation;
            Key.Rotation = pD[0];
            #ifdef TARGET_GCN
            SwapEndian( Key.Rotation );
            #endif
        }

    }

    //
    // Get Translation
    //
    {
        if( m_TranslationFormat == SINGLE_VALUE_16 )
        {
            s16* pD = (s16*)m_pTranslation;
            s16 D[3];
            D[0] = pD[0];
            D[1] = pD[1];
            D[2] = pD[2];

            #ifdef TARGET_GCN
            SwapEndian(D[0]);
            SwapEndian(D[1]);
            SwapEndian(D[2]);
            #endif

            Key.Translation.X = (f32)D[0]*(1.0f/16.0f);
            Key.Translation.Y = (f32)D[1]*(1.0f/16.0f);
            Key.Translation.Z = (f32)D[2]*(1.0f/16.0f);
        }
        else
        if( m_TranslationFormat == PRECISION_16 )
        {
            s16* pD = (s16*)m_pTranslation;
            pD += (iFrame*3);
            s16 D[3];
            D[0] = pD[0];
            D[1] = pD[1];
            D[2] = pD[2];

            #ifdef TARGET_GCN
            SwapEndian(D[0]);
            SwapEndian(D[1]);
            SwapEndian(D[2]);
            #endif

            Key.Translation.X = (f32)D[0]*(1.0f/16.0f);
            Key.Translation.Y = (f32)D[1]*(1.0f/16.0f);
            Key.Translation.Z = (f32)D[2]*(1.0f/16.0f);
        }
        else
        if( m_TranslationFormat == FULL_PRECISION )
        {
            vector3* pD = (vector3*)m_pTranslation;
            Key.Translation = pD[iFrame];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Translation );  
            #endif
        }
        else
        if( m_TranslationFormat == SINGLE_VALUE )
        {
            vector3* pD = (vector3*)m_pTranslation;
            Key.Translation = pD[0];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Translation );  
            #endif
        }
    }
}

//=========================================================================

xbool anim_key_set::IsMasked( void )
{
    return m_IsMasked;
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_DATA
//=========================================================================
//=========================================================================
//=========================================================================

anim_data::anim_data( void )
{
}

//=========================================================================

anim_data::~anim_data( void )
{
}

//=========================================================================

s32 anim_data::GetNFrames( void ) const
{
    return m_nFrames;
}

//=========================================================================

s32 anim_data::GetNBytes( void ) const
{
    s32 NBytes=0;
    for( s32 i=0; i<m_nBones; i++ )
        NBytes += m_pAnimKeySet[i].GetNBytes();
    return NBytes;
}

//=========================================================================
/*void anim_data::GetAbsoluteKey(const anim_group& AnimGroup, f32  Frame, s32 iBone, anim_key& Key ) const
{    
    matrix4 MatTmp, MatTotal;
    MatTmp.Identity();
    MatTotal.Identity();
    s32 curBone = iBone;
    s32 RevArray[128];
    s32 nBones = 0;
    while(curBone != -1)
    {
        RevArray[nBones] = curBone;
        nBones++;
        curBone = AnimGroup.GetSkel().GetBoneParent(curBone);
    }
    for(s32 i = nBones - 1; i >= 0; i--)
    {
        curBone = RevArray[i];
        anim_key KeyTmp;
        GetInterpKey( Frame, curBone, KeyTmp);
        MatTmp.Setup(KeyTmp.Scale, KeyTmp.Rotation, KeyTmp.Translation);
        MatTotal = MatTotal * MatTmp;
    }
    Key.Translation = MatTotal.GetTranslation();
    Key.Rotation = MatTotal.GetRotation();
    Key.Scale = MatTotal.GetScale();

}*/

//=========================================================================

void anim_data::GetRawKey( s32 iFrame, s32 iBone, anim_key& Key ) const
{
    iFrame = iFrame % m_nFrames;
    m_pAnimKeySet[iBone].GetRawKey( iFrame, Key );
}

//=========================================================================

void anim_data::GetInterpKey( f32  Frame, s32 iBone, anim_key& Key ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    m_pAnimKeySet[iBone].GetInterpKey( Frame, Key );
}

//=========================================================================

void anim_data::GetRawKeys( s32 iFrame, anim_key* pKey ) const
{
    iFrame = iFrame % m_nFrames;
    for( s32 i=0; i<m_nBones; i++ )
        m_pAnimKeySet[i].GetRawKey( iFrame, pKey[i] );
}

//=========================================================================

void anim_data::GetInterpKeys( f32  Frame, anim_key* pKey ) const
{
    s32 i;

    Frame = x_fmod(Frame,(f32)(m_nFrames-1));

    for( i=0; i<m_nBones; i++ )
        m_pAnimKeySet[i].GetInterpKey( Frame, pKey[i] );
/*
    for( i=0; i<m_nBones; i++ )
    {
        pKey[i].Translation.Zero();
        pKey[i].Rotation.Identity();
        pKey[i].Scale = vector3(1,1,1);
    }
*/

}

//=========================================================================

s32 anim_data::GetPropChannel( const char *pChannelName  ) const
{
    for(s32 i = 0; i < m_nProps; i++)
    if( x_stricmp(m_pProp[i].m_Type, pChannelName) == 0 )
        return i;

    return -1;
}
 
//=========================================================================
   
void anim_data::GetPropRawKey( s32 iChannel, s32 iFrame, anim_key& Key ) const
{
    iFrame = iFrame % m_nFrames;
    ASSERT(iChannel >= 0);
    ASSERT(iChannel < m_nProps);
    m_pProp[iChannel].m_pAnimKeySet->GetRawKey( iFrame, Key );
    return;
}

//=========================================================================

void anim_data::GetPropInterpKey( s32 iChannel, f32 Frame,  anim_key& Key ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    ASSERT(iChannel >= 0);
    ASSERT(iChannel < m_nProps);
    m_pProp[iChannel].m_pAnimKeySet->GetInterpKey( Frame, Key );
    return;
}

//=========================================================================

s32 anim_data::GetPropParentBoneIndex( s32 iChannel ) const
{
    return m_pProp[iChannel].m_iBone;
}

//=========================================================================

/*void anim_data::GetPropAbsoluteKey( const anim_group& AnimGroup, s32 iChannel, f32 Frame,  anim_key& Key ) const
{
    Frame = x_fmod(Frame,(f32)(m_nFrames-1));
    ASSERT(iChannel >= 0);
    ASSERT(iChannel < m_nProps);
    m_pProp[iChannel].m_pAnimKeySet->GetInterpKey( Frame, Key );
    anim_key ParentKey;
    GetAbsoluteKey(AnimGroup, Frame, m_pProp[iChannel].m_iParentBone, ParentKey);
    matrix4 KeyMat;
    KeyMat.Setup(Key.Scale, Key.Rotation, Key.Translation);
    matrix4 ParentKeyMat;
    ParentKeyMat.Setup(ParentKey.Scale, ParentKey.Rotation, ParentKey.Translation);
    KeyMat = ParentKeyMat * KeyMat;
    Key.Rotation = KeyMat.GetRotation();
    Key.Translation = KeyMat.GetTranslation();
    Key.Scale = KeyMat.GetScale();
    return;
}*/


//=========================================================================

const char* anim_data::GetName( void ) const
{
    return m_Name;
}

//=========================================================================

radian anim_data::GetTotalMoveDir  ( void ) const
{
    return m_TotalMoveDir;
}

//=========================================================================

radian anim_data::GetTotalYaw  ( void ) const
{
    anim_key key1;
    anim_key key2;

    GetRawKey(0, 0, key1);
    GetRawKey( GetNFrames()-1, 0, key2 );

    radian yaw1 = key1.Rotation.GetRotation().Yaw;
    radian yaw2 = key2.Rotation.GetRotation().Yaw;
    return ( x_MinAngleDiff( yaw2, yaw1 ) );
}

//=========================================================================

radian anim_data::GetHandleAngle      ( void ) const
{
    return m_HandleAngle;
}

//=========================================================================

vector3 anim_data::GetTotalTranslation ( void ) const
{
    return m_TotalTranslation;
}

//=========================================================================

s32 anim_data::GetFPS( void ) const
{
    return m_FPS;
}

//=========================================================================

f32 anim_data::GetSpeed( void ) const
{
    f32 Dist = m_TotalTranslation.Length();
    f32 Time = (f32)(m_nFrames) / (f32)m_FPS;
    return Dist / Time;
}

//=========================================================================

xbool anim_data::DoesLoop( void ) const
{
    return (m_Flags & ANIM_DATA_FLAG_LOOPING);
}

//=========================================================================

xbool anim_data::HasMasks( void ) const
{
    return (m_Flags & ANIM_DATA_FLAG_HAS_MASKS);
}

//=========================================================================

xbool anim_data::IsBoneMasked( s32 iBone ) const
{
    return m_pAnimKeySet[iBone].IsMasked();
}

//=========================================================================

s32 anim_data::GetNEvents( void ) const
{
    return m_nEvents;
}

//=========================================================================

const anim_event& anim_data::GetEvent( s32 iEvent ) const
{
    ASSERT( (iEvent>=0) && (iEvent<m_nEvents) );
    return m_pEvent[iEvent];
}

//=========================================================================

xbool anim_data::IsEventActive( s32 iEvent, f32 Frame ) const
{
    ASSERT( (iEvent>=0) && (iEvent<m_nEvents) );
    const anim_event& E = m_pEvent[iEvent];

    if( (Frame>=E.m_iFrame0) && (Frame<=E.m_iFrame1) )
        return TRUE;

    return FALSE;
}

//=========================================================================

xbool anim_data::IsEventTypeActive( s32 Type, f32 Frame ) const
{
    for( s32 i=0; i<m_nEvents; i++ )
    if( m_pEvent[i].m_Type == Type )
    {
        const anim_event& E = m_pEvent[i];

        if( (Frame>=E.m_iFrame0) && (Frame<=E.m_iFrame1) )
            return TRUE;
    }

    return FALSE;
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_GROUP
//=========================================================================
//=========================================================================
//=========================================================================

anim_group::anim_group( void )
{
    m_Skel.m_nBones     = 0;
    m_Skel.m_pBone      = NULL;
    m_pAnim             = NULL;
    m_pAnimKeySet       = NULL;
    m_pAnimKeyData      = NULL;
    m_nKeySets          = 0;
    m_pProp             = NULL;
    m_pEvent            = NULL;
}

//=========================================================================

anim_group::~anim_group( void )
{
    Clear();
}

//=========================================================================

void anim_group::Clear( void )
{
    if( (m_pAnimKeyData == NULL) && (m_nKeySets>0) )
    {
        s32 nBytesInChunk=S32_MAX;
        s32 iFirst = 0;
        s32 iLast  = 0;
        while( iFirst < m_nKeySets )
        {
            delete[] m_pAnimKeySet[iFirst].m_pScale;

            nBytesInChunk = m_pAnimKeySet[iFirst].GetNBytes();
            iLast = iFirst;
            while(1)
            {
                // Is this the last keyset?
                if( iLast == m_nKeySets-1 ) 
                    break;

                // Will the next piece push us over out limit?
                if( (nBytesInChunk + m_pAnimKeySet[iLast+1].GetNBytes()) > CHUNK_SIZE )
                    break;

                iLast++;
                nBytesInChunk += m_pAnimKeySet[iLast].GetNBytes();
            }
            iFirst = iLast+1;
        }
    }
    else
    {
        delete[] m_pAnimKeyData;
        m_pAnimKeyData = NULL;
    }

    delete[] m_Skel.m_pBone;
    m_Skel.m_pBone = NULL;

    delete[] m_pAnim;
    m_pAnim = NULL;

    delete[] m_pAnimKeySet;
    m_pAnimKeySet = NULL;

    delete[] m_pProp;
    m_pProp = NULL;

    delete[] m_pEvent;
    m_pEvent = NULL;
}

//=========================================================================

void anim_group::ConvertToPointers( void )
{
    if( !m_UsingIndices )
        return;
    m_UsingIndices = FALSE;

    s32 i;

    // Loop through anims 
    //s32 PropsSoFar = 0;           // UNUSED - SH
    for( i=0; i<m_nAnims; i++ )
    {
        s32 SaveIndex = ((s32)m_pAnim[i].m_pAnimKeySet);
        m_pAnim[i].m_pAnimKeySet = &m_pAnimKeySet[ SaveIndex * m_Skel.m_nBones ];
        
        // Handle events
        m_pAnim[i].m_pEvent = &m_pEvent[ ((s32)m_pAnim[i].m_pEvent) ];

        // Handle Props
        m_pAnim[i].m_pProp = &m_pProp[ ((s32)m_pAnim[i].m_pProp) ];
    }

    // Loop through props
    for( i=0; i<m_nProps; i++ )
    {
        m_pProp[i].m_pAnimKeySet = &m_pAnimKeySet[ ((s32)m_pProp[i].m_pAnimKeySet) ];
    }

    // Loop through key sets
    for( i=0; i<m_nKeySets; i++ )
    {
        m_pAnimKeySet[i].m_pScale       = m_pAnimKeyData + ((s32)m_pAnimKeySet[i].m_pScale);
        m_pAnimKeySet[i].m_pRotation    = m_pAnimKeyData + ((s32)m_pAnimKeySet[i].m_pRotation);
        m_pAnimKeySet[i].m_pTranslation = m_pAnimKeyData + ((s32)m_pAnimKeySet[i].m_pTranslation);
    }
}

//=========================================================================

void anim_group::ConvertToIndices( void )
{
    if( m_UsingIndices )
        return;
    m_UsingIndices = TRUE;

    s32 i;

    // Loop through anims 

    CRT_SANITY;

    for( i=0; i<m_nAnims; i++ )
    {
        m_pAnim[i].m_pAnimKeySet = (anim_key_set*)((m_pAnim[i].m_pAnimKeySet - m_pAnimKeySet)/m_Skel.m_nBones);
        m_pAnim[i].m_pProp = (anim_prop*)(m_pAnim[i].m_pProp - m_pProp);
        m_pAnim[i].m_pEvent = (anim_event*)(m_pAnim[i].m_pEvent - this->m_pEvent);
    }
    CRT_SANITY;

    // Loop through the props
    for( i=0; i<m_nProps; i++ )
    {
        m_pProp[i].m_pAnimKeySet = (anim_key_set*)(m_pProp[i].m_pAnimKeySet - m_pAnimKeySet);            
    }
    
    // Loop through key sets
    for( i=0; i<m_nKeySets; i++ )
    {
        m_pAnimKeySet[i].m_pScale       = (byte*)(m_pAnimKeySet[i].m_pScale - m_pAnimKeyData);
        m_pAnimKeySet[i].m_pRotation    = (byte*)(m_pAnimKeySet[i].m_pRotation - m_pAnimKeyData);
        m_pAnimKeySet[i].m_pTranslation = (byte*)(m_pAnimKeySet[i].m_pTranslation - m_pAnimKeyData);
    }

    CRT_SANITY;    
}

//=========================================================================

s32 anim_group::GetNAnims( void ) const
{
    return m_nAnims;
}

//=========================================================================

s32 anim_group::GetNBones( void ) const
{
    ASSERT(m_Skel.m_nBones >= 0);
    return m_Skel.m_nBones;
}

//=========================================================================

const anim_skel& anim_group::GetSkel( void ) const
{
    return m_Skel;
}

//=========================================================================

const anim_data& anim_group::GetAnimData( s32 iAnim ) const
{
    ASSERT( (iAnim>=0) && (iAnim<m_nAnims) );
    return m_pAnim[iAnim];
}

//=========================================================================

s32 anim_group::GetBoneIndex( const char* pBoneName, xbool FindAnywhere ) const
{
    s32 i;
    for( i=0; i<m_Skel.m_nBones; i++ )
    {
        if(FindAnywhere)
        {
            xstring S1(m_Skel.m_pBone[i].Name);
            xstring S2(pBoneName);
            if(S1.Find(S2) != -1)
                return i;
        }
        else
        {
            if( x_stricmp( m_Skel.m_pBone[i].Name, pBoneName ) == 0 )
                return i;
        }
    }

    return -1;
}

//=========================================================================

void anim_group::GetL2W( const matrix4& L2W, f32 Frame, s32 iAnim, matrix4* pBoneL2W ) const
{
    // Be sure we have pointers to data
//    ASSERT( !m_UsingIndices );    // The data is no longer being converted to pointers!  Indices are valid!!

    //x_printf("%f\n",Frame);

    // Allocate location for keys
    anim_key* pKey = (anim_key*)x_malloc(sizeof(anim_key)*m_Skel.m_nBones);
    VERIFY(pKey);

    // Get keys from anim
    m_pAnim[iAnim].GetInterpKeys( Frame, pKey );

    // Build matrices
    m_Skel.ComputeBonesL2W( L2W, pKey, pBoneL2W );

    // Free key array
    x_free(pKey);
}

//=========================================================================

void anim_group::DumpFrames( s32 iAnim, const char* pFilename )
{
    s32 i,j;
    anim_data& Anim = m_pAnim[iAnim];

    X_FILE* fp = x_fopen(pFilename,"wt");
    if( !fp ) return;

    for( j=0; j<m_Skel.m_nBones; j++ )
    {
        for( i=0; i<Anim.m_nFrames; i++ )
        {
            anim_key Key;
            Anim.GetRawKey( i, j, Key );

            x_fprintf(fp,"%3d %32s ",j,m_Skel.m_pBone[j].Name);

            x_fprintf(fp,"   |   %8.5f %8.5f %8.5f %8.5f",
                Key.Rotation.X,
                Key.Rotation.Y,
                Key.Rotation.Z,
                Key.Rotation.W);

            x_fprintf(fp,"   |   %8.2f %8.2f %8.2f",
                Key.Translation.X,
                Key.Translation.Y,
                Key.Translation.Z);

            x_fprintf(fp,"   |   %4.2f %4.2f %4.2f\n",
                Key.Scale.X,
                Key.Scale.Y,
                Key.Scale.Z);
        }
    }

    x_fclose(fp);
}

//=========================================================================

xbool anim_group::Save( const char* pFileName, xbool SwapEndian )
{
    X_FILE* fp = x_fopen(pFileName,"wb");
    if( !fp )
        return FALSE;

    xbool Result = Save(fp,SwapEndian);

    x_fclose(fp);

    return Result;
}

//=========================================================================

xbool anim_group::Load( const char* pFileName )
{
    X_FILE* fp = x_fopen(pFileName,"rb");
    if( !fp )
        return FALSE;

    xbool Result = Load(fp);

    if( !Result )
    {
        e_throw(xfs("AnimData <%s> has wrong version",pFileName));
    }

    x_fclose(fp);

    return Result;
}

//=========================================================================

xbool anim_group::Save( X_FILE* fp, xbool SwapEndian )
{
    if( SwapEndian )
    {
        anim_group AG;
        CRT_SANITY;
        AG.CopyFrom( *this );
        CRT_SANITY;
        AG.SwitchEndian();
        CRT_SANITY;

        x_fwrite( &AG, sizeof(anim_group), 1, fp );
        x_fwrite( AG.m_Skel.m_pBone, sizeof(anim_bone)*m_Skel.m_nBones, 1, fp );
        x_fwrite( AG.m_pAnim, sizeof(anim_data)*m_nAnims, 1, fp );
        x_fwrite( AG.m_pAnimKeySet, sizeof(anim_key_set)*m_nKeySets, 1, fp );
        x_fwrite( AG.m_pAnimKeyData, m_AnimKeyDataBytes, 1, fp );
        x_fwrite( AG.m_pProp, sizeof(anim_prop)*m_nProps, 1, fp );
        x_fwrite( AG.m_pEvent, sizeof(anim_event)*m_nEvents, 1, fp );
    }
    else
    {
        ConvertToIndices();
        x_fwrite( this, sizeof(anim_group), 1, fp );
        x_fwrite( m_Skel.m_pBone, sizeof(anim_bone)*m_Skel.m_nBones, 1, fp );
        x_fwrite( m_pAnim, sizeof(anim_data)*m_nAnims, 1, fp );
        x_fwrite( m_pAnimKeySet, sizeof(anim_key_set)*m_nKeySets, 1, fp );
        x_fwrite( m_pAnimKeyData, m_AnimKeyDataBytes, 1, fp );        
        x_fwrite( m_pProp, sizeof(anim_prop)*m_nProps, 1, fp );
        x_fwrite( m_pEvent, sizeof(anim_event)*m_nEvents, 1, fp );
        ConvertToPointers();
    }

    return TRUE;
}

//=========================================================================

xbool anim_group::Load( X_FILE* fp )
{
    s32 i;

    Clear();

    x_fread( this, sizeof(anim_group), 1, fp );
    ASSERT(m_Skel.m_nBones >= 0);
    m_pAnimKeyData = NULL;
    
    if( m_Version != ANIM_DATA_VERSION )
        return FALSE;

    m_Skel.m_pBone = new anim_bone[ m_Skel.m_nBones ];
    x_fread( m_Skel.m_pBone, sizeof(anim_bone)*m_Skel.m_nBones, 1, fp );

    m_pAnim = new anim_data[ m_nAnims ];
    x_fread( m_pAnim, sizeof(anim_data)*m_nAnims, 1, fp );
    
    m_pAnimKeySet = new anim_key_set[ m_nKeySets ];
    x_fread( m_pAnimKeySet, sizeof(anim_key_set)*m_nKeySets, 1, fp );

/*
    m_pAnimKeyData = new byte[ m_AnimKeyDataBytes ];
    x_fread( m_pAnimKeyData, m_AnimKeyDataBytes, 1, fp );

    m_pProp = new anim_prop[ m_nProps ];
    x_fread( m_pProp, sizeof(anim_prop)*m_nProps, 1, fp );

    m_pEvent = new anim_event[ m_nEvents ];
    x_fread( m_pEvent, sizeof(anim_event)*m_nEvents, 1, fp );

    ConvertToPointers();
*/

    {
        s32 nTotalBytes=0;
        s32 nBytesInChunk=S32_MAX;
        s32 iFirst = 0;
        s32 iLast  = 0;
        while( iFirst < m_nKeySets )
        {
            nBytesInChunk = m_pAnimKeySet[iFirst].GetNBytes();
            iLast = iFirst;
            while(1)
            {
                // Is this the last keyset?
                if( iLast == m_nKeySets-1 ) 
                    break;

                // Will the next piece push us over out limit?
                if( (nBytesInChunk + m_pAnimKeySet[iLast+1].GetNBytes()) > CHUNK_SIZE )
                    break;

                iLast++;
                nBytesInChunk += m_pAnimKeySet[iLast].GetNBytes();
            }
            nTotalBytes += nBytesInChunk;

            // Allocate the key data
            byte* pData = new byte[nBytesInChunk];
            ASSERT(pData);
            x_fread( pData, nBytesInChunk, 1, fp );

            // Setup keysets
            u32 InitialOffset = (u32)m_pAnimKeySet[iFirst].m_pScale;
            for( i=iFirst; i<=iLast; i++ )
            {
                m_pAnimKeySet[i].m_pScale        = pData + ((u32)m_pAnimKeySet[i].m_pScale - InitialOffset);
                m_pAnimKeySet[i].m_pRotation     = pData + ((u32)m_pAnimKeySet[i].m_pRotation - InitialOffset);
                m_pAnimKeySet[i].m_pTranslation  = pData + ((u32)m_pAnimKeySet[i].m_pTranslation - InitialOffset);
            }
            iFirst = iLast+1;
        }
    }

    //ASSERT( nTotalBytes == m_AnimKeyDataBytes );

    m_pProp = new anim_prop[ m_nProps ];
    x_fread( m_pProp, sizeof(anim_prop)*m_nProps, 1, fp );

    m_pEvent = new anim_event[ m_nEvents ];
    x_fread( m_pEvent, sizeof(anim_event)*m_nEvents, 1, fp );

    //
    // Convert other offsets to addresses
    //
    // Loop through anims 
    //s32 PropsSoFar = 0;           // UNUSED - SH
    for( i=0; i<m_nAnims; i++ )
    {
        s32 SaveIndex = ((s32)m_pAnim[i].m_pAnimKeySet);
        m_pAnim[i].m_pAnimKeySet = &m_pAnimKeySet[ SaveIndex * m_Skel.m_nBones ];

        // Handle events
        m_pAnim[i].m_pEvent = &m_pEvent[ ((s32)m_pAnim[i].m_pEvent) ];

        // Handle Props
        m_pAnim[i].m_pProp = &m_pProp[ ((s32)m_pAnim[i].m_pProp) ];
    }

    // Loop through props
    for( i=0; i<m_nProps; i++ )
    {
        m_pProp[i].m_pAnimKeySet = &m_pAnimKeySet[ ((s32)m_pProp[i].m_pAnimKeySet) ];
    }

/*
    s32 i,j;

    // Point anims into anim key set array
    for( i=0; i<m_nAnims; i++ )
    {
        s32 SaveIndex = ((s32)m_pAnim[i].m_pAnimKeySet);
        m_pAnim[i].m_pAnimKeySet = &m_pAnimKeySet[ SaveIndex * m_Skel.m_nBones ];
    }


    s32 NBytesTotal = 0;
    for( i=0; i<m_nAnims; i++ )
    {
        s32 NBytesForAnim = m_pAnim[i].GetNBytes();
        byte* pData = new byte[NBytesForAnim];
        u32 InitialOffset = (u32)m_pAnim[i].m_pAnimKeySet[0].m_pScale;

        for( j=0; j<m_nBones; j++ )
        {
            m_pAnim[i].m_pAnimKeySet[j].m_pScale        = pData + ((u32)m_pAnim[i].m_pAnimKeySet[j].m_pScale - InitialOffset);
            m_pAnim[i].m_pAnimKeySet[j].m_pRotation     = pData + ((u32)m_pAnim[i].m_pAnimKeySet[j].m_pRotation - InitialOffset);
            m_pAnim[i].m_pAnimKeySet[j].m_pTranslation  = pData + ((u32)m_pAnim[i].m_pAnimKeySet[j].m_pTranslation - InitialOffset);
        }

        // Read in data
        x_fread( pData, NBytesForAnim, 1, fp );
        NBytesTotal += NBytesForAnim;
        ASSERT( NBytesTotal <= m_AnimKeyDataBytes );
    }

    m_pProp = new anim_prop[ m_nProps ];
    x_fread( m_pProp, sizeof(anim_prop)*m_nProps, 1, fp );

    m_pEvent = new anim_event[ m_nEvents ];
    x_fread( m_pEvent, sizeof(anim_event)*m_nEvents, 1, fp );

    //
    // Convert other offsets to addresses
    //
    // Loop through anims 
    //s32 PropsSoFar = 0;           // UNUSED - SH
    for( i=0; i<m_nAnims; i++ )
    {
        // Handle events
        m_pAnim[i].m_pEvent = &m_pEvent[ ((s32)m_pAnim[i].m_pEvent) ];

        // Handle Props
        m_pAnim[i].m_pProp = &m_pProp[ ((s32)m_pAnim[i].m_pProp) ];
    }

    // Loop through props
    for( i=0; i<m_nProps; i++ )
    {
        m_pProp[i].m_pAnimKeySet = &m_pAnimKeySet[ ((s32)m_pProp[i].m_pAnimKeySet) ];
    }

    // Loop through key sets
    for( i=0; i<m_nKeySets; i++ )
    {
        m_pAnimKeySet[i].m_pScale       = m_pAnimKeyData + ((s32)m_pAnimKeySet[i].m_pScale);
        m_pAnimKeySet[i].m_pRotation    = m_pAnimKeyData + ((s32)m_pAnimKeySet[i].m_pRotation);
        m_pAnimKeySet[i].m_pTranslation = m_pAnimKeyData + ((s32)m_pAnimKeySet[i].m_pTranslation);
    }

*/

    return TRUE;
}

//=========================================================================

s32 anim_group::GetAnimIndex( const char* pAnimName ) const
{
    for( s32 i=0; i<m_nAnims; i++ )
    {
        const anim_data& AnimData = GetAnimData(i);

        if( x_stricmp( AnimData.GetName(), pAnimName ) == 0 )
            return i;
    }

    return -1;
}

//=========================================================================

f32 anim_group::GetAnimSpeed( s32 iAnim ) const
{
    const anim_data& AnimData = GetAnimData(iAnim);
    return AnimData.GetSpeed();
}

//=========================================================================

void anim_group::CopyFrom( anim_group& AG )
{
    AG.ConvertToIndices();

    x_memcpy( this, &AG, sizeof(anim_group) );

    // Allocate arrays
    m_Skel.m_pBone = new anim_bone[ m_Skel.m_nBones ]; 
    m_pAnim = new anim_data[m_nAnims];
    m_pEvent= new anim_event[m_nEvents];
    m_pAnimKeySet = new anim_key_set[m_nKeySets];
    m_pAnimKeyData = new byte[m_AnimKeyDataBytes];
    m_pProp = new anim_prop[m_nProps];

    // Copy data
    x_memcpy( m_Skel.m_pBone, AG.m_Skel.m_pBone, sizeof(anim_bone)*m_Skel.m_nBones );
    x_memcpy( m_pAnim, AG.m_pAnim, sizeof(anim_data)*m_nAnims);
    x_memcpy( m_pEvent, AG.m_pEvent, sizeof(anim_event)*m_nEvents);
    x_memcpy( m_pAnimKeySet, AG.m_pAnimKeySet, sizeof(anim_key_set)*m_nKeySets);
    x_memcpy( m_pAnimKeyData, AG.m_pAnimKeyData, sizeof(byte)*m_AnimKeyDataBytes);
    x_memcpy( m_pProp, AG.m_pProp, sizeof(anim_prop) * m_nProps);

    AG.ConvertToPointers();
    ConvertToPointers();
}

//=========================================================================

void anim_group::SwitchEndian( void )
{
    s32 i;
    CRT_SANITY;
    ConvertToIndices();

    //
    // Skeleton
    //
    CRT_SANITY;
    for( i=0; i<m_Skel.m_nBones; i++ )
    {
        SwapEndian( m_Skel.m_pBone[i].iBone );
        SwapEndian( m_Skel.m_pBone[i].iParent );
        SwapEndian( m_Skel.m_pBone[i].nChildren );
        SwapEndian( m_Skel.m_pBone[i].BindTranslation );
        SwapEndian( m_Skel.m_pBone[i].BindRotation );
        SwapEndian( m_Skel.m_pBone[i].BindScale );
        SwapEndian( m_Skel.m_pBone[i].BindMatrix );
        SwapEndian( m_Skel.m_pBone[i].BindMatrixInv );
    }
    //SwapEndian( m_Skel.m_pBone );
    SwapEndian( m_Skel.m_nBones );

    //
    // AnimData
    //
    CRT_SANITY;
    for( i=0; i<m_nAnims; i++ )
    {
        SwapEndian( m_pAnim[i].m_nFrames );
        SwapEndian( m_pAnim[i].m_nBones );
        SwapEndian( m_pAnim[i].m_nProps );
        SwapEndian( m_pAnim[i].m_nEvents );
        SwapEndian( m_pAnim[i].m_FPS );
        SwapEndian( m_pAnim[i].m_TotalMoveDir );
        SwapEndian( m_pAnim[i].m_HandleAngle );
        SwapEndian( m_pAnim[i].m_Flags );
        SwapEndian( m_pAnim[i].m_TotalTranslation );

        SwapEndian( m_pAnim[i].m_pAnimKeySet );
        SwapEndian( m_pAnim[i].m_pProp );
        SwapEndian( m_pAnim[i].m_pEvent );
    }

    //
    // AnimKeyData - converted at runtime
    //

    //
    // KeySets
    //
    CRT_SANITY;
    for( i=0; i<m_nKeySets; i++ )
    {
        SwapEndian( m_pAnimKeySet[i].m_nFrames );

        SwapEndian( m_pAnimKeySet[i].m_pScale );
        SwapEndian( m_pAnimKeySet[i].m_pRotation );
        SwapEndian( m_pAnimKeySet[i].m_pTranslation );
    }

    //
    // Props
    //
    CRT_SANITY;
    for( i=0; i<m_nProps; i++ )
    {
        SwapEndian( m_pProp[i].m_iBone );
        //SwapEndian( m_pProp[i].m_Type );
        SwapEndian( m_pProp[i].m_pAnimKeySet );
    }
    CRT_SANITY;
    
    //
    // Events
    //
    for( i=0; i<m_nEvents; i++ )
    {
        SwapEndian( m_pEvent[i].m_iBone );
        SwapEndian( m_pEvent[i].m_iFrame0 );
        SwapEndian( m_pEvent[i].m_iFrame1 );
        SwapEndian( m_pEvent[i].m_Offset );
        SwapEndian( m_pEvent[i].m_Radius );
        SwapEndian( m_pEvent[i].m_Type );
    }

    SwapEndian( m_Version );
    SwapEndian( m_UsingIndices );
    SwapEndian( m_TotalNFrames );
    SwapEndian( m_nAnims );
    SwapEndian( m_nProps );
    SwapEndian( m_nEvents );
    SwapEndian( m_nKeySets );
    SwapEndian( m_AnimKeyDataBytes );
}

//=========================================================================

s32 anim_skel::GetNBones           ( void ) const
{
    return m_nBones;
}
//==============================================================================
//  GLOBAL INITIALIZATION (CALLED ONLY ONCE EVER)
//==============================================================================
//  STORAGE
//==============================================================================

class anim_loader : public rsc_loader
{
public:
            anim_loader( const char* pType, const char* pExt ){ Init(pType,pExt); }
    void*   Load            ( X_FILE* pFP, const char* pFileName );
    void    Unload          ( void* pData, const char* pFileName  );
};

//==============================================================================

anim_loader AnimLoader("Animation",".anim");
anim_loader AnimLoader2("Animation",".charanim");

//==============================================================================

void* anim_loader::Load( X_FILE* pFP, const char* pFileName )
{
    (void)pFileName;
    x_DebugMsg("loading animation\n");

    // Allocate a new instance of geometry
    anim_group* pGeom = new anim_group();
    ASSERT( pGeom );

    // Load geom
    pGeom->Load( pFP );
    
    return pGeom;
}

//==============================================================================

void anim_loader::Unload( void* pData, const char* pFileName  )
{
    (void)pFileName;
    //x_DebugMsg("unloading animation\n");
    delete (anim_group*)pData;
}

//==============================================================================
