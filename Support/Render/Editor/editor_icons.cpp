#include "Entropy.hpp"
#include "editor_icons.hpp"

#ifdef X_EDITOR
//=========================================================================
// editor_icons setup
//=========================================================================

struct vertex 
{
    f32 X, Y, Z;
    u8  R, G, B, A;
    f32 U, V;
};

//=========================================================================
//
//=========================================================================
// editor_icons matx data
//=========================================================================
//
//=========================================================================

#include "icons/icon_ai_nav_con.hpp"
#include "icons/icon_alien_shield.hpp"
#include "icons/icon_arrow.hpp"
#include "icons/icon_bot_spawn_point.hpp"
#include "icons/icon_bp_anchor.hpp"
#include "icons/icon_bp_bag.hpp"
#include "icons/icon_camera.hpp"
#include "icons/icon_cinema_obj.hpp"
#include "icons/icon_character_light.hpp"
#include "icons/icon_closed_door.hpp"
#include "icons/icon_controller.hpp"
#include "icons/icon_coupler.hpp"
#include "icons/icon_cover.hpp"
#include "icons/icon_ctf_flag.hpp"
#include "icons/icon_damage.hpp"
#include "icons/icon_dynamic_light.hpp"
#include "icons/icon_focus_obj.hpp"
#include "icons/icon_gear.hpp"
#include "icons/icon_group.hpp"
#include "icons/icon_ai_nav_node.hpp"
#include "icons/icon_input_settings.hpp"
#include "icons/icon_jump_pad.hpp"
#include "icons/icon_level_settings.hpp"
#include "icons/icon_light.hpp"
#include "icons/icon_loop.hpp"
#include "icons/icon_marker.hpp"
#include "icons/icon_mp_settings.hpp"
#include "icons/icon_music_logic.hpp"
#include "icons/icon_note.hpp"
#include "icons/icon_open_door.hpp"
#include "icons/icon_padlock.hpp"
#include "icons/icon_particle_emitter.hpp"
#include "icons/icon_path.hpp"
#include "icons/icon_pip.hpp"
#include "icons/icon_player_obj.hpp"
#include "icons/icon_portal.hpp"
#include "icons/icon_projector.hpp"
#include "icons/icon_simple_trigger.hpp"
#include "icons/icon_sound.hpp"
#include "icons/icon_spacial_trigger.hpp"
#include "icons/icon_spawner.hpp"
#include "icons/icon_spawn_point.hpp"
#include "icons/icon_task_obj.hpp"
#include "icons/icon_tracker.hpp"
#include "icons/icon_trigger.hpp"
#include "icons/icon_viewable_trigger.hpp"
#include "icons/icon_volumetric_light.hpp"
#include "icons/icon_sphere.hpp"
#include "icons/icon_hud_obj.hpp"


//=========================================================================
//
//=========================================================================
// editor_icons draw routines
//=========================================================================
//
//=========================================================================

//=========================================================================

void draw_icon( s32 nFacets, s32 nVertices, vertex* pVertexArray, s16* pIndexArray )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
    g_pd3dDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST,
                                          0,
                                          nVertices,
                                          nFacets,
                                          pIndexArray,
                                          D3DFMT_INDEX16,
                                          pVertexArray,
                                          sizeof(vertex) );
}

//=========================================================================

void EditorIcon_Draw( editor_icon Icon, const matrix4& L2W, xbool bSelected, xcolor TintColor )
{
    if( !g_pd3dDevice )
        return;

    xcolor Color;

    // If we have scales in the matrix remove them
    f32 s = L2W(0,0) * L2W(0,0) + L2W(1,0) * L2W(1,0) + L2W(2,0) * L2W(2,0);
    if( s >= (1.01f*1.01f) || s <= ( 0.98f*0.98f))
    {
        matrix4 M(vector3(1,1,1), L2W.GetRotation(), L2W.GetTranslation());
        g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&M );
    }
    else
    {
        g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );
    }

    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, TintColor );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,    D3DTOP_MODULATE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,  D3DTA_TFACTOR     );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,  D3DTA_DIFFUSE     );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,  D3DTA_DIFFUSE     );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,    D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,    D3DTOP_DISABLE    );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );

    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     D3DZB_TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE,         D3DFILL_SOLID );

    // The draw macros below are generated as part of the icon definitions. 
    // these are stored in header files in the render/icons folder.
    // They simplify the calls to draw_icon().
    switch(Icon)
    {
    case EDITOR_ICON_ANCHOR:
        DRAW_ICON_BP_ANCHOR();
        break;
    case EDITOR_ICON_TRIGGER:
        DRAW_ICON_TRIGGER();
        break;
    case EDITOR_ICON_SPEAKER:
        DRAW_ICON_SOUND();
        break;
    case EDITOR_ICON_PORTAL:
        DRAW_ICON_PORTAL();
        break;
    case EDITOR_ICON_NAV_NODE:
        DRAW_ICON_AI_NAV_NODE();
        break;
    case EDITOR_ICON_LIGHT:
        DRAW_ICON_LIGHT();
        break;
    case EDITOR_ICON_LIGHT_CHARACTER:
        DRAW_ICON_CHARACTER_LIGHT();
        break;
    case EDITOR_ICON_LIGHT_DYNAMIC:
        DRAW_ICON_DYNAMIC_LIGHT();
        break;
    case EDITOR_ICON_PARTICLE_EMITTER:
        DRAW_ICON_PARTICLE_EMITTER();
        break;
    case EDITOR_ICON_NOTE:
        DRAW_ICON_NOTE();
        break;
    case EDITOR_ICON_COVER_NODE:
        DRAW_ICON_COVER();
        break;
    case EDITOR_ICON_PROJECTOR:
        DRAW_ICON_PROJECTOR();
        break;
    case EDITOR_ICON_CHARACTER_TASK:
        DRAW_ICON_TASK_OBJ();
        break;
    case EDITOR_ICON_LOOP:
        DRAW_ICON_LOOP();
        break;      
    case EDITOR_ICON_GEAR:
        DRAW_ICON_GEAR();
        break;      
    case EDITOR_ICON_MARKER:
        DRAW_ICON_MARKER();
        break;     
    case EDITOR_ICON_TRIGGER_SIMPLE:
        DRAW_ICON_SIMPLE_TRIGGER();
        break;      
    case EDITOR_ICON_TRIGGER_VIEWABLE:
        DRAW_ICON_VIEWABLE_TRIGGER();
        break;      
    case EDITOR_ICON_TRIGGER_SPATIAL:
        DRAW_ICON_SPACIAL_TRIGGER();
        break;             
    case EDITOR_ICON_DAMAGE:
        DRAW_ICON_DAMAGE();
        break;
    case EDITOR_ICON_ALIEN_SHIELD:
        DRAW_ICON_ALIEN_SHIELD();
        break;
    case EDITOR_ICON_BLUEPRINT_BAG:
        DRAW_ICON_BP_BAG();
        break;
    case EDITOR_ICON_BOT_SPAWN_POINT:
        DRAW_ICON_BOT_SPAWN_POINT();
        break;
    case EDITOR_ICON_DOOR_CLOSED:
        DRAW_ICON_CLOSED_DOOR();
        break;
    case EDITOR_ICON_COUPLER:
        DRAW_ICON_COUPLER();
        break;
    case EDITOR_ICON_CTF_FLAG:
        DRAW_ICON_CTF_FLAG();
        break;
    case EDITOR_ICON_FOCUS_OBJECT:
        DRAW_ICON_FOCUS_OBJ();
        break;
    case EDITOR_ICON_GROUP:
        DRAW_ICON_GROUP();
        break;
    case EDITOR_ICON_JUMP_PAD:
        DRAW_ICON_JUMP_PAD();
        break;
    case EDITOR_ICON_MP_SETTINGS:
        DRAW_ICON_MP_SETTINGS();
        break;
    case EDITOR_ICON_DOOR_OPEN:
        DRAW_ICON_OPEN_DOOR();
        break;
    case EDITOR_ICON_PADLOCK:
        DRAW_ICON_PADLOCK();
        break;
    case EDITOR_ICON_PLAYER_OBJECT:
        DRAW_ICON_PLAYER_OBJ();
        break;
    case EDITOR_ICON_SPAWN_POINT:
        DRAW_ICON_SPAWN_POINT();
        break;
    case EDITOR_ICON_TWO_WAY_ARROW:
        DRAW_ICON_ARROW();
        break;
    case EDITOR_ICON_LIGHT_VOLUMETRIC:
        DRAW_ICON_VOLUMETRIC_LIGHT();
        break;
    case EDITOR_ICON_AI_NAV_CON:
        DRAW_ICON_AI_NAV_CON();
        break;
    case EDITOR_ICON_CAMERA:
        DRAW_ICON_CAMERA();
        break;
    case EDITOR_ICON_CONTROLLER:
        DRAW_ICON_CONTROLLER();
        break;
    case EDITOR_ICON_INPUT_SETTINGS:
        DRAW_ICON_INPUT_SETTINGS();
        break;
    case EDITOR_ICON_LEVEL_SETTINGS:
        DRAW_ICON_LEVEL_SETTINGS();
        break;
    case EDITOR_ICON_MUSIC_LOGIC:
        DRAW_ICON_MUSIC_LOGIC();
        break;
    case EDITOR_ICON_PATH:
        DRAW_ICON_PATH();
        break;
    case EDITOR_ICON_PIP:
        DRAW_ICON_PIP();
        break;
    case EDITOR_ICON_SPAWNER:
        DRAW_ICON_SPAWNER();
        break;
    case EDITOR_ICON_TRACKER:
        DRAW_ICON_TRACKER();
        break;
    case EDITOR_ICON_SPHERE:
        DRAW_ICON_SPHERE();
        break;
    case EDITOR_ICON_HUD_OBJECT:
        DRAW_ICON_HUD_OBJ();
        break;
    case EDITOR_ICON_CINEMA_OBJECT:
        DRAW_ICON_CINEMA_OBJ();
        break;
    }

    // Render the sphere
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,    D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,  D3DTA_TFACTOR     );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,    D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,  D3DTA_TFACTOR     );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,    D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,    D3DTOP_DISABLE    );

    g_pd3dDevice->SetRenderState( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );

    if( bSelected )
    {
        g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor( 255,0,0, 128) );
        DRAW_ICON_SPHERE();
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor( 255,255,255, 64) );
        DRAW_ICON_SPHERE();
    }

    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE );
}

#else

//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================

void EditorIcon_Draw( editor_icon Icon, const matrix4& L2W, xbool bSelected, xcolor TintColor )
{
    (void)Icon;
    (void)L2W;
    (void)bSelected;
    (void)TintColor;
    ASSERT(0);
    return;
}

#endif // X_EDITOR
