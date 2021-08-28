//=========================================================================
//
//  font.cpp
//
//=========================================================================

#include "font.hpp"

#include "..\..\..\xCore\Auxiliary\Bitmap\aux_bitmap.hpp"

//=========================================================================


//=========================================================================

#define OFFSET_X    (2048-(512/2))
#define OFFSET_Y    (2048-(512/2))
#define CHAR_WIDTH  13
#define CHAR_HEIGHT 18
#define XBORDER      8
#define YBORDER      8

//=========================================================================

#ifdef TARGET_PS2

struct header
{
	dmatag  DMA;        // DMA tag
	giftag  PGIF;       // GIF for setting PRIM register
	s64     Prim;       // PRIM register
    s64     Dummy;
	giftag  GIF;		// GIF for actual primitives
};

struct char_info
{
	s64     Color;		// RGBAQ register
    s64     T0;
    s64     P0;
    s64     T1;
    s64     P1;
    s64     Dummy;
};

#endif

//=========================================================================

#ifdef TARGET_PS2

static header*          s_pHeader;
static s32              s_NChars;
static giftag           s_GIF       PS2_ALIGNMENT(16);
static giftag           s_PGIF      PS2_ALIGNMENT(16);

#endif

//=========================================================================
//  HUD Font
//=========================================================================

xbool font::Load( const char* pPathName )
{
    // Load font image
    VERIFY( m_Bitmap.Load( pPathName ) );

    // Setup info
    m_Height = m_Bitmap.GetHeight()-1;
    x_memset( &m_Characters, 0, sizeof(Character)*256 );

    // Get info from bitmap size
    m_BmWidth   = m_Bitmap.GetWidth();
    m_BmHeight  = m_Bitmap.GetHeight();

    // Clear Data
    m_MaxWidth  = 0;
    m_AvgWidth  = 0;

    // Scan through font building character map
    s32 y = 0;
    for( s32 Row=0 ; Row<7 ; Row++ )
    {
        // Initialize for character row
        s32 x1 = 0;
        for( s32 Col=0 ; Col<16 ; Col++ )
        {
            // Scan registration marks for character
            s32 x2 = x1+1;
            while( (x2 < m_BmWidth) && !(m_Bitmap.GetPixelColor( x2, y ).R < 128) )
                x2++;

            // Add character
            m_Characters[16+Row*16+Col].X = x1;
            m_Characters[16+Row*16+Col].Y = y+1;
            m_Characters[16+Row*16+Col].W = x2-x1;

            // Update MaxWidth
            if( (x2-x1) > m_MaxWidth )
                m_MaxWidth = (x2-x1);

            // Set start of next character
            x1 = x2+1;
        }

        // Scan down to next row
        if( Row < 6 )
        {
            m_RowHeight = y;
            y++;
            while( (y < m_BmHeight) && !(m_Bitmap.GetPixelColor( 0, y ).R < 128) )
                y++;

			// Skip out if not found
			if( y >= m_BmHeight )
				break;

            m_RowHeight = y - m_RowHeight;
            m_Height    = m_RowHeight - 1;
        }
    }

    // Set AvgWidth
    m_AvgWidth = m_Characters['x'].W;

    // Register the bitmap
    vram_Register( m_Bitmap );

    // Return success
    return TRUE;
}

//=========================================================================

void font::Kill( void )
{
    // UnRegister the bitmap
    vram_Unregister( m_Bitmap );
    m_Bitmap.Kill();
}

//=========================================================================

void font::TextSize( irect& Rect, const char* pString, s32 Count ) const
{
    s32 Height    = m_Height;
    s32 BestWidth = 0;
    s32 Width     = 0;

    // Loop until end of string or end of count.
    while( *pString && (Count != 0) )
    {
        s32 c = *pString++;

        // Check for newline.
        if( c == '\n' )
        {
            BestWidth = MAX( BestWidth, Width-1 );
            Width     = 0;
            Height   += m_Height;
        }
        else
        // Normal character.
        {
            // Add character to width.
            Width += m_Characters[c].W + 1;
        }             

        // Decrease character count
        Count--;
    }

    BestWidth = MAX( BestWidth, Width-1 );

    // We have all we need.
    Rect.Set( 0, 0, BestWidth, Height );
}

//=========================================================================

void font::TextSize( irect& Rect, const xwchar* pString, s32 Count ) const
{
    s32 Height    = m_Height;
    s32 BestWidth = 0;
    s32 Width     = 0;

    // Loop until end of string or end of count.
    while( *pString && (Count != 0) )
    {
        s32 c = *pString++;

        // Check for embedded color code.
        if( (c & 0xFF00) == 0xFF00 )
        {
            // Skip 2nd character in embedded color code.
            pString++;

            // Decrease character count one extra for 2nd char in code.
            Count--;
        }
        else
        // Check for newline.
        if( c == '\n' )
        {
            BestWidth = MAX( BestWidth, Width-1 );
            Width     = 0;
            Height   += m_Height;
        }
        else
        // Normal character.
        {
            // Add character to width.
            Width += m_Characters[c].W + 1;
        }             

        // Decrease character count
        Count--;
    }

    BestWidth = MAX( BestWidth, Width-1 );

    // We have all we need.
    Rect.Set( 0, 0, BestWidth, Height );
}

//=========================================================================

s32 font::TextWidth( const xwchar* pString, s32 Count ) const
{
    s32 BestWidth = 0;
    s32 Width     = 0;

    // Loop until end of string or end of count.
    while( *pString && (Count != 0) )
    {
        s32 c = *pString++;

        // Check for embedded color code.
        if( (c & 0xFF00) == 0xFF00 )
        {
            // Skip 2nd character in embedded color code.
            pString++;

            // Decrease character count one extra for 2nd char in code.
            Count--;
        }
        else
        // Check for newline.
        if( c == '\n' )
        {
            BestWidth = MAX( BestWidth, Width-1 );
            Width     = 0;
        }
        else
        // Normal character.
        {
            // Add character to width
            Width += m_Characters[c].W + 1;
        }             

        // Decrease character count.
        Count--;
    }

    BestWidth = MAX( BestWidth, Width-1 );

    // Return best width.
    return( BestWidth );
}

//=========================================================================

s32 font::TextHeight( const xwchar* pString, s32 Count ) const
{
    s32 Height = m_Height;

    // Loop until end of string or end of count.
    while( *pString && (Count != 0) )
    {
        s32 c = *pString++;

        // Check for embedded color code.
        if( (c & 0xFF00) == 0xFF00 )
        {
            // Skip 2nd character in embedded color code.
            pString++;

            // Decrease character count one extra for 2nd char in code.
            Count--;
        }
        else
        // Check for newline.
        if( c == '\n' )
        {
            Height += m_Height;
        }

        // Decrease character count.
        Count--;
    }

    // Return height
    return( Height );

}

//=========================================================================

const font::Character& font::GetCharacter( s32 Index ) const
{
    ASSERT( (Index >= 0) && (Index < 256) );

    return m_Characters[Index];
}

//=========================================================================

void font::RenderText( const irect&  Rect, 
                                u32     Flags, 
                          const xcolor& aColor, 
                          const xwchar* pString, 
                                xbool   IgnoreEmbeddedColor ) const
{
    s32     c;
    s32     tx       = Rect.l;
    s32     ty       = Rect.t;
    s32     iStart   = 0;
    s32     iEnd     = 0;
    s32     Width;
    s32     Height;    
    xcolor  Color    = aColor;

    #ifdef TARGET_PC
        vector2 uv0;
        vector2 uv1;   
        vector2 Size( 0, (f32)m_Height );
//        f32     BmWidth  = 1.0f / (f32)m_BmWidth;
//        f32     BmHeight = 1.0f / (f32)m_BmHeight;

        // Prepare to draw characters.
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER );
        draw_SetTexture( m_Bitmap );

        // Turn off BILINEAR.
//        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
//        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
//        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_POINT );
    #endif

    #ifdef TARGET_PS2
        // Setup Texture
        vram_Activate( m_Bitmap );
        gsreg_Begin();
        gsreg_SetClamping( TRUE );
        gsreg_SetMipEquation( FALSE, 1.0f, 0, MIP_MAG_POINT, MIP_MIN_POINT );
        gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC, C_DST, A_SRC, C_DST) );
        gsreg_SetZBuffer(FALSE);
        gsreg_End();

        // Build GIF Tags
        s_PGIF.Build( GIF_MODE_REGLIST, 2, 1, 0, 0, 0, 1 );
        s_PGIF.Reg  ( GIF_REG_PRIM, GIF_REG_NOP );
        s_GIF.Build ( GIF_MODE_REGLIST, 6, 0, 0, 0, 0, 1 );
        s_GIF.Reg   ( GIF_REG_RGBAQ, GIF_REG_UV, GIF_REG_XYZ2, GIF_REG_UV, GIF_REG_XYZ2, GIF_REG_NOP );

        // Compute size of header and skip over
        s_pHeader = DLStruct(header);
        s_NChars  = 0;
    #endif

    // Get size for vertical positioning.
    Height = TextHeight( pString );

    // Position start vertically.
    if( Flags & v_center )
    {
        ty += (Rect.GetHeight() - Height + 4) / 2;
    }
    else if( Flags & v_bottom )
    {
        ty += (Rect.GetHeight() - Height);
    }

    // Render strips of text on same line.
    while( pString[iStart] )
    {
        if( pString[iStart] == '\n' )
        {
            iEnd = iStart;
        }
        else
        {
            // Find end of line.
            iEnd = iStart+1;
            while( pString[iEnd] && (pString[iEnd] != '\n') )
                iEnd++;
        }

        // Determine width of line.
        Width = TextWidth( &pString[iStart], iEnd-iStart );

        // Adjust lateral position for alignment flags.
        if( Flags & h_center )
        {
            tx = Rect.l + (Rect.GetWidth() - Width) / 2;
        }
        else if( Flags & h_right )
        {
            tx = Rect.l + (Rect.GetWidth() - Width);
        }
        else
        {
            tx = Rect.l;
        }

        // Check for justification when clipping.
        if( Width > Rect.GetWidth() )
        {
            if( Flags & clip_l_justify ) 
                tx = Rect.l;
            else
            if( Flags & clip_r_justify ) 
                tx = Rect.r - Width;
        }

        // Render each character.
        for( ; iStart < iEnd; iStart++ )
        {
            c = pString[iStart];

            // Look for an embedded color code.
            if( (c & 0xFF00) == 0xFF00 )
            {
                if( IgnoreEmbeddedColor )
                {
                    iStart++;
                }
                else
                {
                    Color.R = (c & 0x00FF);
                    iStart++;
                    c = pString[iStart];
                    Color.G = (c & 0xFF00) >> 8;
                    Color.B = (c & 0x00FF);
                }
                continue;
            }

            s32 x  = m_Characters[c].X;
            s32 y  = m_Characters[c].Y;
            s32 w  = m_Characters[c].W;

            #ifdef TARGET_PC
            {
                f32 u0 = (x            + 0.1f) / m_BmWidth;
                f32 u1 = (x + w        + 0.1f) / m_BmWidth;
                f32 v0 = (y            + 0.1f) / m_BmHeight;
                f32 v1 = (y + m_Height + 0.1f) / m_BmHeight;

                Size.X = (f32)w;
                uv0.Set( u0, v0 );
                uv1.Set( u1, v1 );

                draw_SpriteUV( vector3((f32)tx,(f32)ty,10.0f), Size, uv0, uv1, Color );
            }
            #endif
            
            #ifdef TARGET_PS2
            {
	            s32         X0,Y0,X1,Y1;
                char_info*  pCH;

	            X0 = (OFFSET_X<<4) + ((tx)<<4);
	            Y0 = (OFFSET_Y<<4) + ((ty)<<4);
	            X1 = (OFFSET_X<<4) + ((tx+w)<<4);
	            Y1 = (OFFSET_Y<<4) + ((ty+m_Height)<<4);

                pCH        = DLStruct(char_info);
	            pCH->Color = SCE_GS_SET_RGBAQ( Color.R>>1, Color.G>>1, Color.B>>1, Color.A>>1, 0x3F800000 );
	            pCH->T0    = SCE_GS_SET_UV( (x<<4)+8, (y<<4)+8 );
	            pCH->P0    = SCE_GS_SET_XYZ(X0,Y0,0xFFFFFFFF);
	            pCH->T1    = SCE_GS_SET_UV( ((x+w)<<4)+8, ((y+m_Height)<<4)+8 );
	            pCH->P1    = SCE_GS_SET_XYZ(X1,Y1,0xFFFFFFFF);

                s_NChars++;
            }
            #endif

            tx += w + 1;
        }

        // Process newline.
        if( pString[iStart] == '\n' )
        {
            ty += m_Height;
            iStart++;
        }
    }

    #ifdef TARGET_PC
    draw_End();
    #endif

    #ifdef TARGET_PS2
    // Render
    s_pHeader->DMA.SetCont( sizeof(header) - sizeof(dmatag) + (s_NChars * sizeof(char_info)) );
    s_pHeader->DMA.MakeDirect();
    s_pHeader->PGIF      = s_PGIF;
    s_pHeader->GIF       = s_GIF;
    s_pHeader->GIF.NLOOP = s_NChars; 

    s_pHeader->Prim = SCE_GS_SET_PRIM(
                            GIF_PRIM_SPRITE,    // type of primative
                            0,    // shading method (flat, gouraud)
                            1,    // texture mapping (off, on)
                            0,    // fogging (off, on)
                            1,    // alpha blending (off, on)
                            0,    // 1 pass anti-aliasing (off, on)
                            1,    // tex-coord spec method (STQ, UV)
                            0,    // context (1 or 2)
                            0 );  // fragment value control (normal, fixed)
    #endif
}

//=========================================================================

void font::RenderText( const irect&  Rect, 
                                u32     Flags, 
                                s32     Alpha, 
                          const xwchar* pString ) const
{
    xcolor Color = XCOLOR_PURPLE;
    Color.A = Alpha;
    RenderText( Rect, Flags, Color, pString, FALSE );
}

//=========================================================================

void font::RenderText( const irect&  R, 
                                u32     Flags, 
                          const xcolor& Color, 
                          const char*   pString ) const
{
    xwstring t( pString );
    RenderText( R, Flags, Color, (const xwchar*)t );
}

//=========================================================================

