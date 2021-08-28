#ifndef TRACERMGR_HPP
#define TRACERMGR_HPP

#include "ResourceMgr\ResourceMgr.hpp"
#include "x_math.hpp"
#include "x_bitmap.hpp"

//=========================================================================

#define NULL_TRACER_ID              0xFFFFFFFF

typedef u32         tracer_id;

class tracer_mgr
{
public:
    enum
    {
        MAX_TRACERS_PER_TYPE = 50
    };

    // Tracer type is limited to a max useable value of 255
    enum tracer_type
    {
        TRACER_TYPE_BULLET = 0,
        TRACER_TYPE_ENERGY_SWOOSH,
        NUM_TRACER_TYPES,

        TRACER_TYPE_UNKNOWN = 256,
    };

     tracer_mgr ( void );
    ~tracer_mgr ( void );
    void        Init        ( void );
    void        Kill        ( void );

    void        OnUpdate    ( f32            DeltaTime );
    void        Render      ( void );
    tracer_id   AddTracer   ( tracer_type    Type,
                              f32            FadeOutTime,
                              const vector3& StartPos,
                              const vector3& EndPos, 
                              const xcolor&  Color = xcolor( 90, 80, 40, 255 ) );

    void        UpdateTracer( tracer_id         ID,
                              const vector3*    pStartPos = NULL,
                              const vector3*    pEndPos   = NULL,
                              const xcolor*     pColor    = NULL );

protected:
    enum tracer_bmp_types
    {
        TRACER_BMP_BULLET = 0,
        TRACER_BMP_SWOOSH = 1,
        NUM_TRACER_BMPS
    };

    struct tracer
    {
        // time is interleaved with the positions so that this could
        // eventually be moved to vu0 and would fit nicely into vector
        // registers
        vector3 StartPos;
        f32     FadeTime;
        vector3 EndPos;
        f32     ElapsedTime;
        xbool   IsAlive;
        xcolor  Color;
        u16     Sequence;
    };
    typedef rhandle<xbitmap> tracer_bitmap;   

    tracer*         m_pTracers[NUM_TRACER_TYPES];
    tracer_bitmap   m_Bitmaps[NUM_TRACER_BMPS];
    u16             m_Sequence;
};

//=========================================================================

extern tracer_mgr g_TracerMgr;

//=========================================================================

#endif // TRACERMGR_HPP