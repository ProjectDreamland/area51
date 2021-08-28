#if !defined(AFX_CXBMP_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_)
#define AFX_CXBMP_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Cxbmp.h : header file
//

class xbitmap;

/////////////////////////////////////////////////////////////////////////////
// Cxbmp

class Cxbmp
{
public:

// Construction
public:
	Cxbmp();

// Attributes
protected:
    s32         m_Width;
    s32         m_Height;
    u32*        m_pData;
    xbitmap*    m_pBitmap;

// Implementation
public:
	virtual ~Cxbmp();

    void        SetSize         ( s32 Width, s32 Height );
    xbitmap*    GetBitmap       ( void );
    s32         GetWidth        ( void ) { return m_Width; }
    s32         GetHeight       ( void ) { return m_Height; }

    void        Clear           ( xcolor Color );
    void        HorizontalLine  ( s32 y, s32 x1, s32 x2, xcolor Color );
    void        VerticalLine    ( s32 x, s32 y1, s32 y2, xcolor Color );
    void        SolidRect       ( const CRect& r, xcolor Color );
    void        FramedRect      ( const CRect& r, xcolor ColorFill, xcolor ColorFrame );
    void        CopyLine        ( s32 ySrc, s32 yDst );

    void        Draw            ( CDC* pDC, CRect& r );

// Private
private:
    void        FreeBitmap      ( void );
    void        AllocateBitmap  ( void );
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_CXBMP_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_)
