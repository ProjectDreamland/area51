//=============================================================================
//
//  nav_connection2_editor.cpp
//
//      - version of the nav_connection used only in the editor.  Inherits from 
//          object
//
//=============================================================================
#include "stdafx.h"
#include "obj_mgr\obj_mgr.hpp"
#include "nav_connection2_editor.hpp"
#include "nav_connection2_anchor.hpp"
#include "worldeditor.hpp"
#include "AI\AIMgr.hpp"

static const f32 k_ConnectionThickness      = 100.0f;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
void NavAnchor2_Link() {};

//========================================================================

static struct nav_connection2_editor_desc : public object_desc
{
    nav_connection2_editor_desc( void ) : object_desc( 
        object::TYPE_NAV_CONNECTION2_EDITOR, 
            "Navigation Connection v2", 
            "AI",

            object::ATTR_COLLISION_PERMEABLE         |
            object::ATTR_SPACIAL_ENTRY               |
            object::ATTR_RENDERABLE                  |  
            object::ATTR_EDITOR_TEMP_OBJECT,

            FLAGS_GENERIC_EDITOR_CREATE |
            FLAGS_IS_DYNAMIC            
            // Can't use FLAGS_NO_ALLOW_COPY because world_editor::InternalObjectCopy( guid ObjectGuid ) checks
            ) 
        {
             m_HandleAlpha      = 50;
             m_ConnectionAlpha  = 50;
        }         

    virtual object* Create      ( void              ) { return new nav_connection2_editor; }
    virtual void    OnEnumProp  ( prop_enum&  List  );
    virtual xbool   OnProperty  ( prop_query& I     );

    virtual s32  OnEditorRender( object& Object ) const
    {
        if( Object.IsKindOf( nav_connection2_editor::GetRTTI() ) )
        {
            nav_connection2_editor& Con = nav_connection2_editor::GetSafeType( Object );   
            Con.RenderNavConnection();            
        }

        // No icon please.
        return -1;
    }


    xcolor      m_ConnectionAlpha;
    u8          m_HandleAlpha;

} s_nav_connection2_editor;

//========================================================================

const object_desc& nav_connection2_editor::GetTypeDesc( void ) const
{
    return s_nav_connection2_editor;
}

//========================================================================

const object_desc& nav_connection2_editor::GetObjectType( void )
{
    return s_nav_connection2_editor;
}

//========================================================================

void nav_connection2_editor_desc::OnEnumProp( prop_enum& List )
{
    object_desc::OnEnumProp( List );

    List.PropEnumHeader( "NavConnection", "Editor Nav Connection Properties", 0 );
    List.PropEnumInt   ( "NavConnection\\HandleAlpha",  "This is the alpha of the handle in the center", 0 );
    List.PropEnumInt   ( "NavConnection\\ConnectionAlpha",  "This is the alpha of the connection box", 0 );
}

//========================================================================

xbool nav_connection2_editor_desc::OnProperty( prop_query& I )
{
    s32  AlphaH = m_HandleAlpha;
    s32  AlphaC = m_ConnectionAlpha;

    if (object_desc::OnProperty( I ))
    {
    }  
    else if ( I.VarInt("NavConnection\\HandleAlpha", AlphaH, 0, 255) )
    {
        if (!I.IsRead())
        {
            m_HandleAlpha = (u8)AlphaH;
        }        
    }
    else if ( I.VarInt("NavConnection\\ConnectionAlpha", AlphaC, 0, 255) )
    {
        if (!I.IsRead())
        {
            m_ConnectionAlpha = (u8)AlphaC;
        }        
    }
    else
    {   
        return FALSE ;        
    }

    return TRUE;
}

//========================================================================

class nav_connection2_anchor;

//========================================================================
nav_connection2_editor::nav_connection2_editor ( void )
{    
    Reset();
}

//========================================================================
nav_connection2_editor::~nav_connection2_editor ( )
{


}

//========================================================================

void nav_connection2_editor::OnInit( void )
{
    if (m_Anchor[0] == 0)
    {   
        /*
        m_Anchor[0] = g_WorldEditor.CreateObject( "[1]NavConnectionAnchor", 
                                                  nav_connection2_anchor::GetObjectType().GetTypeName(), 
                                                  vector3(-100,0,0),
                                                  g_WorldEditor.GetActiveLayer(), "\\");
        */
        m_Anchor[0] = g_ObjMgr.CreateObject( nav_connection2_anchor::GetObjectType().GetTypeName() );
        if (0 == m_Anchor[0])
        {
            x_throw("CreateTemporaryObject failed for nav_connection2_anchor!" );
            return;
        }
        object* pObj = g_ObjMgr.GetObjectByGuid( m_Anchor[0] );
        ASSERT(pObj);
        pObj->OnMove( vector3(-100,0,0) );
    }


    if (m_Anchor[1] == 0)
    {
        /*
        m_Anchor[1] = g_WorldEditor.CreateObject( "[1]NavConnectionAnchor", 
                                              nav_connection2_anchor::GetObjectType().GetTypeName(), 
                                              vector3(+100,0,0),
                                              g_WorldEditor.GetActiveLayer(), "\\");
        */
        m_Anchor[1] = g_ObjMgr.CreateObject( nav_connection2_anchor::GetObjectType().GetTypeName() );
        if (0 == m_Anchor[1])
        {
            x_throw("CreateTemporaryObject failed for nav_connection2_anchor!" );
            return;
        }
        object* pObj = g_ObjMgr.GetObjectByGuid( m_Anchor[1] );
        ASSERT(pObj);
        pObj->OnMove( vector3(+100,0,0) );
    }

    //
    //  Attach the connection to the anchors
    //
    nav_connection2_anchor* pAnchor = GetAnchor(0);
    if (NULL == pAnchor)
    {
        x_throw("nav_connection2_anchor [0] for nav_connection2_editor is missing!" );
        return;
    }
    pAnchor->SetConnection( GetGuid() );

    pAnchor = GetAnchor(1);
    if (NULL == pAnchor)
    {
        x_throw("nav_connection2_anchor [1] for nav_connection2_editor is missing!" );
        return;
    }
    pAnchor->SetConnection( GetGuid() );

}

//========================================================================
bbox    nav_connection2_editor::GetLocalBBox ( void ) const
{
    return m_LocalRenderBBox;
}

//========================================================================
void    nav_connection2_editor::Reset  ( void )
{
    m_Width             = 25.0f; 
    m_Flags             = ng_connection2::HINT_NONE ;
    m_ClearLineOfSight  = TRUE;

    m_Anchor[0] = m_Anchor[1] = 0;

    m_AnchorPosition[0].Zero();
    m_AnchorPosition[1].Zero();

    m_CollisionBBox.Clear();
    m_LocalRenderBBox.Set( vector3(0,0,0), 25.0f );
    
    m_IndexInSavedList = -1;
    
    m_bIsEnabled = TRUE;

}

//========================================================================
void    nav_connection2_editor::OnEnumProp ( prop_enum& List )
{
    object::OnEnumProp(List);
    
    List.PropEnumHeader(     "Nav Connection",                    
                                "", 0                                                 );
    
    List.PropEnumButton(     "Nav Connection\\Switch Direction",
                                "Toggles the connection of the node", 0                );
    
    List.PropEnumFloat(      "Nav Connection\\Width",             
                                "Width of the connection", 0                           );

    List.PropEnumFloat(      "Nav Connection\\Length",             
                                "Length of the connection",
                                PROP_TYPE_READ_ONLY  | PROP_TYPE_DONT_SAVE          );

    List.PropEnumVector3(    "Nav Connection\\Starting Anchor Pos",
                                "Position of the starting anchor",                 
                                PROP_TYPE_DONT_SHOW );

    List.PropEnumVector3(    "Nav Connection\\End Anchor Pos",
                                "Position of the end anchor",
                                PROP_TYPE_DONT_SHOW  );

    List.PropEnumBool(       "Nav Connection\\Initially Active", "Is the connection active at the start of the level?", 0 );

    List.PropEnumHeader(     "Nav Connection\\Flags",             
                                "List of flags for the Node", 0 );
    
    List.PropEnumBool(       "Nav Connection\\Flags\\HINT_DARK",  
                                "Connection is in a dark area", 0      );

    List.PropEnumBool(       "Nav Connection\\Flags\\HINT_COVER",  
                                "Connection has good cover", 0      );

    List.PropEnumBool(       "Nav Connection\\Flags\\HINT_SNEAKY",  
                                "Connection is along a sneaky path", 0      );

    List.PropEnumBool(       "Nav Connection\\Flags\\HINT_JUMP",  
                                "Connection requires jumping", 0      );

    List.PropEnumBool(       "Nav Connection\\Flags\\HINT_ONE_WAY",  
                                "Connection is one way", 0      );

    List.PropEnumBool(       "Nav Connection\\Flags\\HINT_SMALL_NPC",
                                "Connection is only useable by 'small' NPCs.  ie: Dustmite size", 0 );
    
}


//========================================================================
xbool   nav_connection2_editor::OnProperty ( prop_query& I )
{
    if ( object::OnProperty(I) )
    {
        return TRUE;
    }

    if (I.IsVar( "Nav Connection\\Initially Active" ))
    {
        if (I.IsRead())
        {
            I.SetVarBool( GetInitiallyEnabled() );
        }
        else
        {
            xbool B = I.GetVarBool();
            SetInitiallyEnabled( B );
        }
        return TRUE;
    }
    
    if( I.IsVar( "Nav Connection\\Switch Direction" ) )
    {

        if( I.IsRead() )
        {
            I.SetVarButton( "Switch" );
        }
        else
        {
            SetAnchors  ( m_Anchor[1], m_Anchor[0] );
        }

        return TRUE;
    }


    if( I.IsVar("Nav Connection\\Width") )
    {
        if(I.IsRead() )
        {
            I.SetVarFloat( m_Width );
        }
        else
        {
            m_Width = MAX( 10, x_abs( I.GetVarFloat() ));
            SetAnchorsDirty();
        }
        return TRUE;
    }

    if( I.IsVar("Nav Connection\\Length") )
    {
        if(I.IsRead() )
        {
            I.SetVarFloat( m_Length );
        }
        else
        {
            m_Length = I.GetVarFloat();
        }
        return TRUE;
    }

    if( I.IsVar("Nav Connection\\Starting Anchor Pos") )
    {
        if(I.IsRead() )
        {
            object* tempObject = NULL;
            if( m_Anchor[0] != 0 )
            {
                tempObject = g_ObjMgr.GetObjectByGuid( m_Anchor[0] );
                if( tempObject != NULL )
                {                
                    I.SetVarVector3( tempObject->GetPosition() );
                    RecalcPosition(TRUE);
                }
            }
            if( tempObject == NULL )
            {
                I.SetVarVector3( vector3( 0.0f, 0.0f, 0.0f ) );
                RecalcPosition(TRUE);
            }
        }
        else
        {
            MoveAnchor( 0, I.GetVarVector3() );
            SetAnchorsDirty();
        }
        return TRUE;
    }


    if( I.IsVar("Nav Connection\\End Anchor Pos") )
    {
        if(I.IsRead() )
        {
            object* tempObject = NULL;
            if( m_Anchor[0] != 0 )
            {
                tempObject = g_ObjMgr.GetObjectByGuid( m_Anchor[1] );
                if( tempObject != NULL )
                {                
                    I.SetVarVector3( tempObject->GetPosition() );
                    RecalcPosition(TRUE);
                }
            }
            if( tempObject == NULL )
            {
                I.SetVarVector3( vector3( 0.0f, 0.0f, 0.0f ) );
                RecalcPosition(TRUE);
            }            
        }
        else
        {
            MoveAnchor( 1, I.GetVarVector3() );
            SetAnchorsDirty();
        }
        return TRUE;
    }

    if( I.IsVar("Nav Connection\\Flags\\HINT_JUMP") )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_Flags & ng_connection2::HINT_JUMP );
            
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ng_connection2::HINT_JUMP;
            }
            else
            {
                m_Flags &= ~ng_connection2::HINT_JUMP;
            }
        }
        return TRUE;
    }


    if( I.IsVar("Nav Connection\\Flags\\HINT_DARK") )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_Flags & ng_connection2::HINT_DARK );
            
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ng_connection2::HINT_DARK;
            }
            else
            {
                m_Flags &= ~ng_connection2::HINT_DARK;
            }
        }
        return TRUE;
    }

    if( I.IsVar("Nav Connection\\Flags\\HINT_COVER") )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_Flags & ng_connection2::HINT_COVER );
            
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ng_connection2::HINT_COVER;
            }
            else
            {
                m_Flags &= ~ng_connection2::HINT_COVER;
            }
        }
        return TRUE;
    }


    if( I.IsVar("Nav Connection\\Flags\\HINT_SNEAKY") )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_Flags & ng_connection2::HINT_SNEAKY );
            
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ng_connection2::HINT_SNEAKY;
            }
            else
            {
                m_Flags &= ~ng_connection2::HINT_SNEAKY;
            }
        }
        return TRUE;
    }
  
    if( I.IsVar("Nav Connection\\Flags\\HINT_ONE_WAY") )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_Flags & ng_connection2::HINT_ONE_WAY );
            
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ng_connection2::HINT_ONE_WAY;
            }
            else
            {
                m_Flags &= ~ng_connection2::HINT_ONE_WAY;
            }
        }
        return TRUE;
    }

    if( I.IsVar("Nav Connection\\Flags\\HINT_SMALL_NPC") )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_Flags & ng_connection2::HINT_SMALL_NPC );
            
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ng_connection2::HINT_SMALL_NPC;
            }
            else
            {
                m_Flags &= ~ng_connection2::HINT_SMALL_NPC;
            }
        }
        return TRUE;
    }
    

    return false;
}


//========================================================================

void   nav_connection2_editor::OnMove ( const vector3& NewPos )
{
    m_Flags |= FLAG_MOVING;
    
    vector3 OriginalPos = GetPosition();
    object::OnMove(NewPos);

    nav_connection2_anchor* pA = NULL;
    nav_connection2_anchor* pB = NULL;

    pA = GetAnchor( 0 );
    pB = GetAnchor( 1 );
    
    if (NULL == pA)
    {
        ASSERTS( FALSE, "A nav_connection2_editor object is missing at least 1 anchor!" );
        return;
    }
    if (NULL == pB)
    {
        ASSERTS( FALSE, "A nav_connection2_editor object is missing at least 1 anchor!" );
        return;
    }

    vector3 DeltaA = pA->GetPosition() - OriginalPos;
    vector3 DeltaB = pB->GetPosition() - OriginalPos;

    if (!pA->IsMoving())
        pA->OnMove( NewPos + DeltaA );
    if (!pB->IsMoving())
        pB->OnMove( NewPos + DeltaB );

    m_AnchorPosition[0] = pA->GetPosition();
    m_AnchorPosition[1] = pB->GetPosition();

    RecalcPosition(FALSE);

    m_Flags &= (~FLAG_MOVING);
}

//========================================================================

void    nav_connection2_editor::RecalcPosition ( xbool callOnMove  )
{
    if (!(m_Flags & FLAG_ANCHORS_DIRTY))
        return;
    
    object* tempObject = NULL;

    if( m_Anchor[0] != 0 )
    {
        tempObject = g_ObjMgr.GetObjectByGuid( m_Anchor[0] );
        if( tempObject != NULL )
        {        
            m_AnchorPosition[0] = tempObject->GetPosition();
        }
    }

    if( tempObject == NULL )
    {
        m_AnchorPosition[0].Set( 0.0f, 0.0f, 0.0f );
    }
   
    tempObject = NULL;
    if( m_Anchor[1] != 0 )
    {
        tempObject = g_ObjMgr.GetObjectByGuid( m_Anchor[1] );
        if( tempObject != NULL )
        {        
            m_AnchorPosition[1] = tempObject->GetPosition();
        }

    }
    if( tempObject == NULL )
    {
        m_AnchorPosition[1].Set( 0.0f, 0.0f, 0.0f );
    }

    m_Length =  ( m_AnchorPosition[0] - m_AnchorPosition[1] ).Length();

    m_Flags &= (~FLAG_ANCHORS_DIRTY);
    
    object::OnMove( ( m_AnchorPosition[0] + m_AnchorPosition[1] )/2.0f );
        /*
    if( callOnMove && (!(m_Flags & FLAG_MOVING)) )
        OnMove( ( m_AnchorPosition[0] + m_AnchorPosition[1] )/2.0f );
*/
    BuildCorners();    
}

//========================================================================

void nav_connection2_editor::SetWidth( f32 Width )
{
    m_Width = Width;
    SetAnchorsDirty();
    RecalcPosition();
}

//========================================================================

void nav_connection2_editor::ScaleWidth( f32 scaleValue )
{
    if(scaleValue > 0.0f )
    {
        m_Width += 25.0f;
        //m_Width *= 1.1f;
    }
    else
    {
        m_Width -= 25.0f;
        //m_Width *= 0.90909090f;
    }

    SetAnchorsDirty();
}


//========================================================================
void    nav_connection2_editor::SetAnchors( guid startAnchor, guid endAnchor )
{
    m_Anchor[0] = startAnchor;
    m_Anchor[1] = endAnchor;

    SetAnchorsDirty();
    RecalcPosition();
}

//========================================================================

void nav_connection2_editor::SetAnchor( s32 iAnchor, guid gAnchor )
{
    // First check to see if the anchor specified is already attached
    // to a connection.  If it is, we cannot proceed.
    // This usually comes from placement creation.
    // It creates a temp connection, and then copies it when you hit SPACE.
    // If we were to proceed, we would orphan the connection by taking it's
    // connections with us.

    if (m_Anchor[ iAnchor ] != 0)
    {
        // Kill off the existing anchor
        nav_connection2_anchor* pA = GetAnchor( iAnchor );
        if (NULL != pA)
        {
            guid RemoteConn = pA->GetConnection();

            pA->SetConnection( 0 );
            g_WorldEditor.DeleteObject( m_Anchor[ iAnchor ] );
        }
        m_Anchor[ iAnchor ] = 0;
    }

    m_Anchor[ iAnchor ] = gAnchor;

    nav_connection2_anchor* pA = GetAnchor( iAnchor );
    if (NULL == pA)
    {
        x_throw("Failed to attach nav_connection2_editor to a nav_connection2_anchor!");
    }
    else
    {
        pA->SetConnection( GetGuid() );
    }
    
    SetAnchorsDirty();
    RecalcPosition();
}

//========================================================================

#ifndef X_RETAIL
void nav_connection2_editor::OnDebugRender ( void )
{
}
#endif // X_RETAIL

//========================================================================

void    nav_connection2_editor::OnRender ( void )
{
    BuildCorners();
}

//========================================================================

void    nav_connection2_editor::RenderNavConnection( void )
{
    BuildCorners();
    xcolor  selected( 250 ,0 ,250 ),
            unselected(0,50,200),
            patrol(50,250,50),
            LOSnotClear(254, 0 , 0 ),
            disabled(255,0,0);
            
    xcolor currentColor;

    if ( GetAttrBits() & ATTR_EDITOR_SELECTED)
    {
        currentColor = selected;
    }
    else if ( m_Flags & ng_connection2::HINT_PATROL_ROUTE )
    {
        currentColor = patrol;
    }
    else if ( GetClearLineOfSight() )
    {
        currentColor = unselected;
    }
    else
    {
        currentColor = LOSnotClear;
    }

    if (g_AIMgr.GetShowGridColoring())
    {
        if ((m_IndexInSavedList >= 0) &&  ( !(GetAttrBits() & ATTR_EDITOR_SELECTED)))
        {
            s32 iGrid = g_NavMap.GetConnectionGridID( m_IndexInSavedList );

            currentColor.Set( (iGrid * 171) % 255,
                              (iGrid * 31) % 255,
                              (iGrid * 97) % 255 );
        }
    }

    // Overrider color if the connectino is disabled
    if (!GetEnabled())
        currentColor = disabled;

    currentColor.A = s_nav_connection2_editor.m_ConnectionAlpha;

    if( m_AnchorPosition[0].LengthSquared() == 0.0f && m_AnchorPosition[1].LengthSquared() == 0.0f )
    {
        RecalcPosition();
    }

    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);

    Draw_Volume( currentColor );
}

//========================================================================

void    nav_connection2_editor::OnColCheck      ( void )
{
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);
    
    g_CollisionMgr.StartApply( GetGuid() );
    g_CollisionMgr.ApplyAABBox(m_CollisionBBox );
    g_CollisionMgr.EndApply();    
}

//========================================================================

void    nav_connection2_editor::OnColNotify ( object& Object )
{


}


//========================================================================

void nav_connection2_editor::OnKill(void )
{
    
    m_Flags |= FLAG_DYING;
    
    if( m_Anchor[0] != 0 )
    {
        nav_connection2_anchor* pA = GetAnchor(0);
        if (pA && !pA->IsDying())
        {        
            g_ObjMgr.DestroyObject( m_Anchor[0] );                       
        }
    }

    if( m_Anchor[1] != 0 )
    {
        nav_connection2_anchor* pA = GetAnchor(1);
        if (pA && !pA->IsDying())
        {        
            g_ObjMgr.DestroyObject( m_Anchor[1] );                       
        }
    }
}

//========================================================================

void nav_connection2_editor::BuildCorners( void )
{ 
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);

    if (!(m_Flags & FLAG_CORNERS_DIRTY))
        return;

    vector3& point1  = m_AnchorPosition[0];
    vector3& point2  = m_AnchorPosition[1];
    f32&     boxSize = m_Width;

    vector3 slope = point1 - point2;

    const f32 squareRootOf2 = 1.412f;
    slope.GetY() = 0.0f;

    f32 MinY1 = point1.GetY();
    f32 MaxY1 = point1.GetY() + k_ConnectionThickness;

    f32 MinY2 = point2.GetY();
    f32 MaxY2 = point2.GetY() + k_ConnectionThickness;
    
    m_RenderCorners[0] = slope;
    m_RenderCorners[0].RotateY( PI/2.0f );
    m_RenderCorners[0].NormalizeAndScale( boxSize    );
    m_RenderCorners[0] += point1;
    m_RenderCorners[0].GetY() = MaxY1;

    m_RenderCorners[1] = slope;
    m_RenderCorners[1].RotateY( (3.0f*PI)/2.0f );
    m_RenderCorners[1].NormalizeAndScale( boxSize  );
    m_RenderCorners[1] += point1;
    m_RenderCorners[1].GetY() = MaxY1;

    m_RenderCorners[2] = slope;
    m_RenderCorners[2].RotateY( PI/2.0f );
    m_RenderCorners[2].NormalizeAndScale( boxSize  );
    m_RenderCorners[2] += point1;
    m_RenderCorners[2].GetY() = MinY1;
    
    m_RenderCorners[3] = slope;
    m_RenderCorners[3].RotateY( (3.0f*PI)/2.0f );
    m_RenderCorners[3].NormalizeAndScale( boxSize  );
    m_RenderCorners[3] += point1;
    m_RenderCorners[3].GetY() = MinY1;


    m_RenderCorners[4] = slope;
    m_RenderCorners[4].RotateY( PI/2.0f );
    m_RenderCorners[4].NormalizeAndScale( boxSize  );
    m_RenderCorners[4] += point2;
    m_RenderCorners[4].GetY() = MaxY2;

    m_RenderCorners[5] = slope;
    m_RenderCorners[5].RotateY( (3.0f*PI)/2.0f );
    m_RenderCorners[5].NormalizeAndScale( boxSize  );
    m_RenderCorners[5] += point2;
    m_RenderCorners[5].GetY() = MaxY2;

    m_RenderCorners[6] = slope;
    m_RenderCorners[6].RotateY( PI/2.0f );
    m_RenderCorners[6].NormalizeAndScale( boxSize  );
    m_RenderCorners[6] += point2;
    m_RenderCorners[6].GetY() = MinY2;

    m_RenderCorners[7] = slope;
    m_RenderCorners[7].RotateY( (3.0f*PI)/2.0f );
    m_RenderCorners[7].NormalizeAndScale( boxSize  );
    m_RenderCorners[7] += point2;
    m_RenderCorners[7].GetY() = MinY2;


    vector3 centerPoint = (point1+point2)/2.0f;
    vector3 normalizedSlope = slope;
    normalizedSlope.NormalizeAndScale(10.0f );


    m_CollisionCorners[0] = normalizedSlope;
    m_CollisionCorners[0].RotateY( PI/2.0f );
    m_CollisionCorners[0] += centerPoint + normalizedSlope;
    m_CollisionCorners[0].GetY() += MIN(boxSize*1.2f,35.0f);

    m_CollisionCorners[1] = normalizedSlope;
    m_CollisionCorners[1].RotateY( (3.0f*PI)/2.0f );
    m_CollisionCorners[1] += centerPoint + normalizedSlope;
    m_CollisionCorners[1].GetY() += MIN(boxSize*1.2f,35.0f);

    m_CollisionCorners[2] = normalizedSlope;
    m_CollisionCorners[2].RotateY( PI/2.0f );
    m_CollisionCorners[2] += centerPoint + normalizedSlope;
    m_CollisionCorners[2].GetY() -= MIN(boxSize*1.2f,35.0f);
    
    m_CollisionCorners[3] = normalizedSlope;
    m_CollisionCorners[3].RotateY( (3.0f*PI)/2.0f );
    m_CollisionCorners[3] += centerPoint + normalizedSlope;
    m_CollisionCorners[3].GetY() -= MIN(boxSize*1.2f,35.0f);


    m_CollisionCorners[4] = normalizedSlope;
    m_CollisionCorners[4].RotateY( PI/2.0f );
    m_CollisionCorners[4] += centerPoint - normalizedSlope;
    m_CollisionCorners[4].GetY() += MIN(boxSize*1.2f,35.0f);

    m_CollisionCorners[5] = normalizedSlope;
    m_CollisionCorners[5].RotateY( (3.0f*PI)/2.0f );
    m_CollisionCorners[5] += centerPoint - normalizedSlope;
    m_CollisionCorners[5].GetY() += MIN(boxSize*1.2f,35.0f);

    m_CollisionCorners[6] = normalizedSlope;
    m_CollisionCorners[6].RotateY( PI/2.0f );
    m_CollisionCorners[6] += centerPoint - normalizedSlope;
    m_CollisionCorners[6].GetY() -= MIN(boxSize*1.2f,35.0f);

    m_CollisionCorners[7] = normalizedSlope;
    m_CollisionCorners[7].RotateY( (3.0f*PI)/2.0f );
    m_CollisionCorners[7] += centerPoint - normalizedSlope;
    m_CollisionCorners[7].GetY() -= MIN(boxSize*1.2f,35.0f);


    m_CollisionBBox.Clear();
    m_CollisionBBox.AddVerts(m_CollisionCorners,8 );

    m_LocalRenderBBox.Clear();

    vector3 MyPos = GetPosition();
    s32     i;

    for (i=0;i<8;i++)
    {
        vector3 Temp = m_RenderCorners[i] - MyPos;
        m_LocalRenderBBox.AddVerts( &Temp, 1 );
    }
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
    m_Flags &= (~FLAG_CORNERS_DIRTY);
}



vector3* nav_connection2_editor::GetRenderCorners( xbool bForceUpdate )
{
    if (bForceUpdate)
        SetCornersDirty();

    BuildCorners();
    
    return m_RenderCorners;
}

void nav_connection2_editor::Draw_Volume( xcolor aColor )
{
//    BuildCorners( point1, point2, boxSize );    
    
    xcolor selectionHotSpotColor( 128,255,128);
    selectionHotSpotColor.A = s_nav_connection2_editor.m_HandleAlpha;
    draw_ClearL2W();
    draw_Begin( DRAW_QUADS , DRAW_USE_ALPHA | DRAW_NO_ZWRITE );
    {

        draw_Color( aColor );

        //  draw the ends
        draw_Vertex ( m_RenderCorners[0] );
        draw_Vertex ( m_RenderCorners[1] );
        draw_Vertex ( m_RenderCorners[3] );
        draw_Vertex ( m_RenderCorners[2] );
 
       
        draw_Vertex ( m_RenderCorners[6] );
        draw_Vertex ( m_RenderCorners[7] );
        draw_Vertex ( m_RenderCorners[5] );
        draw_Vertex ( m_RenderCorners[4] );


        // draw the top and bottom
        draw_Vertex ( m_RenderCorners[4] );
        draw_Vertex ( m_RenderCorners[5] );
        draw_Vertex ( m_RenderCorners[1] );
        draw_Vertex ( m_RenderCorners[0] );

        draw_Vertex ( m_RenderCorners[6] );
        draw_Vertex ( m_RenderCorners[2] );
        draw_Vertex ( m_RenderCorners[3] );
        draw_Vertex ( m_RenderCorners[7] );

        //  draw the sides
        draw_Vertex ( m_RenderCorners[6] );
        draw_Vertex ( m_RenderCorners[4] );
        draw_Vertex ( m_RenderCorners[0] );
        draw_Vertex ( m_RenderCorners[2] );

        draw_Vertex ( m_RenderCorners[3] );
        draw_Vertex ( m_RenderCorners[1] );
        draw_Vertex ( m_RenderCorners[5] );
        draw_Vertex ( m_RenderCorners[7] );


                
        draw_Color( selectionHotSpotColor );
        
        //  draw the ends
        draw_Vertex ( m_CollisionCorners[0] );
        draw_Vertex ( m_CollisionCorners[1] );
        draw_Vertex ( m_CollisionCorners[3] );
        draw_Vertex ( m_CollisionCorners[2] );
 
       
        draw_Vertex ( m_CollisionCorners[6] );
        draw_Vertex ( m_CollisionCorners[7] );
        draw_Vertex ( m_CollisionCorners[5] );
        draw_Vertex ( m_CollisionCorners[4] );


        // draw the top and bottom
        draw_Vertex ( m_CollisionCorners[4] );
        draw_Vertex ( m_CollisionCorners[5] );
        draw_Vertex ( m_CollisionCorners[1] );
        draw_Vertex ( m_CollisionCorners[0] );

        draw_Vertex ( m_CollisionCorners[6] );
        draw_Vertex ( m_CollisionCorners[2] );
        draw_Vertex ( m_CollisionCorners[3] );
        draw_Vertex ( m_CollisionCorners[7] );

        //  draw the sides
        draw_Vertex ( m_CollisionCorners[6] );
        draw_Vertex ( m_CollisionCorners[4] );
        draw_Vertex ( m_CollisionCorners[0] );
        draw_Vertex ( m_CollisionCorners[2] );

        draw_Vertex ( m_CollisionCorners[3] );
        draw_Vertex ( m_CollisionCorners[1] );
        draw_Vertex ( m_CollisionCorners[5] );
        draw_Vertex ( m_CollisionCorners[7] );
    }
    draw_End();

    if (m_Flags & ng_connection2::HINT_ONE_WAY)
    {
        draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_CULL_NONE );
        {
            draw_Color( xcolor(200,200,0,aColor.A) );

            //  draw the ends
            draw_Vertex ( m_RenderCorners[1] );
            draw_Vertex ( m_RenderCorners[0] );

            vector3 EndPt = m_RenderCorners[5] - m_RenderCorners[4];
            EndPt *= 0.5f;
            EndPt += m_RenderCorners[4];

            draw_Vertex( EndPt );
        }
        draw_End();
    }
}


nav_connection2_anchor* nav_connection2_editor::GetAnchor( s32 iAnchor )
{
    iAnchor = MINMAX(0,iAnchor,1);

    object* pObj = g_ObjMgr.GetObjectByGuid( m_Anchor[ iAnchor ] );
    if (NULL == pObj)
    {
        //x_throw("temp nav_connection2_editor is missing an anchor!");    
        return NULL;
    }
    if( pObj->IsKindOf(nav_connection2_anchor::GetRTTI() ))
    {
        return (nav_connection2_anchor*)pObj;        
    }
    
    x_throw("temp nav_connection2_anchor guid doesn't refer to a nav_connection2_anchor object!");    

    return NULL;
}


void nav_connection2_editor::MoveAnchor(s32 iAnchor, const vector3& Pos )
{
    nav_connection2_anchor* pA = GetAnchor( iAnchor );
    if (NULL == pA)
        return;

    pA->OnMove( Pos );
}

f32 nav_connection2_editor::GetWidth( void )
{
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);
    return m_Width;    
}


f32 nav_connection2_editor::GetLength( void )
{
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);
    return m_Length;    
}


radian nav_connection2_editor::GetYaw( void )
{
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);

    vector3 Ray = m_AnchorPosition[1] - m_AnchorPosition[0];
    Ray.GetY() = 0;

    return Ray.GetYaw();
}

plane nav_connection2_editor::GetPlane( void )
{
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);

    BuildCorners();

    // build the middle plane
    plane   P;

    vector3 A,B,C;
    A = m_RenderCorners[3];
    B = m_RenderCorners[2];
    C = m_RenderCorners[6];

    A.GetY() += k_ConnectionThickness / 2.0f;
    B.GetY() += k_ConnectionThickness / 2.0f;
    C.GetY() += k_ConnectionThickness / 2.0f;

    P.Setup(A,B,C);

    return P;
}

vector3 nav_connection2_editor::GetAnchorPosition( s32 iAnchor )
{
    if (m_Flags & FLAG_ANCHORS_DIRTY)
        RecalcPosition(TRUE);

    iAnchor = MINMAX(0,iAnchor,1);

    return m_AnchorPosition[ iAnchor ];        
}


static const f32 k_MaxYDiff                 = 200.0f;

xbool nav_connection2_editor::IsPointInConnection( const vector3& Pt, f32 bufferAmount )
{
    vector3 startNodePosition   = m_AnchorPosition[0];
    vector3 endNodePosition     = m_AnchorPosition[1];
    vector3 pointToCheck        = Pt;

    // do a quick y check, then ignore the y value.
    if( (pointToCheck.GetY() > startNodePosition.GetY() + k_MaxYDiff && pointToCheck.GetY() > endNodePosition.GetY() + k_MaxYDiff)
     || (pointToCheck.GetY() < startNodePosition.GetY() - k_MaxYDiff && pointToCheck.GetY() < endNodePosition.GetY() - k_MaxYDiff) )
    {    
        return FALSE;
    }

    startNodePosition.GetY() = 0;
    endNodePosition.GetY() = 0;
    pointToCheck.GetY() = 0;

    vector3 Diff = pointToCheck - startNodePosition;
    vector3 Dir  = endNodePosition - startNodePosition;
    f32     T    = Diff.Dot( Dir );

    if( T > 0.0f )
    {
        f32 SqrLen = Dir.Dot( Dir );

        if ( T >= SqrLen )
        {
            return FALSE;
        }
        else
        {
            T    /= SqrLen;
            Diff -= T * Dir;
            return Diff.Length() <= GetWidth()+bufferAmount;
        }
    }
    return FALSE;
}

void nav_connection2_editor::SetEnabled( xbool bOnOff )
{
    
    m_bIsEnabled = bOnOff;
}

void nav_connection2_editor::SetInitiallyEnabled( xbool bOnOff )
{
    if (bOnOff)
        m_Flags &= (~ng_connection2::HINT_DISABLED);        
    else
        m_Flags |= ng_connection2::HINT_DISABLED;

    m_bIsEnabled = bOnOff;
}

xbool nav_connection2_editor::GetInitiallyEnabled ( void )
{
    return !(m_Flags & ng_connection2::HINT_DISABLED);
}

xbool nav_connection2_editor::GetEnabled          ( void )
{
    return m_bIsEnabled;
}