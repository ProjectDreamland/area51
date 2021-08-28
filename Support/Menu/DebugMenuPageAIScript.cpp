//==============================================================================
//  DebugMenuPageAIScript.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu AI page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "StateMgr\StateMgr.hpp"
#include "characters\character.hpp"
#include "TriggerEx\TriggerEx_Manager.hpp"
#include "Configuration/GameConfig.hpp"
#include "Navigation/Nav_Map.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Ui\ui_font.hpp"
#include "Objects\DamageField.hpp"
#include "TriggerEx\TriggerEx_Object.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

debug_menu_page_aiscript::debug_menu_page_aiscript( ) : debug_menu_page()
{
    m_bShowActiveAIs        = FALSE;
    m_bToggleWallRender     = FALSE;
    m_bToggleTriggerRender  = FALSE;
    m_bToggleDamageRender   = FALSE;

#if !defined(X_RETAIL) || defined(X_QA)
    m_pTitle = "AI and Scripting";

    m_pItemTriggerDump          = AddItemButton   ( "Dump Triggers to File" );
                                  AddItemSeperator();
    #if !defined(X_RETAIL) || defined(X_QA)
                                  AddItemBool     ( "Debug characters"    , character::s_bDebugInGame );
    #endif
                                  AddItemBool     ( "Debug game logic"    , g_GameLogicDebug );
                                  AddItemBool     ( "Debug navigation"    , nav_map::s_bDebugNavigation );
                                  AddItemSeperator();
                                  AddItemBool     ( "Show active AIs"     , m_bShowActiveAIs );
#endif


#if !defined(X_RETAIL)
                                  AddItemBool     ( "Show Spatial Trigger Render",   m_bToggleTriggerRender );
                                  AddItemBool     ( "Show Invisible Wall Render" ,   m_bToggleWallRender );
                                  AddItemBool     ( "Show Damage Field Render" ,     m_bToggleDamageRender );
#endif
}

//==============================================================================

void debug_menu_page_aiscript::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pItemTriggerDump )
    {
#if !defined(X_RETAIL) || defined(X_QA)
        char Dir[128];
        char LevelName[32];
        x_splitpath( (const char*)xstring(g_ActiveConfig.GetLevelPath()),NULL,Dir,NULL,NULL);
        Dir[ x_strlen(Dir)-1 ] = 0;
        x_splitpath(Dir,NULL,NULL,LevelName,NULL);

        g_TriggerExMgr.DumpData( xfs("c:\\TriggerDump_%s.txt",LevelName) );
#endif
    }
}

//==============================================================================

void debug_menu_page_aiscript::OnPreRender( void )
{
    if( m_bShowActiveAIs )
    {
        static const s32 kTextTop  = 40;
        static const s32 kNameLeft = 10;
        static const s32 kGuidLeft = 150;
        static const s32 kPosLeft  = 330;

        // Figure out the small font ID and line height
        s32 FontID     = g_UiMgr->FindFont( "small" );
        s32 LineHeight = g_UiMgr->GetLineHeight( FontID );

        // count up how many active actors we have so we'll know how many lines of
        // text we'll need
        s32    NumLines = 0;
        actor* pActor   = actor::m_pFirstActive;
        while( pActor != NULL )
        {
            if( pActor->IsKindOf( character::GetRTTI() ) )
            {
                character* pCharacter = (character*)pActor;
                if( pCharacter->GetDoRunLogic() )
                    NumLines++;
            }
            else
            {
                NumLines++;
            }

            pActor = pActor->m_pNextActive;
        }

        // figure out the dimensions we can render to
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        s32 TextBottom = kTextTop + LineHeight * NumLines;

        // render a black background for the text
        irect Rect;
        Rect.Set( kNameLeft, kTextTop, XRes-kNameLeft, kTextTop + LineHeight * NumLines );
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        // render informative text for each of the actors...
        // NOTE: We are rendering the text visually from bottom to top. This will put
        // the more static guys (such as the player and his hazmat buddies) towards
        // the top and the dynamic guys will be towards the bottom. The longer an actor
        // sticks around, the more he should percolate to the top.
        s32 Count  = 0;
        pActor = actor::m_pFirstActive;
        while( pActor != NULL )
        {
            xbool   DisplayInfo = FALSE;
            if( pActor->IsKindOf( character::GetRTTI() ) )
            {
                character* pCharacter = (character*)pActor;
                if( pCharacter->GetDoRunLogic() )
                    DisplayInfo = TRUE;
            }
            else
            {
                DisplayInfo = TRUE;
            }

            if( DisplayInfo )
            {
                irect    Rect;

                // render the logical name
                xwstring NameText = pActor->GetLogicalName();
                g_UiMgr->TextSize( FontID, Rect, NameText, NameText.GetLength() );
                Rect.Translate( kNameLeft, TextBottom-LineHeight*(Count+1) );
                g_UiMgr->RenderText( FontID, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), NameText );

                // render the guid text
                guid     Guid = pActor->GetGuid();
                xwstring GuidText = (const char*)xfs( "%08X:%08X", Guid.GetHigh(), Guid.GetLow() );
                g_UiMgr->TextSize( FontID, Rect, GuidText, GuidText.GetLength() );
                Rect.Translate( kGuidLeft, TextBottom-LineHeight*(Count+1) );
                g_UiMgr->RenderText( FontID, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), GuidText );

                // render the position text
                vector3  Pos = pActor->GetPosition();
                xwstring PosText = (const char*)xfs( "(%4.0f,%4.0f,%4.0f)", Pos.GetX(), Pos.GetY(), Pos.GetZ() );
                g_UiMgr->TextSize( FontID, Rect, PosText, PosText.GetLength() );
                Rect.Translate( kPosLeft, TextBottom-LineHeight*(Count+1) );
                g_UiMgr->RenderText( FontID, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), PosText );

                Count++;
            }

            pActor = pActor->m_pNextActive;
        }

        ASSERT( Count == NumLines );
    }
#if !defined(X_RETAIL)
    if (m_bToggleWallRender)
    {
        slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_INVISIBLE_WALL_OBJ);
        while( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            if( pObject != NULL )
            {
                draw_ClearL2W();
                draw_Volume( pObject->GetBBox(), xcolor(0,255,0,64) );
            }

            SlotID = g_ObjMgr.GetNext( SlotID );
        }
    }
    if (m_bToggleDamageRender)
    {
        slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_DAMAGE_FIELD);
        while( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            if( pObject != NULL )
            {
                damage_field& DamageField = damage_field::GetSafeType( *pObject );
                xcolor FieldColor(255,0,0,64);
                if (!DamageField.IsActive())
                {
                    FieldColor = xcolor(255,128,128,64);
                }

                draw_ClearL2W();
                if (DamageField.GetSpatialType() == damage_field::SPATIAL_TYPE_AXIS_CUBE)
                {
                    draw_Volume( DamageField.GetBBox(), FieldColor );
                    draw_BBox( DamageField.GetBBox(), FieldColor );
                }
                else if (DamageField.GetSpatialType() == damage_field::SPATIAL_TYPE_SPHERICAL)
                {
                    draw_Sphere( DamageField.GetPosition(), DamageField.GetDimension(0) , FieldColor );
                }
            }

            SlotID = g_ObjMgr.GetNext( SlotID );
        }
    }
    if (m_bToggleTriggerRender)
    {
        slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_TRIGGER_EX);
        while( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            if( pObject != NULL )
            {
                trigger_ex_object& TriggerObject = trigger_ex_object::GetSafeType( *pObject );
                if (TriggerObject.GetTriggerType() == trigger_ex_object::TRIGGER_TYPE_SPATIAL) 
                {
                    xcolor FieldColor(0,0,255,64);
                    if (!TriggerObject.IsActive())
                    {
                        FieldColor = xcolor(128,128,255,64);
                    }

                    draw_ClearL2W();                
                    if (TriggerObject.GetSpatialType() == trigger_ex_object::SPATIAL_TYPE_AXIS_CUBE)
                    {
                        draw_Volume( TriggerObject.GetBBox(), FieldColor );
                        draw_BBox( TriggerObject.GetBBox(), FieldColor );
                    }
                    else if (TriggerObject.GetSpatialType() == trigger_ex_object::SPATIAL_TYPE_SPHERICAL)
                    {
                        draw_Sphere( TriggerObject.GetPosition(), TriggerObject.GetDimension(0) , FieldColor );
                    }
                }
                else if (TriggerObject.GetTriggerType() == trigger_ex_object::TRIGGER_TYPE_VIEWABLE) 
                {
                    xcolor FieldColor(255,0,255,64);
                    if (!TriggerObject.IsActive())
                    {
                        FieldColor = xcolor(255,128,255,64);
                    }

                    draw_ClearL2W();                
                    if (TriggerObject.GetSpatialType() == trigger_ex_object::SPATIAL_TYPE_AXIS_CUBE)
                    {
                        draw_Volume( TriggerObject.GetBBox(), FieldColor );
                        draw_BBox( TriggerObject.GetBBox(), FieldColor );

                    }
                    else if (TriggerObject.GetSpatialType() == trigger_ex_object::SPATIAL_TYPE_SPHERICAL)
                    {
                        draw_Sphere( TriggerObject.GetPosition(), TriggerObject.GetDimension(0) , FieldColor );
                    }
                }

            }

            SlotID = g_ObjMgr.GetNext( SlotID );
        }
    }
#endif
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
