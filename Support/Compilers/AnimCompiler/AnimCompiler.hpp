//=========================================================================
//
//  ANIMCOMPILER.HPP
//
//=========================================================================

#ifndef ANIM_COMPILER_HPP
#define ANIM_COMPILER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "AnimData.hpp"
#include "meshutil/rawanim.hpp"
#include "meshutil/rawmesh2.hpp"
#include "x_bitstream.hpp"

//=========================================================================
// ANIM_COMPILER
//=========================================================================

struct compiler_anim
{
    // Structure containing all compiler params that can be passed 
    // around functions easily
    struct params
    {
        // Data
        xstring FileName ;              // Raw matx filename
        xstring Name ;                  // Game name of anim
        radian  HandleAngle;            // Yaw handle angle
        s32     FPS;                    // Speed of playback in frames per sec
        f32     Downsample;
        xbool   bLooping;               // TRUE if anim cycle loops
        s32     iLoopFrame;             // Frame to loop to (if it's a looping anim)
        s32     EndFrameOffset;         // Offset from end frame that flags anim has ended
        xbool   bAccumHorizMotion;      // TRUE if XZ motion updates position
        xbool   bAccumVertMotion;       // TRUE if Y motion uupdates position
        xbool   bAccumYawMotion;        // TRUE if yaw motion updates yaw
        xbool   bGravity;               // TRUE if gravity should be used on anim
        xbool   bWorldCollision;        // TRUE if world collision should be used with anim
        f32     Weight ;                // Weight of animation (compared for anims with same name)
        f32     BlendTime;              // Blend time for animation (-1 = use default)
        xstring ChainAnim;              // Anim to chain to (-1 = none)
        s32     iChainFrame;            // Frame to chain to (-1 = current)
        f32     ChainCyclesMin;         // Min cycles to play before chaining
        f32     ChainCyclesMax;         // Max cycles to play before chaining
        xbool   bChainCyclesInteger;    // Only use integer cycles
        xbool   bBlendFrames;           // Is frame interpolation on?
        xbool   bBlendLoop;             // Is frame loop interpolation on?
    
        // Constructor - always sets defaults
        params()
        {
            SetDefaults() ;
        }

        // Set all members to default value
        void SetDefaults( void )
        {
            FileName            = "NULL";   // Raw matx filename
            Name                = "NULL";   // Game name of anim 
            HandleAngle         = R_0;      // Yaw handle angle
            FPS                 = 30;       // Speed of playback in frames per sec
            Downsample          = 1.0f;
            bLooping            = FALSE;    // TRUE if anim cycle loops
            iLoopFrame          = 0;        // Frame to loop to (if it's a looping anim)
            EndFrameOffset      = 0;        // Offset from end frame that flags anim has ended
            bAccumHorizMotion   = TRUE;     // TRUE if XZ motion updates position
            bAccumVertMotion    = FALSE;    // TRUE if Y motion uupdates position
            bAccumYawMotion     = FALSE;    // TRUE if yaw motion updates yaw
            bGravity            = TRUE;     // TRUE if gravity should be used on anim
            bWorldCollision     = TRUE;     // TRUE if world collision should be used with anim
            Weight              = 1.0f;     // Weight of animation (compared for anims with same name)
            BlendTime           = -1.0f;    // Blend time for animation (-1 = use default)
            ChainAnim           = "";       // Anim to chain to
            iChainFrame         = -1;       // Frame to chain to (-1 = current)
            ChainCyclesMin      = 1.0f;     // Min cycles to play before chaining
            ChainCyclesMax      = 1.0f;     // Max cycles to play before chaining
            bChainCyclesInteger = TRUE;     // Only use integer cycles
            bBlendFrames        = TRUE;     // Is frame interpolation on?
            bBlendLoop          = TRUE;     // Is frame loop interpolation on?
        }
    } ;

    s32         Index;
    rawanim     RawAnim;
    params      Params ;

    compiler_anim( void );
    ~compiler_anim( void );

    void SpecialSwap( compiler_anim& Anim )
    {
        s32     I = Index;  Index  = Anim.Index;  Anim.Index = I;
        params  P = Params; Params = Anim.Params; Anim.Params = P;
        
        // Do shallow copy without destructor destroying everything
        byte Buffer[sizeof(rawanim)];
        x_memcpy( Buffer, &Anim.RawAnim, sizeof(rawanim) );
        x_memcpy( &Anim.RawAnim, &RawAnim, sizeof(rawanim) );
        x_memcpy( &RawAnim, Buffer, sizeof(rawanim) );
    };

};

//=========================================================================

class anim_compiler
{

//-------------------------------------------------------------------------
public:
            anim_compiler     ( void );
           ~anim_compiler     ( void );

    // *INEV* *SB* - Added for A51
    void    SetBindPose ( const char* pBindPose );
    void    AddAnimation( const compiler_anim::params& Params ) ;

    xbool   Compile( const char* pInputFile, anim_group& AnimGroup );
    xbool   Compile( const char* pBindPoseFile, const char* pBindMeshName, int nAnims, xstring *AnimNames, xstring *AnimFiles, anim_group& AnimGroup );
    const char* GetBindFileName( void ) { return m_BindFileName; }
    void    SetOutputName( const char* pFileName );

//-------------------------------------------------------------------------
private:
    xbool   Setup   ( const char*   pInputFile );

    xbool   Setup   ( const char*   pBindPoseFile, 
                      const char*   pBindMeshName, 
                      int           nAnims, 
                      xstring*      AnimNames, 
                      xstring*      AnimFiles );

public: // *INEV* *SB - Added for A51
    xbool   Compile ( anim_group& AnimGroup, xbool bKeepBind = FALSE );
    
private:
    void    PrepareSourceAnims( xbool bKeepBind = FALSE );
    void    BuildSkeleton   ( anim_group& AnimGroup );
    void    BuildAnimInfo   ( anim_group& AnimGroup );
    bbox    ComputeBounds   ( anim_group& AG, s32 iAnim, xbool bUseAccumFlags );
    void    BuildBounds     ( anim_group& AnimGroup );
    void    BuildEvents     ( anim_group& AnimGroup );
    void    BuildProps      ( anim_group& AnimGroup );
    void    BuildAnimKeys   ( anim_group& AnimGroup );

    void    AttachUncompressedData( anim_group& AnimGroup );
    void    AttachCompressedData( anim_group& AnimGroup );
    void    DisplayAnimCompressStats( anim_group& AG, const char* pFileName );

    void    ClearUserInfo   ( void );
    void    SetUserInfo     ( const rawmesh2& RawMesh );
    void    SetUserInfo     ( const rawanim&  RawAnim );
    void    ReportWarning   ( const char* pWarning );
    void    ReportError     ( const char* pError );
    void    ThrowError      ( const char* pError );


//-------------------------------------------------------------------------
private:

    #define MAX_ANIMS 256

    // Compute info
    char            m_BindFileName[256];
    char            m_BindMeshName[256];
    char            m_OutputFileName[256];
    rawanim         m_Bind;
    rawmesh2        m_BindMesh;
    compiler_anim*  m_Anim;
    s32             m_nAnims;
    bitstream       m_BS;
    f32*            m_MaxDistToVerts;    
    
    // User info of current anim being processed
    char            m_SourceFile[256];
    char            m_UserName[256];
    char            m_ComputerName[256];
    s32             m_ExportDate[3];
};

//=========================================================================
#endif // END ANIM_COMPILER_HPP
//=========================================================================
