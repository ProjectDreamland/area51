//=========================================================================
//
//  ANIMGROUP.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "e_Virtual.hpp"



class anim_loader : public rsc_loader
{
public:
    anim_loader( const char* pType, const char* pExt ) : rsc_loader(pType,pExt) {}
    
    virtual void*   PreLoad         ( X_FILE* pFP   );
    virtual void*   Resolve         ( void*   pData ) {return pData;}
    virtual void    Unload          ( void*   pData ) ;
};

//==============================================================================

anim_loader AnimLoader("Animation",".anim");

//==============================================================================

void* anim_loader::PreLoad( X_FILE* pFP )
{
    MEMORY_OWNER( "ANIMATION DATA" );
    
    // Allocate a new instance of geometry
    anim_group* pAnimGroup = new anim_group();
    ASSERT( pAnimGroup );
    
    // If the load fails then delete the AnimGroup
    if( !pAnimGroup->Load( pFP ) )
    {
        delete pAnimGroup;
        pAnimGroup = NULL;
    }
    
    return pAnimGroup;
}

//==============================================================================

void anim_loader::Unload( void* pData  )
{
    //x_DebugMsg("unloading animation\n");
    delete (anim_group*)pData;
}

//=========================================================================
//  GLOBAL INITIALIZATION (CALLED ONLY ONCE EVER)
//=========================================================================
//  STORAGE
//=========================================================================
/*
class anim_loader : public rsc_loader
{
public:
            anim_loader( const char* pType, const char* pExt ){ Init(pType,pExt); }
    void*   Load            ( X_FILE* pFP, const char* pFileName );
    void    Unload          ( void* pData, const char* pFileName  );
};

//=========================================================================

anim_loader AnimLoader("Animation",".anim");
anim_loader AnimLoader2("Animation",".charanim");

//=========================================================================

void* anim_loader::Load( X_FILE* pFP, const char* pFileName )
{
    CONTEXT( "anim_loader::Load" );

    (void)pFileName;
    x_DebugMsg("loading animation\n");

    // Allocate a new instance of an anim_group
    anim_group* pAnimGroup = new anim_group();
    ASSERT( pAnimGroup );

    // Load geom
    pAnimGroup->Load( pFP, pFileName );
    
    return pAnimGroup;
}

//=========================================================================

void anim_loader::Unload( void* pData, const char* pFileName  )
{
    (void)pFileName;
    //x_DebugMsg("unloading animation\n");
    delete (anim_group*)pData;
}
*/
//=========================================================================
//=========================================================================
//=========================================================================
// SUPPORT
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================

static inline u32 ComputeUppercaseStringHash( const char* pString)
{
    u32 Hash = 5381;
    s32 C;
 
    // Process each character to generate the hash key
    while( (C = *pString++) )
    {
        if( (C >= 'a') && (C <= 'z') )
            C += ('A' - 'a');
        Hash = ((Hash<<5) + Hash) ^ C;
    }

    return Hash;
}

//==============================================================================

void SwapEndian( byte& V )
{
    (void)V;
}

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
// ANIM_GROUP
//=========================================================================
//=========================================================================
//=========================================================================

static s32 s_nAnimGroupsExisting=0;

anim_group::anim_group( void )
{
    x_memset( this, 0, sizeof(anim_group) );
    m_pEvent = NULL;

    // Increment number of animgroups existing
    s_nAnimGroupsExisting++;
}

//=========================================================================

anim_group::~anim_group( void )
{
    Clear();

    // If the last animgroup to be destroyed then clean up global data
    s_nAnimGroupsExisting--;
    if( s_nAnimGroupsExisting==0 )
    {
        ReleaseCompDataPool();
    }
}

//=========================================================================

const char* anim_group::GetFileName( void ) const
{
    return m_FileName;
}

//=========================================================================

void anim_group::Clear( void )
{
    // Only allow this call if it's a valid anim_group, -ve m_Version indicates previous failure to load
    if( m_Version >= 0 )
    {
        // Be sure all animblocks have released their decompressed data
        for( s32 i=0; i<m_nKeyBlocks; i++ )
            m_pKeyBlock[i].ReleaseStreams();

        if( 1 )
        {
            if( m_pUncompressedData )
                x_free( m_pUncompressedData );
            if( m_pCompressedData )
                x_free( m_pCompressedData   );
        }
        else
        {
            if( m_pUncompressedData )
                vm_Free( m_pUncompressedData );
            if( m_pCompressedData )
                vm_Free( m_pCompressedData   );
        }

        delete [] m_pEvent;

        x_free( m_pHashTable );

        RemoveAnimKeyBlocksFromHashTable();
    }

    // Clear out the class
    x_memset( this, 0, sizeof(anim_group) );
}

//=========================================================================

const byte* anim_group::GetCompressedDataPtr( void ) const
{
    return m_pCompressedData;
}

//=========================================================================

s32 anim_group::GetBoneIndex( const char* pBoneName, xbool FindAnywhere ) const
{
    ASSERT( pBoneName );

    // If bone name is empty then just return
    if( pBoneName[0] == 0 )
        return -1;

    // Check if pBoneName is found anywhere in the string
    if( FindAnywhere )
    {
        for( s32 i=0; i<m_nBones; i++ )
        if( x_stristr( m_pBone[i].Name, pBoneName ) != NULL )
            return i;
    }
    else
    {
        if( m_pHashTable )
        {
            // Find entry in the hash table
            s32 HashTableSize = (m_nAnims+m_nBones)*2;
            u32 Hash = ComputeUppercaseStringHash( pBoneName );
            s32 iStartIndex = Hash % HashTableSize;
            s32 iIndex = iStartIndex;

            while( m_pHashTable[iIndex].Hash )
            {
                if( (m_pHashTable[iIndex].Hash==Hash) && (m_pHashTable[iIndex].iBone!=-1))
                {
                    s32 iBone = m_pHashTable[iIndex].iBone;
                    if( x_stricmp( m_pBone[iBone].Name, pBoneName ) == 0 )
                        return iBone;
                }

                iIndex = iIndex+1;
                if( iIndex == HashTableSize ) iIndex = 0;
            }

            return -1;
        }
        else
        {
            for( s32 i=0; i<m_nBones; i++ )
            if( x_stricmp( m_pBone[i].Name, pBoneName ) == 0 )
                return i;
        }
    }

    return -1;
}

//=========================================================================

void anim_group::GetL2W( const matrix4& L2W, f32 Frame, s32 iAnim, matrix4* pBoneL2W ) const
{
    // Allocate location for keys
    anim_key* pKey = (anim_key*)x_malloc(sizeof(anim_key)*m_nBones);
    VERIFY(pKey);

    // Get keys from anim
    m_pAnimInfo[iAnim].GetInterpKeys( Frame, pKey );

    // Build matrices
    ComputeBonesL2W( L2W, pKey, m_nBones, pBoneL2W );

    // Free key array
    x_free(pKey);
}

//=========================================================================

s32 anim_group::GetAnimIndex( const char* pAnimName ) const
{
    if( m_pHashTable )
    {
        // Find entry in the hash table
        s32 HashTableSize = (m_nAnims+m_nBones)*2;
        u32 Hash = ComputeUppercaseStringHash( pAnimName );
        s32 iStartIndex = Hash % HashTableSize;
        s32 iIndex = iStartIndex;

        while( m_pHashTable[iIndex].Hash )
        {
            if( (m_pHashTable[iIndex].Hash==Hash) && (m_pHashTable[iIndex].iAnim!=-1))
            {
                s32 iAnim = m_pHashTable[iIndex].iAnim;
                const anim_info& AnimInfo = GetAnimInfo( iAnim );
                if( x_stricmp( AnimInfo.GetName(), pAnimName ) == 0 )
                    return iAnim;
            }

            iIndex = iIndex+1;
            if( iIndex == HashTableSize ) iIndex = 0;
        }
    }
    else
    {
        for( s32 i=0; i<m_nAnims; i++ )
        if( x_stricmp( m_pAnimInfo[i].GetName(), pAnimName ) == 0 )
            return i;
    }


    return -1;
}

//=========================================================================

s32 anim_group::GetRandomAnimIndex( const char* pAnimName, s32 iSkipAnim /*= -1*/ ) const
{
    // Lookup start of animations with this name
    s32 iStartAnim = GetAnimIndex( pAnimName );
    if( iStartAnim == -1 )
        return -1;

    // Now select a random anim
    return GetRandomAnimIndex( iStartAnim, iSkipAnim );
}

//=========================================================================

s32 anim_group::GetRandomAnimIndex( s32 iStartAnim, s32 iSkipAnim /*= -1*/ ) const
{
    // If there is just one anim of this type, then use it
    const anim_info& StartAnimInfo = GetAnimInfo( iStartAnim ) ;
    
    // Get # of anims of the same name
    s32 nAnims = StartAnimInfo.GetNAnims();
    
    // If there is just one anim, then use it
    if( nAnims == 1 )
        return iStartAnim;

    // Compute total weight
    f32 TotalWeight = StartAnimInfo.GetAnimsWeight();
    
    // If skip anim has the same name as the requested anim, then skip it so
    // it's not chosen twice in a row
    if( ( iSkipAnim >= iStartAnim ) && ( iSkipAnim < ( iStartAnim + nAnims ) ) )
        TotalWeight -= GetAnimInfo( iSkipAnim ).GetWeight();

    // Choose a random weight
    f32 WeightChosen = x_frand(0.0f, TotalWeight);

    // Search for anim with chosen weight
    s32 iAnim     = iStartAnim;
    f32 WeightSum = 0.0f;
    while(1)
    {
        // Skip?
        if( iAnim != iSkipAnim )
        {
            // Update sum
            WeightSum += GetAnimInfo( iAnim ).GetWeight() ;

            // Found?
            if( WeightSum >= WeightChosen )
                break;
        }
        
        // Check next
        iAnim++;
    }

    // Make sure this logic works!
    ASSERT( iAnim != iSkipAnim );
    ASSERT( ( iAnim >= iStartAnim ) && ( iAnim < ( iStartAnim + nAnims ) ) );
    ASSERT( (iAnim>=0) && (iAnim < GetNAnims()) ) ;

    return iAnim;
}

//=========================================================================

void anim_group::ComputeBonesL2W ( const matrix4& L2W, anim_key* pKey, s32 nBones, matrix4* pBoneL2W, xbool bApplyTheBindPose ) const
{
    CONTEXT("anim_group::ComputeBonesL2W") ;

    s32 i;
   
    ASSERT(nBones <= m_nBones) ;

    // Convert all keys to matrices and put into world space
    for( i=0; i< nBones; i++ )
    {
        // Setup L2W
        pKey[i].Setup(pBoneL2W[i]) ;

        // Concatenate with parent or L2W
        const matrix4* PM = (m_pBone[i].iParent == -1) ? (&L2W):(&pBoneL2W[m_pBone[i].iParent]);
        pBoneL2W[i] = (*PM) * pBoneL2W[i] ;
    }

    if( bApplyTheBindPose == FALSE )
        return;

    // Apply bind matrices
    for( i=0; i< nBones; i++ )
    {
        pBoneL2W[i] = pBoneL2W[i] * m_pBone[i].BindMatrixInv;
    }
}

//=========================================================================

void anim_group::ComputeBoneL2W ( s32 iBone, const matrix4& L2W, anim_key* pKey, matrix4& BoneL2W ) const
{
    ASSERT(iBone < m_nBones) ;

    // Clear bone matrix
    BoneL2W.Identity();

    // Run hierarchy from bone to root node
    s32 I = iBone;
    while( I != -1 )
    {
        matrix4 LM;
        pKey[I].Setup(LM) ;
        BoneL2W = LM * BoneL2W;
        I = m_pBone[I].iParent;
    }

    // Concatenate L2W 
    BoneL2W = L2W * BoneL2W;

    // Apply bind matrix
    BoneL2W = BoneL2W * m_pBone[iBone].BindMatrixInv;
}

//=========================================================================

vector3 anim_group::GetEventPos( s32 iBone, const vector3& Offset, anim_key* pKey ) const
{
    matrix4 BoneM;
    matrix4 IdentM;
    IdentM.Identity();

    ComputeBoneL2W( iBone, IdentM, pKey, BoneM );

    vector3 P = BoneM * Offset;
    return P;
}

//=========================================================================

radian3 anim_group::GetEventRot( s32 iBone, const vector3& Offset, anim_key* pKey ) const
{
    matrix4 BoneM;
    matrix4 IdentM;
    IdentM.Identity();

    ComputeBoneL2W( iBone, IdentM, pKey, BoneM );

    // Offset is an angular offset
    radian3 Rot( Offset.GetX(), Offset.GetY(), Offset.GetZ() );

    matrix4 RotM;
    RotM.Identity();
    RotM.Rotate(Rot);
    RotM = BoneM * RotM;
    return RotM.GetRotation();
}

//=========================================================================

void anim_group::DumpFrames( s32 iAnim, const char* pFilename )
{
    s32 i,j;
    anim_info& Anim = m_pAnimInfo[iAnim];

    X_FILE* fp = x_fopen(pFilename,"wt");
    if( !fp ) return;

    for( j=0; j<m_nBones; j++ )
    {
        for( i=0; i<Anim.m_nFrames; i++ )
        {
            anim_key Key;
            Anim.GetRawKey( i, j, Key );

            x_fprintf(fp,"%3d %32s ",j,m_pBone[j].Name);

            x_fprintf(fp,"   |   %8.5f %8.5f %8.5f %8.5f",
                Key.Rotation.X,
                Key.Rotation.Y,
                Key.Rotation.Z,
                Key.Rotation.W);

            x_fprintf(fp,"   |   %8.2f %8.2f %8.2f",
                Key.Translation.GetX(),
                Key.Translation.GetY(),
                Key.Translation.GetZ());

#if USE_SCALE_KEYS
            x_fprintf(fp,"   |   %4.2f %4.2f %4.2f\n",
                Key.Scale.GetX(),
                Key.Scale.GetY(),
                Key.Scale.GetZ());
#endif
        }
    }

    x_fclose(fp);
}

//=========================================================================

xbool anim_group::Save( const char* pFileName, xbool bForGCN )
{
    X_FILE* fp = x_fopen(pFileName,"wb");
    if( !fp )
        return FALSE;

    xbool Result = Save(fp,bForGCN);

    x_fclose(fp);

    return Result;
}

//=========================================================================

xbool anim_group::Load( const char* pFileName )
{
    X_FILE* fp = x_fopen(pFileName,"rb");
    if( !fp )
        return FALSE;

    xbool Result = Load(fp,pFileName);

    if( !Result )
    {
        x_throw(xfs("AnimInfo <%s> has wrong version - delete your .anim files and recompile!",pFileName));
    }

    x_fclose(fp);

    return Result;
}

//=========================================================================

xbool anim_group::Save( X_FILE* fp, xbool bForGCN )
{
    anim_group AG;
    AG.CopyFrom( *this );

    s32 UncompDataSize = AG.m_UncompressedDataSize;
    s32 CompDataSize = AG.m_CompressedDataSize;

    // Convert base ptrs into offsets
    AG.SetupForSaving( TRUE, bForGCN );
    x_fwrite( &AG, sizeof(anim_group), 1, fp );
    x_fwrite( AG.m_pUncompressedData,   UncompDataSize,  1, fp );
    x_fwrite( AG.m_pCompressedData,     CompDataSize,    1, fp );
    x_fwrite( AG.m_pEventData,          sizeof( event_data ), m_nEvents, fp );

    // Restore endian
    AG.SetupForSaving( FALSE, bForGCN );

    return TRUE;
}

//=========================================================================

xbool anim_group::Load( X_FILE* fp, const char* pFileName )
{
    s32 i;

    x_fread( this, sizeof(anim_group), 1, fp );

    // Copy filename into anim_group
    if( pFileName )
    {
        char FileName[128];
        x_splitpath( pFileName, NULL, NULL, FileName, NULL );
        x_strncpy( m_FileName, FileName, 63 );
    }

    // Confirm version
    if( m_Version != ANIM_DATA_VERSION )
    {
#ifdef X_EDITOR
        x_try;
        x_throw( xfs( "You have old animation data, please recompile resources." ) );
        x_catch_display;
#else
        ASSERTS( FALSE, xfs( "You have old animation data, please recompile resources." ) );
#endif

        // Negate version number to indicate an error loading
        m_Version = -m_Version;

        return FALSE;
    }
    ASSERT(m_nBones >= 0);

    // Allocate room for uncompressed and compressed data
    {
        m_pUncompressedData = (byte*)x_malloc( m_UncompressedDataSize );
        m_pCompressedData   = (byte*)x_malloc( m_CompressedDataSize );
        ASSERT( m_pUncompressedData && m_pCompressedData );
    }

    // Allocate room for event data
    m_pEventData = new event_data[m_nEvents];
    ASSERT( m_pEventData );

    // Read data
    x_fread( m_pUncompressedData,   m_UncompressedDataSize, 1, fp );
    x_fread( m_pCompressedData,     m_CompressedDataSize,   1, fp );
    x_fread( m_pEventData,          sizeof( event_data ),   m_nEvents, fp );

    // Convert ptr offsets back into ptrs
    SetupOffsetsAndPtrs( FALSE );

    //
    // Convert event data into bytestream
    //
    m_pEvent = new anim_event[m_nEvents];
    ASSERT( NULL != m_pEvent );

    for ( i = 0; i < m_nEvents; ++i )
    {
        m_pEvent[i].SetData( m_pEventData[i] );
    }

    delete [] m_pEventData; // were done with event data
    m_pEventData = NULL;


    // Clear anim_key_block Stream ptrs
    for( i=0; i<m_nKeyBlocks; i++ )
    {
        m_pKeyBlock[i].pStream = NULL;
        m_pKeyBlock[i].pNext   = NULL;
        m_pKeyBlock[i].pPrev   = NULL;
        m_pKeyBlock[i].pFactoredCompressedData = ((byte*)GetCompressedDataPtr()) + m_pKeyBlock[i].CompressedDataOffset;
    }
    RefactorAnimKeyBlocks();

    // Setup all pAnimGroup ptrs
    for( i=0; i<m_nAnims; i++ )
        m_pAnimInfo[i].m_pAnimGroup = this;

    // Build a hash table
    {
        s32 nMissed=0;
        s32 nHit=0;
        s32 MissDist=0;
        s32 HashTableSize = (m_nAnims + m_nBones)*2;
        m_pHashTable = (hash_entry*)x_malloc(sizeof(hash_entry)*HashTableSize);
        ASSERT(m_pHashTable);
        for( i=0; i<HashTableSize; i++ )
        {
            m_pHashTable[i].Hash  =  0;
            m_pHashTable[i].iAnim = -1;
            m_pHashTable[i].iBone = -1;
        }

        for( i=0; i<m_nAnims; i++ )
        {
            u32 Hash   = ComputeUppercaseStringHash( m_pAnimInfo[i].m_Name );

            // Find entry in the hash table
            xbool bMissed = FALSE;
            s32 iIndex = Hash % HashTableSize;
            while( m_pHashTable[iIndex].Hash != 0 )
            {
                iIndex = iIndex+1;
                if( iIndex == HashTableSize ) iIndex = 0;
                MissDist++;
                bMissed = TRUE;
            }
            if( bMissed )   nMissed++;
            else            nHit++;

            // Insert hash entry
            m_pHashTable[iIndex].Hash = Hash;
            m_pHashTable[iIndex].iAnim = i;
        }

        for( i=0; i<m_nBones; i++ )
        {
            u32 Hash   = ComputeUppercaseStringHash( m_pBone[i].Name );

            // Find entry in the hash table
            xbool bMissed = FALSE;
            s32 iIndex = Hash % HashTableSize;
            while( m_pHashTable[iIndex].Hash != 0 )
            {
                iIndex = iIndex+1;
                if( iIndex == HashTableSize ) iIndex = 0;
                MissDist++;
                bMissed = TRUE;
            }
            if( bMissed )   nMissed++;
            else            nHit++;

            // Insert hash entry
            m_pHashTable[iIndex].Hash = Hash;
            m_pHashTable[iIndex].iBone = i;
        }

        //x_DebugMsg("AnimIndex HashTable Hit:%d Miss:%d Dist:%d\n",nHit,nMissed,MissDist);
    }

    return TRUE;
}

//=========================================================================

void anim_group::CopyFrom( anim_group& AG )
{

    // Convert ptrs to indices and copy structure over
    AG.SetupOffsetsAndPtrs(TRUE);
    x_memcpy( this, &AG, sizeof(anim_group) );

    // overwrite m_pEvent with some new memory
    m_pEvent = new anim_event[m_nEvents];
    
    // Reset originial back to ptrs
    AG.SetupOffsetsAndPtrs(FALSE);

    // Allocate and copy new arrays
    m_pUncompressedData = (byte*)x_malloc( m_UncompressedDataSize );
    m_pCompressedData   = (byte*)x_malloc( m_CompressedDataSize );
    ASSERT( m_pUncompressedData && m_pCompressedData );
    x_memcpy( m_pUncompressedData, AG.m_pUncompressedData, m_UncompressedDataSize );
    x_memcpy( m_pCompressedData, AG.m_pCompressedData, m_CompressedDataSize );
    x_memcpy( m_pEvent, AG.m_pEvent, sizeof( anim_event ) * m_nEvents );

    SetupOffsetsAndPtrs(FALSE);
}

//=========================================================================

void anim_group::SetupOffsetsAndPtrs ( xbool UseIndices )
{
    if( UseIndices )
    {
        // Clear all pAnimGroup ptrs
        for( s32 i=0; i<m_nAnims; i++ )
            m_pAnimInfo[i].m_pAnimGroup = NULL;

        // Convert base ptrs into offsets
        m_pBone         = (anim_bone*)      ((u32)m_pBone        - (u32)m_pUncompressedData);
        m_pAnimInfo     = (anim_info*)      ((u32)m_pAnimInfo    - (u32)m_pUncompressedData);
        m_pProp         = (anim_prop*)      ((u32)m_pProp        - (u32)m_pUncompressedData);
        m_pKeyBlock     = (anim_key_block*) ((u32)m_pKeyBlock    - (u32)m_pUncompressedData);
    }
    else
    {
        m_pBone         = (anim_bone*)      ((u32)m_pBone        + (u32)m_pUncompressedData);
        m_pAnimInfo     = (anim_info*)      ((u32)m_pAnimInfo    + (u32)m_pUncompressedData);
        m_pProp         = (anim_prop*)      ((u32)m_pProp        + (u32)m_pUncompressedData);
        m_pKeyBlock     = (anim_key_block*) ((u32)m_pKeyBlock    + (u32)m_pUncompressedData);

        // Setup all pAnimGroup ptrs
        for( s32 i=0; i<m_nAnims; i++ )
            m_pAnimInfo[i].m_pAnimGroup = this;
    }
}

//=========================================================================

void anim_group::SetupForSaving ( xbool bSetupIndices, xbool bToggleEndian )
{
    s32 i;

    // Check version ( a negative version represents an anim_group that failed to load )
    ASSERT( m_Version >= 0 );

    if( !bSetupIndices )
    {
        if( bToggleEndian )
        {
            SwapEndian( m_pBone         );
            SwapEndian( m_pAnimInfo     );
            SwapEndian( m_pProp         );
            SwapEndian( m_pKeyBlock     );

            //
            // AnimGroup
            //
            SwapEndian( m_Version       );
            SwapEndian( m_TotalNFrames  );
            SwapEndian( m_TotalNKeys    );
            SwapEndian( m_UncompressedDataSize  );
            SwapEndian( m_CompressedDataSize    );
            SwapEndian( m_nBones );
            SwapEndian( m_nAnims );
            SwapEndian( m_nProps );
            SwapEndian( m_nEvents );
            SwapEndian( m_nKeyBlocks );
        }

        m_pBone         = (anim_bone*)      ((u32)m_pBone        + (u32)m_pUncompressedData);
        m_pAnimInfo     = (anim_info*)      ((u32)m_pAnimInfo    + (u32)m_pUncompressedData);
        m_pProp         = (anim_prop*)      ((u32)m_pProp        + (u32)m_pUncompressedData);
        m_pKeyBlock     = (anim_key_block*) ((u32)m_pKeyBlock    + (u32)m_pUncompressedData);
    }

    //
    // Setup event data
    //
    if ( NULL == m_pEventData )
    {
        //
        // We must be setting up before a save
        // Allocate an array of event_data and swap it
        //
        m_pEventData = new event_data[m_nEvents];
        ASSERT( m_pEventData );

        for( i=0; i<m_nEvents; i++ )
        {
            // extract data
            m_pEventData[i] = m_pEvent[i].GetData();
        }
    }
    else
    {
        //
        // We must be swapping back after save
        // all we need to do is free up our memory
        //
        delete [] m_pEventData;
        m_pEventData = NULL;
    }

    //
    // Setup endian
    //
    if( bToggleEndian )
    {
        ASSERT(FALSE);
        /*
        //
        // Bones
        //
        for( i=0; i<m_nBones; i++ )
        {
            SwapEndian( m_pBone[i].iBone );
            SwapEndian( m_pBone[i].iParent );
            SwapEndian( m_pBone[i].nChildren );
            SwapEndian( m_pBone[i].LocalTranslation );
            SwapEndian( m_pBone[i].BindTranslation );
            SwapEndian( m_pBone[i].BindMatrixInv );
        }

        //
        // AnimInfo
        //
        for( i=0; i<m_nAnims; i++ )
        {
            anim_info& AnimInfo = m_pAnimInfo[i];

            SwapEndian( AnimInfo.m_nAnims );
            SwapEndian( AnimInfo.m_AnimsWeight );
            SwapEndian( AnimInfo.m_Weight );
            SwapEndian( AnimInfo.m_BlendTime );
            SwapEndian( AnimInfo.m_nChainFramesMin );
            SwapEndian( AnimInfo.m_nChainFramesMax );
            SwapEndian( AnimInfo.m_iChainAnim );
            SwapEndian( AnimInfo.m_iChainFrame );
            SwapEndian( AnimInfo.m_nFrames );
            SwapEndian( AnimInfo.m_iLoopFrame );
            SwapEndian( AnimInfo.m_EndFrameOffset );
            SwapEndian( AnimInfo.m_nEvents );
            SwapEndian( AnimInfo.m_iEvent );
            SwapEndian( AnimInfo.m_nProps );
            SwapEndian( AnimInfo.m_iProp );
            SwapEndian( AnimInfo.m_Flags );
            SwapEndian( AnimInfo.m_FPS );
            SwapEndian( AnimInfo.m_HandleAngle );
            SwapEndian( AnimInfo.m_TotalYaw );
            SwapEndian( AnimInfo.m_TotalMoveDir );
            SwapEndian( AnimInfo.m_TotalTranslation );
            SwapEndian( AnimInfo.m_AnimKeys.m_nFrames );
            SwapEndian( AnimInfo.m_AnimKeys.m_nBones );
            SwapEndian( AnimInfo.m_AnimKeys.m_nProps );
            SwapEndian( AnimInfo.m_AnimKeys.m_nKeyBlocks );
            SwapEndian( AnimInfo.m_AnimKeys.m_iKeyBlock );
        }

        //
        // Props
        //
        for( i=0; i<m_nProps; i++ )
        {
            SwapEndian( m_pProp[i].m_iBone );
        }

        //
        // Events
        //
        if ( NULL != m_pEventData )
        {
            for( i=0; i<m_nEvents; i++ )
            {
                // swap it
                m_pEventData[i].SwitchEndian();
            }
        }
        SwapEndian( m_pEvent ); // not doing data, since it's saved above -- just maintaining the ptr

        //
        // KeyBlocks
        //
        for( i=0; i<m_nKeyBlocks; i++ )
        {
            SwapEndian( m_pKeyBlock[i].CompressedDataOffset );
            SwapEndian( m_pKeyBlock[i].DecompressedDataSize );
            SwapEndian( m_pKeyBlock[i].nFrames );
        }
        */
    }

    if( bSetupIndices )
    {
        // Convert base ptrs into offsets
        m_pBone         = (anim_bone*)      ((u32)m_pBone        - (u32)m_pUncompressedData);
        m_pAnimInfo     = (anim_info*)      ((u32)m_pAnimInfo    - (u32)m_pUncompressedData);
        m_pProp         = (anim_prop*)      ((u32)m_pProp        - (u32)m_pUncompressedData);
        m_pKeyBlock     = (anim_key_block*) ((u32)m_pKeyBlock    - (u32)m_pUncompressedData);

        if( bToggleEndian )
        {
            SwapEndian( m_pBone         );
            SwapEndian( m_pAnimInfo     );
            SwapEndian( m_pProp         );
            SwapEndian( m_pKeyBlock     );

            //
            // AnimGroup
            //
            SwapEndian( m_Version       );
            SwapEndian( m_TotalNFrames  );
            SwapEndian( m_TotalNKeys    );
            SwapEndian( m_UncompressedDataSize  );
            SwapEndian( m_CompressedDataSize    );
            SwapEndian( m_nBones );
            SwapEndian( m_nAnims );
            SwapEndian( m_nProps );
            SwapEndian( m_nEvents );
            SwapEndian( m_nKeyBlocks );
        }
    }
}


//=========================================================================

void anim_group::GetEventNames( xarray<xstring>& Names ) const
{
    s32 i;

    for ( i = 0; i < m_nEvents; ++i )
    {
        const anim_event& Event = m_pEvent[i];
        if ( Names.Find( Event.GetName() ) < 0 )
        {
            Names.Append( Event.GetName() );
        }
    }
}


//=========================================================================

void anim_group::GetPropNames( xarray<xstring>& Names ) const
{
    s32 i;

    for ( i = 0; i < m_nProps; ++i )
    {
        const anim_prop& Prop = m_pProp[i];
        if ( Names.Find( Prop.m_Type ) < 0 )
        {
            Names.Append( Prop.m_Type );
        }
    }

}


//=========================================================================
/*
void anim_group::SwitchEndian( xbool bForGCN )
{
    s32 i;

    if( !bForGCN )
    {
        SwapEndian( m_nBones        );
        SwapEndian( m_nAnims        );
        SwapEndian( m_nProps        );
        SwapEndian( m_nEvents       );
        SwapEndian( m_nKeyBlocks    );
    }

    //
    // Bones
    //
    for( i=0; i<m_nBones; i++ )
    {
        SwapEndian( m_pBone[i].iBone );
        SwapEndian( m_pBone[i].iParent );
        SwapEndian( m_pBone[i].nChildren );
        SwapEndian( m_pBone[i].LocalTranslation );
        SwapEndian( m_pBone[i].BindTranslation );
        SwapEndian( m_pBone[i].BindMatrixInv );
    }

    //
    // AnimInfo
    //
    for( i=0; i<m_nAnims; i++ )
    {
        SwapEndian( m_pAnimInfo[i].m_nFrames );
        SwapEndian( m_pAnimInfo[i].m_nEvents );
        SwapEndian( m_pAnimInfo[i].m_iEvent );
        SwapEndian( m_pAnimInfo[i].m_nProps );
        SwapEndian( m_pAnimInfo[i].m_iProp );
        SwapEndian( m_pAnimInfo[i].m_Flags );
        SwapEndian( m_pAnimInfo[i].m_FPS );
        SwapEndian( m_pAnimInfo[i].m_HandleAngle );
        SwapEndian( m_pAnimInfo[i].m_TotalYaw );
        SwapEndian( m_pAnimInfo[i].m_TotalMoveDir );
        SwapEndian( m_pAnimInfo[i].m_TotalTranslation );
        SwapEndian( m_pAnimInfo[i].m_AnimKeys.m_nFrames );
        SwapEndian( m_pAnimInfo[i].m_AnimKeys.m_nBones );
        SwapEndian( m_pAnimInfo[i].m_AnimKeys.m_nProps );
        SwapEndian( m_pAnimInfo[i].m_AnimKeys.m_nKeyBlocks );
        SwapEndian( m_pAnimInfo[i].m_AnimKeys.m_iKeyBlock );
    }

    //
    // Props
    //
    for( i=0; i<m_nProps; i++ )
    {
        SwapEndian( m_pProp[i].m_iBone );
    }

    //
    // Events
    //
    for( i=0; i<m_nEvents; i++ )
    {
        SwapEndian( m_pEvent[i].m_Type );
        SwapEndian( m_pEvent[i].m_iBone );
        SwapEndian( m_pEvent[i].m_iFrame0 );
        SwapEndian( m_pEvent[i].m_iFrame1 );
        SwapEndian( m_pEvent[i].m_Radius );
        SwapEndian( m_pEvent[i].m_Offset );
    }

    //
    // KeyBlocks
    //
    for( i=0; i<m_nKeyBlocks; i++ )
    {
        SwapEndian( m_pKeyBlock[i].CompressedDataOffset );
        SwapEndian( m_pKeyBlock[i].DecompressedDataSize );
        SwapEndian( m_pKeyBlock[i].nFrames );
    }

    //
    // AnimGroup
    //
    SwapEndian( m_Version       );
    SwapEndian( m_TotalNFrames  );
    SwapEndian( m_TotalNKeys    );
    SwapEndian( m_pBone         );
    SwapEndian( m_pAnimInfo     );
    SwapEndian( m_pProp         );
    SwapEndian( m_pEvent        );
    SwapEndian( m_pKeyBlock     );
    SwapEndian( m_UncompressedDataSize  );
    SwapEndian( m_pUncompressedData     );
    SwapEndian( m_CompressedDataSize    );
    SwapEndian( m_pCompressedData       );

    if( bForGCN )
    {
        SwapEndian( m_nBones        );
        SwapEndian( m_nAnims        );
        SwapEndian( m_nProps        );
        SwapEndian( m_nEvents       );
        SwapEndian( m_nKeyBlocks    );
    }
}

//=========================================================================
*/

//=========================================================================
//=========================================================================
//=========================================================================
//== Anim Key Block Refactoring
//=========================================================================
//=========================================================================
//=========================================================================

anim_key_block** anim_group::m_AnimKeyBlockHash = NULL;
s32              anim_group::m_AnimKeyBlockHashSize = 0;
s32              anim_group::m_iCompDataPool = 0;
anim_group::comp_data_pool   anim_group::m_CompDataPool[] = {{0,0}};
s32 COMP_DATA_SIZE_SAVED = 0;
s32 COMP_DATA_SIZE_POOLED = 0;
#define COMP_DATA_POOL_SIZE     (128*1024)

//=========================================================================

void anim_group::ReleaseCompDataPool( void )
{
    s32 i;
    for( i=0; i<256; i++ )
    {
        x_free( m_CompDataPool[i].pCompData );
        m_CompDataPool[i].pCompData = NULL;
        m_CompDataPool[i].nBytesUsed = 0;
    }
}

//=========================================================================

byte* anim_group::MoveCompDataIntoPool( byte* pSrc, s32 nSrcBytes )
{
    s32 i;
    for( i=0; i<256; i++ )
    {
        s32 NewBytesUsed = m_CompDataPool[i].nBytesUsed + nSrcBytes;
        if( NewBytesUsed < COMP_DATA_POOL_SIZE )
        {
            // If this entry isn't allocated then allocate
            if( m_CompDataPool[i].pCompData==NULL )
            {
                m_CompDataPool[i].pCompData = (byte*)x_malloc(COMP_DATA_POOL_SIZE);
                ASSERT(m_CompDataPool[i].pCompData);
                ASSERT(m_CompDataPool[i].nBytesUsed==0);
            }

            byte* pDst = m_CompDataPool[i].pCompData + m_CompDataPool[i].nBytesUsed;

            // Copy over the source data
            x_memcpy( pDst, pSrc, nSrcBytes );

            // Update the pool
            m_CompDataPool[i].nBytesUsed = NewBytesUsed;

            // Return the result
            return pDst;
        }
    }

    ASSERT(FALSE);
    return NULL;
}

//=========================================================================

void anim_group::RefactorAnimKeyBlocks( void )
{
    s32 i;

    // Adjust size of hash table to include new anim group
    s32 nHashEntries = m_AnimKeyBlockHashSize / 2;
        nHashEntries += m_nKeyBlocks;
        nHashEntries *= 2;
    ResizeAnimKeyBlockHashTable( nHashEntries );

    // Loop through anim key blocks
    for( i=0; i<m_nKeyBlocks; i++ )
    {
        anim_key_block& BK = m_pKeyBlock[i];

        // Lookup a matching block from hash table
        anim_key_block* pParent = NULL;
        {
            s32 iHash = BK.Checksum % m_AnimKeyBlockHashSize;
            while( 1 )
            {
                if( m_AnimKeyBlockHash[iHash] == NULL )
                    break;

                if( (m_AnimKeyBlockHash[iHash]->Checksum == BK.Checksum) &&
                    (m_AnimKeyBlockHash[iHash]->nFrames  == BK.nFrames) &&
                    (m_AnimKeyBlockHash[iHash]->DecompressedDataSize == BK.DecompressedDataSize))
                {
                    pParent = m_AnimKeyBlockHash[iHash];
                    break;
                }

                iHash++;
                if( iHash==m_AnimKeyBlockHashSize )
                    iHash = 0;
            }
        }

        // Compute compressed data size 
        s32 CompDataSize=0;
        if( i<m_nKeyBlocks-1 )
            CompDataSize = m_pKeyBlock[i+1].CompressedDataOffset - m_pKeyBlock[i].CompressedDataOffset;
        else
            CompDataSize = m_CompressedDataSize - m_pKeyBlock[i].CompressedDataOffset;

        // Was there a match to refactor from?
        if( pParent )
        {
            COMP_DATA_SIZE_SAVED += CompDataSize;
            m_pKeyBlock[i].pFactoredCompressedData = pParent->pFactoredCompressedData;
        }
        else
        // Move compressed data into pools
        {
            COMP_DATA_SIZE_POOLED += CompDataSize;
            byte* pSrcCompData = ((byte*)GetCompressedDataPtr()) + m_pKeyBlock[i].CompressedDataOffset;
            m_pKeyBlock[i].pFactoredCompressedData = MoveCompDataIntoPool(pSrcCompData,CompDataSize);
        }
        
        // Add this block to the hash table
        {
            s32 iHash = BK.Checksum % m_AnimKeyBlockHashSize;
            while( 1 )
            {
                if( m_AnimKeyBlockHash[iHash] == NULL )
                {
                    m_AnimKeyBlockHash[iHash] = &m_pKeyBlock[i];
                    break;
                }

                iHash++;
                if( iHash==m_AnimKeyBlockHashSize )
                    iHash = 0;
            }
        }
    }

    // Release current compressed data
    x_free(m_pCompressedData);
    m_pCompressedData = NULL;
}

//=========================================================================

void anim_group::ResizeAnimKeyBlockHashTable( s32 nHashEntries )
{
    if( nHashEntries==0 )
    {
        x_free( m_AnimKeyBlockHash );
        m_AnimKeyBlockHash = NULL;
        m_AnimKeyBlockHashSize = 0;
        return;
    }

    anim_key_block** pNewTable = (anim_key_block**)x_malloc(sizeof(anim_key_block*)*nHashEntries);
    ASSERT(pNewTable);
    x_memset(pNewTable,0,sizeof(anim_key_block*)*nHashEntries);

    s32 i;
    for( i=0; i<m_AnimKeyBlockHashSize; i++ )
    if( m_AnimKeyBlockHash[i] )
    {
        anim_key_block* pBlock = m_AnimKeyBlockHash[i];

        // Add this block to the hash table
        {
            s32 iHash = pBlock->Checksum % nHashEntries;
            while( 1 )
            {
                if( pNewTable[iHash] == NULL )
                {
                    pNewTable[iHash] = pBlock;
                    break;
                }

                iHash++;
                if( iHash==nHashEntries )
                    iHash = 0;
            }
        }
    }

    x_free( m_AnimKeyBlockHash );
    m_AnimKeyBlockHash = pNewTable;
    m_AnimKeyBlockHashSize = nHashEntries;
}

//=========================================================================

void anim_group::RemoveAnimKeyBlocksFromHashTable( void )
{
    if( m_AnimKeyBlockHashSize==0 )
        return;

    anim_key_block* pStart = m_pKeyBlock;
    anim_key_block* pEnd   = m_pKeyBlock + (m_nKeyBlocks-1);

    s32 i;
    for( i=0; i<m_AnimKeyBlockHashSize; i++ )
    {
        anim_key_block* pBlock = m_AnimKeyBlockHash[i];
        if( (pBlock>=pStart) && (pBlock<=pEnd) )
            m_AnimKeyBlockHash[i] = NULL;
    }

    s32 nHashEntries  = m_AnimKeyBlockHashSize / 2;
        nHashEntries -= m_nKeyBlocks;
        nHashEntries *= 2;
        ASSERT(nHashEntries>=0);
    ResizeAnimKeyBlockHashTable( nHashEntries );
}

//=========================================================================
