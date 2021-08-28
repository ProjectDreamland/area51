//==============================================================================
//  LensFilter.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class handles the logic for applying a pulsing lens color over the
//  screen (modulating the vertex colors, so that flashlight can still make it
//  brighter)
//==============================================================================

#ifndef __LENSFILTER_HPP__
#define __LENSFILTER_HPP__

#include "Obj_mgr\Obj_mgr.hpp"

//==============================================================================
// CLASS DEFINITION
//==============================================================================

class lens_filter : public object
{
public:
    CREATE_RTTI( lens_filter, object, object )

         lens_filter( void );
        ~lens_filter( void );

    virtual const object_desc& GetTypeDesc  ( void ) const;
    static  const object_desc& GetObjectType( void );

    virtual s32         GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    virtual bbox        GetLocalBBox    ( void ) const;
    virtual void        OnEnumProp      ( prop_enum& List );
    virtual xbool       OnProperty      ( prop_query& I );
    virtual void        OnActivate      ( xbool bFlag );
    virtual void        OnAdvanceLogic  ( f32 DeltaTime );

protected:
    enum
    {
        STATE_INACTIVE = 0,
        STATE_ACTIVE,
        STATE_FADE_IN,
        STATE_FADE_OUT,
    };

    // colors that we interpolate between
    xcolor          m_PulseColorMin;
    xcolor          m_PulseColorMax;

    // variables for fading in and out from the filter effect
    f32             m_FadeInTime;
    f32             m_FadeOutTime;
    f32             m_FadeTimeElapsed;

    // a variable for describing the pulse speed in terms of a sine wave
    f32             m_PulseWaveLength;  // using the formula y = sin(2*pi*time/wavelength)

    // the current state of this object
    s32             m_CurrentState;
    f32             m_CurrentWaveTime;  // modulated elapsed time (0 <= wave time < wavelength)
};

//==============================================================================

#endif // __LENSFILTER_HPP__