//==============================================================================
//
//  KeySet.h
//
//	A container for a track of keys
//
//==============================================================================

#ifndef __KEYSET_H__
#define __KEYSET_H__

//----------------------+
//	Includes			|
//----------------------+

#include "KeyBarKey.h"

//------------------------+
// Keyset - Definition    |
//------------------------+
class KeySet
{
public:
    
    CString         m_Category;
    int             m_nKeys;
    int             m_KeyDataSize;
    KeyBarKey*      m_pKeys;
    DWORD           m_UserData;     // For user to hook up whatever they want(ie controller) to a KeySet
    
    KeySet() : m_nKeys(0), m_KeyDataSize(0), m_pKeys(NULL), m_UserData(0)
    {
        m_Category      = "";
    }

    ~KeySet()
    {
        if( m_pKeys )
        {
            delete[] m_pKeys;
            m_pKeys = NULL;
        }
    }
};

typedef CList< KeySet*, KeySet* >    keylist;

//------------------------------------------------------------------------------+
//------------------------------------------------------------------------------+

#endif  //#ifndef __KEYSET_H__
