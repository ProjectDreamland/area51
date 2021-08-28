//==============================================================================
//  
//  e_Draw.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\Entropy.hpp"
#include "..\e_Draw.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Line( const vector3& P0,
                const vector3& P1,
                      xcolor   Color )
{
    draw_Begin( DRAW_LINES );
        draw_Color( Color );
        draw_Vertex( P0 );
        draw_Vertex( P1 );
    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_BBox( const bbox&    BBox,
                      xcolor   Color )
{
    static s16 Index[] = {1,5,5,7,7,3,3,1,0,4,4,6,6,2,2,0,3,2,7,6,5,4,1,0};
    vector3 P[8];

    P[0].GetX() = BBox.Min.GetX();    P[0].GetY() = BBox.Min.GetY();    P[0].GetZ() = BBox.Min.GetZ();
    P[1].GetX() = BBox.Min.GetX();    P[1].GetY() = BBox.Min.GetY();    P[1].GetZ() = BBox.Max.GetZ();
    P[2].GetX() = BBox.Min.GetX();    P[2].GetY() = BBox.Max.GetY();    P[2].GetZ() = BBox.Min.GetZ();
    P[3].GetX() = BBox.Min.GetX();    P[3].GetY() = BBox.Max.GetY();    P[3].GetZ() = BBox.Max.GetZ();
    P[4].GetX() = BBox.Max.GetX();    P[4].GetY() = BBox.Min.GetY();    P[4].GetZ() = BBox.Min.GetZ();
    P[5].GetX() = BBox.Max.GetX();    P[5].GetY() = BBox.Min.GetY();    P[5].GetZ() = BBox.Max.GetZ();
    P[6].GetX() = BBox.Max.GetX();    P[6].GetY() = BBox.Max.GetY();    P[6].GetZ() = BBox.Min.GetZ();
    P[7].GetX() = BBox.Max.GetX();    P[7].GetY() = BBox.Max.GetY();    P[7].GetZ() = BBox.Max.GetZ();

    draw_Begin( DRAW_LINES, DRAW_USE_ALPHA );
        draw_Color( Color );
        draw_Verts( P, 8 );
        draw_Execute( Index, sizeof(Index)/sizeof(s16) );
    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Volume ( const bbox&    BBox, 
                         xcolor   Color )
{
    vector3 BBoxCenter = BBox.GetCenter();
    vector3 P0( BBoxCenter );
    vector3 P1( BBoxCenter );
    P0.GetZ() = BBox.Min.GetZ();
    P1.GetZ() = BBox.Max.GetZ();

    vector3 BBoxSize = BBox.GetSize();
    draw_Volume( P0, P1, BBoxSize.GetX()*0.5f, BBoxSize.GetY()*0.5f, Color );
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Volume ( const vector3& P0, 
                   const vector3& P1, 
                         f32      Width,
                         f32      Height,
                         xcolor   Color )
{
    vector3 corners[8];
    vector3 slope = P0 - P1;

    slope.GetY() = 0.0f;
    
    corners[0] = slope;
    corners[0].RotateY( PI/2.0f );
    corners[0].NormalizeAndScale( Width    );
    corners[0] += P0;
    corners[0].GetY() += Height;

    corners[1] = slope;
    corners[1].RotateY( (3.0f*PI)/2.0f );
    corners[1].NormalizeAndScale( Width  );
    corners[1] += P0;
    corners[1].GetY() += Height;

    corners[2] = slope;
    corners[2].RotateY( PI/2.0f );
    corners[2].NormalizeAndScale( Width  );
    corners[2] += P0;
    corners[2].GetY() -= Height;
    
    corners[3] = slope;
    corners[3].RotateY( (3.0f*PI)/2.0f );
    corners[3].NormalizeAndScale( Width  );
    corners[3] += P0;
    corners[3].GetY() -= Height;

    corners[4] = slope;
    corners[4].RotateY( PI/2.0f );
    corners[4].NormalizeAndScale( Width  );
    corners[4] += P1;
    corners[4].GetY() += Height;

    corners[5] = slope;
    corners[5].RotateY( (3.0f*PI)/2.0f );
    corners[5].NormalizeAndScale( Width  );
    corners[5] += P1;
    corners[5].GetY() += Height;

    corners[6] = slope;
    corners[6].RotateY( PI/2.0f );
    corners[6].NormalizeAndScale( Width  );
    corners[6] += P1;
    corners[6].GetY() -= Height;

    corners[7] = slope;
    corners[7].RotateY( (3.0f*PI)/2.0f );
    corners[7].NormalizeAndScale( Width  );
    corners[7] += P1;
    corners[7].GetY() -= Height;
    
#ifdef TARGET_PS2
    // darrin - The PS2 doesn't clip quads, but does clip tris,
    // so we'll use that instead for now.
    draw_Begin( DRAW_TRIANGLES, DRAW_CULL_NONE | DRAW_USE_ALPHA | DRAW_NO_ZWRITE );
    {
        draw_Color( Color );

        //  draw it in one direction....

        //  draw the ends
        draw_Vertex ( corners[0] );
        draw_Vertex ( corners[1] );
        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[0] );
        draw_Vertex ( corners[2] );
        draw_Vertex ( corners[3] );

        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[7] );
        draw_Vertex ( corners[5] );
        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[4] );
        draw_Vertex ( corners[5] );

        // draw the top and bottom
        draw_Vertex ( corners[4] );
        draw_Vertex ( corners[5] );
        draw_Vertex ( corners[1] );
        draw_Vertex ( corners[4] );
        draw_Vertex ( corners[0] );
        draw_Vertex ( corners[1] );

        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[2] );
        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[7] );
        draw_Vertex ( corners[3] );

        //  draw the sides
        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[4] );
        draw_Vertex ( corners[0] );
        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[2] );
        draw_Vertex ( corners[0] );

        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[1] );
        draw_Vertex ( corners[5] );
        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[7] );
        draw_Vertex ( corners[5] );
    }
    draw_End();
#else
    draw_Begin( DRAW_QUADS , DRAW_USE_ALPHA | DRAW_NO_ZWRITE );
    {
        draw_Color( Color );
       
        //  draw it in one direction....

        //  draw the ends
        draw_Vertex ( corners[0] );
        draw_Vertex ( corners[1] );
        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[2] );
       
        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[7] );
        draw_Vertex ( corners[5] );
        draw_Vertex ( corners[4] );

        // draw the top and bottom
        draw_Vertex ( corners[4] );
        draw_Vertex ( corners[5] );
        draw_Vertex ( corners[1] );
        draw_Vertex ( corners[0] );

        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[2] );
        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[7] );

        //  draw the sides
        draw_Vertex ( corners[6] );
        draw_Vertex ( corners[4] );
        draw_Vertex ( corners[0] );
        draw_Vertex ( corners[2] );

        draw_Vertex ( corners[3] );
        draw_Vertex ( corners[1] );
        draw_Vertex ( corners[5] );
        draw_Vertex ( corners[7] );
    }
    draw_End();
#endif
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_NGon( const vector3* pPoint,
                      s32      NPoints,
                      xcolor   Color,
                      xbool    DoWire )
{
    if( DoWire )
    {
        s32 P = NPoints-1;
        draw_Begin(DRAW_LINES, DRAW_USE_ALPHA);
            draw_Color(Color);
            for( s32 i=0; i<NPoints; i++ )
            {
                draw_Vertex(pPoint[P]);
                draw_Vertex(pPoint[i]);
                P = i;
            }
        draw_End();
    }
    else
    {
        draw_Begin(DRAW_TRIANGLES, DRAW_USE_ALPHA);
            draw_Color(Color);
            for( s32 i=1; i<NPoints-1; i++ )
            {
                draw_Vertex(pPoint[0]);
                draw_Vertex(pPoint[i]);
                draw_Vertex(pPoint[i+1]);
            }
        draw_End();
    }
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Sphere( const vector3& Pos,
                        f32      Radius,
                        xcolor   Color)
{
    #define A  0.8506f
    #define B  0.5257f
    static vector3  SphereV[12] = 
    { 
      vector3(+A, 0,+B), vector3(+A, 0,-B), vector3(-A, 0,+B), vector3(-A, 0,-B),
      vector3(+B,+A, 0), vector3(-B,+A, 0), vector3(+B,-A, 0), vector3(-B,-A, 0),
      vector3( 0,+B,+A), vector3( 0,-B,+A), vector3( 0,+B,-A), vector3( 0,-B,-A),
    };
    #undef A
    #undef B
    
    static s16 Index[] = 
    { 0,1,1,4,0,4,0,6,1,6,2,3,2,5,3,5,3,7,2,7,4,5,5,8,4,8,4,10,5,10,6,
      7,6,9,7,9,7,11,6,11,0,8,0,9,8,9,2,8,2,9,1,10,10,11,1,11,3,10,3,11 };
        
    vector3 P[12];
    for( s32 i=0; i<12; i++ )
        P[i] = Pos + SphereV[i] * Radius;

    draw_Begin( DRAW_LINES, DRAW_USE_ALPHA );
        draw_Color( Color );
        draw_Verts( P, 12 );
        draw_Execute( Index, sizeof(Index)/sizeof(s16) );
    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Marker( const vector3& Pos,
                        xcolor   Color )
{
    (void)Pos;
    (void)Color;

    const view* pView = eng_GetView();

    // Get screen coordinates of projected point
    vector3 ScreenPos;
    ScreenPos = pView->PointToScreen( Pos );

    // Get screen viewport bounds
    s32 X0,Y0,X1,Y1;
    pView->GetViewport(X0,Y0,X1,Y1);

    // Peg screen coordinates to viewport bounds
    ScreenPos.GetX() = MAX( X0, ScreenPos.GetX() );
    ScreenPos.GetX() = MIN( X1, ScreenPos.GetX() );
    ScreenPos.GetY() = MAX( Y0, ScreenPos.GetY() );
    ScreenPos.GetY() = MIN( Y1, ScreenPos.GetY() );

    // Make screen pos relative to viewport
    ScreenPos.GetX() -= X0;
    ScreenPos.GetY() -= Y0;

    // Draw away.
    draw_Begin( DRAW_QUADS, DRAW_2D );
    {
        draw_Color( Color );
        draw_Vertex( ScreenPos.GetX(),   ScreenPos.GetY()-8, 0 );
        draw_Vertex( ScreenPos.GetX()-8, ScreenPos.GetY(),   0 );
        draw_Vertex( ScreenPos.GetX(),   ScreenPos.GetY()+8, 0 );
        draw_Vertex( ScreenPos.GetX()+8, ScreenPos.GetY(),   0 );

        // If behind the screen, add a black center.
        if( ScreenPos.GetZ() < 0 )
        {
            draw_Color( XCOLOR_BLACK );
            draw_Vertex( ScreenPos.GetX(),   ScreenPos.GetY()-4, 0 );
            draw_Vertex( ScreenPos.GetX()-4, ScreenPos.GetY(),   0 );
            draw_Vertex( ScreenPos.GetX(),   ScreenPos.GetY()+4, 0 );
            draw_Vertex( ScreenPos.GetX()+4, ScreenPos.GetY(),   0 );
        }
    }
    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Point( const vector3& Pos,
                       xcolor   Color,
                       s32      Size )
{
/*
    s32 i;
    //u32 ActiveViewMask;

    // Get currently active views and shut them all off

    ActiveViewMask = eng_GetActiveViewMask();
    for( i=0; i<ENG_MAX_VIEWS; i++ )
        eng_DeactivateView(i);

     Loop through active views and render
    for( i=0; i<ENG_MAX_VIEWS; i++ )
    if( ActiveViewMask & (1<<i) )
*/
    {
        const view* pView = eng_GetView();

        // Get screen coordinates of projected point
        vector3 ScreenPos;
        ScreenPos = pView->PointToScreen( Pos );

        // If behind camera, just forget about it.
        if( ScreenPos.GetZ() < 0 )
            return;

        // Get screen viewport bounds
        s32 X0,Y0,X1,Y1;
        pView->GetViewport(X0,Y0,X1,Y1);

        // Make screen pos relative to viewport
        //ScreenPos.X -= X0;
        //ScreenPos.Y -= Y0;

        // For view 'i'...
        //eng_ActivateView(i);
        
        // Draw away.
        draw_Begin( DRAW_QUADS, DRAW_2D );
        {
            draw_Color( Color );
            draw_Vertex( ScreenPos.GetX()-Size, ScreenPos.GetY()-Size, 0 );
            draw_Vertex( ScreenPos.GetX()-Size, ScreenPos.GetY()+Size, 0 );
            draw_Vertex( ScreenPos.GetX()+Size, ScreenPos.GetY()+Size, 0 );
            draw_Vertex( ScreenPos.GetX()+Size, ScreenPos.GetY()-Size, 0 );
        }
        draw_End();
        //eng_DeactivateView(i);
    }
/*
    // Reactivate old views
    for( i=0; i<ENG_MAX_VIEWS; i++ )
    if( ActiveViewMask & (1<<i) )
        eng_ActivateView(i);
*/
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Frustum( const view&    View,
                         xcolor   Color,
                         f32      Dist )
{
    static s16 Index[8*2] = {0,1,0,2,0,3,0,4,1,2,2,3,3,4,4,1};
    vector3 P[6];
    s32     X0,X1,Y0,Y1;

    View.GetViewport(X0,Y0,X1,Y1);

    P[0] = View.GetPosition();
    P[1] = View.RayFromScreen( (f32)X0, (f32)Y0, view::VIEW );
    P[2] = View.RayFromScreen( (f32)X0, (f32)Y1, view::VIEW );
    P[3] = View.RayFromScreen( (f32)X1, (f32)Y1, view::VIEW );
    P[4] = View.RayFromScreen( (f32)X1, (f32)Y0, view::VIEW );

    // Normalize so that Z is Dist and move into world
    for( s32 i=1; i<=4; i++ )
    {
        P[i] *= Dist / P[i].GetZ();
        P[i]  = View.ConvertV2W( P[i] );
    }

    P[5] = (P[1] + P[2] + P[3] + P[4]) * 0.25f;

    draw_Begin( DRAW_LINES );
        draw_Color( Color );
        draw_Verts( P, 5 );
        draw_Execute( Index, 8*2 );
        draw_Color( XCOLOR_GREY );
        draw_Vertex(P[0]);
        draw_Vertex(P[5]);
    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Axis( f32 Size )
{
    draw_Begin( DRAW_LINES );

        draw_Color(XCOLOR_WHITE);   draw_Vertex(0,0,0);
        draw_Color(XCOLOR_RED);     draw_Vertex(Size,0,0);
        draw_Color(XCOLOR_WHITE);   draw_Vertex(0,0,0);
        draw_Color(XCOLOR_GREEN);   draw_Vertex(0,Size,0);
        draw_Color(XCOLOR_WHITE);   draw_Vertex(0,0,0);
        draw_Color(XCOLOR_BLUE);    draw_Vertex(0,0,Size);

        /*
        draw_Color( xcolor(255,0,0,255) );
        draw_Vertex ( Size, 0, 0 );  draw_Vertex ( 0, W, 0 );
        draw_Vertex ( Size, 0, 0 );  draw_Vertex ( 0,-W, 0 );
        draw_Vertex ( Size, 0, 0 );  draw_Vertex ( 0, 0,-W );
        draw_Vertex ( Size, 0, 0 );  draw_Vertex ( 0, 0, W );

        draw_Color( xcolor(0,255,0,255) );
        draw_Vertex ( 0, Size, 0 );  draw_Vertex ( W, 0, 0 );
        draw_Vertex ( 0, Size, 0 );  draw_Vertex (-W, 0, 0 );
        draw_Vertex ( 0, Size, 0 );  draw_Vertex ( 0, 0,-W );
        draw_Vertex ( 0, Size, 0 );  draw_Vertex ( 0, 0, W );

        draw_Color( xcolor(0,0,255,255) );
        draw_Vertex ( 0, 0, Size );  draw_Vertex ( 0, W, 0 );
        draw_Vertex ( 0, 0, Size );  draw_Vertex ( 0,-W, 0 );
        draw_Vertex ( 0, 0, Size );  draw_Vertex (-W, 0, 0 );
        draw_Vertex ( 0, 0, Size );  draw_Vertex ( W, 0, 0 );
        */

    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Grid( const vector3& Corner,
                const vector3& Edge1,
                const vector3& Edge2,
                      xcolor   Color, 
                      s32      NSubdivisions )
{
    s32 NWires = NSubdivisions+1;

    draw_Begin( DRAW_LINES );
    draw_Color( Color );

        for( s32 w=0; w<NWires; w++ )
        {
            f32 T = (f32)w/(f32)(NWires-1);
            draw_Vertex( Corner + Edge1*T );
            draw_Vertex( Corner + Edge1*T + Edge2 );
            draw_Vertex( Corner + Edge2*T );
            draw_Vertex( Corner + Edge2*T + Edge1 );
        }
    
    draw_End();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Label( const vector3& Pos,
                       xcolor   Color,
                 const char* pFormatStr, ... )
{
    (void)Pos;
    (void)Color;
    (void)pFormatStr;

    s32 i;
    s32 CharsPerLine = 32;
    
    // Build message
    char Msg[128];
    x_va_list   Args;
    x_va_start( Args, pFormatStr );
    x_vsprintf( Msg, pFormatStr, Args );
    s32 MsgLen = x_strlen(Msg);
    s32 NLines = MsgLen/CharsPerLine;
    if( NLines*CharsPerLine < MsgLen ) NLines++;

    // Search for newlines
    for( i=0; i<MsgLen; i++ )
        if( Msg[i] == '\n' ) NLines++;

    // Get character sizes
    s32 Dummy;
    (void)Dummy;
    s32 CharWidth;
    s32 CharHeight;
    text_GetParams( Dummy, Dummy, Dummy, Dummy,
                    CharWidth, CharHeight, Dummy );

    // Compute pixel offset from center
    s32 OffsetY = -(CharHeight*NLines/2);
    (void)OffsetY;

    // Push requested color
    text_PushColor(Color);

    // Loop through active views and render
    //u32 ActiveViewMask = eng_GetActiveViewMask();
    //for( i=0; i<ENG_MAX_VIEWS; i++ )
    //if( ActiveViewMask & (1<<i) )
    {
        const view* pView = eng_GetView();

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

        // Compute 
        if( !SkipDraw )
        {
            s32 SX;
            s32 SY = (s32)ScreenPos.GetY();
            s32 j = 0;
            s32 LenLeft = MsgLen;
            while( j<MsgLen )
            {
                xbool Newline = FALSE;
                s32 NChars = MIN(LenLeft,CharsPerLine);

                // Check for newline
                for( s32 k=j; k<j+NChars; k++ )
                if( Msg[k] == '\n' )
                {
                    NChars = k-j;
                    Newline = TRUE;
                }

                char C = Msg[j+NChars];
                Msg[j+NChars] = 0;

                SX = (s32)(ScreenPos.GetX() -(CharWidth*NChars)/2);
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
        }
    }

    // Return to original color
    text_PopColor();
}

#endif // X_RETAIL

//==============================================================================

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

void draw_Rect( const rect&    Rect,
                xcolor         Color,
                xbool          DoWire)
{
    f32 Near = 0.001f;

    if( DoWire )
    {
        if( (Rect.GetWidth() == 1) && (Rect.GetHeight() == 1) )
        {
            draw_Begin( DRAW_POINTS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
            draw_Color( Color );

            draw_Vertex( Rect.Min.X, Rect.Min.Y, Near );
        }
        else
        {
            draw_Begin( DRAW_LINES, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
            draw_Color( Color );


            draw_Vertex( Rect.Min.X  , Rect.Min.Y  , Near );
            draw_Vertex( Rect.Min.X  , Rect.Max.Y-1, Near );

            draw_Vertex( Rect.Min.X  , Rect.Max.Y-1, Near );
            draw_Vertex( Rect.Max.X-1, Rect.Max.Y-1, Near );

            draw_Vertex( Rect.Max.X-1, Rect.Max.Y-1, Near );
            draw_Vertex( Rect.Max.X-1, Rect.Min.Y  , Near );

            draw_Vertex( Rect.Max.X-1, Rect.Min.Y  , Near );
            draw_Vertex( Rect.Min.X  , Rect.Min.Y  , Near );
        }
    }
    else
    {
        draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_CULL_NONE );
        draw_Color( Color );

        draw_Vertex( Rect.Min.X, Rect.Min.Y, Near );
        draw_Vertex( Rect.Min.X, Rect.Max.Y, Near );
        draw_Vertex( Rect.Max.X, Rect.Max.Y, Near );
        draw_Vertex( Rect.Max.X, Rect.Min.Y, Near );
    }

    draw_End();
}

#endif // X_RETAIL

//==============================================================================

void draw_Rect( const irect&   Rect,
                xcolor         Color,
                xbool          DoWire)
{
    f32 Near = 0.001f;

    if( DoWire )
    {
        if( (Rect.GetWidth() == 1) && (Rect.GetHeight() == 1) )
        {
            draw_Begin( DRAW_POINTS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
            draw_Color( Color );

            draw_Vertex( (f32)Rect.l, (f32)Rect.t, Near );
        }
        else
        {
            draw_Begin( DRAW_LINES, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
            draw_Color( Color );


            draw_Vertex( (f32)Rect.l    , (f32)Rect.t    , Near );
            draw_Vertex( (f32)Rect.l    , (f32)(Rect.b-1), Near );

            draw_Vertex( (f32)Rect.l    , (f32)(Rect.b-1), Near );
            draw_Vertex( (f32)(Rect.r-1), (f32)(Rect.b-1), Near );

            draw_Vertex( (f32)(Rect.r-1), (f32)(Rect.b-1), Near );
            draw_Vertex( (f32)(Rect.r-1), (f32)Rect.t    , Near );

            draw_Vertex( (f32)(Rect.r-1), (f32)Rect.t    , Near );
            draw_Vertex( (f32)Rect.l    , (f32)Rect.t    , Near );
        }
    }
    else
    {
        draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_CULL_NONE );
        draw_Color( Color );

        draw_Vertex( (f32)Rect.l, (f32)Rect.t, Near );
        draw_Vertex( (f32)Rect.l, (f32)Rect.b, Near );
        draw_Vertex( (f32)Rect.r, (f32)Rect.b, Near );
        draw_Vertex( (f32)Rect.r, (f32)Rect.t, Near );
    }

    draw_End();
}

//==============================================================================

void draw_GouraudRect( const irect&   Rect,
                       const xcolor&  c1,
                       const xcolor&  c2,
                       const xcolor&  c3,
                       const xcolor&  c4,
                       xbool          DoWire,
                       xbool          DoAdditive)
{
    f32 Near = 0.001f;

    if( DoWire )
    {
        if( (Rect.GetWidth() == 1) && (Rect.GetHeight() == 1) )
        {
            draw_Begin( DRAW_POINTS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );

            draw_Color( c1 );
            draw_Vertex( (f32)Rect.l, (f32)Rect.t, Near );
        }
        else
        {
            draw_Begin( DRAW_LINES, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );

            draw_Color( c1 );
            draw_Vertex( (f32)Rect.l    , (f32)Rect.t    , Near );
            draw_Color( c2 );
            draw_Vertex( (f32)Rect.l    , (f32)(Rect.b-1), Near );

            draw_Vertex( (f32)Rect.l    , (f32)(Rect.b-1), Near );
            draw_Color( c3 );
            draw_Vertex( (f32)(Rect.r-1), (f32)(Rect.b-1), Near );

            draw_Vertex( (f32)(Rect.r-1), (f32)(Rect.b-1), Near );
            draw_Color( c4 );
            draw_Vertex( (f32)(Rect.r-1), (f32)Rect.t    , Near );

            draw_Vertex( (f32)(Rect.r-1), (f32)Rect.t    , Near );
            draw_Color( c1 );
            draw_Vertex( (f32)Rect.l    , (f32)Rect.t    , Near );
        }
    }
    else
    {
        if (DoAdditive)
            draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_BLEND_ADD|DRAW_CULL_NONE );
        else
            draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_CULL_NONE );

        draw_Color( c1 );
        draw_Vertex( (f32)Rect.l, (f32)Rect.t, Near );
        draw_Color( c2 );
        draw_Vertex( (f32)Rect.l, (f32)Rect.b, Near );
        draw_Color( c3 );
        draw_Vertex( (f32)Rect.r, (f32)Rect.b, Near );
        draw_Color( c4 );
        draw_Vertex( (f32)Rect.r, (f32)Rect.t, Near );
    }

    draw_End();
}
