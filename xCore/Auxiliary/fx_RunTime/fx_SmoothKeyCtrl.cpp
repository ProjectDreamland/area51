//==============================================================================
//  
//  fx_SmoothKeyCtrl.cpp
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_SmoothKeyCtrl;

//==============================================================================
//  TYPES
//==============================================================================

struct fx_cdef_smooth_keys : fx_ctrl_def
{
    s32     NKeyFrames;
};

//==============================================================================

class smooth_key_ctrl : public fx_ctrl
{
public:
    void Evaluate( f32 Time ) const;
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void smooth_key_ctrl::Evaluate( f32 Time ) const
{
    fx_cdef_smooth_keys* pDef     = (fx_cdef_smooth_keys*)(m_pCtrlDef);
    f32*                 pKeyTime = (f32*)(pDef+1);
    f32*                 pKeyData = pKeyTime + pDef->NKeyFrames;
    s32                  i;
    s32                  LowerKey, UpperKey;
    f32                  t, t2, t3;
    f32                  C0, C1, C2, C3;    // Coefficients.

    // The first and last key frames must be at time 0 and 1 respectively.
    ASSERT( pDef->NKeyFrames >= 2 );
    ASSERT( pKeyTime[         0          ] == 0.0f );
    ASSERT( pKeyTime[ pDef->NKeyFrames-1 ] == 1.0f );

    //
    // Easy cases first.
    //

    // At the 0.0 key?
    if( Time <= 0.0f )
    {   
        for( i = 0; i < pDef->NOutputValues; i++ )
        {
            m_pOutput[ pDef->OutputIndex + i ] = *pKeyData;
            pKeyData += pDef->NKeyFrames * 2;
        }

        return;
    }

    // At the 1.0 key?
    if( Time >= 1.0f )
    {
        // Need last key frame.
        pKeyData += (pDef->NKeyFrames-1) * 2;

        for( i = 0; i < pDef->NOutputValues; i++ )
        {
            m_pOutput[ pDef->OutputIndex + i ] = *pKeyData;
            pKeyData += pDef->NKeyFrames * 2;
        }

        return;
    }

    //
    // Figure out which two key frames we are between.
    // [[Optimization: Use a local binary search.]]
    //

    for( i = 1; i < pDef->NKeyFrames-1; i++ )
        if( Time < pKeyTime[i] )
            break;
    UpperKey = i;
    LowerKey = i-1;

    // Determine time t for this spline segment.
    t = (      Time         - pKeyTime[LowerKey]) /
        (pKeyTime[UpperKey] - pKeyTime[LowerKey]);

    //
    // Compute the coefficients for a Hermite spline.
    //

    t2 = t * t;                         //          t^2
    t3 = t * t2;                        //   t^3

    C0 = ( 2 * t3) - (3 * t2) + (1);    //  2t^3 - 3t^2 + 1
    C1 = (     t3) - (2 * t2) + (t);    //   t^3 - 2t^2 + t
    C2 = (-2 * t3) + (3 * t2);          // -2t^3 + 3t^2
    C3 = (     t3) - (    t2);          //   t^3 -  t^2

    //
    // Go ahead and generate the values.
    //

    pKeyData += LowerKey * 2;

    for( i = 0; i < pDef->NOutputValues; i++ )
    {           
        m_pOutput[ pDef->OutputIndex + i ] = 
            (C0 * (*(pKeyData+0))) +    // C0 * p0
            (C1 * (*(pKeyData+1))) +    // C1 * t0
            (C2 * (*(pKeyData+2))) +    // C2 * p1
            (C3 * (*(pKeyData+3)));     // C3 * t1

        pKeyData += pDef->NKeyFrames * 2;
    }
}

//==============================================================================

#undef new
REGISTER_FX_CONTROLLER_CLASS( smooth_key_ctrl, "SMOOTH KEY" );

//==============================================================================
