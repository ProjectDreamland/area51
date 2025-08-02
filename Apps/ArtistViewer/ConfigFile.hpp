//==============================================================================
//
//  File:           ConfigFile.hpp
//
//  Description:    ConfigFile class
//
//  Author:         Stephen Broumley
//
//  Date:           Started July29th, 2002 (C) Inevitable Entertainment Inc.
//
//==============================================================================

#ifndef __CONFIG_FILE_HPP__
#define __CONFIG_FILE_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Config.hpp"
#include "Util.hpp"
#include "Parsing\Tokenizer.hpp"



//==============================================================================
//  CLASSES
//==============================================================================

class config_file : public token_stream
{
private:

    // Data
    xbool   m_bFound;

public:

    // Constructor/Destructor
    config_file();
    ~config_file();

    // Functions
    void Read( void );
    xbool Is ( const char* pVar );
    xbool Read( const char* pVar, s32& I );
    xbool Read( const char* pVar, f32& F );
    xbool Read( const char* pVar, xcolor& C );
    xbool Read( const char* pVar, char* S );
    xbool Read( const char* pVar, char* S0, char* S1 );
    xbool Read( const char* pVar, char* S0, char* S1, char* S2 );
    xbool Read( const char* pVar, char* S, f32& F );
    xbool Read( const char* pVar, config_options::path& P );
    xbool Read( const char* pVar, vector3& V );
};


//==============================================================================


#endif  //#ifndef __CONFIG_FILE_HPP__
