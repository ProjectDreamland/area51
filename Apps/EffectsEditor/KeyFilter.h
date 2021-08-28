//==============================================================================
//
//  KeyFilter.h
//
//	A filter for KeySets
//
//==============================================================================

#ifndef __KEYFILTER_H__
#define __KEYFILTER_H__

//----------------------+
//	Includes			|
//----------------------+


//---------------------------+
// KeyFilter - Definition    |
//---------------------------+

struct  KeyFilter
{
    CString     m_Name;
    COLORREF    m_Color;
    bool        m_IsVisible;

    KeyFilter()
    {
        m_Name      = "";
        m_Color     = RGB(0,0,0);
        m_IsVisible = true;
    }

    KeyFilter( CString Name, COLORREF Color, bool IsVisible = true )
    {
        m_Name      = Name;
        m_Color     = Color;
        m_IsVisible = IsVisible;
    }

    ~KeyFilter()
    {

    }
};

//------------------------------------------------------------------------------+
//------------------------------------------------------------------------------+

#endif  //#ifndef __KEYFILTER_H__
