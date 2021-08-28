//==============================================================================
//  
//  IniFile.hpp
//  
//==============================================================================

#ifndef INIFILE_HPP
#define INIFILE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_string.hpp"

//==============================================================================
//	TYPES
//==============================================================================

class ini_file
{
public:
                    ini_file            ( void );
                   ~ini_file            ( void );

    xbool           Load                ( const char* pFileName );

    xbool           GetString           ( const char* pKey, char*  pBuffer );
    xbool           GetS32              ( const char* pKey, s32&   Value   );
    xbool           GetF32              ( const char* pKey, f32&   Value   );
    xbool           GetBool             ( const char* pKey, xbool& Value   );

protected:

    const char*     FindValue           ( const char* pKey );
    xstring         m_Data;
};

//==============================================================================
#endif  //INIFILE_HPP
//==============================================================================
