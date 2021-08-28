//==============================================================================
//
//  Crc.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

//=========================================================================
// GLOBALS
//=========================================================================

xbool    Crc::m_CrcTableComputed = FALSE;           // Flag: has the table been computed? Initially false. 
u32      Crc::m_CrcTable[256];                      // Table of CRCs of all 8-bit messages

//=========================================================================
// Update a running CRC with the bytes pBuffer[0..Length-1]--the CRC
// should be initialized to all 1's, and the transmitted value
// is the 1's complement of the final running CRC (see the
// Make() routine below)).

u32 Crc::Update( u32 CrcValue, char *pBuffer, s32 Length )
{
    u32 c = CrcValue;
    s32 n;
    
    if (!m_CrcTableComputed)
        MakeTable();
    
    for (n = 0; n < Length; n++) 
    {
        c = m_CrcTable[(c ^ pBuffer[n]) & 0xff] ^ (c >> 8);
    }
    
    return c;
}

//=========================================================================
// Return the CRC of the bytes pBuffer[0..Length-1].

u32 Crc::Make( char *pBuffer, s32 Length )
{
    return Update(0xffffffffL, pBuffer, Length) ^ 0xffffffffL;
}

//=========================================================================
// Make the table for a fast CRC.

void Crc::MakeTable( void )
{
    u32 c;
    s32 n, k;
    
    for (n = 0; n < 256; n++) {
        c = (u32) n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        m_CrcTable[n] = c;
    }
    m_CrcTableComputed = TRUE;
}