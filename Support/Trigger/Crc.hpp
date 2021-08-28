///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_
#define _TRIGGER_ACTIONS_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"

//NOTE : THIS CODE COMES FROM THE PNG SOURCE...

class Crc
{
public:

    u32                 Update      ( u32 CrcValue, char* pBuffer, s32 Length );
    u32                 Make        ( char* pBuffer, s32 Length );
    
protected:

    void                MakeTable   ( void );

    static xbool        m_CrcTableComputed;          // Flag: has the table been computed? Initially false. 
    static u32          m_CrcTable[256];             // Table of CRCs of all 8-bit messages
};

//=========================================================================
// END
//=========================================================================

#endif
