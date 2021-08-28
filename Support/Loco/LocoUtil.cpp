//==============================================================================
//
//  LocoUtil.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "Loco.hpp"
#include "Dictionary\global_dictionary.hpp"


//==============================================================================
// FUNCTIONS
//==============================================================================

void LocoUtil_OnEnumPropAnimFlags( prop_enum& rPropList, 
                                          u32 PropAnimFlags,
                                          u32 CurrAnimFlags )
{
    rPropList.PropEnumExternal( "AnimGroupName", "Resource\0animexternal", "Select the animation group and animation.", PROP_TYPE_MUST_ENUM | PROP_TYPE_EXTERNAL );
    
    rPropList.PropEnumString  ( "AnimName",   "Name of the animation to play.", PROP_TYPE_MUST_ENUM);

    if (PropAnimFlags & loco::ANIM_FLAG_PLAY_TYPE_ALL)
        rPropList.PropEnumEnum    ( "AnimPlayType",  "DEFAULT\0CYCLIC\0TIMED\0", "Specifies playback control.\nDEFAULT = Play 1 cycle.\nCYCLIC = Play for specfic # of animation cycles.\nTIMED = Play for specific # of seconds.", PROP_TYPE_MUST_ENUM );

    if (CurrAnimFlags & loco::ANIM_FLAG_PLAY_TYPE_CYCLIC)
    {
        // Cycles
        rPropList.PropEnumFloat("AnimPlayTime", "How many cycles of the animation to play. (-1 for inifinite).", 0 );
    }

    if (CurrAnimFlags & loco::ANIM_FLAG_PLAY_TYPE_TIMED)
    {
        // Seconds
        rPropList.PropEnumFloat("AnimPlayTime","How many seconds to play the animation for. (-1 for infinite).", 0 );
    }

    if (PropAnimFlags & loco::ANIM_FLAG_END_STATE_ALL)
        rPropList.PropEnumEnum    ( "AnimEndState",  "DEFAULT\0RESUME\0HOLD\0",  "Specifies the pose, upon completion of playback.\nDEFAULT = Goto previous animation (usually idle).\nRESUME = Goto previous animation (usually idle).\nHOLD = Stay on frame within this animation (the last frame if not a looping anim).", 0 );
    
    if (PropAnimFlags & loco::ANIM_FLAG_MASK_TYPE_ALL)
        rPropList.PropEnumEnum    ( "AnimMaskType",  "DEFAULT\0FULL_BODY\0UPPER_BODY\0FACE\0DYNAMIC\0",  "Specifies what bones the animation will be applied to.\nDEFAULT = Full body.\nFULL_BODY = Full body.\nUPPER_BODY = Waist and above.\nFACE = Face only.", 0 );

    if (PropAnimFlags & loco::ANIM_FLAG_INTERRUPT_BLEND)
        rPropList.PropEnumBool("AnimInterruptBlend", "If SET, the animation will play immediately regardless of current blending (may cause a pop). If FALSE, play will begin once blending has ended (smooth).", 0 ) ;

    if (PropAnimFlags & loco::ANIM_FLAG_TURN_OFF_AIMER)
        rPropList.PropEnumBool("AnimTurnOffAimer", "If SET, the aimer will be turned off during playback.", 0 ) ;

    if (PropAnimFlags & loco::ANIM_FLAG_PRIORITY_LOW)
        rPropList.PropEnumBool("AnimLowPriority", "Flags animation as low priority", 0 ) ;

    if (PropAnimFlags & loco::ANIM_FLAG_PRIORITY_MEDIUM)
        rPropList.PropEnumBool("AnimMediumPriority", "Flags animation as medium priority", 0 ) ;

    if (PropAnimFlags & loco::ANIM_FLAG_RESTART_IF_SAME_ANIM)
        rPropList.PropEnumBool("AnimRestartIfSameAnim", "If SET, and the same animation is currently playing, it is restarted. (Default is to not interrupt)", 0 ) ;

    if (PropAnimFlags & loco::ANIM_FLAG_TURN_OFF_RANDOM_SELECTION)
        rPropList.PropEnumBool("AnimTurnOffRandomSelection", "If SET, the same animation (alphabetical first) is always played from animations with the same name", 0 ) ;
}

//==============================================================================

xbool LocoUtil_OnPropertyAnimFlags( prop_query& rPropQuery, 
                                           s32& AnimGroupName, 
                                           s32& AnimName, 
                                           u32& AnimFlags, 
                                           f32& PlayTime )
{
    // AnimGroup, AnimName
    if(     ( rPropQuery.IsVar( "AnimGroupName" ) )
        ||  ( rPropQuery.IsVar( "AnimSelect"    ) )     // Backwards compatible for LocoUtil
        ||  ( rPropQuery.IsVar( "Anim Package"  ) ) )   // Backwards compatible for cinema_object
    {
        if( rPropQuery.IsRead() )
        {
            if ( AnimGroupName >= 0 )
                rPropQuery.SetVarExternal( g_StringMgr.GetString(AnimGroupName), 256 );
            else
                rPropQuery.SetVarExternal("", 256);
        }
        else
        {
            // Get the FileName
            xstring String = rPropQuery.GetVarExternal();
            if( !String.IsEmpty() )
            {
                s32 PkgIndex = String.Find( '\\', 0 );
                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    Pkg += "\0\0";
 
                    if( Pkg != xstring( "<null>" ) )
                    {
                        AnimName = g_StringMgr.Add( String.Right( String.GetLength() - PkgIndex - 1) );
                        AnimGroupName = g_StringMgr.Add( Pkg );
                    }
                    else
                    {
                        AnimName      = -1;
                        AnimGroupName = -1;
                    }
                }
                else
                {
                    if( String != xstring( "<null>" ) )
                    {
                        AnimGroupName = g_StringMgr.Add( String );
                    }
                    else
                    {
                        AnimName      = -1;
                        AnimGroupName = -1;
                    }
                }
            }
            else
            {
                AnimName = -1;
                AnimGroupName = -1;
            }
        }
        return TRUE;
    }

    // AnimName
    if(         ( rPropQuery.IsVar("AnimName") )
            ||  ( rPropQuery.IsVar("Anim Name") ) )    // Backwards compatible for cinema_object
    {
        if( rPropQuery.IsRead() )
        {
            if (AnimName >= 0)
                rPropQuery.SetVarString( g_StringMgr.GetString(AnimName), 256 );
            else
                rPropQuery.SetVarString( "", 256);
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarString()) > 0)
            {
                AnimName = g_StringMgr.Add( rPropQuery.GetVarString() );
            }
            else
            {
                AnimName = -1;
                AnimGroupName = -1;
            }
        }
        return TRUE;
    }

    // AnimPlayType: loco::ANIM_FLAG_PLAY_TYPE_???
    if( rPropQuery.IsVar( "AnimPlayType" ) )
    {
        if( rPropQuery.IsRead () )
        {
            if (AnimFlags & loco::ANIM_FLAG_PLAY_TYPE_CYCLIC)
            {
                rPropQuery.SetVarEnum( "CYCLIC" ) ;
            }
            else if (AnimFlags & loco::ANIM_FLAG_PLAY_TYPE_TIMED)
            {
                rPropQuery.SetVarEnum( "TIMED" ) ;
            }
            else
            {
                // Default
                rPropQuery.SetVarEnum( "DEFAULT" ) ;
            }
        }
        else
        {
            AnimFlags &= ~loco::ANIM_FLAG_PLAY_TYPE_ALL ;

            if (x_stricmp( "CYCLIC", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_PLAY_TYPE_CYCLIC ;
            }
            else if (x_stricmp( "TIMED", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_PLAY_TYPE_TIMED ;
            }
        }
        return TRUE;
    }    

    // AnimEndState: loco::ANIM_FLAG_END_STATE_???
    if( rPropQuery.IsVar( "AnimEndState" ) )
    {
        if( rPropQuery.IsRead () )
        {
            if (AnimFlags & loco::ANIM_FLAG_END_STATE_RESUME)
            {
                rPropQuery.SetVarEnum( "RESUME" ) ;
            }
            else if (AnimFlags & loco::ANIM_FLAG_END_STATE_HOLD)
            {
                rPropQuery.SetVarEnum( "HOLD" ) ;
            }
            else
            {
                // Default
                rPropQuery.SetVarEnum( "DEFAULT" ) ;
            }
        }
        else
        {
            AnimFlags &= ~loco::ANIM_FLAG_END_STATE_ALL ;
            
            if (x_stricmp( "RESUME", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_END_STATE_RESUME ;
            }
            else if (x_stricmp( "HOLD", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_END_STATE_HOLD ;
            }
        }
        return TRUE;
    }    

    // AnimMaskType: loco::ANIM_FLAG_MASK_TYPE_???
    if( rPropQuery.IsVar( "AnimMaskType" ) )
    {
        if( rPropQuery.IsRead () )
        {
            if (AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY)
            {
                rPropQuery.SetVarEnum( "FULL_BODY" ) ;
            }
            else if (AnimFlags & loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY)
            {
                rPropQuery.SetVarEnum( "UPPER_BODY" ) ;
            }
            else if (AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FACE)
            {
                rPropQuery.SetVarEnum( "FACE" ) ;
            }
            else if (AnimFlags & loco::ANIM_FLAG_MASK_TYPE_DYNAMIC)
            {
                rPropQuery.SetVarEnum( "DYNAMIC" ) ;
            }
            else
            {
                // Default
                rPropQuery.SetVarEnum( "DEFAULT" ) ;
            }
        }
        else
        {
            AnimFlags &= ~loco::ANIM_FLAG_MASK_TYPE_ALL;
            
            if (x_stricmp( "FULL_BODY", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_MASK_TYPE_FULL_BODY;
            }
            else if (x_stricmp( "UPPER_BODY", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY;
            }
            else if (x_stricmp( "FACE", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_MASK_TYPE_FACE;
            }
            else if (x_stricmp( "DYNAMIC", rPropQuery.GetVarEnum()) == 0)
            {
                AnimFlags |= loco::ANIM_FLAG_MASK_TYPE_DYNAMIC;
            }
        }
        return TRUE;
    }    

    // AnimPlayTime
    if (rPropQuery.VarFloat("AnimPlayTime", PlayTime))
        return TRUE;

    // AnimInterruptBlend
    if( rPropQuery.IsVar("AnimInterruptBlend") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_INTERRUPT_BLEND) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_INTERRUPT_BLEND ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_INTERRUPT_BLEND ;
        }
        return TRUE ;
    }

    // AnimTurnOffAimer
    if( rPropQuery.IsVar("AnimTurnOffAimer") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_TURN_OFF_AIMER) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_TURN_OFF_AIMER ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_TURN_OFF_AIMER ;
        }
        return TRUE ;
    }

    // AnimPriorityLow
    if( rPropQuery.IsVar("AnimPriorityLow") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_PRIORITY_LOW) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_PRIORITY_LOW ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_PRIORITY_LOW ;
        }
        return TRUE ;
    }

    // AnimPriorityMedium
    if( rPropQuery.IsVar("AnimPriorityMedium") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_PRIORITY_MEDIUM) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_PRIORITY_MEDIUM ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_PRIORITY_MEDIUM ;
        }
        return TRUE ;
    }

    // AnimPriorityHigh
    if( rPropQuery.IsVar("AnimPriorityHigh") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_PRIORITY_HIGH) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_PRIORITY_HIGH ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_PRIORITY_HIGH ;
        }
        return TRUE ;
    }

    // AnimRestartIfSameAnim
    if( rPropQuery.IsVar("AnimRestartIfSameAnim") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_RESTART_IF_SAME_ANIM) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_RESTART_IF_SAME_ANIM ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_RESTART_IF_SAME_ANIM ;
        }
        return TRUE ;
    }

    // AnimTurnOffRandomSelection
    if( rPropQuery.IsVar("AnimTurnOffRandomSelection") )
    {
        if( rPropQuery.IsRead () )
        {
            rPropQuery.SetVarBool((AnimFlags & loco::ANIM_FLAG_TURN_OFF_RANDOM_SELECTION) != 0) ;
        }
        else
        {
            if (rPropQuery.GetVarBool())
                AnimFlags |= loco::ANIM_FLAG_TURN_OFF_RANDOM_SELECTION ;
            else
                AnimFlags &= ~loco::ANIM_FLAG_TURN_OFF_RANDOM_SELECTION ;
        }
        return TRUE ;
    }

    return FALSE;
}

//==============================================================================

xbool LocoUtil_OnPropertyAnimFlags( prop_query& rPropQuery, 
                                           s32& AnimGroupName, 
                                           s32& AnimName, 
                                           u32& AnimFlags )
{
    f32 PlayTime ;
    return LocoUtil_OnPropertyAnimFlags(rPropQuery, AnimGroupName, AnimName, AnimFlags, PlayTime) ;
}

//==============================================================================

// Same as above function, but you don't need a "PlayTime" parameter and it sets the anim group name
xbool LocoUtil_OnPropertyAnimFlags(         prop_query& rPropQuery, 
                                                   s32& AnimGroupName, 
                                                   s32& AnimName, 
                                                   u32& AnimFlags,
                                    anim_group::handle& hAnimGroup )
{
    f32 PlayTime ;
    xbool bFound = LocoUtil_OnPropertyAnimFlags(rPropQuery, AnimGroupName, AnimName, AnimFlags, PlayTime) ;
    if( bFound )
    {
        // Reading from UI/File?
        if( rPropQuery.IsRead() == FALSE )
        {
            // AnimGroup property?
            if(     ( rPropQuery.IsVar( "AnimGroupName" ) )
                ||  ( rPropQuery.IsVar( "AnimSelect"    ) )     // Backwards compatible for LocoUtil
                ||  ( rPropQuery.IsVar( "Anim Package"  ) ) )   // Backwards compatible for cinema_object
            {
                // Name string specified?
                if ( AnimGroupName >= 0 )
                {
                    // Get anim group name
                    const char* pAnimGroup = g_StringMgr.GetString( AnimGroupName );
                    ASSERT( pAnimGroup );
                    
                    // Set handle
                    hAnimGroup.SetName( pAnimGroup );                
                }
            }
        }
    }
            
    return bFound;    
}

//==============================================================================


