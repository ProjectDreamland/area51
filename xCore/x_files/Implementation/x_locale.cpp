//==============================================================================
//  x_locale.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for generalizing basic localization functions.
//  Note that unlike many of the other x_file functions, these functions do not
//  adhere closely to any standard functions such as setLocal(). Because consoles
//  and game apps have very simple needs, these functions generally only support 
//  language settings and time/date formats. 
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\x_types.hpp"

#ifndef X_LOCALE_HPP
#include "..\x_locale.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "..\x_debug.hpp"
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_XBOX
#include <xtl.h>
#endif

#ifdef TARGET_PS2
#include <libscf.h>
#endif

//==============================================================================
//  VARIABLES
//==============================================================================

static  x_language          s_LocaleLang = XL_LANG_ENGLISH;
static  x_console_territory s_Territory = XL_TERRITORY_AMERICA;

// these strings correspond to the ISO 639 3 letter language codes.
// please stick to this standard when adding new languages.
// Note also, that these must appear in the same order as the language enums!!
static const char* const s_pLanguageStr[] = 
{
    "ENG",      // XL_LANG_ENGLISH
    "FRE",      // XL_LANG_FRENCH
    "GER",      // XL_LANG_GERMAN
    "ITA",      // XL_LANG_ITALIAN
    "SPA",      // XL_LANG_SPANISH
    "DUT",      // XL_LANG_DUTCH
    "JPN",      // XL_LANG_JAPANESE
    "KOR",      // XL_LANG_KOREAN
    "POR",      // XL_LANG_PORTUGUESE
    "CHI"       // XL_LANG_TCHINESE
};

//==============================================================================
//  IMPLEMENTATION
//==============================================================================

//==============================================================================
// Returns language enumeration for the currently set language on console hardware.
//
// Parameters:
//
// Returns:
//  x_language enumeration of consoles' set language. (english for PC)
//
// Remarks:
//  Use this during application init to determine the platform's language setting.
//  Application is responsible for determining whether the returned language is 
//  supported, and supply a suitable default.
//==============================================================================
const x_language x_GetConsoleLanguage( void )
{
#if defined (TARGET_XBOX)

    switch( XGetLanguage() )
    {
        case XC_LANGUAGE_PORTUGUESE:return XL_LANG_PORTUGUESE;	break;
        case XC_LANGUAGE_JAPANESE:	return XL_LANG_JAPANESE;	break;
        case XC_LANGUAGE_TCHINESE:	return XL_LANG_TCHINESE;	break;
        case XC_LANGUAGE_KOREAN:	return XL_LANG_KOREAN;	    break;
        case XC_LANGUAGE_ENGLISH:	return XL_LANG_ENGLISH;	    break;
        case XC_LANGUAGE_FRENCH:	return XL_LANG_FRENCH;  	break;
        case XC_LANGUAGE_GERMAN:	return XL_LANG_GERMAN;	    break;
        case XC_LANGUAGE_SPANISH:	return XL_LANG_SPANISH;	    break;
        case XC_LANGUAGE_ITALIAN:	return XL_LANG_ITALIAN;	    break;

        default:
            ASSERTS(0, "XBOX returned unknown language.");
            return XL_LANG_ENGLISH;
    }

#elif defined (TARGET_PS2)

    switch( sceScfGetLanguage() )
    {
        case SCE_JAPANESE_LANGUAGE:     return XL_LANG_JAPANESE;    break;
        case SCE_ENGLISH_LANGUAGE:	    return XL_LANG_ENGLISH;	    break;
        case SCE_FRENCH_LANGUAGE:	    return XL_LANG_FRENCH;	    break;
        case SCE_SPANISH_LANGUAGE:	    return XL_LANG_SPANISH; 	break;
        case SCE_GERMAN_LANGUAGE:	    return XL_LANG_GERMAN;	    break;
        case SCE_ITALIAN_LANGUAGE:      return XL_LANG_ITALIAN;	    break;
        case SCE_DUTCH_LANGUAGE:        return XL_LANG_DUTCH;       break;
        case SCE_PORTUGUESE_LANGUAGE:   return XL_LANG_PORTUGUESE;  break;

        default:
            ASSERTS(0, "PS2 returned unknown language.");
            return XL_LANG_ENGLISH;
    }

#elif defined (TARGET_PC)

    return XL_LANG_ENGLISH;

#endif
}

//==============================================================================
// Returns territory enumeration for the currently set region on console hardware.
//
// Parameters:
//
// Returns:
//  x_console_territory enumeration of consoles' set region.
//
// Remarks:
//  This is only supported on Xbox - PS2 and PC have no equivelent.
//==============================================================================
const x_console_territory x_GetConsoleRegion  ( void )
{
#if defined (TARGET_XBOX)
    switch( XGetGameRegion() )
    {
        default:
        case XC_GAME_REGION_NA:         return XL_TERRITORY_AMERICA;
        case XC_GAME_REGION_JAPAN:      return XL_TERRITORY_JAPAN; 
        case XC_GAME_REGION_RESTOFWORLD:return XL_TERRITORY_EUROPE;
    }
#else
    ASSERTS(0, "This function is no supported on this platform");
    return XL_TERRITORY_AMERICA;
#endif
}


//==============================================================================
// Sets the application's selected language.
//
// Parameters:
//  x_language enumeration.
//
// Returns:
//
// Remarks:
//  Once the application determines the correct default language (or it is changed
//  in the case we have a menu), we set the system language here.
//==============================================================================
void x_SetLocale( const x_language lang )
{
    s_LocaleLang = lang;
}

//==============================================================================
// returns the application's selected language.
//
// Parameters:
//
// Returns:
//  x_language enumeration for current language.
//
// Remarks:
//  Use THIS instead of GetLanguage() for run-time operations that 
//  require the current language.
//==============================================================================
const x_language x_GetLocale( void )
{
    return s_LocaleLang;
}

//==============================================================================
// returns 3 character code string for the current language.
//
// Parameters:
//
// Returns:
//  pointer to string code for current language.
//
// Remarks:
//  Use for filename manipulation to select localized assets.
//==============================================================================
const char * x_GetLocaleString( void )
{
    ASSERT( s_LocaleLang < XL_NUM_LANGUAGES );

    return s_pLanguageStr[s_LocaleLang];
}

//==============================================================================
// returns 3 character code string for the requested language.
//
// Parameters:
//
// Returns:
//  pointer to string code for requested language.
//
// Remarks:
//  
//==============================================================================
const char * x_GetLocaleString( const x_language lang )
{
    ASSERT( lang < XL_NUM_LANGUAGES );

    return s_pLanguageStr[lang];
}

//==============================================================================
// Sets an enumeration for the console territory
//
// Parameters:
//  x_console_territory enumeration
//
// Returns:
//
// Remarks:
//  
//==============================================================================
void x_SetTerritory( const x_console_territory territory )
{
    s_Territory = territory;
}

//==============================================================================
// returns console territory.
//
// Parameters:
//
// Returns:
//  x_console_territory enumeration
//
// Remarks:
//  
//==============================================================================
const x_console_territory x_GetTerritory( void )
{
    return s_Territory;
}

//==============================================================================
// returns TRUE if build is European
//
// Parameters:
//
// Returns:
//  TRUE if the territory corresponds to a censored build.
//
// Remarks:
//   Currently, European build is censored.
//  
//==============================================================================
const xbool x_IsBuildCensored( void )
{
    return (x_GetTerritory() == XL_TERRITORY_EUROPE);
}
