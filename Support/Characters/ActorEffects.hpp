//==============================================================================
//  
//  ActorEffects.hpp
//
//==============================================================================

#ifndef ACTOR_EFFECTS_HPP
#define ACTOR_EFFECTS_HPP

//==============================================================================

#include "objects\object.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//==============================================================================

#define MAX_FRY_POINTS      8   // Flame and shock effect.
#define MAX_CLOAK_POINTS   11   // Various clocking effects.

class actor;

//==============================================================================

class actor_effects
{

//------------------------------------------------------------------------------
public:

        enum effect_type
        {
            FX_FLASHLIGHT = 0,  // Must be 0.
            FX_FLAME,
            FX_SHOCK,
            FX_MUTATE,
            FX_UNMUTATE,
            FX_SPAWN,
            FX_CLOAK,
            FX_DECLOAK,
            FX_CLOAK_PAIN,
            FX_CONTAIGON,

            FX_MAX,
            FX_FIRST = 0,
        };

                    actor_effects       ( void );
                   ~actor_effects       ( void );

        void        Init                ( void );
        void        Kill                ( void );
        void        UpdateFlashLight    ( effect_type Type, object* pParent );
        void        Update              ( object* pParentObj, f32 DeltaTime );
        void        Render              ( object* pParentObj );   
        void        RenderTransparent   ( object* pParentObj, f32 Alpha = 1.0f );

        xbool       IsActive            ( void );

        void        InitEffect          ( effect_type Type, object* pParent = NULL, s32 iBone = -1 );
        void        KillEffect          ( effect_type Type );
        xbool       IsEffectOn          ( effect_type Type );

        void        SetDeathTimer       ( f32 DeathTimer );
        f32         GetDeathTimer       ( void ) { return( m_DeathTimer ); }

        void        SetShockTimer       ( f32 Timer );        

//------------------------------------------------------------------------------
protected:

        void            InitBasicEffect (       effect_type   Type,
                                          const vector3&      Position,
                                                s32           Zone,
                                          const char*         pEffectName,
                                          const char*         pAudioName );

        void            InitCloakEffect (       effect_type   Type,
                                                object*       pParent, 
                                          const char*         pEffectName,
                                                s32           iBone = -1 );

        void            InitFryEffect   (       effect_type   Type,
                                                object*       pParent, 
                                          const char*         pEffectName,
                                          const char*         pAudioName );

        void            KillFryEffect   ( void );

const   vector3         GetBonePosition ( object* pParent, s32 iBone );
        void            GetBoneL2W      ( object* pParent, s32 iBone, matrix4& L2W );

        struct fx_bone
        { 
            s32       iBone;
            fx_handle FXHandle;
        };

        f32             m_DeltaTime;    // To avoid repeated arg passing.
        f32             m_DeathTimer;
        f32             m_ShockTimer;

        xbool           m_bActive     [ FX_MAX ];
        fx_handle       m_FXHandle    [ FX_MAX ];
        s32             m_AudioID     [ FX_MAX ];

        s32             m_nFryPoints;

        fx_bone         m_FryBone     [ MAX_FRY_POINTS   ];
        fx_bone         m_CloakBone   [ MAX_CLOAK_POINTS ];

};

//==============================================================================
#endif // ACTOR_EFFECTS_HPP
//==============================================================================
