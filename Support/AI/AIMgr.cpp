//=============================================================================
//
//
//
///=============================================================================
#include "AIMgr.hpp"
#include "ManagerRegistration.hpp"

//=============================================================================

ai_mgr      g_AIMgr;

//=============================================================================

ai_mgr::ai_mgr()
{
    Reset();   
}

//=============================================================================

ai_mgr::~ai_mgr()
{

}

//=============================================================================

void ai_mgr::Init( void )
{
    g_RegGameMgrs.AddManager( "AI Manager", &g_AIMgr );
}

//=============================================================================

void ai_mgr::Reset()
{
    m_bShowNavSpine = FALSE;
    m_OverlapReductionAmt= 1.0f;
    m_bShowConnectionIDs = FALSE;
    m_bRenderConnectionsBright = FALSE;
}

//=============================================================================

void ai_mgr::OnEnumProp( prop_enum&    List )
{
    List.PropEnumString  (  "AI Manager", "Manager for all things AI related", PROP_TYPE_HEADER );
    List.PropEnumString  (  "AI Manager\\Navigation", "Manager for all things navigation related", PROP_TYPE_HEADER );
    List.PropEnumBool    (  "AI Manager\\Navigation\\Render Nav Spine", "Toggles rendering of the navigation spine", 0 );
    List.PropEnumFloat   (  "AI Manager\\Navigation\\Overlap Construction Scale", "Scales the overlap area down by this multiplier.", 0 );
    List.PropEnumBool    (  "AI Manager\\Navigation\\Render Grid Coloring", "Colors connections based on their grid ID", 0 );
    
    List.PropEnumString  (  "AI Manager\\Navigation\\Debug", "Debugging assistance", PROP_TYPE_HEADER );

    List.PropEnumBool    (  "AI Manager\\Navigation\\Debug\\Show Connection IDs", "Toggles rendering of the connection IDs", 0 );
    List.PropEnumBool    (  "AI Manager\\Navigation\\Debug\\Render Tactical Overlay", "Toggles rendering of the tactical overlay", 0 );
    List.PropEnumBool    (  "AI Manager\\Navigation\\Debug\\Render Connections Bright", "Render the connection coverage brightly", 0 );
}

//=============================================================================

xbool ai_mgr::OnProperty( prop_query&   I    )
{
    if( I.VarBool( "AI Manager\\Navigation\\Render Nav Spine", m_bShowNavSpine ) )
    {
        return TRUE;
    }
    if( I.VarBool( "AI Manager\\Navigation\\Debug\\Show Connection IDs", m_bShowConnectionIDs ) )
    {
        return TRUE;
    }
    else if (I.VarFloat("AI Manager\\Navigation\\Overlap Construction Scale", m_OverlapReductionAmt ))
    {
        // Keeping m_OverlapReductionAmt for now, until we verify that 
        // we no longer need it.
        //
        //  Clamp the value to 1.0f always.
        //
        m_OverlapReductionAmt = 1.0f;
        return TRUE;
    }
    if( I.VarBool( "AI Manager\\Navigation\\Render Grid Coloring", m_bShowGridColoring ) )
    {
        return TRUE;
    }
    if( I.VarBool( "AI Manager\\Navigation\\Debug\\Render Tactical Overlay", m_bShowTacticalOverlay ) )
    {
        return TRUE;
    }
    if( I.VarBool( "AI Manager\\Navigation\\Debug\\Render Connections Bright", m_bRenderConnectionsBright ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

//=============================================================================

//=============================================================================

//=============================================================================