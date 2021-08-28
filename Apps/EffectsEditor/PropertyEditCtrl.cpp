// PropertyEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "GridEditCtrl.h"
#include "PropertyEditCtrl.h"
#include "expression.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditCtrl

CPropertyEditCtrl::CPropertyEditCtrl(int iItem, int iSubItem, CString sInitText) :
CGridEditCtrl(iItem, iSubItem, sInitText),
m_enumType(PET_STRING),
m_fMin(0.0),
m_fMax(0.0),
m_iListLen(1)
{
}

CPropertyEditCtrl::~CPropertyEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CPropertyEditCtrl, CGridEditCtrl)
	//{{AFX_MSG_MAP(CPropertyEditCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditCtrl message handlers


void CPropertyEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//validate on enter, if not, beep?
	if( nChar == VK_RETURN )	
	{		
		if (!ValidateFormat())
		{
			Beep(1000,100);
			return;	
		}
	}
	
	CGridEditCtrl::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CPropertyEditCtrl::ValidateFormat() 
{
	BOOL bReturn = TRUE;
	CString strText;
	GetWindowText(strText);

	//do
	switch(m_enumType)
	{
		case PET_STRING:
			CheckSize(strText);
			break;	
		case PET_DEGREE:
			bReturn = TestDegree(strText);
			break;
		case PET_INT:
			bReturn = TestInt(strText, (int)m_fMin, (int)m_fMax);
			break;
		case PET_FLOAT:
			bReturn = TestFloat(strText, m_fMin, m_fMax);
			break;
		case PET_DEGREE_LIST:
			bReturn = TestDegreeList(strText, m_iListLen);
			break;
		case PET_INT_LIST:
			bReturn = TestIntList(strText, (int)m_fMin, (int)m_fMax, m_iListLen);
			break;
		case PET_FLOAT_LIST:
			bReturn = TestFloatList(strText, m_fMin, m_fMax, m_iListLen);
			break;
	}

	SetWindowText(strText);

	return bReturn;
}


BOOL CPropertyEditCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if( pMsg->message == WM_KEYDOWN )	
	{		
		if(pMsg->wParam == VK_RETURN)
		{			
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return 1;
		}	
	}
	
	return CGridEditCtrl::PreTranslateMessage(pMsg);
}

void CPropertyEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	if (ValidateFormat())
	{
		CGridEditCtrl::OnKillFocus(pNewWnd);  //let the GridEditCtrl take over
	}
	else
	{
		CEdit::OnKillFocus(pNewWnd);  
		DestroyWindow();
	}
}

BOOL CPropertyEditCtrl::CheckSize(CString &strText)
{
	BOOL bReturn = TRUE;

	//only check min max if set
	if (m_fMin != m_fMax)
	{
		int iLength = strText.GetLength();
		if (iLength < m_fMin)
		{
			bReturn = FALSE;
		}

		if (iLength > m_fMax)
		{
			bReturn = FALSE;
			strText = strText.Left(m_fMax);
		}
	}

	return bReturn;
}

////////////////////////////////////////////////////////////////////////////////////
// Static determination code
////////////////////////////////////////////////////////////////////////////////////

BOOL CPropertyEditCtrl::TestFloat(CString &strValue, float fMin, float fMax)
{
	float f = 0.0;
	if (!EvaluateFloat(strValue,f))
	{
		return FALSE;
	}
	
	//only check min max if set
	if (fMin != fMax)
	{
		if (f<fMin || f> fMax) 
		{
			return FALSE;
		}
	}
	strValue.Format("%g",f);

	return TRUE;
}

BOOL CPropertyEditCtrl::TestInt(CString &strValue, int iMin, int iMax)
{
	int i = 0;
	if (!EvaluateInt(strValue,i))
	{
		return FALSE;
	}

	//only check min max if set
	if (iMin != iMax)
	{
		if (i<iMin || i> iMax) 
		{
			return FALSE;
		}
	}
	strValue.Format("%d",i);

	return TRUE;
}

BOOL CPropertyEditCtrl::TestDegree(CString &strValue)
{
	float f = 0.0;
	if (!EvaluateFloat(strValue,f))
	{
		return FALSE;
	}
	
	//wrap the value (ie for degrees)
//	f = RAD_TO_DEG(x_ModAngle(DEG_TO_RAD(f)));
	strValue.Format("%g",f);

	return TRUE;
}

BOOL CPropertyEditCtrl::TestFloatList(CString &strValue, float fMin, float fMax, int iLength)
{
	BOOL bReturn = TRUE;
	CStringList lstStrings;
	if (ParseCompoundString(strValue, lstStrings, iLength))
	{
		CString strOutput;
		for (int i=0; i<iLength;i++)
		{
			CString strData = lstStrings.GetAt(lstStrings.FindIndex(i));
			if (!TestFloat(strData, fMin, fMax))
			{
				bReturn = FALSE;
			}
			//lstStrings.SetAt(lstStrings.FindIndex(i),strData);
			if (!strOutput.IsEmpty()) strOutput += ", ";
			strOutput += strData;
		}

		if (bReturn)
		{
			strValue=strOutput;
		}
	}
	else
	{
		bReturn = FALSE;
	}
	return bReturn;
}

BOOL CPropertyEditCtrl::TestIntList(CString &strValue, int iMin, int iMax, int iLength)
{
	BOOL bReturn = TRUE;
	CStringList lstStrings;
	if (ParseCompoundString(strValue, lstStrings, iLength))
	{
		CString strOutput;
		for (int i=0; i<iLength;i++)
		{
			CString strData = lstStrings.GetAt(lstStrings.FindIndex(i));
			if (!TestInt(strData, iMin, iMax))
			{
				bReturn = FALSE;
			}
			//lstStrings.SetAt(lstStrings.FindIndex(i),strData);
			if (!strOutput.IsEmpty()) strOutput += ", ";
			strOutput += strData;
		}

		if (bReturn)
		{
			strValue=strOutput;
		}
	}
	else
	{
		bReturn = FALSE;
	}
	return bReturn;
}

BOOL CPropertyEditCtrl::TestDegreeList(CString &strValue, int iLength)
{
	BOOL bReturn = TRUE;
	CStringList lstStrings;
	if (ParseCompoundString(strValue, lstStrings, iLength))
	{
		CString strOutput;
		for (int i=0; i<iLength;i++)
		{
			CString strData = lstStrings.GetAt(lstStrings.FindIndex(i));
			if (!TestDegree(strData))
			{
				bReturn = FALSE;
			}
			//lstStrings.SetAt(lstStrings.FindIndex(i),strData);
			if (!strOutput.IsEmpty()) strOutput += ", ";
			strOutput += strData;
		}

		if (bReturn)
		{
			strValue=strOutput;
		}
	}
	else
	{
		bReturn = FALSE;
	}
	return bReturn;
}


BOOL CPropertyEditCtrl::ParseCompoundString(CString &strValue, CStringList &lstStrings, int iLength)
{
	int iCount = 0;

	CString strCurrent = strValue;

	BOOL bRun = TRUE;
	do	{
		int iIndex = strCurrent.Find( ',' );
		CString strElement = strCurrent.Left(iIndex);
		if (iIndex==-1)
		{
			strElement = strCurrent;
			bRun = FALSE;
		}
		strCurrent = strCurrent.Right(strCurrent.GetLength()-(iIndex+1));
		if (!strElement.IsEmpty())
		{
			iCount++;
			lstStrings.AddTail(strElement);
		}
	}
	while (bRun);

	return (iCount==iLength);
}

BOOL CPropertyEditCtrl::EvaluateInt(CString &strValue, int &i)
{
	float f = 0.0;

	if (!EvaluateFloat(strValue,f))
	{
		return FALSE;
	}

	i = (int)f;
	return TRUE;
}

BOOL CPropertyEditCtrl::EvaluateFloat(CString &strValue, float &f)
{
    Expression expression;

    const char* variableName[] = { "rand" };
//    const char* functionName[] = { "sqr" };

//    Expression::UserFunction function[] = { sqrt };  

    if (expression.Initialize(strValue, variableName, 1, NULL, NULL, 0))
	{
  
		float variableValue[1];
 
		variableValue[0] = rand();
  
		f = expression.Evaluate(variableValue);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
