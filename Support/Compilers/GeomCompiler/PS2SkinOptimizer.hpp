#ifndef PS2SKINOPTIMIZER_HPP
#define PS2SKINOPTIMIZER_HPP

#include "x_files.hpp"

///////////////////////////////////////////////////////////////////////////
// VU Skinning optimizer - original planning by TA and DS, coding by DS
///////////////////////////////////////////////////////////////////////////

class ps2skin_optimizer
{
public:
    ///////////////////////////////////////////////////////////////////////
    // Some consts--these values should match whatever the VU memory
    // layout and microcode can support.
    ///////////////////////////////////////////////////////////////////////
    enum
    {
        // IMPORTANT - THESE MUST MATCH VU1!!!!!!!!!!!!!!!
        kMaxWeights         = 8,
        kMatrixCacheSize    = 36,
        kSkinVertBufferSize = 68,
    };

    ///////////////////////////////////////////////////////////////////////
    // Structs used by the optimizer
    ///////////////////////////////////////////////////////////////////////

    struct optimizer_vert
    {
        virtual xbool operator== ( const optimizer_vert& V ) const;

        s32     iOrigIndex;                     // index to the original vert pool
        s32     nWeights;                       // number of weights the vert uses
        s32     iOrigBones[kMaxWeights];        // index to the original matrix pool
        f32     fWeights[kMaxWeights];          // the weights for each bone (must total 1)

        // the following members will be filled in by te optimizer
        s32     iCacheBones[kMaxWeights];       // final index to the vu cached bone matrix
        xbool   ADC;                            // start of a new strip?
        xbool   CCW;                            // is the current vert ending a CCW or CW polygon?
    };

    struct optimizer_tri
    {
        optimizer_vert  Verts[3];               // Duh...self explanatory.
        xbool           Added;                  // INTERNAL USE ONLY
    };

    struct tri_batch
    {
        tri_batch();

        s32                     nBonesToLoad;
        s32                     iOrigBones[kMatrixCacheSize];
        s32                     iCacheBones[kMatrixCacheSize];
        xarray<optimizer_tri>   Tris;
    };

    struct final_batch
    {
        s32                     nBonesToLoad;
        s32                     iOrigBones[kMatrixCacheSize];
        s32                     iCacheBones[kMatrixCacheSize];
        xarray<optimizer_vert>  Verts;
    };

    ///////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ///////////////////////////////////////////////////////////////////////
             ps2skin_optimizer  ( void );
    virtual ~ps2skin_optimizer  ( void );

    ///////////////////////////////////////////////////////////////////////
    // Functions for feeding the optimizer
    ///////////////////////////////////////////////////////////////////////
    void     Reset              ( void );
    void     AddTri             ( const optimizer_tri& Tri );
    void     Optimize           ( void );
    void     PrintStats         ( void );

    ///////////////////////////////////////////////////////////////////////
    // Functions for retrieving the optimized results
    ///////////////////////////////////////////////////////////////////////
    s32                 GetNFinalBatches    ( void );
    const final_batch&  GetFinalBatch       ( s32 Index );


protected:
    ///////////////////////////////////////////////////////////////////////
    // Functions for internal use
    ///////////////////////////////////////////////////////////////////////

    s32     FindMatrixCacheIndices  ( optimizer_tri& Tri );
    xbool   AddMatricesToCache      ( optimizer_tri& Tri, tri_batch& Batch );
    void    FlushMatrixCache        ( void );
    void    ClearMatrixCache        ( void );
    void    BuildFinalBatches       ( void );
    s32     CountSharedVerts        ( tri_batch& Batch, optimizer_tri& Tri );

    xbool                   m_bOptimized;
    s32                     m_MatrixCache[kMatrixCacheSize];
    xarray<optimizer_tri>   m_lOrigTris;
    xarray<tri_batch>       m_lTriBatches;
    xarray<final_batch>     m_lFinalBatches;
    xtimer                  m_TotalTime;
    xtimer                  m_CacheBuildTime;
    xtimer                  m_StripBuildTime;
    byte*                   m_pVertInBatch;
};

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////

#endif // PS2SKINOPTIMIZER_HPP