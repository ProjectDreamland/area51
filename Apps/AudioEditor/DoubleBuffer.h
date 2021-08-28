// DoubleBuffer.h
//

#if !defined(AFX_DOUBLEBUFFER_H__38CD6D41_CAF4_4B1B_98B1_45AF73A2F963__INCLUDED_)
#define AFX_DOUBLEBUFFER_H__38CD6D41_CAF4_4B1B_98B1_45AF73A2F963__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//==============================================================================
// DOUBLE BUFFER
//==============================================================================

class CDoubleBuffer
{
public:
                        CDoubleBuffer   ( CDC*&     pDC );                  // Construct double buffer
                        CDoubleBuffer   ( CDC*&     pDC, s32 AddWidth );    // Construct double buffer with some additional width
                       ~CDoubleBuffer   ( void );                           // Destruct double buffer

        const CRect&    GetRect         ( void );
        void            SetFullScreen   ( void );

public:
        int         w;                                      // Width of double buffer
        int         h;                                      // Height of double buffer

protected:
        CDC*        m_pDC;                                  // Real DC that has been double buffered
        CPaintDC*   m_pPaintDC;                             // Real DC that has been double buffered
        CDC         m_BufferedDC;                           // The buffering DC
        CBitmap     m_BufferedBitmap;                       // The buffering Bitmap
        CRect       m_Rect;                                 // Rect for client that owns the DC
        CBitmap*    m_pOldBitmap;                           // Bitmap from selecting buffering Bitmap
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_DOUBLEBUFFER_H__38CD6D41_CAF4_4B1B_98B1_45AF73A2F963__INCLUDED_)
