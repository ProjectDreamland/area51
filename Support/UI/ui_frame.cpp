//=========================================================================
//
//  ui_frame.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_frame.hpp"
#include "ui_manager.hpp"
#include "ui_font.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//#include "..\\Demo1\fe_colors.hpp"	//-- Jhowa

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_frame_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_frame* pframe = new ui_frame;
    pframe->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pframe;
}

//=========================================================================
//  ui_frame
//=========================================================================

ui_frame::ui_frame( void )
{
}

//=========================================================================

ui_frame::~ui_frame( void )
{
    Destroy();
}

//=========================================================================

xbool ui_frame::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_iElement = m_pManager->FindElement( "frame" );
    ASSERT( m_iElement != -1 );
    m_BackgroundColor = xcolor(0,0,0,0);

    if( m_Flags & WF_TITLE )
        m_Flags &= ~WF_TITLE;

    m_NewFrame = FALSE;
    m_TextWidth = 0;

    return Success;
}

//=========================================================================

void ui_frame::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Calculate rectangle
        irect r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // Render background rectangle first
        irect rb;
        rb = r;
        rb.Deflate( 1, 1 );
        if( m_BackgroundColor.A > 0 )
            m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );

        // Render title
        if( m_Flags & WF_TITLE )
        {
            irect rect = r;
            if( m_BigTitle )
                rect.SetHeight( 40 );
            else
                rect.SetHeight( 22 );

         
            //xcolor c1( 30,100,150,  0 );//FECOL_TITLE1;	//-- Jhowa
            //xcolor c2( 30,100,150,  0 );//FECOL_TITLE2;	//-- Jhowa

            LocalToScreen(rb);
            rect.Deflate  ( 1, 0 );
            rect.Translate( 0, 1 );
            //m_pManager->RenderGouraudRect( rect, c1, c1, c2, c2, FALSE );

            rect.Deflate( 8, 0 );
            rect.Translate( 0, -2 );
            //m_pManager->RenderText( 1, rect, ui_font::h_left|ui_font::v_center, XCOLOR_WHITE, m_Title );
            rect.Translate( 1, 1 );
			
			// Check what size font we want.
			s32 TextSize;

			if( m_BigTitle )
				TextSize = 2;
			else
				TextSize = 1;

            m_pManager->RenderText( TextSize, rect, ui_font::h_left|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Title );
            rect.Translate( -1, -1 );
            m_pManager->RenderText( TextSize, rect, ui_font::h_left|ui_font::v_center, xcolor(XCOLOR_WHITE), m_Title );
        }

        // Render appropriate frame rectangle
        if( m_NewFrame )
            RenderNewFrame( r );
        else
            m_pManager->RenderElement( m_iElement, r, 0 );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_frame::SetBackgroundColor( xcolor Color )
{
    m_BackgroundColor = Color;
}

//=========================================================================

xcolor ui_frame::GetBackgroundColor( void ) const
{
    return m_BackgroundColor;
}

//=========================================================================

void ui_frame::EnableTitle ( const xwstring&   Text, xbool BigTitle )
{
    m_Flags |= WF_TITLE;
    m_Title = Text;
	m_BigTitle = BigTitle;
}

//=========================================================================

void ui_frame::EnableTitle ( const xwchar*     Text, xbool BigTitle )
{
    m_Flags |= WF_TITLE;
    m_Title = Text;
	m_BigTitle = BigTitle;
}

//=========================================================================

void ui_frame::EnableNewFrame( xbool bFrame, s32 TextWidth )
{
    m_NewFrame  = bFrame;
    m_TextWidth = TextWidth;
}

//=========================================================================

void ui_frame::RenderNewFrame( irect& Position )
{
    xbool                   ScaleX = FALSE;
    xbool                   ScaleY = FALSE;
    s32                     ix;
    s32                     iy;
    s32                     ie;
    vector3                 p(0.0f, 0.0f, 0.5f );
    vector2                 wh;
    vector2                 uv0;
    vector2                 uv1;
    ui_manager::element*    pElement;
    xbitmap*                pBitmap;
    xbool                   IsAdditive = FALSE;
    xcolor                  Color = XCOLOR_WHITE;

    pElement = (ui_manager::element* )m_pManager->GetElement( m_iElement );
    pBitmap = pElement->Bitmap.GetPointer();

    // Determine what type we are, scaled horizontal, vertical, or both
    if( Position.GetWidth()  != 0 ) ScaleX = TRUE;
    if( Position.GetHeight() != 0 ) ScaleY = TRUE;

    // Being drawing
    draw_Begin( DRAW_SPRITES, DRAW_2D|DRAW_TEXTURED|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
    draw_SetTexture( *pBitmap );

#ifdef TARGET_PS2
    draw_DisableBilinear();
#endif

    // Set Additive Mode
    if( IsAdditive )
    {
#ifdef TARGET_PC
//        g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
//        g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE );
#endif

#ifdef TARGET_XBOX
        g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE );
#endif

#ifdef TARGET_PS2
        gsreg_Begin( 1 );
        gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_SRC,C_DST) );
        gsreg_End();
#endif

#ifdef TARGET_GCN
		GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP );
#endif

    }

    // Render all the parts of the element
    ie       = 0;
    p.GetY() = (f32)Position.t;

    // Loop on y
    for( iy=0 ; iy<pElement->cy ; iy++ )
    {
        // Reset x position
        p.GetX() = (f32)Position.l;

        // Set Height
        if( (pElement->cy == 3) && (iy == 1) )
        {
            wh.Y = (f32)Position.GetHeight() - ((f32)pElement->r[2*pElement->cx].GetHeight() + (f32)pElement->r[0].GetHeight());
        }
        else
        {
            wh.Y = (f32)pElement->r[ie].GetHeight();
        }


        // Loop on x
        for( ix=0 ; ix<pElement->cx ; ix++ )
        {
            xbitmap* pBitmap = pElement->Bitmap.GetPointer();

            // Calculate UVs
            uv0.X = ((f32)pElement->r[ie].l + 0.5f) / pBitmap->GetWidth();
            uv0.Y = ((f32)pElement->r[ie].t + 0.5f) / pBitmap->GetHeight();
            uv1.X = ((f32)pElement->r[ie].r - 0.0f) / pBitmap->GetWidth();
            uv1.Y = ((f32)pElement->r[ie].b - 0.0f) / pBitmap->GetHeight();

            // Set Width
            if( (pElement->cx == 3) && (ix == 1) )
            {
                wh.X = (f32)Position.GetWidth() - ((f32)pElement->r[2].GetWidth() + (f32)pElement->r[0].GetWidth());
            }
            else
            {
                wh.X = (f32)pElement->r[ie].GetWidth();
            }

            if( iy == 0 && ix == 1 )
            {
                // Calculate UVs
                uv0.X = ((f32)pElement->r[ie].l + 0.5f) / pBitmap->GetWidth();
                uv0.Y = ((f32)pElement->r[ie].t + 0.5f) / pBitmap->GetHeight();
                uv1.X = ((f32)pElement->r[ie].r - 0.0f) / pBitmap->GetWidth();
                uv1.Y = ((f32)pElement->r[ie].b - 0.0f) / pBitmap->GetHeight();
                
                wh.X = (((f32)Position.GetWidth() - ((f32)pElement->r[2].GetWidth() + (f32)pElement->r[0].GetWidth())) - (m_TextWidth+8))/2;
                // Draw sprite
                draw_SpriteUV( p, wh, uv0, uv1, Color );

                // Advance position on x
                p.GetX() += wh.X+m_TextWidth+8;

                // Draw sprite
                draw_SpriteUV( p, wh, uv0, uv1, Color );

                // Advance position on x
                p.GetX() += wh.X;

            }
            else
            {
                // Draw sprite
                draw_SpriteUV( p, wh, uv0, uv1, Color );

                // Advance position on x
                p.GetX() += wh.X;

            }



            // Advance index to element
            ie++;
        }

        // Advance position on y
        p.GetY() += wh.Y;
    }

    // End drawing
    draw_End();
}

//=========================================================================

void ui_frame::ChangeElement ( const char* element )
{
    
    m_iElement = m_pManager->FindElement( element );

}

//=========================================================================