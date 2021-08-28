// CMyMDIWndTab : header file
/////////////////////////////////////////////////////////////////////////////

#ifndef __MYMDIWNDTAB_H
#define __MYMDIWNDTAB_H

/////////////////////////////////////////////////////////////////////////////
// CMyMDIWndTab class

class CMyMDIWndTab : public CXTMDIWndTab
{
// Construction / destruction
public:

	// Constructs a CMyMDIWndTab object.
	CMyMDIWndTab();

	// Destroys a CMyMDIWndTab object, handles cleanup and de-allocation.
	virtual ~CMyMDIWndTab();

// Member variables
protected:

    virtual void OnAddPadding(CXTString& strLabelText);

// Member functions
public:

};

/////////////////////////////////////////////////////////////////////////////

#endif // __MYMDIWNDTAB_H

