#ifndef __ELEMENT_SPEMITTER_HPP
#define __ELEMENT_SPEMITTER_HPP

#include "element.hpp"

namespace fx_core
{

//============================================================================
//  element_sprite
//============================================================================

class element_spemitter : public element
{
protected:

    struct key
    {
        xcolor  Color;
        f32     Scale;
    };

    struct particle
    {
        f32     EmitTime;       // Time particle activates in particle set cycle.
        xbool   Active;         // Active?
        vector3 Position;       // Current position
        vector3 Velocity;       // Current velocity
        radian  Rotation;       // Current Rotation
        radian  RotationSpeed;  // Rotation speed (Radians/Sec)
        xcolor  Color;          // Current color
        f32     Scale;          // Current scale
        f32     Age;            // Parametric for particle life.
        f32     AgeRate;        // Converts real time to particle parametric time.
    };

    // Sprite element data
    xstring             m_BitmapName;
    xbool               m_UseEmissionVolume;            // if false, emit from point, else, emit from volume (scale)
    xbool               m_WorldSpace;                   // local/world space
    s32                 m_CombineMode;                  // Combine mode
    xbool               m_IsBurst;                      // Is a burst mode emitter
    xbool               m_Reverse;                      // Reverse the flow
    xbool               m_Oriented;                     // Oriented based on particle velocity
    xbool               m_ScaleSprite;                  // Scale sprite rendered for particle
    f32                 m_ZBias;                        // Offset in Camera's Z axis

//    s32                 m_StartFrame;                   // Start emitting frame
//    s32                 m_StopFrame;                    // Stop emitting frame
    s32                 m_nParticles;                   // Number of particles in emitter
    f32                 m_EmitInterval;                 // Interval of emission
    f32                 m_EmitIntervalVar;              // Variance on interval of emission
    f32                 m_LifeSpan;                     // Lifespan of particles

    f32                 m_Gravity;                      // Gravity applied to particles
    f32                 m_Acceleration;                 // Acceleration applied to particles

    f32                 m_Speed;                        // Maximum speed at emission
    vector3             m_MinVelocity;                  // Minimum emission velocity
    vector3             m_MaxVelocity;                  // Maximum emission velocity
    xbool               m_ShowVelocity;                 // Shows a representation of the velocity in the viewport

    f32                 m_MinRotSpeed;                  // Minimum rotation speed
    f32                 m_MaxRotSpeed;                  // Maximum rotation speed

    xarray<key>         m_Keys;                         // Array of particle key frames

    f32                 m_CycleTime;                    // Time to complete a cycle of the particle pool (nParticles*EmitInterval)

    s32                 m_nAllocatedParticles;          // Number of allocated particle
    particle*           m_pParticles;                   // Pointer to allocated particle
    s32                 m_iLastEmitted;                 // Index of last particle emitted

private:
    xbool               SolveForTimeAtZeroVelocity  ( f32 v, f32 a, f32 g, f32& t ) const;
    f32                 SolveForPositionAtTime      ( f32 v, f32 a, f32 g, f32 t ) const;

public:
                        element_spemitter       ( );
                       ~element_spemitter       ( );

    void                Create                  ( const char* pElementID, effect& Effect );
    virtual element*    Duplicate               ( void );

    virtual void        PostCopyPtrFix          ( void );

    virtual xbool       ExistsAtTime            ( f32 T ) const;
    virtual xbool       GetLocalBBoxAtTime      ( f32 T, bbox& BBox ) const;

    virtual void        Render                  ( f32 T );
            void        RenderVelocity          ( f32 T, const matrix4& L2W ) const;

    xbool               GetShowVelocity         ( void ) const              { return m_ShowVelocity; }
    void                SetShowVelocity         ( xbool ShowVelocity )      { m_ShowVelocity = ShowVelocity; }

    //virtual void        ShowProperties          ( s32 T, CProperties* pProps );
    //virtual xbool       OnPropertyChanged       ( s32 T, CString& Field, CString& Value );


    virtual void        Save                    ( igfmgr& Igf );
    virtual void        Load                    ( igfmgr& Igf );

    virtual void        ExportData              ( export::fx_elementhdr&    ElemHdr, 
                                                  xstring&                  Type,
                                                  xbytestream&              Stream, 
                                                  s32                       ExportTarget );
    virtual void        FlagExportTextures      ( void );
    virtual void        ActivateTextures        ( void );

    void                AllocateParticles       ( xbool Force = FALSE );
    void                SetParticlePositions    ( f32   T );
    xbool               GetWorldStaticBBox      ( bbox& BBox ) const;

    void                SetWorldSpace           ( xbool UseWorldSpace );
    
    f32                 GetParticleLife         ( void ) const              { return m_LifeSpan; }

    const xbool&        GetBurstMode            ( void ) const              { return m_IsBurst;    }
    void                SetBurstMode            ( const xbool& IsBurst)     { m_IsBurst = IsBurst; }
    
    virtual xbool       OnPropertyChanged       ( s32 T, xstring& Field, xstring& Value );

    //========================================================================
    // Function for enumerating element properties
    //========================================================================
    virtual xbool       GetProperty             ( s32               Idx,
                                                  s32               T,
                                                  xcolor&           UIColor,
                                                  xstring&          Name,
                                                  xstring&          Value,
                                                  xbool&            IsDisabled,
                                                  base::prop_type&  Type );
};

//============================================================================

} // namespace fx_core

#endif
