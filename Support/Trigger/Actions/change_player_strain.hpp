///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_CHANGE_PLAYER_STRAIN
#define _TRIGGER_ACTIONS_CHANGE_PLAYER_STRAIN

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\Support\Objects\Player.hpp"

//=========================================================================
// CHANGE_PLAYER_STRAIN
//=========================================================================

class change_player_strain : public actions_base
{
public:
                    change_player_strain            ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Change the Players strain"; }
    virtual         const char*         GetTypeInfo ( void )    { return "Changes the players strain."; } 

    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_CHANGE_PLAYER_STRAIN;}

protected:
    
    player::player_mutation_strain m_StrainToSet;
};

#endif
    