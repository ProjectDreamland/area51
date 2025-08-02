//==============================================================================
//
//  notepad_object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"

#include "notepad_object.hpp"
#include "x_color.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "Parsing\TextIn.hpp"
#include "Render\Editor\editor_icons.hpp"

const f32 c_Sphere_Radius = 25.0f;

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
        object::ATTR_COLLISION_PERMEABLE    |
        object::ATTR_EDITOR_TEMP_OBJECT,

        FLAGS_GENERIC_EDITOR_CREATE ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new notepad_object; }

    //-------------------------------------------------------------------------

    virtual s32  OnEditorRender( object& Object ) const
    {
        (void)Object;
        return EDITOR_ICON_NOTE;
    }

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

//==============================================================================
notepad_object::notepad_object(void)
{
    m_Note[0]     = 0;
    m_nCurrentLength = 0;
    m_crText = xcolor(0,0,0,255);
    m_crNote = xcolor(240,240,140,255);
    m_nCharsPerLine = 24;
    m_fMaxRenderDist = 3000.0f;
}

//==============================================================================
notepad_object::~notepad_object(void)
{
}

//==============================================================================

void notepad_object::OnImport ( text_in& TextIn )
{
}

//==============================================================================
void notepad_object::OnInit(void)
{
    object::OnInit();
}

//==============================================================================

void notepad_object::OnRender ( void )
{
    const view* pView = eng_GetActiveView(0);
    vector3 &Pos = GetPosition();
    vector3 &vPos = pView->GetPosition();

    f32 fX = vPos.X - Pos.X;
	f32 fY = vPos.Y - Pos.Y;
	f32 fZ = vPos.Z - Pos.Z;
	if ((fX*fX + fY*fY + fZ*fZ) < (m_fMaxRenderDist*m_fMaxRenderDist))
    {
        if (m_nCurrentLength>0)
        {

            // Get screen coordinates of projected point
            vector3 ScreenPos;
            ScreenPos = pView->PointToScreen( Pos );

            // Get screen viewport bounds
            s32 X0,Y0,X1,Y1;
            pView->GetViewport(X0,Y0,X1,Y1);

            xbool SkipDraw = FALSE;
            if( ScreenPos.X < X0 ) SkipDraw = TRUE;
            if( ScreenPos.X > X1 ) SkipDraw = TRUE;
            if( ScreenPos.Y < Y0 ) SkipDraw = TRUE;
            if( ScreenPos.Y > Y1 ) SkipDraw = TRUE;
            if( ScreenPos.Z < 0  ) SkipDraw = TRUE;

            // Compute 
            if( !SkipDraw )
            {
                s32 i;
    
                // Build message
                char Msg[MAX_NOTE_LENGTH+1];
                x_va_list   Args;
                x_va_start( Args, m_Note );
                x_vsprintf( Msg, m_Note, Args );
                s32 MsgLen = x_strlen(Msg);
                s32 NLines = MsgLen/m_nCharsPerLine;
                if( NLines*m_nCharsPerLine < MsgLen ) NLines++;

                // Search for newlines
                for( i=0; i<MsgLen; i++ )
                    if( Msg[i] == '\n' ) NLines++;

                // Get character sizes
                s32 Dummy;
                s32 CharWidth;
                s32 CharHeight;
                text_GetParams( Dummy, Dummy, Dummy, Dummy,
                                CharWidth, CharHeight, Dummy );

#ifdef TARGET_PC
                //hack fix for PC
                CharWidth -= 2;
#endif

                // Compute pixel offset from center
                s32 OffsetY = -(CharHeight*NLines/2);

                // Push requested color
                text_PushColor(m_crText);

                const view* pView = eng_GetActiveView(0);

                // Get screen coordinates of projected point
                vector3 ScreenPos;
                ScreenPos = pView->PointToScreen( Pos );

                // Get screen viewport bounds
                s32 X0,Y0,X1,Y1;
                pView->GetViewport(X0,Y0,X1,Y1);

                s32 SX;
                s32 SY = (s32)ScreenPos.Y;
                s32 j = 0;
                s32 LenLeft = MsgLen;
                s32 nWidthMax = 0;
                while( j<MsgLen )
                {
                    xbool Newline = FALSE;
                    s32 NChars = MIN(LenLeft,m_nCharsPerLine);

                    // Check for newline
                    for( s32 k=j; k<j+NChars; k++ )
                    if( Msg[k] == '\n' )
                    {
                        NChars = k-j;
                        Newline = TRUE;
                    }

                    char C = Msg[j+NChars];
                    Msg[j+NChars] = 0;

                    if ((CharWidth*NChars) > nWidthMax)
                    {
                        nWidthMax = CharWidth*NChars;
                    }
                    SX = (s32)(ScreenPos.X -(CharWidth*NChars)/2);

                    //draw text
                    text_PrintPixelXY( &Msg[j], 
                                       (s32)(SX), 
                                       (s32)(SY + OffsetY) );

                    Msg[j+NChars] = C;
                    j+=NChars;
                    LenLeft-=NChars;
                    if( Newline )
                    { 
                        j++;
                        LenLeft--;
                    }
                    SY += CharHeight;
                }

                // Return to original color
                text_PopColor();

                // Draw away.
                draw_Begin( DRAW_QUADS, DRAW_2D );
                {
                    s32 nX = nWidthMax/2 + 4;
                    s32 nY = (CharHeight*NLines)/2 + 4;

                    draw_Color( m_crNote );
                    draw_Vertex( ScreenPos.X-nX,       ScreenPos.Y+nY, 0 );
                    draw_Vertex( ScreenPos.X+nX,       ScreenPos.Y+nY, 0 );
                    draw_Vertex( ScreenPos.X+nX,       ScreenPos.Y-nY, 0 );
                    draw_Vertex( ScreenPos.X-nX,       ScreenPos.Y-nY, 0 );

                }
                draw_End();
                draw_Frustum(*eng_GetActiveView(0));
            }
        }
    }
    /*
    else
    {
        if ( GetAttrBits() & ATTR_EDITOR_PLACEMENT_OBJECT )
        {
            draw_editor_icon(EDITOR_ICON_NOTE, Pos, xcolor(255,0,0,150));
        }
        else if (GetAttrBits() & ATTR_EDITOR_BLUE_PRINT)
        {
            draw_editor_icon(EDITOR_ICON_NOTE, Pos);
            if (GetAttrBits() & ATTR_EDITOR_SELECTED)
                draw_editor_icon(EDITOR_ICON_NOTE, Pos, xcolor(100,255,0,100));
        }
        else
        {
            draw_editor_icon(EDITOR_ICON_NOTE, Pos);
            if (GetAttrBits() & ATTR_EDITOR_SELECTED)
                draw_editor_icon(EDITOR_ICON_NOTE, Pos, xcolor(255,0,100,100));
        }
    }
    */
}
     
//==============================================================================

bbox notepad_object::GetLocalBBox( void ) const 
{ 
    return bbox(vector3(0,0,0), c_Sphere_Radius);
}

//==============================================================================

void notepad_object::OnEnumProp ( prop_enum& List )
{
    object::OnEnumProp(List);

    List.AddHeader  ( "Note",                       "Editor Only object properties." );
    List.AddString  ( "Note\\Text",                 "Text to render." );
    List.AddInt     ( "Note\\MaxChars",             "Maximum number of characters per line." );
    List.AddFloat   ( "Note\\RenderDist",           "Maximum distance from camera to render note." );
    List.AddColor   ( "Note\\Color(Text)",          "Text color.");
    List.AddColor   ( "Note\\Color(Background)",    "Background color.");
}

//==============================================================================
xbool   notepad_object::OnProperty( prop_query&        I    )
{
    xbool bFound = FALSE;
    if ( object::OnProperty(I) )
    {
        bFound = TRUE;
    }
    else if( I.VarString( "Note\\Text", m_Note, MAX_NOTE_LENGTH ) )
    {
        bFound = TRUE;
        m_nCurrentLength = x_strlen(m_Note);
    }
    else if( I.VarFloat( "Note\\RenderDist", m_fMaxRenderDist))
    {
        bFound = TRUE;
    }
    else if( I.VarInt( "Note\\MaxChars", m_nCharsPerLine))
    {
        bFound = TRUE;
    }
    else if( I.IsVar( "Note\\Color(Text)" ) )
    {
        bFound = TRUE;
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
        bFound = TRUE;
        if( I.IsRead() )
        {
            I.SetVarColor( m_crNote );
        }
        else
        {
            m_crNote = I.GetVarColor();
        }
    }
    return bFound;
}

//==============================================================================
void  notepad_object::OnMove( const vector3& newPos )
{
    object::OnMove(newPos);
}
