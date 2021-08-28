#ifndef _RESPONSE_LIST_HPP_
#define _RESPONSE_LIST_HPP_

#include "x_files\x_types.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"

struct response_list
{
    enum response_flags
    {
        RF_NULL                         =      0,
        RF_IGNORE_ATTACKS               = BIT( 1),  
        RF_IGNORE_SIGHT                 = BIT( 2),  
        RF_IGNORE_SOUND                 = BIT( 3),  
        RF_IGNORE_ALERTS                = BIT( 4),  
        RF_INVINCIBLE                   = BIT( 5)
    };

    response_list() { m_ResponseFlags = RF_IGNORE_ATTACKS|RF_IGNORE_SIGHT|RF_IGNORE_SOUND|RF_IGNORE_ALERTS; }    
    virtual void    OnEnumProp              ( prop_enum&    List);
    virtual xbool   OnProperty              ( prop_query&   I   );    
            void    AddFlags                ( u32 newFlags );
            void    RemoveFlags             ( u32 newFlags );
            xbool   HasFlags                ( u32 newFlags );
            void    SetFlags                ( u32 newFlags );
            u32     GetFlags                ( void );
protected:
    u32 m_ResponseFlags;
};

#endif