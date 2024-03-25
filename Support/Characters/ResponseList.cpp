#include "ResponseList.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
#include "MiscUtils\SimpleUtils.hpp"

void response_list::AddFlags( u32 newFlags )
{
    m_ResponseFlags = (m_ResponseFlags | newFlags);
}

void response_list::RemoveFlags( u32 newFlags )
{
    m_ResponseFlags = (m_ResponseFlags & ~newFlags);
}

xbool response_list::HasFlags( u32 newFlags )
{
    return ( (m_ResponseFlags & newFlags) == newFlags );
}

void response_list::SetFlags( u32 newFlags )
{
    m_ResponseFlags = newFlags;
}

u32 response_list::GetFlags( void )
{
    return ( m_ResponseFlags );
}

void response_list::OnEnumProp( prop_enum&  List )
{
    List.PropEnumHeader  ( "ResponseList",                   "List Various criteria for ignoring external stimuli", 0 );
    List.PropEnumBool    ( "ResponseList\\IgnoreAttacks",    "Ignore all attacks and stick to tasks.", 0 );
    List.PropEnumBool    ( "ResponseList\\IgnoreSight",      "Ignore all spotted enemies and stick to tasks.", 0 );
    List.PropEnumBool    ( "ResponseList\\IgnoreSound",      "Ignore all threatening sounds and stick to tasks.", 0 );
    List.PropEnumBool    ( "ResponseList\\IgnoreAlerts",     "Ignore all alerts and stick to tasks.", 0 );
    List.PropEnumBool    ( "ResponseList\\Invincible",       "Ignore all pain and damage and stick to tasks. (use with caution)", 0 );
}

xbool response_list::OnProperty( prop_query& I )
{
    if ( I.IsVar("ResponseList\\IgnoreAttacks") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(HasFlags(RF_IGNORE_ATTACKS) );
        }
        else
        {
            if (I.GetVarBool())
            {
                AddFlags(RF_IGNORE_ATTACKS);
            }
            else
            {
                RemoveFlags(RF_IGNORE_ATTACKS);
            }
        }
        return TRUE;
    }

    if ( I.IsVar("ResponseList\\IgnoreSight") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(HasFlags(RF_IGNORE_SIGHT) );
        }
        else
        {
            if (I.GetVarBool())
            {
                AddFlags(RF_IGNORE_SIGHT);
            }
            else
            {
                RemoveFlags(RF_IGNORE_SIGHT);
            }
        }
        return TRUE;
    }

    if ( I.IsVar("ResponseList\\IgnoreSound") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(HasFlags(RF_IGNORE_SOUND) );
        }
        else
        {
            if (I.GetVarBool())
            {
                AddFlags(RF_IGNORE_SOUND);
            }
            else
            {
                RemoveFlags(RF_IGNORE_SOUND);
            }
        }
        return TRUE;
    }

    if ( I.IsVar("ResponseList\\IgnoreAlerts") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(HasFlags(RF_IGNORE_ALERTS) );
        }
        else
        {
            if (I.GetVarBool())
            {
                AddFlags(RF_IGNORE_ALERTS);
            }
            else
            {
                RemoveFlags(RF_IGNORE_ALERTS);
            }
        }
        return TRUE;
    }
    
    if ( I.IsVar("ResponseList\\Invincible") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(HasFlags(RF_INVINCIBLE) );
        }
        else
        {
            if (I.GetVarBool())
            {
                AddFlags(RF_INVINCIBLE);
            }
            else
            {
                RemoveFlags(RF_INVINCIBLE);
            }
        }
        return TRUE;
    }
    return FALSE;
}
