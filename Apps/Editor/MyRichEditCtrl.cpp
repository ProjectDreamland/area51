#include "BaseStdAfx.h"
#include "MyRichEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyRichEditCtrl

CMyRichEditCtrl::CMyRichEditCtrl()
{
}

CMyRichEditCtrl::~CMyRichEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyRichEditCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(CMyRichEditCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyRichEditCtrl message handlers


// My callback procedure that writes a file content
//----------------------------------------------------------------------------
// Function CMyRichEditCtrl::MyStreamInCallback
// @func    writes the file content to the rich edit control
// @parm    DWORD  | dwCookie | handle to an open file
// @parm    LPBYTE | pbBuff   | Pointer to a buffer to read from to.
//                              For a stream-in (read) operation, 
//								the callback function fills this 
//								buffer with data to transfer into the
//								rich edit control
// @parm    LONG   | cb       | Number of bytes to read 
// @parm    LONG   | *pcb     | Pointer to a variable that the 
//								callback function sets to the number 
//								of bytes actually read  
//----------------------------------------------------------------------------

static
DWORD CALLBACK MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   CFile* pFile = (CFile*) dwCookie;
   ASSERT_KINDOF(CFile,pFile);
   *pcb = pFile->Read(pbBuff, cb);
   return 0;
}

void CMyRichEditCtrl::SetRtf(CFile* pInputFile )
{
	// Read the text in
	EDITSTREAM es;
	es.dwError     = 0;
	es.pfnCallback = MyStreamInCallback;
	es.dwCookie    = (DWORD)pInputFile;
	StreamIn(SF_RTF, es);	// Do it.

    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
}

void CMyRichEditCtrl::SetText(CFile* pInputFile )
{
    ASSERT( 0 && "Untested" );

	// Read the text in
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = MyStreamInCallback;
	es.dwCookie = (DWORD)pInputFile;
	StreamIn(SF_TEXT, es);	// Do it.

    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
}


void CMyRichEditCtrl::SetSelectionBold()
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	cf.dwMask = CFM_BOLD;
	if ( IsBold() == FALSE )	
		 cf.dwEffects |= CFE_BOLD;
	else 
		 cf.dwEffects &= ~CFE_BOLD;
	SetSelectionCharFormat(cf);
}

BOOL CMyRichEditCtrl::IsBold()
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);
	return ((cf.dwMask & CFM_BOLD ) && ((cf.dwEffects & CFE_BOLD)) );
}

void CMyRichEditCtrl::SetSelectionItalic()
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	cf.dwMask = CFM_ITALIC;
	if ( IsItalic() == FALSE )	
		 cf.dwEffects |= CFE_ITALIC;
	else 
		 cf.dwEffects &= ~CFE_ITALIC;

	SetSelectionCharFormat(cf);
}

BOOL CMyRichEditCtrl::IsItalic()
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);
	return ((cf.dwMask & CFM_ITALIC ) && ((cf.dwEffects & CFE_ITALIC)) );
}

void CMyRichEditCtrl::SetSelectionUnderline()
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	cf.dwMask = CFM_UNDERLINE;
	if ( IsUnderline() == FALSE )	
		 cf.dwEffects |= CFE_UNDERLINE;
	else 
		 cf.dwEffects &= ~CFE_UNDERLINE;

	SetSelectionCharFormat(cf);
}

BOOL CMyRichEditCtrl::IsUnderline()
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);
	return ((cf.dwMask & CFM_UNDERLINE ) && ((cf.dwEffects & CFE_UNDERLINE)) );
}

void CMyRichEditCtrl::SetColor()
{
    ::CColorDialog dlg;
	
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	if (cf.dwEffects & CFE_AUTOCOLOR) 
		cf.dwEffects -= CFE_AUTOCOLOR;
	
	// Get a color from the common color dialog box
	if ( dlg.DoModal() == IDOK )
	{
		cf.crTextColor = dlg.GetColor();
	}
	cf.dwMask = CFM_COLOR;	

	SetSelectionCharFormat(cf);
}

void CMyRichEditCtrl::SetParagraphLeft()
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
	pf.dwMask = PFM_ALIGNMENT;    
	pf.wAlignment = PFA_LEFT;
	
	SetParaFormat(pf);
}

void CMyRichEditCtrl::SetParagraphRight()
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
	pf.dwMask = PFM_ALIGNMENT;    
	pf.wAlignment = PFA_RIGHT;
	
	SetParaFormat(pf);
}

void CMyRichEditCtrl::SetParagraphCenter()
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
	pf.dwMask = PFM_ALIGNMENT;    
	pf.wAlignment = PFA_CENTER;
	
	SetParaFormat(pf);
}

void CMyRichEditCtrl::SetParagraphBulleted()
{
    PARAFORMAT pf;
	GetParaFormat(pf);

	if ( (pf.dwMask & PFM_NUMBERING) && (pf.wNumbering == PFN_BULLET) )
	{
		pf.wNumbering = 0;
		pf.dxOffset = 0;
		pf.dxStartIndent = 0;
		pf.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	}
	else
	{
		pf.wNumbering = PFN_BULLET;
		pf.dwMask = PFM_NUMBERING;
		if (pf.dxOffset == 0)
		{
			pf.dxOffset = 4;
			pf.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
		}
	}

	SetParaFormat(pf);
}

void CMyRichEditCtrl::SetFontName(CString sFontName)
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	// Set the font name.
	strcpy (cf.szFaceName, sFontName);
	cf.dwMask |=CFM_FACE;
	SetSelectionCharFormat(cf);
}

void CMyRichEditCtrl::SetFontSize(int nFontSize)
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	nFontSize *= 2;	// convert from to twips
	cf.yHeight = nFontSize ;
	cf.dwMask = CFM_SIZE;

	SetSelectionCharFormat ( cf );
}

void CMyRichEditCtrl::SetColor(COLORREF clr)
{
    CHARFORMAT cf;
    GetSelectionCharFormat(cf);

	if (cf.dwEffects & CFE_AUTOCOLOR) 
		cf.dwEffects -= CFE_AUTOCOLOR;
	
		cf.crTextColor = clr;

		cf.dwMask = CFM_COLOR;	

	SetSelectionCharFormat(cf);
}

BOOL CMyRichEditCtrl::CanRedo( void )
{
    return SendMessage( EM_CANREDO, 0, 0 );
}

void CMyRichEditCtrl::Redo( void )
{
    SendMessage( EM_REDO, 0, 0 );
}

BOOL CMyRichEditCtrl::IsParagraphLeft( void )
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
    return ( (pf.dwMask & PFM_ALIGNMENT) && ( pf.wAlignment == PFA_LEFT)) ;
}

BOOL CMyRichEditCtrl::IsParagraphCenter( void )
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
    return ( (pf.dwMask & PFM_ALIGNMENT) && ( pf.wAlignment == PFA_CENTER)); 
}

BOOL CMyRichEditCtrl::IsParagraphRight( void )
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
    return ( (pf.dwMask & PFM_ALIGNMENT) && ( pf.wAlignment == PFA_RIGHT)) ;
}

BOOL CMyRichEditCtrl::IsBulleted( void )
{
    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
    return ( (pf.dwMask & PFM_NUMBERING) && (pf.wNumbering == PFN_BULLET) );
}

void CMyRichEditCtrl::SetFont( void )
{
    ::CFontDialog dlg( NULL, CF_BOTH | CF_EFFECTS );
	if ( dlg.DoModal() != IDOK )
		return;

	if ( dlg.IsUnderline() )
	{
		SetSelectionUnderline ();
	}
	
	CString faceName = dlg.GetFaceName();
	SetFontName( faceName ) ;
	
	COLORREF clr= dlg.GetColor ();
	SetColor(clr); 

	int nPointSize = dlg.GetSize();
	SetFontSize( nPointSize ) ;    
}

void CMyRichEditCtrl::ClearAll( void )
{
	SetSel(0, -1);
	Clear();	
	SetModify(FALSE); 

    PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
    GetParaFormat( pf );
}

static
DWORD CALLBACK MyStreamOutCallback(
    DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   CFile* pFile = (CFile*) dwCookie;
   pFile->Write(pbBuff, cb);
   *pcb = cb;
   return 0;
}


BOOL CMyRichEditCtrl::FileOpen( CString filename ) 
{
    CFile fis;
    if( !fis.Open(filename, CFile::modeRead | CFile::shareExclusive   ) )
    {
        return FALSE;
    }

    ClearAll();
    SetRtf(&fis);
    fis.Close();		

    // if I open a file, don't ask me if to save the 
    // content or not
    SetModify(FALSE); 

    return TRUE;
}

BOOL CMyRichEditCtrl::FileSave( CString filename ) 
{
    // I define the file dialog
    CFile fis;

    EDITSTREAM es;
    es.dwCookie = (DWORD) &fis;
    es.pfnCallback = MyStreamOutCallback; 

    // I open the file with tne modeWrite and modeCreate mode
    if( !fis.Open(filename, CFile::modeCreate | CFile::modeWrite) )
    {
        return FALSE;
    }
    
    es.dwCookie = (DWORD) &fis;
    es.pfnCallback = MyStreamOutCallback;
    // I put the content in the file
    StreamOut(SF_RTF, es);
    fis.Close();

    SetModify(FALSE); 

    return TRUE;
}

BOOL CMyRichEditCtrl::ImportText ( CString filename )
{    
    FILE* Fp = fopen( filename, "rt" );
    if( Fp == NULL )
    {
        return FALSE;
    }

    ClearAll();

    while( feof(Fp) == FALSE ) 
    {
        char C[256];

        fgets( C, 256, Fp );

        ReplaceSel( C );
    }

    fclose( Fp );

    SetModify(FALSE); 

    return TRUE;
}

