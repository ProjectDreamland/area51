
#include "Property.hpp"

//=========================================================================

void prop_query::ParseString( const char* pString )
{
    s32 i,j;

    m_nIndices = 0;

    // Find the length of the string
    for( j=i=0; pString[j]; i++, j++ )
    {
        u8  C = pString[j];
        m_String[i] = C;

        // Decode array entry. Take the number out as well
        if( C == '[' )
        {
            s32 Total=0;

            // Decode the number
            while( (C = pString[++j]) != ']' )
            {
                ASSERT( (C >= '0') && (C <= '9') );

                // Accumulate digit.
                Total = (10 * Total) + (C - '0');
            }

            // Roll back j
            j--;

            // Set the number 
            ASSERT( m_nIndices < 16 );
            ASSERT( i != j );
            m_Index[ m_nIndices++ ] = Total;
        }
    }

    // make the end char
    m_String[i] = 0;
    m_StrLen    = i;
}
