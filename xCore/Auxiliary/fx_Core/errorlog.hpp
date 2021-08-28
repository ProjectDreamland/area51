#ifndef __ERROR_LOG_H
#define __ERROR_LOG_H

//============================================================================
//  INCLUDES
//============================================================================

#include "x_files.hpp"

namespace fx_core
{

//============================================================================
// error_log
//============================================================================

class error_log
{
protected:
    xarray<xstring>     m_Log;

public:
                        error_log           ( void );
                       ~error_log           ( void );

    void                Clear               ( void );
    void                Append              ( const char* pString );

    s32                 GetCount            ( void ) const;
    const xstring&      GetError            ( s32 iError ) const;
};

//============================================================================

} // namespace fx_core

#endif
