//==============================================================================
//
//  Circuit.hpp
//
//==============================================================================

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "MP_Settings.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class circuit
{

public:
                                    circuit         ( void );
                                   ~circuit         ( void );

                    void            OnEnumProp      ( prop_enum&  List );
                    xbool           OnProperty      ( prop_query& List );

                    u32             GetTeamBits     ( void ) const;
                    s32             GetCircuit      ( void ) const;
                    void            SetCircuit      ( s32 Circuit );

#ifdef X_EDITOR
                    xcolor          GetColor        ( void );
                    void            SpecialRender   ( vector3& Position );
#endif

protected:      
    s32             m_CircuitInit[2];   // 4 bits per 16 game types
    s32             m_Circuit;

    // NOTE: I've gone ahead and simply hard coded m_CircuitInit to have 2
    // elements.  This is log2(MAX_CIRCUITS) * MAX_CIRCUIT_GAMES / bits(s32).
    // If this code were to have a longer expected life cycle, I'd be a bit more
    // rigorous about this...  But its not, so I'm not.
};

//==============================================================================
#endif
//==============================================================================
