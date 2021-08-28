#ifndef CHARACTER_DEATH_STATE_HPP
#define CHARACTER_DEATH_STATE_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "..\CharacterState.hpp"

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================
class corpse;

//=========================================================================
// CLASS
//=========================================================================
class character_death_state : public character_state
{
// Functions
public:

                 character_death_state  ( character& ourCharacter, states State ) ;

    virtual void OnEnter                ( void ) ;
    virtual void OnAdvance              ( f32 DeltaTime ) ;

    corpse*         CreateAndInitCorpse         ( void );
    xbool           SetupTriggerDeath           ( void );
    xbool           SetupAutoRagdollDeath       ( void );
    xbool           SetupCoverNodeDeath         ( void );
    xbool           SetupSimpleDeath            ( void );
    xbool           SetupAnimatedDeath          ( void );
    xbool           SetupExplosiveDeath         ( void );
    xbool           SetupGeneralRagdollDeath    ( void );

    // debugging information
    virtual const char* GetStateName ( void )    { return "CHARACTER_DEATH"; }    
    virtual const char* GetPhaseName ( s32 thePhase = PHASE_NONE ) { (void)thePhase; return "NONE"; }

// Data
public:

    u32     m_bDeathFromTrigger : 1;        // Did death come from a trigger?
} ;


//=========================================================================
// END
//=========================================================================
#endif