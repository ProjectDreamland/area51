// TextEditorView.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "editor.h"
#include "TextEditorView.h"
#include "TextEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TextEditorView

IMPLEMENT_DYNCREATE(TextEditorView, CView)

TextEditorView::TextEditorView()
{
}

TextEditorView::~TextEditorView()
{
}


BEGIN_MESSAGE_MAP(TextEditorView, CView)
	//{{AFX_MSG_MAP(TextEditorView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TextEditorView drawing

void TextEditorView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// TextEditorView diagnostics

#ifdef _DEBUG
void TextEditorView::AssertValid() const
{
	CView::AssertValid();
}

void TextEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TextEditorView message handlers

BOOL TextEditorView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

    
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void TextEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
    CRect R;
	GetClientRect( R );

    CTextEditor* pEditor = (CTextEditor*)GetParent();
    pEditor->m_rtf.MoveWindow( R );    	
}

int TextEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
    CTextEditor* pEditor = (CTextEditor*)GetParent();

    if( pEditor->m_rtf.Create( WS_VSCROLL | WS_CLIPCHILDREN | WS_CHILD | 
                      WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NOHIDESEL |
                      ES_WANTRETURN | ES_AUTOVSCROLL, 
                      CRect(10,10,100,200), this, IDW_PROJ_TEXT_RICHEDITCTRL ) == -1)
    {
        TRACE0("Failed to create Rich edit control window\n");
        return -1;
    }
    
    pEditor->m_rtf.SetFontName(pEditor->m_strFontName);
    pEditor->m_rtf.SetFontSize(atoi(pEditor->m_strFontSize)*10);
    pEditor->m_rtf.SetSelectionBold();

	return 0;
}
