#if !defined(AFX_CGridItemInfo_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_)
#define AFX_CGridItemInfo_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// GridItemInfo.h : header file
//
// container for each row

#include <afxtempl.h>

class CGridItemInfo 
{
public:

	CGridItemInfo();
	~CGridItemInfo();

	//text accessors
	void SetItemText(const CString& strItem);
	void AddSubItemText(const CString& strSubItem);
	void SetSubItemText(int iSubItem, const CString& strSubItem);
	
	const CString& GetItemText(void);
	CString GetSubItem(int iSubItem);

	int GetItemCount(void);

	enum CONTROLTYPE {
		GCT_STRING_EDIT,		//default 
		GCT_NUMERIC_EDIT,
		GCT_COMBOBOX, 
		GCT_DIR_BUTTON,
		GCT_COLOR_BUTTON,
		GCT_3D_COORDINATE,
		GCT_FLOAT_EDIT,
		GCT_ROTATION_EDIT,
		GCT_DEGREE_EDIT,
        GCT_GUID_EDIT,
		GCT_BUTTON,
		GCT_BOUNDING_BOX,
        GCT_BOOL,
        GCT_EXTERNAL,
        GCT_NULL_ENTRY
	};

	enum COMPOUND_OBJ_NOTIFICATION {
		NTFY_NONE,
		NTFY_CHILD,
		NTFY_PARENT,
		NTFY_BOTH,
	};

	//makes entire row readonly
	BOOL IsReadOnly() { return m_bReadOnly; }
	void SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; }
	BOOL IsHeader() { return m_bHeader; }
    void SetIsHeader(BOOL bHeader) { m_bHeader = bHeader; }
    void SetDataIndex(int iIndex) { m_iDataIndex = iIndex; }
    int GetDataIndex() { return m_iDataIndex; }

	//all cols in this row is default edit
	void SetControlType(CONTROLTYPE enumCtrlType, int nCol = -1);
	BOOL GetControlType(int nCol, CONTROLTYPE& controlType);
	void SetListData(int iSubItem, CStringList *strInitArr);
	BOOL GetListData(int iSubItem, CStringList*& pList);

	//copy
	void CopyObjects(CGridItemInfo* pItemInfo);
	void CopyControls(CGridItemInfo* pItemInfo);

	void SetNotification(COMPOUND_OBJ_NOTIFICATION type) { m_eNotify = type; }
	COMPOUND_OBJ_NOTIFICATION CheckNotificationStatus() { return m_eNotify; }

private:
	LPARAM				m_lParam;
	CONTROLTYPE			m_enumCtrlType; 
	CString				m_strItemName;		//col 0 text
	CStringArray		m_SubItems;			//col 1 to col N text
	BOOL				m_bReadOnly;
    BOOL                m_bHeader;
    int                 m_iDataIndex;
	COMPOUND_OBJ_NOTIFICATION	m_eNotify;

	CMap<int,int, CONTROLTYPE, CONTROLTYPE&>	m_controlType;
	CMap<int,int, CStringList*, CStringList*>	m_listdata;			//listbox
};


#endif // !defined(AFX_CGridItemInfo_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_)
