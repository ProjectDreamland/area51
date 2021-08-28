//==============================================================================
//  DebugMenuItem.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu items.
//  
//==============================================================================

#include "DebugMenu2.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

const char* debug_menu_item::BoolOnOff[2]       = { "Off"   , "On"   };
const char* debug_menu_item::BoolYesNo[2]       = { "No"    , "Yes"  };
const char* debug_menu_item::BoolTrueFalse[2]   = { "False" , "True" };

//==============================================================================

debug_menu_item::debug_menu_item( void )
{
    m_Type  = TYPE_NULL;
    m_pName = "";
    m_iFlashCount   = 0;
}

//==============================================================================

debug_menu_item::~debug_menu_item( void )
{
}

//==============================================================================

xbool debug_menu_item::InitSeperator( const char* pName )
{
    m_Type          = TYPE_SEPERATOR;
    if( pName )
    {
        m_pName     = pName;
    }
    else
    {
        m_pName     = "";
    }

    return TRUE;
}

//==============================================================================

xbool debug_menu_item::InitButton( const char* pName )
{
    m_Type          = TYPE_BUTTON;
    m_pName         = pName;
    m_iFlashCount   = 0;

    return TRUE;
}

//==============================================================================

xbool debug_menu_item::InitBool( const char*    pName,
                                xbool&         Value,
                                const char**   pValues )
{
    m_Type          = TYPE_BOOL;
    m_pName         = pName;
    m_pBoolValue    = &Value;
    m_pBoolValues   = pValues ? pValues : BoolOnOff;

    return TRUE;
}

//==============================================================================

xbool debug_menu_item::InitInt( const char* pName,
                               s32&        Value,
                               s32         Min,
                               s32         Max )
{
    m_Type      = TYPE_INT;
    m_pName     = pName;
    m_pIntValue = &Value;
    m_IntMin    = Min;
    m_IntMax    = Max;

    return TRUE;
}

//==============================================================================

xbool debug_menu_item::InitFloat( const char* pName,
                                  f32&        Value,
                                  f32         Min,
                                  f32         Max,
                                  f32         Increment )
{
    m_Type              = TYPE_FLOAT;
    m_pName             = pName;
    m_pFloatValue       = &Value;
    m_FloatMin          = Min;
    m_FloatMax          = Max;
    m_FloatIncrement    = Increment;

    return TRUE;
}

//==============================================================================

xbool debug_menu_item::InitEnum( const char*    pName,
                                 s32&           Value,
                                 const char**   pValues,
                                 s32            nValues )
{
    m_Type          = TYPE_ENUM;
    m_pName         = pName;
    m_pEnumValue    = &Value;
    m_pEnumValues   = pValues;
    m_nEnumValues   = nValues;

    return TRUE;
}

//==============================================================================

const char* debug_menu_item::GetName( void )
{
    return m_pName;
}

//==============================================================================

const char* debug_menu_item::GetValueAsString( void )
{
    switch( m_Type )
    {
    case TYPE_SEPERATOR:
        return "";

    case TYPE_BUTTON:
        return "";

    case TYPE_NULL:
        return "<null>";

    case TYPE_BOOL:
        if( *m_pBoolValue )
            return m_pBoolValues[1];
        else
            return m_pBoolValues[0];

    case TYPE_INT:
        m_String.Format( "%d", *m_pIntValue );
        return (const char*)m_String;

    case TYPE_FLOAT:
        m_String.Format( "%.3f", *m_pFloatValue );
        return (const char*)m_String;

    case TYPE_ENUM:
        return m_pEnumValues[*m_pEnumValue];

    default:
        ASSERT( 0 );
        return "<bad type>";
    }
}

//==============================================================================

debug_menu_item::type debug_menu_item::GetType( void )
{
    return m_Type;
}

//==============================================================================

void debug_menu_item::Render( s32   ScreenX, 
                              s32   ScreenY,
                              xbool bHighlight )
{
    const s32 ITEM_FONT = 0;
    const s32 ITEM_DATA_FONT = 0;

    // Display item label.
    irect TextPos;

    // TODO: make this better.
    TextPos.l = ScreenX;
    TextPos.t = ScreenY;
    TextPos.r = TextPos.l + 400;
    TextPos.b = TextPos.t + 20;

    ASSERT( m_pName != NULL );
    debug_menu2::RenderLine( ITEM_FONT, m_pName, TextPos, bHighlight, m_iFlashCount != 0 );

    // Update Cursor position for item state.
    TextPos.l = 350;
    debug_menu2::RenderLine( ITEM_DATA_FONT, GetValueAsString(), TextPos, bHighlight, m_iFlashCount != 0 );

    if( m_iFlashCount > 0 )
        m_iFlashCount--;
}

//==============================================================================

void debug_menu_item::SetName( const char* pName )
{
    ASSERT( pName );
    m_pName = pName;
}

//==============================================================================

void debug_menu_item::SetIntLimits( s32 Min, s32 Max )
{
    m_IntMin = Min;
    m_IntMax = Max;
}

//==============================================================================

void debug_menu_item::SetValueBool( xbool Value )
{
    ASSERT( m_Type == TYPE_BOOL );
    *m_pBoolValue = Value;
}

//==============================================================================

void debug_menu_item::SetValueInt( s32 Value )
{
    ASSERT( m_Type == TYPE_INT );
    *m_pIntValue = Value;
}

//==============================================================================

void debug_menu_item::SetValueFloat( f32 Value )
{
    ASSERT( m_Type == TYPE_FLOAT );
    *m_pFloatValue = Value;
}

//==============================================================================

void debug_menu_item::SetValueEnum( s32 Value )
{
    ASSERT( m_Type == TYPE_ENUM );
    *m_pEnumValue = Value;
}

//==============================================================================

void debug_menu_item::Increment( void )
{
    switch( m_Type )
    {
    case TYPE_BOOL:
        *m_pBoolValue = !*m_pBoolValue;
        break;

    case TYPE_INT:
        if( m_IntMin != m_IntMax )
        {
            (*m_pIntValue)++;
            if( *m_pIntValue > m_IntMax )
            {
                *m_pIntValue = m_IntMin;
            }                
        }
        break;

    case TYPE_FLOAT:
        if( m_FloatMin != m_FloatMax )
        {
            *m_pFloatValue += m_FloatIncrement;
            if( *m_pFloatValue > m_FloatMax )
            {
                *m_pFloatValue = m_FloatMax;
            }
        }
        break;

    case TYPE_ENUM:
        (*m_pEnumValue)++;
        if( *m_pEnumValue >= m_nEnumValues )
            *m_pEnumValue = 0;
        break;
    }

    // render function will count this down.
    m_iFlashCount = DEFAULT_FLASH_COUNT;
}

//==============================================================================

void debug_menu_item::Decrement( void )
{
    switch( m_Type )
    {
    case TYPE_BOOL:
        *m_pBoolValue = !*m_pBoolValue;
        break;

    case TYPE_INT:
        if( m_IntMin != m_IntMax )
        {
            (*m_pIntValue)--;
            if( *m_pIntValue < m_IntMin )
            {
                *m_pIntValue = m_IntMax;
            }                
        }
        break;

    case TYPE_FLOAT:
        if( m_FloatMin != m_FloatMax )
        {
            *m_pFloatValue -= m_FloatIncrement;
            if( *m_pFloatValue < m_FloatMin )
            {
                *m_pFloatValue = m_FloatMin;
            }
        }
        break;

    case TYPE_ENUM:
        (*m_pEnumValue)--;
        if( *m_pEnumValue < 0 )
            *m_pEnumValue = m_nEnumValues-1;
        break;
    }

    // render function will count this down.
    m_iFlashCount = DEFAULT_FLASH_COUNT;
}

//==============================================================================

xbool debug_menu_item::GetValueBool( void )
{
    ASSERT( m_Type == TYPE_BOOL );
    return *m_pBoolValue;
}

//==============================================================================

s32 debug_menu_item::GetValueInt( void )
{
    ASSERT( m_Type == TYPE_INT );
    return *m_pIntValue;
}

//==============================================================================

f32 debug_menu_item::GetValueFloat( void )
{
    ASSERT( m_Type == TYPE_FLOAT );
    return *m_pFloatValue;
}

//==============================================================================

s32 debug_menu_item::GetValueEnum( void )
{
    ASSERT( m_Type == TYPE_ENUM );
    return *m_pEnumValue;
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
