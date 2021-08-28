#ifndef LIGHTMGR_HPP
#define LIGHTMGR_HPP

#include "x_color.hpp"
#include "x_math.hpp"

class light_mgr
{
public:
    enum    { MAX_CHAR_LIGHTS      = 60 };
    enum    { MAX_DYNAMIC_LIGHTS   = 10 };
    enum    { MAX_FADING_LIGHTS    = 10 };
    enum    { MAX_COLLECTED_LIGHTS = 10 };

             light_mgr( void );
    virtual ~light_mgr( void );

    void    OnUpdate                ( f32 DeltaTime );

    // Functions for adding lights to the light manager. You should probably do this once
    // per frame. You clear the light list first, and then for each light that is visible
    // re-add it to the light manager. The exception is a fading light (such as a muzzle
    // flash). These will automatically fade and disappear over time.
    void    ClearLights             ( void );   // does not clear the fading lights
    void    AddFadingLight          ( const vector3& Pos,
                                      const xcolor&  C,
                                      f32            Radius,
                                      f32            Intensity,
                                      f32            FadeTime );
    void    AddDynamicLight         ( const vector3& Pos,
                                      const xcolor&  C,
                                      f32            Radius,
                                      f32            Intensity,
                                      xbool          CharOnly );

    // Here are the functions for getting lights that actually hit an object. You should
    // make sure you are in the begin/end pair before asking for lights. Also, make
    // sure you only call begin/end once per frame, because it can be an expensive
    // operation. (Although worthwhile because it will do some optimizations on the data
    // before you make a bunch of ligh queries.)
    void    BeginLightCollection    ( void );
    void    EndLightCollection      ( void );
    void    ResetAfterException     ( void );
    s32     CollectLights           ( const bbox&    WorldBBox,
                                      s32            MaxLightCount = 3 );
    void    GetCollectedLight       ( s32            Index,
                                      vector3&       Pos,
                                      f32&           Radius,
                                      xcolor&        C );
    s32     CollectCharLights       ( const matrix4& L2W,
                                      const bbox&    B,
                                      s32            MaxLightCount = 3 );
    void    GetCollectedCharLight   ( s32            Index,
                                      vector3&       Dir,
                                      xcolor&        C );
    s32     GetNNonCharLights       ( void ) const;
    void    GetLight                ( s32            Index,
                                      vector3&       Pos,
                                      f32&           Radius,
                                      xcolor&        C ) const;

protected:
    struct fading_light
    {
        vector3 Pos;
        f32     Radius;
        xcolor  StartColor;
        xcolor  CurrentColor;
        f32     FadeTime;
        f32     ElapsedTime;
        xbool   Valid;
        f32     InterpolationT;
        f32     Intensity;

        s32     PrevLink;
        s32     NextLink;
    };

    struct dynamic_light
    {
        vector3 Pos;
        f32     Radius;
        xcolor  Color;
        f32     Intensity;
    };

    struct dir_light
    {
        vector3 Dir;
        xcolor  Col;
    };

    struct spad_light
    {
        // WARNING: Make sure position and radius are the first elements
        // of this structure, and the structure itself needs to be
        // 16-byte aligned.
        vector3 Pos;
        f32     Radius;
        xcolor  Color;
        f32     Intensity;
        f32     Score;
        xbool   CharOnly;
    };

    // internal helper routines
    s32     AddLight                ( void );
    void    RemoveLight             ( s32            LightIndex );
    void    ReduceCollectedLights   ( s32            MaxLightCount );
    xbool   CalcDirLight            ( dir_light*     pDst,
                                      const matrix4& L2W,
                                      const bbox&    Box,
                                      const vector3& Pos,
                                      f32            Radius,
                                      f32            Intensity,
                                      xcolor&        C );
    friend s32 SpadLightSortFn      ( const void* pA,
                                      const void* pB );

    // linked-list of fading lights (caused by muzzle flashes, explosions, etc.)
    s32             m_FirstLink;
    s32             m_NFadingLights;
    fading_light    m_FadingLights[MAX_FADING_LIGHTS];

    // list of dynamic lights (no need for it to be a linked-list, since they
    // should be accessed linearly)
    s32             m_NDynamicLights;
    dynamic_light   m_DynamicLights[MAX_DYNAMIC_LIGHTS];
    s32             m_NCharLights;
    dynamic_light   m_CharLights[MAX_CHAR_LIGHTS];

    // list of potential collectors
    xbool           m_bInCollection;
    s32             m_NSpadLights;
    s32             m_nNonCharLightsInSpad;
    spad_light*     m_pSpadLights;

    // collection information for a particular instance
    s32         m_NCollectedLights;
    s32         m_CollectedLights[MAX_COLLECTED_LIGHTS];
    dir_light   m_CollectedCharLights[MAX_COLLECTED_LIGHTS];
};

extern light_mgr    g_LightMgr;

//=========================================================================

inline
void light_mgr::ClearLights( void )
{
    m_NDynamicLights = 0;
    m_NCharLights    = 0;
}

//=========================================================================

inline
s32 light_mgr::GetNNonCharLights( void ) const
{
    ASSERT( m_bInCollection );

    return m_nNonCharLightsInSpad;
}

//=========================================================================
// EOF
//=========================================================================

#endif // LIGHTMGR_HPP