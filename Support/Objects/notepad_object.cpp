//==============================================================================
//
//  notepad_object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

//#include "stdafx.h"

#include "notepad_object.hpp"
#include "x_color.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "Parsing\TextIn.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "Font\font.hpp"

#ifdef X_EDITOR
xbool g_DisplayNotepads=TRUE;
#else // X_EDITOR
xbool g_DisplayNotepads=FALSE;
#endif // X_EDITOR

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

void NotepadObject_Link( void ){}

//==============================================================================

static struct notepad_object_desc : public object_desc
{
    notepad_object_desc( void ) : object_desc( 
        object::TYPE_EDITOR_NOTEPAD_OBJECT, 
        "notepad_object",
        "EDITOR",

        object::ATTR_SPACIAL_ENTRY          |
        object::ATTR_RENDERABLE             |
        object::ATTR_DRAW_2D                |
        object::ATTR_EDITOR_TEMP_OBJECT,

        FLAGS_GENERIC_EDITOR_CREATE ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new notepad_object; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        (void)Object;
        return EDITOR_ICON_NOTE;
    }

#endif // X_EDITOR

} s_notepad_object_Desc;

//==============================================================================

const object_desc& notepad_object::GetTypeDesc( void ) const
{
    return s_notepad_object_Desc;
}

//==============================================================================

const object_desc& notepad_object::GetObjectType   ( void )
{
    return s_notepad_object_Desc;
}

//==============================================================================
// notepad_object
//==============================================================================

notepad_object::notepad_object(void)
{
    m_Note[0]           = 0;
    m_crText            = xcolor(0,0,0,255);
    m_crNote            = xcolor(240,240,140,64);
    m_fMaxRenderDist    = 1500.0f;
    m_bVisibleInGame    = TRUE;
    m_GuidMoveToObject  = 0;
}

//==============================================================================

notepad_object::~notepad_object(void)
{
}

//==============================================================================

void notepad_object::OnImport ( text_in& TextIn )
{
    (void)TextIn;
}

//==============================================================================

void notepad_object::OnInit(void)
{
    object::OnInit();
}

//==============================================================================

void notepad_object::OnRender ( void )
{
    #ifndef X_EDITOR
    if( (!m_bVisibleInGame) || (!g_DisplayNotepads) )
    {
        return;
    } 
    else
    {
        #ifdef aharp
        ASSERTS( FALSE, "Guess we were rendering notepads afterall" );
        #endif
    }
    #else 

    if( m_GuidMoveToObject )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_GuidMoveToObject );
        if( pObject )
        {
            // Make sure that we are in the right position.
            OnMove( pObject->GetPosition() + vector3( 0, 100, 0 ) );
            SetZone1( pObject->GetZone1() );
            SetZone2( pObject->GetZone2() );
        }
    }


    s32 i;
    vector3 Pos = GetPosition();


    //
    // Decide if we should render the note
    //
    const view* pView = eng_GetView();
    vector3 vPos = pView->GetPosition();

    vector3 Diff = vPos - Pos;
    if( Diff.Dot(Diff) > (m_fMaxRenderDist*m_fMaxRenderDist) )
        return;

    if( m_Note[0]==0 )
        return;


    // Get screen coordinates of projected point
    vector3 ScreenPos;
    ScreenPos = pView->PointToScreen( Pos );

    // Get screen viewport bounds
    s32 X0,Y0,X1,Y1;
    pView->GetViewport(X0,Y0,X1,Y1);

    xbool SkipDraw = FALSE;
    if( ScreenPos.GetX() < X0 ) SkipDraw = TRUE;
    if( ScreenPos.GetX() > X1 ) SkipDraw = TRUE;
    if( ScreenPos.GetY() < Y0 ) SkipDraw = TRUE;
    if( ScreenPos.GetY() > Y1 ) SkipDraw = TRUE;
    if( ScreenPos.GetZ() < 0  ) SkipDraw = TRUE;

    if( SkipDraw )
        return;

    //
    // We should render it !!!
    //

    //
    // Build substrings
    //
    char  Msg[MAX_NOTE_LENGTH+1];
    char* pSubMsg[32];
    s32   SubMsgW[32];
    s32   SubMsgH[32];
    s32   nSubMsgs;
    {
        x_strcpy( Msg, m_Note );

        s32 i = 0;
        nSubMsgs = 0;
        pSubMsg[nSubMsgs] = Msg;
        SubMsgW[nSubMsgs] = 0;
        SubMsgH[nSubMsgs] = 0;
        while( Msg[i] )
        {
            if( Msg[i] == ' ' )
            {
                if( (&Msg[i] - pSubMsg[nSubMsgs]) > 20 )
                {
                    ASSERT( nSubMsgs < 32 );
                    Msg[i] = 0;
                    SubMsgW[nSubMsgs] = g_Font.TextWidth( xwstring(pSubMsg[nSubMsgs]) );
                    SubMsgH[nSubMsgs] = g_Font.TextHeight( xwstring(pSubMsg[nSubMsgs]) );
                    nSubMsgs++;
                    pSubMsg[nSubMsgs] = Msg + i + 1;
                }
            }
            i++;
        }
        if( (&Msg[i] - pSubMsg[nSubMsgs]) > 0 )
        {
            SubMsgW[nSubMsgs] = g_Font.TextWidth( xwstring(pSubMsg[nSubMsgs]) );
            SubMsgH[nSubMsgs] = g_Font.TextHeight( xwstring(pSubMsg[nSubMsgs]) );
            nSubMsgs++;
        }
    }

    //
    // Determine box bounds
    //
    s32 XWidth=0;
    s32 YHeight=0;
    for( i=0; i<nSubMsgs; i++ )
    {
        YHeight += SubMsgH[i];
        XWidth   = MAX( XWidth, SubMsgW[i] );
    }

    //
    // draw bounding box
    //
    {
        draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA );
        {
            s32 nX = (XWidth/2) + 4;
            s32 nY = (YHeight/2) + 4;

            draw_Color( m_crNote );
            draw_Vertex( ScreenPos.GetX()-nX,       ScreenPos.GetY()+nY, 0 );
            draw_Vertex( ScreenPos.GetX()+nX,       ScreenPos.GetY()+nY, 0 );
            draw_Vertex( ScreenPos.GetX()+nX,       ScreenPos.GetY()-nY, 0 );
            draw_Vertex( ScreenPos.GetX()-nX,       ScreenPos.GetY()-nY, 0 );
        }
        draw_End();
    }

    //
    // Render strings
    //
    s32 YOffset = -YHeight/2;
    for( i=0; i<nSubMsgs; i++ )
    {
        irect Rect;
        Rect.l = (s32)ScreenPos.GetX() - (XWidth/2);
        Rect.t = (s32)ScreenPos.GetY() + YOffset;
        Rect.r = 512;
        Rect.b = 512;
        YOffset += SubMsgH[i];

        g_Font.RenderText(  Rect,
                            font::clip_l_justify | font::v_top,
                            m_crText,
                            xwstring(pSubMsg[i]));
    }
    #endif // X_EDITOR
}
     
//==============================================================================

bbox notepad_object::GetLocalBBox( void ) const 
{ 
    return bbox(vector3(0,0,0), 25);
}

//==============================================================================

void notepad_object::OnEnumProp ( prop_enum& List )
{
    object::OnEnumProp(List);

    List.PropEnumHeader  ( "Note",                       "Editor Only object properties.", 0 );
    List.PropEnumString  ( "Note\\Text",                 "Text to render.", 0 );
    List.PropEnumFloat   ( "Note\\RenderDist",           "Maximum distance from camera to render note.", 0 );
    List.PropEnumBool    ( "Note\\VisibleInGame",        "Decides whether note is visible in the game.", 0 );
    List.PropEnumColor   ( "Note\\Color(Text)",          "Text color.", 0 );
    List.PropEnumColor   ( "Note\\Color(Background)",    "Background color.", 0 );
    List.PropEnumGuid    ( "Note\\MoveToObject",         "Move the note pad to the object position. So the object you select with this guid, the note pad object will fallow. Very handy so we don't need to keep moving both objects.", 0 );
}

//==============================================================================

xbool   notepad_object::OnProperty( prop_query&        I    )
{
    if ( object::OnProperty(I) )
    {
    }
    else if( I.VarString( "Note\\Text", m_Note, MAX_NOTE_LENGTH ) )
    {
    }
    else if( I.VarFloat( "Note\\RenderDist", m_fMaxRenderDist))
    {
    }
    else if( I.VarBool( "Note\\VisibleInGame", m_bVisibleInGame))
    {
    }
    else if( I.IsVar( "Note\\Color(Text)" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarColor( m_crText );
        }
        else
        {
            m_crText = I.GetVarColor();
        }
    }
    else if( I.IsVar( "Note\\Color(Background)" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarColor( m_crNote );
        }
        else
        {
            m_crNote = I.GetVarColor();
        }
    }
    else if( I.VarGUID( "Note\\MoveToObject", m_GuidMoveToObject))
    {
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

void  notepad_object::OnMove( const vector3& newPos )
{
    object::OnMove(newPos);
}

//==============================================================================

