#include "ManagerRegistration.hpp"


//=========================================================================
// GAMER MANAGERS REGISTRATION
//=========================================================================

reg_game_mgrs     g_RegGameMgrs;

//=========================================================================

void reg_game_mgrs::AddManager( const char* pManagerName, prop_interface* pInterface )
{
    reg_game_mgr Mgr;
    Mgr.Create(pManagerName, pInterface);
    m_lstMgrs.Append(Mgr);
}

//=========================================================================

prop_interface* reg_game_mgrs::GetManagerInterface( const char* pManagerName )
{
    ASSERT(pManagerName);
    for (s32 i = 0; i < m_lstMgrs.GetCount(); i++)
    {            
        reg_game_mgr& Mgr = m_lstMgrs.GetAt(i);
        if( x_stricmp( pManagerName, Mgr.ManagerName ) == 0 )
            return Mgr.pInterface;
    }
    return NULL;
}

//=========================================================================

s32 reg_game_mgrs::GetCount()
{
    return m_lstMgrs.GetCount();
}

//=========================================================================

const char* reg_game_mgrs::GetName(s32 Index)
{
    ASSERT(Index < m_lstMgrs.GetCount());

    reg_game_mgr& Mgr = m_lstMgrs.GetAt(Index);
    return Mgr.ManagerName;
}

//=========================================================================

prop_interface* reg_game_mgrs::GetInterface(s32 Index)
{
    ASSERT(Index < m_lstMgrs.GetCount());

    reg_game_mgr& Mgr = m_lstMgrs.GetAt(Index);
    return Mgr.pInterface;
}
