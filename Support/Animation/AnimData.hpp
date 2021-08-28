//=========================================================================
//
//  ANIMDATA.HPP
//
//=========================================================================
#ifndef ANIM_DATA_HPP
#define ANIM_DATA_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "x_math.hpp"
#include "x_bytestream.hpp"
#include "..\ResourceMgr\ResourceMgr.hpp"



//=========================================================================
// Use scale keys?
//=========================================================================
#ifdef ANIM_COMPILER
#define USE_SCALE_KEYS  TRUE    // Always put in data
#else
#define USE_SCALE_KEYS  FALSE   // A51 doesn't currently use scale keys
#endif

//=========================================================================
#define ANIM_DATA_VERSION           2014
//=========================================================================
#define MAX_ANIM_BONES              80
//=========================================================================
#define MAX_KEYS_PER_BLOCK_SHIFT    (5) // 32
#define MAX_KEYS_PER_BLOCK          (1<<MAX_KEYS_PER_BLOCK_SHIFT)
#define MAX_KEYS_PER_BLOCK_MASK     ((1<<MAX_KEYS_PER_BLOCK_SHIFT)-1)
//=========================================================================
#define STREAM_SCL_NBITS            (2) // 4 scale formats
#define STREAM_ROT_NBITS            (2) // 4 rot formats
#define STREAM_TRS_NBITS            (2) // 4 trans formats
#define STREAM_FLG_NBITS            (8) // 8 stream flags
#define STREAM_OFT_NBITS            (18)// 256k offset
                                    
#define STREAM_SCL_SHIFT            (32              -STREAM_SCL_NBITS)
#define STREAM_ROT_SHIFT            (STREAM_SCL_SHIFT-STREAM_ROT_NBITS)
#define STREAM_TRS_SHIFT            (STREAM_ROT_SHIFT-STREAM_TRS_NBITS)
#define STREAM_FLG_SHIFT            (STREAM_TRS_SHIFT-STREAM_FLG_NBITS)
#define STREAM_OFT_SHIFT            (STREAM_FLG_SHIFT-STREAM_OFT_NBITS)
                                    
#define STREAM_SCL_MASK             ((1<<STREAM_SCL_NBITS)-1)
#define STREAM_ROT_MASK             ((1<<STREAM_ROT_NBITS)-1)
#define STREAM_TRS_MASK             ((1<<STREAM_TRS_NBITS)-1)
#define STREAM_FLG_MASK             ((1<<STREAM_FLG_NBITS)-1)
#define STREAM_OFT_MASK             ((1<<STREAM_OFT_NBITS)-1)
//=========================================================================
#define STREAM_FLAG_MASKED          (1<<0)
#define STREAM_FLAG_UNUSED_0        (1<<1)
#define STREAM_FLAG_UNUSED_1        (1<<2)
#define STREAM_FLAG_UNUSED_2        (1<<3)
#define STREAM_FLAG_UNUSED_3        (1<<4)
#define STREAM_FLAG_UNUSED_4        (1<<5)
#define STREAM_FLAG_UNUSED_5        (1<<6)
#define STREAM_FLAG_UNUSED_6        (1<<7)
//=========================================================================

struct anim_bone;
struct anim_key;
struct anim_prop;
struct anim_event;

class anim_compiler;
class anim_keys;
class anim_info;
class anim_group;

//=========================================================================
// STRUCT ANIM_BONE
//=========================================================================

struct anim_bone
{
    matrix4     BindMatrixInv;
    vector3     LocalTranslation;
    vector3     BindTranslation;

    s16         iBone;
    s16         iParent;
    s16         nChildren;
    char        Name[32+2];     // 1 bytes for terminating 0, the other for 4 byte alignement with the s16's
};

//=========================================================================
// CLASS ANIM_KEY
//=========================================================================

struct anim_key
{
//-------------------------------------------------------------------------
public:

    void Interpolate( const anim_key& Key0, const anim_key& Key1, f32 T );
    void Identity( void ) ;
    void Setup( matrix4& M ) ;

//-------------------------------------------------------------------------
public:

#if USE_SCALE_KEYS    
    vector3     Scale;
#endif

    quaternion  Rotation;
    vector3     Translation;
};


inline void anim_key::Interpolate( const anim_key& K0, const anim_key& K1, f32 T )
{
#if USE_SCALE_KEYS    
    Scale       = K0.Scale       + T*(K1.Scale       - K0.Scale);
#endif

    Rotation    = Blend( K0.Rotation, K1.Rotation, T );
    Translation = K0.Translation + T*(K1.Translation - K0.Translation);
}


//=========================================================================
// CLASS ANIM_KEY_SET
//=========================================================================

enum anim_key_format
{
    CONSTANT_VALUE  =0,
    SINGLE_VALUE    =1,
    PRECISION_16    =2,
    PRECISION_32    =3,
};

//=========================================================================


struct anim_key_stream
{
    //  2 bits for the scale format = 4 formats
    //  2 bits for the rot   format = 4 formats
    //  2 bits for the trans format = 4 formats
    //  8 bits for bone flag bits   = 8 custom flags per bone per anim
    // 18 bits for the offset to the scale data = 0-262143 range
    //-----
    // 32

    u32 Offset;

    u32             GetFlags                ( void ) const;
    void            GetOffsetsAndFormats    ( s32 nFrames,
                                              s32& SO, 
                                              s32& RO, 
                                              s32& TO,
                                              anim_key_format& SF, 
                                              anim_key_format& RF, 
                                              anim_key_format& TF ) const;

    void            GetRawKey               ( byte* pData, s32 nFrames, s32 iFrame, anim_key& Key );
    void            GetInterpKey            ( byte* pData, s32 nFrames, s32 iFrame, f32 T, anim_key& Key );
    
    void            SetFlags                ( u32 Flags );
    void            SetFormats              ( anim_key_format SF, anim_key_format RF, anim_key_format TF );
    void            SetOffset               ( s32 ScaleOffset );

private:

    void            GrabKey                 ( s32 iFrame, anim_key& Key );

    static  s32     s_SF;
    static  s32     s_RF;
    static  s32     s_TF;
    static  s32     s_SO;
    static  s32     s_RO;
    static  s32     s_TO;
    static  byte*   s_pData;
};

//=========================================================================

struct anim_key_block
{
    anim_key_block*     pNext;                  // Links in currently decompressed list
    anim_key_block*     pPrev;
    anim_key_stream*    pStream;                // Points to decompressed data if available
    u32                 Checksum;
    byte*               pFactoredCompressedData;
    s32                 CompressedDataOffset;   // Offset into compressed data for this key set
    s32                 nFrames:8,
                        DecompressedDataSize:24;

    anim_key_stream*    AcquireStreams( const anim_group& AG );
    void                ReleaseStreams( void ); // Will deallocate decompressed streams
    void                DetachFromList( void );
    void                AttachToList( void );
};

//=========================================================================

class anim_keys
{
//-------------------------------------------------------------------------
public:
            anim_keys( void );
           ~anim_keys( void );

//-------------------------------------------------------------------------
private:

    // Only returns bone streams
    xbool   IsBoneMasked        ( const anim_group& AnimGroup, s32 iBone ) const;
    void    GetRawKeys          ( const anim_group& AnimGroup, s32 iFrame, anim_key* pKey ) const;
    void    GetInterpKeys       ( const anim_group& AnimGroup, f32  Frame, anim_key* pKey ) const;
    void    GetInterpKeys       ( const anim_group& AnimGroup, f32  Frame, anim_key* pKey, s32 nBones ) const;

    // Can return bone or prop streams
    void    GetRawKey           ( const anim_group& AnimGroup, s32 iFrame, s32 iStream, anim_key& Key ) const;
    void    GetInterpKey        ( const anim_group& AnimGroup, f32  Frame, s32 iStream, anim_key& Key ) const;


//-------------------------------------------------------------------------
private:

    s16                 m_nFrames;
    s16                 m_nBones;
    s16                 m_nProps;
    s16                 m_nKeyBlocks;
    s16                 m_iKeyBlock;

friend anim_info;
friend anim_compiler;
friend anim_group;
};
//=========================================================================
// CLASS EVENT_FORMAT
//=========================================================================
#define EVENT_MAX_INTS          5
#define EVENT_MAX_FLOATS        8
#define EVENT_MAX_POINTS        2
#define EVENT_MAX_BOOLS         8
#define EVENT_MAX_STRINGS       5
#define EVENT_MAX_COLORS        4
#define EVENT_MAX_STRING_LENGTH 32


class event_data_format
{
public:
					event_data_format( void );
    s32             GetNInts        ( void ) const;
    s32             GetNFloats      ( void ) const;
    s32             GetNPoints      ( void ) const;
    s32             GetNBools       ( void ) const;
    s32             GetNStrings     ( void ) const;
    s32				CountSetBits    ( s32 NumBitsBefore, s32 NumBitsPossible ) const;
	void			SetInt			( s32 Idx );
	void			SetFloat        ( s32 Idx );
	void			SetPoint		( s32 Idx );
	void			SetBool			( s32 Idx );
	void			SetString		( s32 Idx );

	u32 m_Flags;
};




//=========================================================================
// CLASS EVENT_DATA
//=========================================================================
class event_data
{
public:
    event_data  ( void );
    void SetType( const char* Type );
    void SetName( const char* Name );
    void StoreInt( s32 Idx, s32 Value );
    void StoreFloat( s32 Idx, f32 Value );
    void StorePoint( s32 Idx, const vector3& Value );
    void StoreBool( s32 Idx, xbool Value );
    void StoreString( s32 Idx, const char* String );

    s32             nInts               ( void ) const  { return m_DataFormat.GetNInts(); }
    s32             nFloats             ( void ) const  { return m_DataFormat.GetNFloats(); }
    s32             nPoints             ( void ) const  { return m_DataFormat.GetNPoints(); }
    s32             nBools              ( void ) const  { return m_DataFormat.GetNBools(); }
    s32             nStrings            ( void ) const  { return m_DataFormat.GetNStrings(); }

    const s32*      Ints                ( void ) const  { return m_Ints; }
    const f32*      Floats              ( void ) const  { return m_Floats; }
    const vector3*  Points              ( void ) const  { return m_Points; }
    const xbool*    Bools               ( void ) const  { return m_Bools; }
    const char**    Strings             ( void ) const  { return (const char**)m_Strings; }
    const char*     String              ( s32 i ) const { return (const char*)m_Strings[i]; }

    const char*     GetType             ( void ) const  { return m_Type; }
    const char*     GetName             ( void ) const  { return m_Name; }

    event_data_format	GetDataFormat	( void ) const						{ return m_DataFormat; }
	void				SetDataFormat	( event_data_format DataFormat )	{ m_DataFormat = DataFormat; }

    void            SwitchEndian        ( void );

private:
    vector3             m_Points[EVENT_MAX_POINTS];
    event_data_format   m_DataFormat;
    s32                 m_Ints[EVENT_MAX_INTS];
    f32                 m_Floats[EVENT_MAX_FLOATS];
    xbool               m_Bools[EVENT_MAX_BOOLS];
    char                m_Type[EVENT_MAX_STRING_LENGTH+1];
    char                m_Name[EVENT_MAX_STRING_LENGTH+1];
    char                m_Strings[EVENT_MAX_STRINGS][EVENT_MAX_STRING_LENGTH+1];
};

enum event_types
{   
    EVENT_TYPE_OLD_EVENT,
    EVENT_TYPE_DO_NOT_USE,
    EVENT_HOT_POINT,
    EVENT_TYPE_AUDIO,
    EVENT_TYPE_PARTICLE,
    EVENT_TYPE_GENERIC,
    EVENT_TYPE_INTENSITY,
    EVENT_TYPE_WORLD_COLLISION,
    EVENT_TYPE_GRAVITY,
    EVENT_TYPE_WEAPON,
    EVENT_TYPE_PAIN,
    EVENT_TYPE_DEBRIS,
    EVENT_TYPE_SET_MESH,
    EVENT_TYPE_SWAP_MESH,
    EVENT_TYPE_FADE_GEOMETRY,
    EVENT_TYPE_SWAP_TEXTURE,
    EVENT_TYPE_CAMERA_FOV,
    NUM_EVENT_TYPES
};

extern char g_EventTypes[NUM_EVENT_TYPES][EVENT_MAX_STRING_LENGTH+1];

//=========================================================================
// CLASS ANIM_INFO
//=========================================================================

class anim_info
{

#define ANIM_DATA_FLAG_LOOPING                  (1<<0)
#define ANIM_DATA_FLAG_HAS_MASKS                (1<<1)
#define ANIM_DATA_FLAG_ACCUM_HORIZ_MOTION       (1<<2)
#define ANIM_DATA_FLAG_ACCUM_VERT_MOTION        (1<<3)
#define ANIM_DATA_FLAG_ACCUM_YAW_MOTION         (1<<4)
#define ANIM_DATA_FLAG_GRAVITY                  (1<<5)
#define ANIM_DATA_FLAG_WORLD_COLLISION          (1<<6)
#define ANIM_DATA_FLAG_CHAIN_CYCLES_INTEGER     (1<<7)
#define ANIM_DATA_FLAG_BLEND_FRAMES             (1<<8)
#define ANIM_DATA_FLAG_BLEND_LOOP               (1<<9)



//-------------------------------------------------------------------------
public:
            anim_info                   ( void );
           ~anim_info                   ( void );

    // Random weight selection
    s32         GetNAnims               ( void ) const;
    f32         GetAnimsWeight          ( void ) const;

    const char* GetName                 ( void ) const;
    f32         GetWeight               ( void ) const;

    f32         GetBlendTime            ( void ) const;
    s32         GetNFrames              ( void ) const;
    s32         GetLoopFrame            ( void ) const;
    s32         GetEndFrameOffset       ( void ) const;

    // Key
    void        GetRawKey               ( s32 iFrame, s32 iBone, anim_key& Key ) const;
    void        GetInterpKey            ( f32  Frame, s32 iBone, anim_key& Key ) const;
    void        GetRawKeys              ( s32 iFrame, anim_key* pKey ) const;
    void        GetInterpKeys           ( f32  Frame, anim_key* pKey ) const;
    void        GetInterpKeys           ( f32  Frame, anim_key* pKey, s32 nBones ) const;
    
    // Misc
    radian      GetTotalMoveDir         ( void ) const;
    radian      GetTotalYaw             ( void ) const;
    radian      GetHandleAngle          ( void ) const;
    vector3     GetTotalTranslation     ( void ) const;
    const bbox& GetBBox                 ( void ) const;
    s32         GetFPS                  ( void ) const;
    f32         GetSpeed                ( void ) const;
    f32         GetYawRate              ( void ) const;
    
    // Flags
    xbool       DoesLoop                ( void ) const;
    xbool       HasMasks                ( void ) const;
    xbool       AccumHorizMotion        ( void ) const;
    xbool       AccumVertMotion         ( void ) const;
    xbool       AccumYawMotion          ( void ) const;
    xbool       Gravity                 ( void ) const;
    xbool       WorldCollision          ( void ) const;
    xbool       ChainCyclesInteger      ( void ) const;
    xbool       BlendFrames             ( void ) const;
    xbool       BlendLoop               ( void ) const;
    xbool       IsBoneMasked            ( s32 iBone ) const;

    // Chain
    s32         GetChainFramesMin       ( void ) const;
    s32         GetChainFramesMax       ( void ) const;
    s32         GetChainAnim            ( void ) const;
    s32         GetChainFrame           ( void ) const;

    // Animated bone info
    s32         GetAnimBoneMinIndex     ( void ) const;
    s32         GetAnimBoneMaxIndex     ( void ) const;

    // Props
    s32         GetNProps               ( void ) const;
    s32         GetPropChannel          ( const char *pChannelName ) const;
    void        GetPropRawKey           ( s32 iChannel, s32 iFrame, anim_key& Key ) const;
    void        GetPropInterpKey        ( s32 iChannel, f32 Frame,  anim_key& Key ) const;
    s32         GetPropParentBoneIndex  ( s32 iChannel ) const;

    // Events
    s32          GetNEvents                 ( void ) const;
    anim_event&  GetEvent                   ( s32 iEvent ) const;
    xbool        IsEventActive              ( s32 iEvent, f32 Frame ) const;
    xbool        IsEventActive              ( s32 iEvent, f32 CurrFrame, f32 PrevFrame ) const;
    xbool        IsEventTypeActive          ( s32 Type, f32 Frame ) const;
    xbool        IsEventTypeActive          ( s32 Type, f32 CurrFrame, f32 PrevFrame ) const;
    void         SetEventData               ( s32 iEvent, const event_data& ED );
    f32          FindLipSyncEventStartFrame ( void ) const; 

//-------------------------------------------------------------------------
private:

    vector3         m_TotalTranslation;     // Total movement
    bbox            m_BBox;                 // BBox of all verts pushed thu anim
    
    anim_group*     m_pAnimGroup;           // Owner animation group
    
    s32             m_nAnims ;              // Number of consecutive anims with this name
    f32             m_AnimsWeight ;         // Total weight of all consecutive anims with this name

    char            m_Name[32];             // Reference name
    f32             m_Weight ;              // Influence random select weight
    f32             m_BlendTime;            // Blend time to use (-1 = use default)
    
    s16             m_nChainFramesMin;      // Min frames to play before chaining
    s16             m_nChainFramesMax;      // Max frames to play before chaining
    s16             m_iChainAnim;           // Anim to chain to (0=none)
    s16             m_iChainFrame;          // Frame to chain to (-1 = current)
    
    s16             m_iAnimBoneMin;         // Index of first animated bone
    s16             m_iAnimBoneMax;         // Index of last animated bone
    
    s16             m_nFrames;              // Number of frames (default is 0)
    s16             m_iLoopFrame;           // The frame to loop if a looping anim
    s16             m_EndFrameOffset;       // Offset from end frame that flags anim has ended

    s16             m_nEvents;              // Number of events
    s16             m_iEvent;               // Index of first event
    
    s16             m_nProps;               // Number of props
    s16             m_iProp;                // Index of first prop
    
    u16             m_HandleAngle;          // Handle yaw
    u16             m_TotalYaw;             // Total yaw
    u16             m_TotalMoveDir;         // Total move yaw
    
    u16             m_Flags;                // Flags
    u16             m_FPS;                  // Frames per second
    
    anim_keys       m_AnimKeys;             // List of keys

friend anim_compiler;
friend anim_group;
};


//=========================================================================
// CLASS ANIM_EVENT
//=========================================================================
struct anim_event
{
//-------------------------------------------------------------------------
public:
            anim_event       ( void );
           ~anim_event       ( void );

//-------------------------------------------------------------------------
public:
	//
	// Old event members (will be removed)
	//
	/*

    s16             m_Type;
    s16             m_iBone;
    s16             m_iFrame0;
    s16             m_iFrame1;
    f32             m_Radius;
    vector3         m_Offset;
	*/

public:
    //
    // These are all indices into the flags (INT_BLAH = X would be the Xth int)
    //
    enum int_idxs
    {
        // Standard ints
        INT_IDX_START_FRAME = 0,
        INT_IDX_END_FRAME,
		INT_IDX_BONE,

        INT_IDX_FIRST_CUSTOM,

        // Old style events
        INT_IDX_OLD_TYPE        = INT_IDX_FIRST_CUSTOM,

        // Effect events
        INT_IDX_EFFECT_GUID_INDEX = INT_IDX_FIRST_CUSTOM,

        // Looped audio events
        INT_IDX_AUDIO_DATA      = INT_IDX_FIRST_CUSTOM,

        // Looped audio events
        INT_IDX_WEAPON_DATA     = INT_IDX_FIRST_CUSTOM,

        // Pain events
        INT_IDX_PAIN_TYPE       = INT_IDX_FIRST_CUSTOM,

        // Fade Geometry events
        INT_IDX_FADE_DIRECTION  = INT_IDX_FIRST_CUSTOM,
    };

    enum float_idxs
    {
        // Standard floats
        FLOAT_IDX_RADIUS = 0,
        FLOAT_IDX_FIRST_CUSTOM,

        // Intensity events.
        FLOAT_IDX_CONTROLLER_INTENSITY = FLOAT_IDX_FIRST_CUSTOM,
        FLOAT_IDX_CONTROLLER_DURATION,
        FLOAT_IDX_CAMERA_SHAKE_TIME,
        FLOAT_IDX_CAMERA_SHAKE_AMOUNT,
        FLOAT_IDX_CAMERA_SHAKE_SPEED,
        FLOAT_IDX_CAMERA_INTENSITY,
        FLOAT_IDX_BLUR_INTENSITY,
        FLOAT_IDX_BLUR_DURATION,

        FLOAT_IDX_DEBRIS_MIN_VELOCITY = FLOAT_IDX_FIRST_CUSTOM,
        FLOAT_IDX_DEBRIS_MAX_VELOCITY,
        FLOAT_IDX_DEBRIS_LIFE,

        // geometry fading events
        FLOAT_IDX_GEOMETRY_FADE_TIME = FLOAT_IDX_FIRST_CUSTOM,

        FLOAT_IDX_CAMERA_FOV = FLOAT_IDX_FIRST_CUSTOM,
        FLOAT_IDX_CAMERA_FOV_TIME,
    };

    enum point_idxs
    {
        // Standard points
        POINT_IDX_OFFSET = 0,
        POINT_IDX_ROTATION,
        POINT_IDX_FIRST_CUSTOM,
    };

    enum bool_idxs
    {
        BOOL_IDX_FIRST_CUSTOM = 0,
        
        BOOL_IDX_WORLD_COLLISION        = BOOL_IDX_FIRST_CUSTOM,

        BOOL_IDX_GRAVITY                = BOOL_IDX_FIRST_CUSTOM,

        BOOL_IDX_PARTICLE_EVENT_ACTIVE  = BOOL_IDX_FIRST_CUSTOM,
        BOOL_IDX_PARTICLE_DONOT_APPLY_TRANSFORM,

        BOOL_IDX_DEBRIS_BOUNCE          = BOOL_IDX_FIRST_CUSTOM,
    };

    enum string_idxs
    {
        STRING_IDX_FIRST_CUSTOM     = 0,

        // Effects strings
        STRING_IDX_EFFECT_RESOURCE  = STRING_IDX_FIRST_CUSTOM,

        // Projectile strings
        STRING_IDX_PARTICLE_TYPE    = STRING_IDX_FIRST_CUSTOM,

        // One-Shot Audio strings
        STRING_IDX_AUDIO_SOUND_ID   = STRING_IDX_FIRST_CUSTOM,
        STRING_IDX_AUDIO_LOCATION,
        STRING_IDX_AUDIO_TYPE,
        
        // HotPoint
        STRING_IDX_HOTPOINT_TYPE    = STRING_IDX_FIRST_CUSTOM,

        // Generic strings
        STRING_IDX_GENERIC_TYPE     = STRING_IDX_FIRST_CUSTOM,

        // Generic strings
        STRING_IDX_PAIN_TYPE        = STRING_IDX_FIRST_CUSTOM,

        // Rigid instance.
        STRING_IDX_DEBRIS_TYPE      = STRING_IDX_FIRST_CUSTOM,

        // Set Mesh
        STRING_IDX_SET_MESH         = STRING_IDX_FIRST_CUSTOM,
        STRING_IDX_SET_MESH_ON_OR_OFF,

        // Swap Mesh
        STRING_IDX_SWAP_MESH_ON     = STRING_IDX_FIRST_CUSTOM,
        STRING_IDX_SWAP_MESH_OFF,

        // Swap Virtual Texture
        STRING_IDX_SET_TEXTURE      = STRING_IDX_FIRST_CUSTOM,
        STRING_IDX_USE_TEXTURE,

    };

    static void     Init            ( void );   
    void            SetData         ( const event_data& Data );
    event_data      GetData         ( void ) const;
    const char*     GetType         ( void ) const;
    const char*     GetName         ( void ) const;
    s32             StartFrame      ( void ) const  { return GetInt( INT_IDX_START_FRAME ); }
    s32             EndFrame        ( void ) const  { return GetInt( INT_IDX_END_FRAME ); }
    s32             GetNInts        ( void ) const { return m_DataFormat.GetNInts(); }
    s32             GetNFloats      ( void ) const { return m_DataFormat.GetNFloats(); }
    s32             GetNPoints      ( void ) const { return m_DataFormat.GetNPoints(); }
    s32             GetNBools       ( void ) const { return m_DataFormat.GetNBools(); }
    s32             GetNStrings     ( void ) const { return m_DataFormat.GetNStrings(); }
    u32             GetDataFlags    ( void ) const { return m_DataFormat.m_Flags; }
    const byte*     GetDataBuffer   ( void ) const;

    // in these methods, "Idx" refers to the index 
    // within that specific type, so GetString(4)
    // gets the 4th string
    s32             GetInt          ( s32 Idx ) const;
    f32             GetFloat        ( s32 Idx ) const;
    vector3         GetPoint        ( s32 Idx ) const;
    xbool           GetBool         ( s32 Idx ) const;
    const char*     GetString       ( s32 Idx ) const;
    /*
    void            SetInt          ( s32 Idx, s32 Value );
    void            SetFloat        ( s32 Idx, f32 Value );
    void            SetPoint        ( s32 Idx, vector3 Value );
    void            SetBool         ( s32 Idx, xbool Value );
    void            SetString       ( s32 Idx, char* pValue );
    */

	static const byte*	GetStreamBuffer     ( void ) { return m_pEventByteStream->GetBuffer(); }
	static const byte*  GetTypeNameBuffer   ( void ) { return m_pEventTypeNameStrings->GetBuffer(); }
	static s32          GetStreamLength     ( void ) { return m_pEventByteStream->GetLength(); }
	static s32          GetTypeNameLength   ( void ) { return m_pEventTypeNameStrings->GetLength(); }

	void			SwitchEndian            ( void );

    static void ResetByteStreams( void );
private:
    s32             m_ByteStreamDataOffset;
    s32             m_TypeOffset;
    s32             m_NameOffset;

	event_data_format m_DataFormat;

    static const char* GetTypeNameString    ( s32 Offset );
    static s32      SaveTypeNameString      ( const char* String );

    static xbytestream* m_pEventByteStream;
    static xbytestream* m_pEventTypeNameStrings;
};

//=========================================================================
// CLASS ANIM_PROP
//=========================================================================

struct anim_prop
{
//-------------------------------------------------------------------------
public:
            anim_prop       ( void );
           ~anim_prop       ( void );

//-------------------------------------------------------------------------
public:

    char            m_Type[32];
    s32             m_iBone;
};

//=========================================================================
// CLASS ANIM_GROUP
//=========================================================================

class anim_group
{

//-------------------------------------------------------------------------
public:
                        anim_group          ( void );
                       ~anim_group          ( void );

    s32                 GetNAnims           ( void ) const;
    
    s32                 GetAnimIndex        ( const char* pAnimName ) const;
    s32                 GetRandomAnimIndex  ( const char* pAnimName, s32 iSkipAnim = -1 ) const;
    s32                 GetRandomAnimIndex  ( s32 iStartAnim, s32 iSkipAnim = -1 ) const;

    const anim_info&    GetAnimInfo         ( s32 iAnim ) const;
    void                GetL2W              ( const matrix4& L2W, f32  Frame, s32 iAnim, matrix4* pBoneL2W ) const;


    // Skeleton related calls
    const anim_bone&    GetBone             ( s32 iBone ) const;
    s32                 GetNBones           ( void ) const;
    s32                 GetBoneIndex        ( const char* pBoneName, xbool FindAnywhere = FALSE ) const;
    s32				    GetBoneParent		( s32 iBone ) const;
    void			    ComputeBoneL2W		( s32 iBone, const matrix4& L2W, anim_key* pKey, matrix4& BoneL2W ) const;
    void			    ComputeBonesL2W		( const matrix4& L2W, anim_key* pKey, s32 nBones, matrix4* BoneL2W, xbool bApplyTheBindPose = TRUE ) const;
	const matrix4&      GetBoneBindInvMatrix( s32 iBone ) const;
    vector3             GetEventPos         ( s32 iBone, const vector3& Offset, anim_key* pKey ) const;
    const bbox&         GetBBox             ( void ) const;
    radian3             GetEventRot         ( s32 iBone, const vector3& Offset, anim_key* pKey ) const;

    // IO related
    void                CopyFrom            ( anim_group& AnimGroup );
    void                SwitchEndian        ( xbool ForGCN );
    xbool               Save                ( const char* pFileName, xbool bForGCN=FALSE );
    xbool               Save                ( X_FILE* fp, xbool bForGCN=FALSE );
    xbool               Load                ( const char* pFileName );
    xbool               Load                ( X_FILE* fp, const char* pFileName = NULL );
    void                DumpFrames          ( s32 iAnim, const char* pFilename );
    void                SetupOffsetsAndPtrs ( xbool UseIndices );
    void                SetupForSaving      ( xbool bSetupIndices, xbool bToggleEndian );


    const byte*         GetCompressedDataPtr( void ) const;
    const char*         GetFileName         ( void ) const;

    void                GetEventNames       ( xarray<xstring>& Names ) const;
    void                GetPropNames        ( xarray<xstring>& Names ) const;

    // Anim key block refactoring
    void                RefactorAnimKeyBlocks               ( void );
    void                ResizeAnimKeyBlockHashTable         ( s32 nHashEntries );
    void                RemoveAnimKeyBlocksFromHashTable    ( void );
    void                ReleaseCompDataPool                 ( void );
    byte*               MoveCompDataIntoPool                ( byte* pSrc, s32 nSrcBytes );

//-------------------------------------------------------------------------
private:

    void                Clear( void );

//-------------------------------------------------------------------------
private:

    struct hash_entry
    {
        u32 Hash;
        s16 iAnim;
        s16 iBone;
    };

    bbox            m_BBox ;    // Bounding box of bind pose geom pushed through all animations

    char            m_FileName[60];
    hash_entry*     m_pHashTable;

    s32             m_Version;          // A negative version indicates this anim_group failed to load
    s32             m_TotalNFrames;
    s32             m_TotalNKeys;
    
    s32             m_nBones;
    anim_bone*      m_pBone;

    s32             m_nAnims;
    anim_info*      m_pAnimInfo;

    s32             m_nProps;
    anim_prop*      m_pProp;

    s32             m_nEvents;
    anim_event*     m_pEvent;
    event_data*     m_pEventData; // temp storage for events during save

    s32             m_nKeyBlocks;
    anim_key_block* m_pKeyBlock;

    s32             m_UncompressedDataSize;
    byte*           m_pUncompressedData;

    s32             m_CompressedDataSize;
    byte*           m_pCompressedData;

    static anim_key_block** m_AnimKeyBlockHash;
    static s32              m_AnimKeyBlockHashSize;

    struct comp_data_pool
    {
        s32     nBytesUsed;
        byte*   pCompData;
    };

    static comp_data_pool   m_CompDataPool[256];
    static s32              m_iCompDataPool;

friend anim_compiler;
friend anim_info;
friend anim_keys;

public:
    typedef rhandle<anim_group> handle ;
};

//=========================================================================
// anim_info inlines
//=========================================================================

inline
f32 anim_info::GetBlendTime( void ) const
{
    return m_BlendTime;
}

//=========================================================================

inline
s32 anim_info::GetNFrames( void ) const
{
    return m_nFrames;
}

//=========================================================================

inline
s32 anim_info::GetLoopFrame( void ) const
{
    return m_iLoopFrame;
}

//=========================================================================

inline
s32 anim_info::GetEndFrameOffset( void ) const
{
    return m_EndFrameOffset;
}

//=========================================================================

inline
s32 anim_info::GetPropParentBoneIndex( s32 iChannel ) const
{
    return m_pAnimGroup->m_pProp[m_iProp + iChannel].m_iBone;
}

//=========================================================================

inline
s32 anim_info::GetNAnims( void ) const
{
    return m_nAnims ;
}

//=========================================================================

inline
f32 anim_info::GetAnimsWeight( void ) const
{
    return m_AnimsWeight ;
}

//=========================================================================

inline
const char* anim_info::GetName( void ) const
{
    return m_Name;
}

//=========================================================================

inline
f32 anim_info::GetWeight( void ) const
{
    return m_Weight ;
}

//=========================================================================

inline
radian anim_info::GetTotalMoveDir  ( void ) const
{
    return (f32)m_TotalMoveDir * (R_360 / 65535.0f);
}

//=========================================================================

inline
radian anim_info::GetTotalYaw  ( void ) const
{
    return (f32)m_TotalYaw * (R_360 / 65535.0f);
}

//=========================================================================

inline
radian anim_info::GetHandleAngle      ( void ) const
{
    return (f32)m_HandleAngle * (R_360 / 65535.0f);
}

//=========================================================================

inline
vector3 anim_info::GetTotalTranslation ( void ) const
{
    return m_TotalTranslation;
}

//=========================================================================

inline
const bbox& anim_info::GetBBox( void ) const
{
    return m_BBox;
}

//=========================================================================

inline
s32 anim_info::GetFPS( void ) const
{
    return m_FPS;
}

//=========================================================================

inline
xbool anim_info::DoesLoop( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_LOOPING) != 0) ;
}

//=========================================================================

inline
xbool anim_info::HasMasks( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_HAS_MASKS) != 0) ;
}

//=========================================================================

inline
xbool anim_info::AccumHorizMotion( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_ACCUM_HORIZ_MOTION) != 0) ;
}

//=========================================================================

inline
xbool anim_info::AccumVertMotion( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_ACCUM_VERT_MOTION) != 0) ;
}

//=========================================================================

inline
xbool anim_info::AccumYawMotion( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_ACCUM_YAW_MOTION) != 0) ;
}

//=========================================================================

inline
xbool anim_info::Gravity( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_GRAVITY) != 0) ;
}

//=========================================================================

inline
xbool anim_info::WorldCollision( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_WORLD_COLLISION) != 0) ;
}

//=========================================================================

inline
xbool anim_info::ChainCyclesInteger( void ) const
{
    return ((m_Flags & ANIM_DATA_FLAG_CHAIN_CYCLES_INTEGER) != 0) ;
}

//=========================================================================

inline
xbool anim_info::BlendFrames( void ) const
{
    return ( ( m_Flags & ANIM_DATA_FLAG_BLEND_FRAMES ) != 0 );
}

//=========================================================================

inline
xbool anim_info::BlendLoop( void ) const
{
    return ( ( m_Flags & ANIM_DATA_FLAG_BLEND_LOOP ) != 0 );
}

//=========================================================================

inline
xbool anim_info::IsBoneMasked( s32 iBone ) const
{
    return m_AnimKeys.IsBoneMasked( *m_pAnimGroup, iBone );
}

//=========================================================================

inline
s32 anim_info::GetChainFramesMin( void ) const
{
    return m_nChainFramesMin;
}

//=========================================================================

inline
s32 anim_info::GetChainFramesMax( void ) const
{
    return m_nChainFramesMax;
}

//=========================================================================

inline
s32 anim_info::GetChainAnim( void ) const
{
    return m_iChainAnim;
}

//=========================================================================

inline
s32 anim_info::GetChainFrame( void ) const
{
    return m_iChainFrame;
}

//=========================================================================

inline
s32 anim_info::GetAnimBoneMinIndex( void ) const
{
    return m_iAnimBoneMin;
}

//=========================================================================

inline
s32 anim_info::GetAnimBoneMaxIndex( void ) const
{
    return m_iAnimBoneMax;
}

//=========================================================================

inline
s32 anim_info::GetNProps( void ) const
{
    return m_nProps;
}

//=========================================================================

inline
s32 anim_info::GetNEvents( void ) const
{
    return m_nEvents;
}

//=========================================================================

inline
anim_event& anim_info::GetEvent( s32 iEvent ) const
{
    ASSERT( (iEvent>=0) && (iEvent<m_nEvents) );
    return m_pAnimGroup->m_pEvent[m_iEvent + iEvent];
}

//=========================================================================

inline
void anim_info::SetEventData( s32 iEvent, const event_data& EventData )
{
    ASSERT( m_pAnimGroup );
    ASSERT( (iEvent>=0) && (iEvent<m_nEvents) );
    m_pAnimGroup->m_pEvent[m_iEvent + iEvent].SetData( EventData );
}

//=========================================================================
// anim_group inlines
//=========================================================================

inline
const bbox& anim_group::GetBBox( void ) const
{
    return m_BBox ;
}

//=========================================================================

inline
s32 anim_group::GetBoneParent   ( s32 iBone ) const
{
    return m_pBone[iBone].iParent;
}

//=========================================================================

inline
const matrix4&  anim_group::GetBoneBindInvMatrix ( s32 iBone ) const
{
    return m_pBone[iBone].BindMatrixInv;
}

//=========================================================================

inline
const anim_bone& anim_group::GetBone( s32 iBone ) const
{
    ASSERT( (iBone>=0) && (iBone<m_nBones) );
    return m_pBone[iBone];
}

//=========================================================================

inline
s32 anim_group::GetNAnims( void ) const
{
    return m_nAnims;
}

//=========================================================================

inline
s32 anim_group::GetNBones( void ) const
{
    return m_nBones;
}

//=========================================================================

inline
const anim_info& anim_group::GetAnimInfo( s32 iAnim ) const
{
    ASSERT( (iAnim>=0) && (iAnim<m_nAnims) );
    return m_pAnimInfo[iAnim];
}

//=========================================================================

extern s32 g_extern_anim_link;

//=========================================================================
#endif // END ANIM_DATA_HPP
//=========================================================================











































