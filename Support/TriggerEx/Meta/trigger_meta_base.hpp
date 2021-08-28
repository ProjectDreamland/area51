///////////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_base.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _trigger_meta_base_
#define _trigger_meta_base_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================== ==============================================
// trigger_meta_base
//=========================================================================

class trigger_meta_base : public actions_ex_base
{
public:
    CREATE_RTTI( trigger_meta_base, actions_ex_base, actions_ex_base )

                    trigger_meta_base                   ( guid ParentGuid );
};


#endif
