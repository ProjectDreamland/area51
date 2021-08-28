#ifndef editor_icons_HPP
#define editor_icons_HPP

#include "x_files.hpp"

//=========================================================================
// begin editor_icons draw Routines
//=========================================================================

enum editor_icon 
{
    EDITOR_ICON_ANCHOR,
    EDITOR_ICON_NAV_NODE,
    EDITOR_ICON_PORTAL,
    EDITOR_ICON_SPEAKER,
    EDITOR_ICON_LIGHT,
    EDITOR_ICON_LIGHT_CHARACTER,
    EDITOR_ICON_LIGHT_DYNAMIC,
    EDITOR_ICON_NOTE,
    EDITOR_ICON_PARTICLE_EMITTER,
    EDITOR_ICON_TRIGGER,
    EDITOR_ICON_COVER_NODE,
    EDITOR_ICON_PROJECTOR,
    EDITOR_ICON_CHARACTER_TASK,  
    EDITOR_ICON_LOOP,  
    EDITOR_ICON_GEAR,    
    EDITOR_ICON_MARKER,          
    EDITOR_ICON_TRIGGER_SIMPLE,          
    EDITOR_ICON_TRIGGER_VIEWABLE,          
    EDITOR_ICON_TRIGGER_SPATIAL,     
    EDITOR_ICON_DAMAGE,
    EDITOR_ICON_ALIEN_SHIELD,
    EDITOR_ICON_BLUEPRINT_BAG,
    EDITOR_ICON_BOT_SPAWN_POINT,
    EDITOR_ICON_DOOR_CLOSED,
    EDITOR_ICON_COUPLER,
    EDITOR_ICON_CTF_FLAG,
    EDITOR_ICON_FOCUS_OBJECT,
    EDITOR_ICON_GROUP,
    EDITOR_ICON_JUMP_PAD,
    EDITOR_ICON_MP_SETTINGS,
    EDITOR_ICON_DOOR_OPEN,
    EDITOR_ICON_PADLOCK,
    EDITOR_ICON_PLAYER_OBJECT,
    EDITOR_ICON_SPAWN_POINT,
    EDITOR_ICON_TWO_WAY_ARROW,
    EDITOR_ICON_LIGHT_VOLUMETRIC,
    EDITOR_ICON_AI_NAV_CON,
    EDITOR_ICON_CAMERA,
    EDITOR_ICON_CONTROLLER,
    EDITOR_ICON_INPUT_SETTINGS,
    EDITOR_ICON_LEVEL_SETTINGS,
    EDITOR_ICON_MUSIC_LOGIC,
    EDITOR_ICON_PATH,
    EDITOR_ICON_PIP,
    EDITOR_ICON_SPAWNER,
    EDITOR_ICON_TRACKER,
    EDITOR_ICON_SPHERE,
    EDITOR_ICON_HUD_OBJECT,
    EDITOR_ICON_CINEMA_OBJECT,
};

void EditorIcon_Draw( editor_icon Icon, const matrix4& L2W,  xbool bSelected, xcolor TintColor );

//=========================================================================
// end editor_icons
//=========================================================================
#endif // editor_icons_HPP

