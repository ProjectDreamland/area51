//=========================================================================
//
//  FASTSTRIP.HPP
//
//=========================================================================
#ifndef FASTSTRIP_HPP
#define FASTSTRIP_HPP
//=========================================================================

#include "x_types.hpp"

//=========================================================================
// CLASS
//=========================================================================
class faststrip
{
//=========================================================================
public:

    enum
    {
        ADC = (1<<31),
        CCW = (1<<30),
    };
    
                faststrip           ( void );
               ~faststrip           ( void );

    void        Open                ( s32 MinStripLen, xbool DoBackfaceCulling );
    void        AddTri              ( s32 I1, s32 I2, s32 I3 );
    void        Close               ( void );

    void        SetWeights          ( f32 W0, f32 W1, f32 W2, f32 W3, f32 W4 );

    // Returns array of final indices
    s32*        GetIndices          ( s32& nIndices );
    
    // Returns array of final ps2 indices
    s32         GetIndicesPS2       ( s32* pIndices, s32 MaxStripLength );
    s32         GetMaxNumIndices    ( s32  MaxStripLength );
    s32         GetIndex            ( s32  Index );
    xbool       IsIndexNewStrip     ( s32  Index );
    xbool       IsIndexCCWTri       ( s32  Index );

    // Allows you to interrogate individual strips
    s32         GetNStrips          ( void ) const;
    s32         GetStripNTris       ( s32 iStrip ) const;
    void        GetStripTris        ( s32 iStrip, s32* pTriIndices );

    //s32         GetNLeftovers       ( void ) const;
    //void        GetLeftover         ( s32 iLeftover, s32& i0, s32& i1, s32& i2 ) const;

//=========================================================================
private:

    struct tri
    {
        s32     IsActive;       // TRUE if triangle does not belong to strip
        s32     Sequence;       // Already encountered in current stripping
        s32     VID[3];         // Vertex indices
        s32     NID[3];         // Neighboring tri indices
        s32     NVID[3];        // Entry vert for neighboring triangle
        s32     FirstInStripID[6]; // Strip ids that contain this tri first
        s32     nActiveNeighbors;
    };

    struct strip
    {
        s32     Len;            // Current length of triangle strip
        s32     Score;          // Current score for this strip
        s32     Sequence;       // Sequence number for rescoring

        s32     iStartTri;      // Index of starting triangle
        s32     iStartVert;     // Index of starting vert
        s32     iStartCW;       // Index of starting orientation

        s32     iEndTri;        // End triangle of final strip
        xbool   Keep;           // Keep this strip
        xbool   IsActive;       // Is strip still being considered

        s32     HID;            // hash id
        s32     HashNext;       // linked list inside hash table
        s32     HashPrev;       // linked list inside hash table
    };


//=========================================================================
private:

    tri*    m_pTri;
    s32     m_nTris;
    s32     m_nTrisAllocated;

    strip*  m_pStrip;
    s32     m_nStrips;
    s32     m_nActiveStrips;
    s32     m_nKeepStrips;

    s32     m_MinStripLen;
    xbool   m_DoBackfaceCulling;
    s32     m_TriSequence;
    s32     m_StripSequence;
    s32     m_nStripsPerTri;

    f32     m_ScoreWeight[5];
    f32     m_RescorePercentage;

    s32*    m_pHash;
    s32     m_nHashEntries;
    s32     m_MinHashIndex;
    s32     m_MaxHashIndex;


//=========================================================================
private:

    void        Clear                   ( void );
    void        ScoreStrip              ( s32 iStrip );

    void        SolveTriangleNeighbors  ( void );
    void        EnumerateAllStrips      ( void );
    void        ScoreAndSortAllStrips   ( void );
    void        UpdateStripScores       ( void );
    void        ChooseOptimalStrips     ( void );
    void        StepTriSequence         ( void );
    void        StepStripSequence       ( void );
    void        DeactivateStrip         ( s32 iStrip );
    void        FindBestStrip           ( void );
    void        SanityCheck             ( void );
    void        DumpScores              ( const char* pFilename );
    void        DumpFinalStats          ( const char* pFilename );

    void        InitHashTable           ( void );
    void        AddStripToHash          ( s32 iStrip );
    void        RemoveStripFromHash     ( s32 iStrip );
    void        ResortStripInHash       ( s32 iStrip );
    void        DumpHashTable           ( const char* pFilename );

    friend s32         StripScoreCompareFn     ( const void* pItem1, const void* pItem2 );

};

//=========================================================================
#endif
//=========================================================================
