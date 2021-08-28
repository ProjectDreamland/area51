//==============================================================================
//
//  fx_SPEmitter.hpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

struct fx_spe_key
{
    xcolor  Color;
    f32     Scale;
};

//==============================================================================

struct fx_edef_spemitter : public fx_element_def
{
    s32         BitmapIndex;
    u32         Flags;          
    s32         NParticles;
    f32         LifeSpan;
    f32         EmitVariance;   // **
    radian      MinSpinRate;    // **
    radian      MaxSpinRate;    // **
    vector3p    MinVelocity;
    vector3p    MaxVelocity;
    f32         Gravity;
    f32         Acceleration;
    f32         ZBias;
    s32         NKeyFrames;
    fx_spe_key  Key[1];         // Must be last!
}; 

//==============================================================================

enum // for Flags field within fx_edef_spemitter
{
    SPE_WORLD_SPACE         = (1 <<  0),    // vs local space
    SPE_SCALE_SPRITE_SIZE   = (1 <<  1),    // vs absolute sprite size
    SPE_EMIT_FROM_VOLUME    = (1 <<  2),    // vs emit from center point
    SPE_BURST_MODE          = (1 <<  3),    // vs streaming mode
    SPE_REVERSE_MODE        = (1 <<  4),    // vs "forward" mode
    SPE_VELOCITY_ORIENTED   = (1 <<  5),    // vs random spin
};

//==============================================================================

struct fx_sparticle
{
    radian      StartSpin;  // Initial angle of rotation.
    f32         EmitTime;   // Time particle activates in particle set cycle.
    f32         Age;        // Parametric for particle life.
    radian      SpinRate;   // Rate of rotation.
};

//==============================================================================

class fx_spemitter : public fx_element
{
//------------------------------------------------------------------------------
public:

virtual void            Initialize      ( const fx_element_def* pElementDef, 
                                                f32*            pInput );

virtual void            AdvanceLogic    ( const fx_effect_base* pEffect, 
                                                f32             DeltaTime );

virtual void            Render          ( const fx_effect_base* pEffect ) const;

virtual xbool           IsFinished      ( const fx_effect_base* pEffect ) const;

virtual void            Reset           ( void );

//------------------------------------------------------------------------------
protected:

virtual void            EmissionLogic   ( const fx_effect_base* pEffect, f32 DeltaTime );
virtual void            ParticleLogic   ( const fx_effect_base* pEffect, f32 DeltaTime );

//------------------------------------------------------------------------------
protected:

        f32             m_EmitClock;    // Time within cycle of all particles.
        f32             m_EmitCycle;    // Time to cycle through all particles.
        f32             m_EmitGap;      // Average time between new particles.
        xbool           m_Emitting;
        xbool           m_PrevL2WReady;
        matrix4         m_PreviousL2W;
        s32             m_NActive;
        s32             m_PCursor;

        // Pointers to various data buffers
        fx_sparticle*   m_pParticles;
        vector4*        m_pPositions;
        u32*            m_pColors;
        vector4*        m_pVelocities;
        vector2*        m_pRotAndScales;
        vector3*        m_pStartPos;
        vector3*        m_pStartVel;
};

//==============================================================================
