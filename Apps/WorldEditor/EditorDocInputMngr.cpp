// EditorDoc.cpp : implementation of the CEditorDoc class
//
#include "StdAfx.h"

#include "EditorDocInputMngr.h"
#include "ResourceBrowserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================

void CInputSettings::OnEnumProp ( prop_enum&    List )
{
    List.PropEnumHeader  ( "Input Manager Controls", "Control for managing input mappings.", 0 );
}

//=========================================================================

xbool CInputSettings::OnProperty ( prop_query&   I    )
{ 
    return FALSE;
}