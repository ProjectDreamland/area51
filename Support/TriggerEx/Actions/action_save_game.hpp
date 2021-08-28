///////////////////////////////////////////////////////////////////////////////
//
//  action_save_game.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_save_game_
#define _action_save_game_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================== ==============================================
// action_save_game
//=========================================================================

class action_save_game : public actions_ex_base
{
public:
                    action_save_game              ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_SAVE_GAME;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Save the Game."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL
};


#endif
