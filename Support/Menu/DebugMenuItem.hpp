//==============================================================================
//  DebugMenuItem.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class represents an individual line option in the debug menu. It manages
//  the current value of a debug menu option, and holds that value.
//==============================================================================

#ifndef DEBUG_MENU_ITEM_HPP
#define DEBUG_MENU_ITEM_HPP

class debug_menu_page;

//==============================================================================

class debug_menu_item
{
public:
    enum type
    {
        TYPE_NULL,
        TYPE_SEPERATOR,
        TYPE_BUTTON,
        TYPE_BOOL,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_ENUM
    };

static const char* BoolOnOff[2];
static const char* BoolYesNo[2];
static const char* BoolTrueFalse[2];

static const s32 DEFAULT_FLASH_COUNT = 15;

public:
                                debug_menu_item     ( void );
                               ~debug_menu_item     ( void );

            xbool               InitSeperator       ( const char*       pName = NULL );

            xbool               InitButton          ( const char*       pName );

            xbool               InitBool            ( const char*       pName,
                                                      xbool&            Value,
                                                      const char**      pValues = NULL );

            xbool               InitInt             ( const char*       pName,
                                                      s32&              Value,
                                                      s32               Min,
                                                      s32               Max );

            xbool               InitFloat           ( const char*       pName,
                                                      f32&              Value,
                                                      f32               Min,
                                                      f32               Max,
                                                      f32               Increment = 1.0f );

            xbool               InitEnum            ( const char*       pName,
                                                      s32&              Value,
                                                      const char**      pValues,
                                                      s32               nValues );

            void                Render              ( s32               ScreenX, 
                                                      s32               ScreenY,
                                                      xbool             bHighlight );

    const   char*               GetName             ( void );
    const   char*               GetValueAsString    ( void );
            type                GetType             ( void );

            void                SetName             ( const char*       pName );
            void                SetIntLimits        ( s32 Min, s32 Max );
            
            void                SetValueBool        ( xbool             value );
            void                SetValueInt         ( s32               value );
            void                SetValueFloat       ( f32               value );
            void                SetValueEnum        ( s32               Value );
            
            void                Increment           ( void );
            void                Decrement           ( void );

            xbool               GetValueBool        ( void );
            s32                 GetValueInt         ( void );
            f32                 GetValueFloat       ( void );
            s32                 GetValueEnum        ( void );

protected:
            type                m_Type;
    const   char*               m_pName;
            xstring             m_String;

            xbool*              m_pBoolValue;
            const char**        m_pBoolValues;

            s32*                m_pIntValue;
            s32                 m_IntMin;
            s32                 m_IntMax;

            f32*                m_pFloatValue;
            f32                 m_FloatMin;
            f32                 m_FloatMax;
            f32                 m_FloatIncrement;

            s32*                m_pEnumValue;
            const char**        m_pEnumValues;
            s32                 m_nEnumValues;

            s32                 m_iFlashCount;
};

//==============================================================================

#endif // DEBUG_MENU_ITEM_HPP
