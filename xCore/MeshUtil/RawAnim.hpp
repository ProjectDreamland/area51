
#ifndef RAW_ANIM_HPP
#define RAW_ANIM_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
//=========================================================================
// DEFINES
//=========================================================================

#define NUM_EVENT_STRINGS       5
#define MAX_EVENT_STRING_LENGTH 32 + 1
#define NUM_EVENT_INTS          5
#define NUM_EVENT_FLOATS        8
#define NUM_EVENT_BOOLS         8
#define NUM_EVENT_COLORS        4         

//=========================================================================
// CLASS
//=========================================================================
struct rawanim
{

//=========================================================================

    struct bone
    {
        s32         iBone;
        s32         iParent;
        s32         nChildren;

        char        Name[256];
        char        ParentName[256] ;

        vector3     BindTranslation;
        quaternion  BindRotation;
        vector3     BindScale;
        matrix4     BindMatrix;
        matrix4     BindMatrixInv;

        xbool       bScaleKeys;
        xbool       bRotationKeys;
        xbool       bTranslationKeys;
        xbool       bIsMasked;
        s32         LODGroup;

        s32         Depth ;
        s32         iBoneRemap ;
    };

    struct frame
    {
        vector3     Scale;
        quaternion  Rotation;
        vector3     Translation;
    };

    struct event
    {
        char        Name[64];
        char        ParentName[64];
        s32            Type;
        f32            Radius;
        s32            Frame0;
        s32            Frame1;
        vector3        Position;
    };

    struct super_event
    {
        char        Name[64];
        char        ParentName[64];

        s32         Type;
        s32         StartFrame;
        s32         EndFrame;
        vector3     Position;
        quaternion  Rotation;
        f32         Radius;

        xbool       ShowAxis;
        xbool       ShowSphere;
        xbool       ShowBox;

        f32         AxisSize;

        f32         Width;
        f32         Length;
        f32         Height;

        char        Strings[NUM_EVENT_STRINGS][MAX_EVENT_STRING_LENGTH];
        s32         Ints[NUM_EVENT_INTS];
        f32         Floats[NUM_EVENT_FLOATS];
        xbool       Bools[NUM_EVENT_BOOLS];

        xcolor      Colors[NUM_EVENT_COLORS];
    };
    struct prop_frame
    {
        vector3     Scale;
        quaternion  Rotation;
        vector3     Translation;
        xbool       bVisible;
    };

    struct prop
    {
        char        Name[64];
        char        ParentName[64];
        char        Type[33];
    };

//=========================================================================

     rawanim( void );
    ~rawanim( void );

    s32     CompareBoneNames        ( s32 iBoneA, s32 iBoneB ) ;
    s32     CompareBoneDepths       ( s32 iBoneA, s32 iBoneB ) ;
    xbool   AreBonesFromSameBranch  ( s32 iBoneA, s32 iBoneB ) ;
    void    PutBonesInLODOrder      ( void ) ;
    void    PrintHierarchy          ( void );

    xbool   Load                ( const char* pFileName );

    void    ComputeBonesL2W     ( matrix4* pMatrix, f32 Frame );

    void    GetMotionPropFrame  ( s32           iFrame,
                                  vector3&      Scale, 
                                  vector3&      Trans, 
                                  quaternion&   Rot );

    void    ComputeBonesL2W     ( matrix4* pMatrix, 
                                  s32      iFrame,
                                  xbool    bRemoveHorizMotion,
                                  xbool    bRemoveVertMotion,
                                  xbool    bRemoveYawMotion ) ;

    void    ComputeBoneL2W      ( s32 iBone, matrix4& Matrix, f32 Frame );
    void    ComputeRawBoneL2W   ( s32 iBone, matrix4& Matrix, s32 iFrame );

    s32     GetBoneIDFromName   ( const char* pBoneName );
    void    ComputeBoneKeys     ( quaternion* pQ, vector3* pS, vector3* pT, f32 Frame );

    void    BakeBindingIntoFrames( xbool BakeScale, xbool BakeRotation, xbool BakeTranslation );
    void    DumpFrames          ( const char* pFileName, xbool InBoneOrder );

    void    Resample            ( s32 NewNFrames );

    void    DeleteBone          ( s32 iBone );
    void    DeleteBone          ( const char* pBoneName );
    void    DeleteDummyBones    ( void );           // Deletes all bones with "dummy" in the name

    xbool   ApplyNewSkeleton    ( const rawanim& BindAnim );
    xbool   HasSameSkeleton     ( const rawanim& Anim );

    void    SanityCheck         ( void );

    xbool   IsMaskedAnim        ( void );

//=========================================================================
    
    char            m_SourceFile[X_MAX_PATH];
    char            m_UserName[256];
    char            m_ComputerName[256];
    s32             m_ExportDate[3];  // month, day, year


    bone*           m_pBone;
    s32             m_nBones;

    frame*          m_pFrame;
    s32             m_nFrames;

    event*            m_pEvent;
    s32                m_nEvents;

    super_event*    m_pSuperEvent;
    s32             m_nSuperEvents;
    prop*           m_pProp;
    s32             m_nProps;

    prop_frame*     m_pPropFrame;
    s32             m_nPropFrames;
};

//=========================================================================
// END
//=========================================================================
#endif










































