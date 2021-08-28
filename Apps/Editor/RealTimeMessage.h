#if !defined(AFX_REALTIMEMESSAGE_H__C7B569C2_8DE7_44C4_80E1_90BF3D8FDFFA__INCLUDED_)
#define AFX_REALTIMEMESSAGE_H__C7B569C2_8DE7_44C4_80E1_90BF3D8FDFFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RealTimeMessage.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CRealTimeMessage thread

class CRealTimeMessage : public CWinThread
{
public:

    CRealTimeMessage( HWND hWnd, s32 SleepMiliseconds, u32 Message );           
	CRealTimeMessage();           // protected constructor used by dynamic creation
	virtual ~CRealTimeMessage();

protected:
    HWND        m_hWnd  ;
    s32         m_WakeUp;
    u32         m_Message;

protected:

	DECLARE_DYNCREATE(CRealTimeMessage)


/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRealTimeMessage)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CRealTimeMessage)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REALTIMEMESSAGE_H__C7B569C2_8DE7_44C4_80E1_90BF3D8FDFFA__INCLUDED_)
