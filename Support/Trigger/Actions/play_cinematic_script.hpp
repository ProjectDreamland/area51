///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_PLAY_CINEMATIC_SCRIPT
#define _TRIGGER_ACTIONS_PLAY_CINEMATIC_SCRIPT

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================== ==============================================
// play_cinematic_script
//=========================================================================

class play_cinematic_script : public actions_base
{
public:
                    play_cinematic_script                 ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Play Cinematic Script"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Plays a cincematic script."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender        ( void );
    
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_PLAY_CINEMATIC_SCRIPT;}

protected:


};


#endif
