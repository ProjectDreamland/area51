#ifndef	__CORPSE_PAIN_HPP__
#define __CORPSE_PAIN_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_math.hpp"

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class corpse;
class pain;
class bitstream;


//=========================================================================
// CLASS
//=========================================================================

// Corpse pain class for setting up, applying, and sending across the net
class corpse_pain
{

// Defines
public:
    enum type
    {
        TYPE_SUICIDE,       // Pain was caused by suicide
        TYPE_MELEE,         // Pain was caused by melee pain
        TYPE_SPLASH,        // Pain was caused by splash damage
        TYPE_RIGID_BODY,    // Pain was caused by rigid body pain
        TYPE_BLAST,         // Pain was caused by blast
        TYPE_COUNT
    };


// Functions
public:
            // Constructor
                        corpse_pain     ();

            // Clears defaults to a suicide
            void        Clear           ( void );            

            // Read/Write to bitstream
            void        Read            ( const bitstream& BS );
            void        Write           (       bitstream& BS );

            // Sets up values from pain hitting a corpse
            void        Setup           ( const pain& Pain, corpse& Corpse );

            // Applies pain to corpse
            void        Apply           ( corpse& Corpse );
    
    // Query functions
            guid        GetOriginGuid   ( void ) const;
    const   vector3&    GetDirection    ( void ) const;
            xbool       IsDirectHit     ( void ) const;
    
// Data
private:
            u8          m_Type;             // Type of pain
            s8          m_iRigidBody;       // Index of rigid body that was hit (or -1 if none)
            u8          m_bOnDeath   : 1;   // TRUE if came from killing an actor
            u8          m_bDirectHit : 1;   // TRUE if direct hit
            s16         m_OriginNetSlot;    // Origin net slot
            vector3     m_Position;         // Position of pain force (relative to victim)
            vector3     m_Direction;        // Direction of pain force
            f32         m_Force;            // Force amount
            f32         m_ForceFarDist;     // Fade off 
};

//=========================================================================
// END
//=========================================================================
#endif
