#ifndef REGISTRATION_OF_INGAME_MANAGERS_HPP
#define	REGISTRATION_OF_INGAME_MANAGERS_HPP

#include "MiscUtils\Property.hpp"

//=========================================================================

class reg_game_mgrs
{
public:
    void            AddManager          ( const char* pManagerName, prop_interface* pInterface );
    prop_interface* GetManagerInterface ( const char* pManagerName );

    s32             GetCount            ( void );
    const char*     GetName             ( s32 Index );
    prop_interface* GetInterface        ( s32 Index );

protected:

    struct reg_game_mgr
    {
        void Create( const char* pManagerName, prop_interface* apInterface )
        {
            x_strcpy( ManagerName, pManagerName );
            pInterface   = apInterface;
        };

        char                           ManagerName[256];
        prop_interface*                pInterface;
    };

protected:

    xarray<reg_game_mgr>    m_lstMgrs;
};

//=========================================================================

extern reg_game_mgrs             g_RegGameMgrs;

#endif // !defined(REGISTRATION_OF_INGAME_MANAGERS_HPP)
