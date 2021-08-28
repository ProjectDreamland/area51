//==============================================================================
//
//  KeyBarKey.h
//
//	A keyframe structure that holds the data that the KeyBar needs to edit it
//
//==============================================================================

#ifndef __KEYBARKEY_H__
#define __KEYBARKEY_H__

//----------------------+
//	Includes			|
//----------------------+


//---------------------------+
// KeyBarKey - Definition    |
//---------------------------+

struct  KeyBarKey
{
    int     m_Time;
    bool    m_IsSelected;
    void*   m_pData;

    KeyBarKey()
    {
        m_Time          = 0;
        m_IsSelected    = false;
        m_pData         = NULL;
    }

    ~KeyBarKey()
    {
        if( m_pData )
        {
            free( m_pData );
        }
    }
};

//------------------------------------------------------------------------------+
//------------------------------------------------------------------------------+

#endif  //#ifndef __KEYBARKEY_H__
