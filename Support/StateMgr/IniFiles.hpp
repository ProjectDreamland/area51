//==============================================================================
//  
//  IniFiles.hpp
//  
//==============================================================================

#ifndef INIFILES_HPP
#define INIFILES_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_string.hpp"

//==============================================================================
//	ini_file
//==============================================================================

class ini_file
{
public:
                    ini_file            ( void );
                    ~ini_file           ( void );

    xbool           Load                ( const char * pFileName);
    xbool           GetValue            ( const char * pSection, const char* pKey, xstring &Result);
    xbool           GetValue_s32        ( const char * pSection, const char* pKey, s32 &Result);
    xbool           GetValue_f32        ( const char * pSection, const char* pKey, f32 &Result);
    const char *    GetInIPathFileName  ( void );
    void            SetInIPathFileName  ( const char * PathName );
 
protected:
    const char *    ParseStringData             (const char * pSection, const char* pKey);
    s32             SearchForSection            (const char * pSection, s32 StartIndex);
    const char *    SearchForField              (const char *pKey, s32 StartIndex);
    void            StripChars                  (xstring &String, const char RemoveChar);
    void            StripLeadingTrailingChars   (xstring &String, const char RemoveChar);

    //data
    xstring         m_LoadedData;
    char            m_InIPathFileName[X_MAX_PATH];
};

//==============================================================================
#endif  //INIFILES_HPP
//==============================================================================
