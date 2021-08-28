//==============================================================================
//  x_locale.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the header for generalized basic localization functions.
//==============================================================================

#ifndef X_LOCALE_HPP
#define X_LOCALE_HPP

//==============================================================================
//  ENUMS
//==============================================================================

// Console territories 
// XBox has 3 - "America", "Japan", "Others"
// PS2 has several, but they can be broken into similar territories if you count
// only 3rd parties. It breaks down as follows: 
// - "Japan" for SCEI/SCEK market
// - "America" for SCEA market
// - "Europe" for SCEE market
enum x_console_territory
{
    XL_TERRITORY_AMERICA,
    XL_TERRITORY_JAPAN,
    XL_TERRITORY_EUROPE,

    XL_NUM_TERRITORIES
};

// languages 
// This is our own language enumeration. It will contain all languages supported by all
// consoles we support - but most builds will only support a subset of these. 
// The application is responsible for determining which languages it supports, and for
// defaulting to an appropriate language when the console is set incorrectly.
// Note that the corresponding strings in s_pLanguageStr must be in this order.
// -- this is in alpha order, with the most common first (EFGIS).
enum x_language
{
    XL_LANG_ENGLISH = 0,
    XL_LANG_FRENCH,
    XL_LANG_GERMAN,
    XL_LANG_ITALIAN,
    XL_LANG_SPANISH,

    XL_LANG_DUTCH,
    XL_LANG_JAPANESE,
    XL_LANG_KOREAN,
    XL_LANG_PORTUGUESE,
    XL_LANG_TCHINESE,

    XL_NUM_LANGUAGES
};

enum x_build
{
    XL_BUILD_US = 0,
    XL_BUILD_EUROPE,
    XL_BUILD_GERMANY,

    XL_NUM_BUILDTYPES
};

//==============================================================================
//  PROTOTYPES
//==============================================================================

const x_language          x_GetConsoleLanguage( void );

const x_console_territory x_GetConsoleRegion  ( void );

      void                x_SetLocale         ( x_language const lang );
const x_language          x_GetLocale         ( void );

const char *              x_GetLocaleString   ( void );
const char *              x_GetLocaleString   ( x_language const lang );

      void                x_SetTerritory      ( x_console_territory const territory );
const x_console_territory x_GetTerritory      ( void );

const xbool               x_IsBuildCensored   ( void );

#endif // X_LOCALE_HPP