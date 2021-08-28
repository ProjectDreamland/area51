///////////////////////////////////////////////////////////////////////////////
//
//  condition_player_button_state.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_player_button_state_
#define _condition_player_button_state_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\TriggerEx_Conditionals.hpp"
#include "InputMgr\GamePad.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_player_button_state : public conditional_ex_base
{
public:
                    condition_player_button_state                ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_BUTTON_STATE;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check the state of a button"; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

protected:

    enum codes
    { 
        INVALID_BUTTON_CODES = -1,
        BUTTON_CODE_PRESSED,
        BUTTON_CODE_NOT_PRESSED,
        BUTTON_CODES_END
    };

    s32                                 m_Code;         // Used to determine what type of conditional check to perform
    ingame_pad::logical_id             m_ButtonID;    
};

#endif
