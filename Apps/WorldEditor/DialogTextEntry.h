#pragma once

class CDialogTextEntry : public CDialog
{
	DECLARE_DYNAMIC(CDialogTextEntry)

public:
	CDialogTextEntry(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogTextEntry();

// Dialog Data
	//{{AFX_DATA(CDialogTextEntry)
	enum { IDD = IDD_CAMERA_FAVORITE_ADD };
    CString m_Name;
	//}}AFX_DATA

    CString m_Caption;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog( );

	DECLARE_MESSAGE_MAP()
public:
};
