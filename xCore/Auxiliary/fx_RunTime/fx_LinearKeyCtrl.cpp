//==============================================================================
//  
//  fx_LinearKeyCtrl.cpp
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_LinearKeyCtrl;

//==============================================================================
//  TYPES
//==============================================================================

struct fx_cdef_linear_keys : public fx_ctrl_def
{
    s32     NKeyFrames;
};

//==============================================================================

class linear_key_ctrl : public fx_ctrl
{
public:
    void Evaluate( f32 Time ) const;
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void linear_key_ctrl::Evaluate( f32 Time ) const
{
    fx_cdef_linear_keys* pDef     = (fx_cdef_linear_keys*)(m_pCtrlDef);
    f32*                 pKeyTime = (f32*)(pDef+1);
    f32*                 pKeyData = pKeyTime + pDef->NKeyFrames;
    s32                  i;
    s32                  LowerKey, UpperKey;
    f32                  Blend;

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
            pKeyData += pDef->NKeyFrames;
        }

        return;
    }

    // At the 1.0 key?
    if( Time >= 1.0f )
    {
        // Need last key frame.
        pKeyData += (pDef->NKeyFrames-1);

        for( i = 0; i < pDef->NOutputValues; i++ )
        {
            m_pOutput[ pDef->OutputIndex + i ] = *pKeyData;
            pKeyData += pDef->NKeyFrames;
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

    // Determine amount of contribution from the upper key.
    Blend = (      Time         - pKeyTime[LowerKey]) /
            (pKeyTime[UpperKey] - pKeyTime[LowerKey]);

    //
    // Go ahead and generate the values.
    //
    pKeyData += LowerKey;

    for( i = 0; i < pDef->NOutputValues; i++ )
    {           
        m_pOutput[ pDef->OutputIndex + i ] = 
                (*(pKeyData  ) * (1.0f-Blend)) + 
                (*(pKeyData+1) * (     Blend));
        pKeyData += pDef->NKeyFrames;
    }
}

//==============================================================================

#undef new
REGISTER_FX_CONTROLLER_CLASS( linear_key_ctrl, "LINEAR KEY" );

//==============================================================================
