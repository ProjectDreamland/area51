// EditorSoundView.cpp

#include "StdAfx.h"
#include "EditorSoundView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorView.h"
#include "Resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEditorSoundView

IMPLEMENT_DYNCREATE(CEditorSoundView, CPaletteView)

BEGIN_MESSAGE_MAP(CEditorSoundView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorSoundView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_CREATE_SOUNDNAV_NODE, OnButtonCreateSoundNode)
	ON_COMMAND(ID_CREATE_SOUNDNAV_CONNECTION, OnButtonCreateSoundConnection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorSoundView diagnostics

#ifdef _DEBUG
void CEditorSoundView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorSoundView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CEditorSoundView::CEditorSoundView() : m_CreateSoundConnectionMode (false)
{

}

//=========================================================================

CEditorSoundView::~CEditorSoundView()
{

}

//=========================================================================

int CEditorSoundView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_SOUNDVIEW_FILTER;   
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_SoundTree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                           TVS_EDITLABELS | TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDC_SOUND_TREE_LIST))
    {
		TRACE0("Failed to create tree\n");
        return -1;	      
    }

	return 0;
}

//=========================================================================

void CEditorSoundView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorSoundView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_SoundTree.MoveWindow(0,size.cy,cx,cy - size.cy);
}

//=========================================================================

void CEditorSoundView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//=========================================================================

void CEditorSoundView::OnButtonCreateSoundNode() 
{ 
//    guid aGuid = soundnav_editor::GetSoundSoundEditor()->CreateSoundNode();

//    g_WorldEditor.SelectObject(aGuid);
//    GetDocument()->GetFramePointer()->GetEditorView()->EnterMovementMode();
}

//=========================================================================

void CEditorSoundView::OnButtonCreateSoundConnection() 
{
	m_CreateSoundConnectionMode = !m_CreateSoundConnectionMode;
}

//=========================================================================

void CEditorSoundView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Sound View");
    }
}

//=========================================================================
