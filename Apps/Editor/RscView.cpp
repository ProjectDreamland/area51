// RscView.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "Editor.h"
#include "RscView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RscView

IMPLEMENT_DYNCREATE(RscView, CXTListView)

RscView::RscView()
{
}

RscView::~RscView()
{
}


BEGIN_MESSAGE_MAP(RscView, CXTListView)
	//{{AFX_MSG_MAP(RscView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RscView drawing

void RscView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// RscView diagnostics

#ifdef _DEBUG
void RscView::AssertValid() const
{
	CXTListView::AssertValid();
}

void RscView::Dump(CDumpContext& dc) const
{
	CXTListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// RscView message handlers

struct col_data 
{ 
	LPCTSTR name; 
	int width; 
	int fmt;
    XT_DATA_TYPE type;
};

static col_data columns[] =
{
	{ _T("Name"),              200, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Type"),              100, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Size"),               80, LVCFMT_RIGHT, DT_INT      },
	{ _T("Modified"),          120, LVCFMT_LEFT,  DT_DATETIME },
	{ _T("Compile(T/F)"),       70, LVCFMT_RIGHT, DT_STRING   },
	{ _T("Internal Dep."),     100, LVCFMT_RIGHT, DT_STRING   },
	{ _T("External Dep."),      90, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Compiler commands"), 200, LVCFMT_LEFT,  DT_STRING   }
};

struct file_info
{
	LPCSTR info[_countof(columns)];
};

static file_info infoarr[] =
{
	// Name                 Type                 Size         Modified        Compile(T/F)   Packed     Attributes  Path
	{ "GUI_WinZip.aps",    "APS file",          "134908",    "18/06/2002",     "A",     "45965",    "A",        "" },
	{ "GUI_WinZip.clw",    "CLW file",          "4474",      "18/06/2002",     "A",     "1029",     "A",        "" },
	{ "GUI_WinZip.cpp",    "C++ Source File",   "4364",      "06/06/2002",     "R",     "1679",     "RA",       "" },
	{ "GUI_WinZip.dsp",    "Project File",      "5231",      "18/06/2002",     "A",     "1276",     "A",        "" },
	{ "GUI_WinZip.dsw",    "Project Workspace", "626",       "07/06/2002",     "R",     "246",      "RA",       "" },
	{ "GUI_WinZip.h",      "C Header File",     "1400",      "06/06/2002",     "R",     "640",      "RA",       "" },
	{ "GUI_WinZip.ico",    "Icon",              "1078",      "06/06/2002",     "R",     "298",      "RA",       "res" },
	{ "GUI_WinZip.obj",    "Intermediate file", "24855",     "18/06/2002",     "A",     "7785",     "A",        "Debug" },
	{ "GUI_WinZip.pch",    "Intermediate file", "8326332",   "18/06/2002",     "A",     "689319",   "A",        "Debug" },
	{ "GUI_WinZip.rc",     "Resource Template", "16370",     "18/06/2002",     "A",     "3644",     "A",        "" },
	{ "GUI_WinZip.rc2",    "RC2 file",          "402",       "06/06/2002",     "R",     "175",      "RA",       "res" },
	{ "GUI_WinZip.res",    "RES file",          "93564",     "18/06/2002",     "A",     "35772",    "A",        "Debug" },
	{ "GUI_WinZip.Tags.WW","WW file",           "1044",      "18/06/2002",     "A",     "208",      "A",        "" },
	{ "GUI_WinZipD.pdb",   "Intermediate file", "508928",    "18/06/2002",     "A",     "20292",    "A",        "Debug" },
	{ "GUI_WinZipDoc.cpp", "C++ Source File",   "1822",      "06/06/2002",     "R",     "619",      "RA",       "" },
	{ "GUI_WinZipDoc.h",   "C Header File",     "1519",      "06/06/2002",     "R",     "666",      "RA",       "" },
	{ "GUI_WinZipDoc.ico", "Icon",              "1078",      "06/06/2002",     "R",     "352",      "RA",       "res" },
	{ "GUI_WinZipDoc.obj", "Intermediate file", "16481",     "18/06/2002",     "A",     "5272",     "A",        "Debug" },
	{ "GUI_WinZipView.cpp","C++ Source File",   "3680",      "18/06/2002",     "A",     "1295",     "A",        "" },
	{ "GUI_WinZipView.h",  "C Header File",     "1995",      "18/06/2002",     "A",     "838",      "A",        "" },
	{ "GUI_WinZipView.obj","Intermediate file", "27203",     "18/06/2002",     "A",     "8468",     "A",        "Debug" },
	{ "highclr1.bmp",      "Bitmap Image",      "35334",     "06/06/2002",     "R",     "11879",    "RA",       "res" },
	{ "highclr2.bmp",      "Bitmap Image",      "35334",     "06/06/2002",     "R",     "12594",    "RA",       "res" },
	{ "highclrsm1.bmp",    "Bitmap Image",      "5814",      "07/06/2002",     "R",     "4053",     "RA",       "res" },
	{ "highclrsm2.bmp",    "Bitmap Image",      "5814",      "07/06/2002",     "R",     "3758",     "RA",       "res" },
	{ "MainFrm.cpp",       "C++ Source File",   "8535",      "18/06/2002",     "A",     "2201",     "A",        "" },
	{ "MainFrm.h",         "C Header File",     "2153",      "17/06/2002",     "R",     "807",      "RA",       "" },
	{ "MainFrm.obj",       "Intermediate file", "41818",     "18/06/2002",     "A",     "11972",    "A",        "Debug" },
	{ "manifest.xml",      "XML Document",      "599",       "17/06/2002",     "R",     "329",      "RA",       "res" },
	{ "ReadMe.txt",        "Readme Document",   "4407",      "06/06/2002",     "R",     "1590",     "RA",       "" },
	{ "resource.h",        "C Header File",     "1713",      "17/06/2002",     "A",     "516",      "A",        "" },
	{ "StdAfx.cpp",        "C++ Source File",   "212",       "06/06/2002",     "R",     "145",      "RA",       "" },
	{ "StdAfx.h",          "C Header File",     "1148",      "06/06/2002",     "R",     "557",      "RA",       "" },
	{ "StdAfx.obj",        "Intermediate file", "113867",    "18/06/2002",     "A",     "48738",    "A",        "Debug" },
	{ "Toolbar.bmp",       "Bitmap Image",      "1078",      "06/06/2002",     "R",     "424",      "RA",       "res" },
	{ "vc60.idb",          "Intermediate file", "427008",    "18/06/2002",     "A",     "8543",     "A",        "Debug" },
	{ "vc60.pdb",          "Intermediate file", "651264",    "18/06/2002",     "A",     "2461",     "A",        "Debug" }
};

void RscView::OnInitialUpdate() 
{
	CXTListView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
    CListCtrl& listCtrl = GetListCtrl();

	int i;
	for (i = 0; i < _countof(columns); ++i)
	{
		listCtrl.InsertColumn(i, columns[i].name, columns[i].fmt, 
			columns[i].width);
	}

	SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    // Fill in fake data
	int j;
	for (i = 0; i < _countof(infoarr); ++i)
	{
		listCtrl.InsertItem(i, infoarr[i].info[0]);
		for (j = 1; j < _countof(columns); ++j)
		{
			listCtrl.SetItem(i, j, LVIF_TEXT, infoarr[i].info[j], 0, 0, 0, 0);
		}
	}

	SubclassHeader();
	GetFlatHeaderCtrl()->ShowSortArrow(TRUE);
    SortList(0, false);
    SetSortImage(0, false);
}

BOOL RscView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= LVS_EX_GRIDLINES | LVS_REPORT;
	return CXTListView::PreCreateWindow(cs);
}

bool RscView::SortList(
	// passed in from control, index of column clicked.
	int nCol,
	// passed in from control, true if sort order should 
	// be ascending.
	bool bAscending )
{
	CXTSortClass csc (&GetListCtrl(), nCol);
	csc.Sort(bAscending, columns[nCol].type);
	return true;
}
