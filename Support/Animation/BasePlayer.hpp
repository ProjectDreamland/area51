//=========================================================================
//
//  BASE_PLAYER.HPP
//
//=========================================================================
#ifndef BASE_PLAYER_HPP
#define BASE_PLAYER_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "AnimData.hpp"


//=========================================================================
// CLASS BASE_PLAYER
//=========================================================================

class base_player
{
//-------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------
public:
    enum mix_buffer
    {
        MIX_BUFFER_PLAYER,          // Main animation player buffer
        MIX_BUFFER_CONTROLLER,      // Controller buffer
        MIX_BUFFER_TEMP,            // General mixer

        MIX_BUFFER_COUNT            // Total
    };

//-------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------
public:
                                    base_player     ( void ){}
        virtual                     ~base_player    ( void ){}

        // Event functions        
        virtual s32                 GetNEvents          ( void ) = 0;
        virtual xbool               IsEventActive       ( s32 iEvent ) = 0;
        virtual const anim_event&   GetEvent            ( s32 iEvent ) = 0;                                
        virtual vector3             GetEventPosition    ( s32 iEvent ) = 0;                 // World position of event.
        virtual radian3             GetEventRotation    ( s32 iEvent ) = 0;                 // World rotation of event.
        virtual vector3             GetEventPosition    ( const anim_event& Event ) = 0;    // World position of event.
        virtual radian3             GetEventRotation    ( const anim_event& Event ) = 0;    // World rotation of event.
        virtual const char*         GetAnimName         ( void ) { return "ANIM_UNKNOWN"; }
        
        // Mix buffer functions
static          anim_key*           GetMixBuffer        ( mix_buffer MixBuffer );           // Returns mix buffer address
};


//=========================================================================
#endif // END BASE_PLAYER_HPP
//=========================================================================
