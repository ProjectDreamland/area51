#if !defined(AFX_RULER_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_)
#define AFX_RULER_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ruler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRuler

//class CRuler
class CRuler
{
public:
    enum units
    {
        UNITS_INTEGER,
        UNITS_FLOAT,
        UNITS_SECONDS
    };

// Construction
public:
	CRuler();

// Attributes
protected:
    CFont   m_Font;
    units   m_Units;

// Implementation
public:
	virtual ~CRuler();

    void    SetUnits        ( units Units ) { m_Units = Units; };
    units   GetUnits        ( void ) { return m_Units; };

    void    DrawRuler       ( CDC* pDC, CRect& r, double Start, double Scale );

// Private
private:
    void    PrettyInt       ( CString& String, int Number );
    void    PrettyFloat     ( CString& String, double Number, int nFractional );
    void    PrettySeconds   ( CString& String, double Seconds, int nFractional );
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_RULER_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_)
