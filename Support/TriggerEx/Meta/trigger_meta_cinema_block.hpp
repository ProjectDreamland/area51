///////////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_cinema_block.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _trigger_meta_cinema_block_
#define _trigger_meta_cinema_block_

//=========================================================================
// INCLUDES
//=========================================================================

#include "trigger_meta_base.hpp"

//=========================== ==============================================
// trigger_meta_delay
//=========================================================================

class trigger_meta_cinema_block : public trigger_meta_base
{
public:
    CREATE_RTTI( trigger_meta_cinema_block, trigger_meta_base, actions_ex_base )

                    trigger_meta_cinema_block           ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_META_CINEMA_BLOCK;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Set a META CINEMA BLOCK."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

protected:
    guid            m_CinemaGUID;
    char            m_BlockingMarker[256];
};


#endif
