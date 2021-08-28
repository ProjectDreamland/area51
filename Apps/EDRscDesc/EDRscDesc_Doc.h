#if !defined(AFX_EDRSCDESC_DOC_H__96C60DF3_831B_4864_BB92_01740AA8E176__INCLUDED_)
#define AFX_EDRSCDESC_DOC_H__96C60DF3_831B_4864_BB92_01740AA8E176__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EDRscDesc_Doc.h : header file
//

#include "RSCDesc.hpp"
#include "BaseDocument.h"

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_Doc document
class CPropertyEditorDoc;
class EDRscDesc_Frame;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define		WM_USER_MSG_LBUTTON_DBLCLK  WM_USER+853

class EDRscDesc_Doc : public CBaseDocument
{
/////////////////////////////////////////////////////////////////////////////
public:
    
    enum update
    {
        UPDATE_NULL,
        UPDATE_ADDED_NEW_RESOURCE,
    };

/////////////////////////////////////////////////////////////////////////////
public:

           void         FrameInit                   ( CPropertyEditorDoc* pRopEditor );
           void         Build                       ( void );
           void         RebuildAll                  ( void );
    static void         StopBuild                   ( void );
    static void         GetTypeList                 ( xarray<xstring>& Types );
    static void         RefreshViews                ( xbool bAllViews = TRUE );
           void         AddRscDesc                  ( const char* pType );
           void         SaveActive                  ( void );
           void         StartStopEdit               ( void );
           void         SetActiveDesc               ( const char* pDesc );
           rsc_desc&    GetActive                   ( void );
           xbool        IsSelectedBeenEditedLocal   ( void );
           void         CleanSelected               ( void );
           void         CheckOutSelected            ( void );

           xbool        IsCompileNintendo           ( void );
           xbool        IsCompilePS2                ( void );
           xbool        IsCompileXBox               ( void );
           xbool        IsCompilePC                 ( void );
           xbool        IsVerboseMode               ( void );
           xbool        IsColorMipsMode             ( void );

           void         ToggleCompileNintendo       ( void );
           void         ToggleCompilePS2            ( void );
           void         ToggleCompileXBox           ( void );
           void         ToggleCompilePC             ( void );
           void         ToggleVerboseMode           ( void );
           void         ToggleColorMipsMode         ( void );

           void         ForceCompileNintendo        ( void );
           void         ForceCompilePS2             ( void );
           void         ForceCompileXBox            ( void );
           void         ForceCompilePC              ( void );
           void         ForceCompileNintendoOff    ( void );
           void         ForceCompilePS2Off         ( void );
           void         ForceCompileXBoxOff        ( void );
           void         ForceCompilePCOff          ( void );

           
           void         DeleteSelectedResource      ( void );
           void         Refresh                     ( void );

           void         ScanResources               ( void );

   virtual void         OnProjectOpen               ( void );
   virtual void         OnProjectClose              ( void );
   virtual void         OnProjectRefresh            ( void );


/////////////////////////////////////////////////////////////////////////////
protected:

    void         SaveNode   ( rsc_desc& RscDesc );

/////////////////////////////////////////////////////////////////////////////
protected:
    xbool               m_bCompileNintendo;
    xbool               m_bCompilePS2;     
    xbool               m_bCompileXBox;    
    xbool               m_bCompilePC;      
    xbool               m_bVerboseMode;
    xbool               m_bColorMipsMode;

    CPropertyEditorDoc* m_pPropEditor;
    xarray<rsc_desc*>   m_LocalEdited;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
// Implementation
	virtual ~EDRscDesc_Doc();
	DECLARE_DYNCREATE(EDRscDesc_Doc)
	EDRscDesc_Doc();           // protected constructor used by dynamic creation

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EDRscDesc_Doc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(EDRscDesc_Doc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDRSCDESC_DOC_H__96C60DF3_831B_4864_BB92_01740AA8E176__INCLUDED_)
