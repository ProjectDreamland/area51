//==============================================================================
//  DebugPage.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  The debug page class is the base class for specific debug menu pages. 
//   
//==============================================================================

#ifndef DEBUG_MENU_PAGE_HPP
#define DEBUG_MENU_PAGE_HPP

//==============================================================================

class debug_menu2;

class debug_menu_page
{
public:
                                debug_menu_page     ( void );
    virtual                    ~debug_menu_page     ( void );

            debug_menu_item&    GetActiveItem       ( void );
            void                NextItem            ( void );
            void                PrevItem            ( void );

            s32                 GetItemCount        ( void );
            debug_menu_item*    GetItem             ( s32 );
            debug_menu_item*    FindItem            ( const char* pName );
            const char*         GetTitle            ( void );

            void                Render              ( void ); 

            debug_menu_item*    AddItemSeperator    ( const char* pName = NULL );
            debug_menu_item*    AddItemButton       ( const char* pName );
            debug_menu_item*    AddItemBool         ( const char* pName, xbool& Var, const char** pValues = NULL );
            debug_menu_item*    AddItemInt          ( const char* pName, s32&   Var, s32 Min, s32 Max );
            debug_menu_item*    AddItemFloat        ( const char* pName, f32&   Var, f32 Min, f32 Max, f32 Increment = 1.0f );
            debug_menu_item*    AddItemEnum         ( const char* pName, s32&   Var, const char** pValues, s32 nValues );

    virtual void                OnEnterMenu         ( void ) { };
    virtual void                OnLeaveMenu         ( void ) { };
    virtual void                OnEnterPage         ( void ) { };
    virtual void                OnLeavePage         ( void ) { };
    
    virtual void                OnFocus             ( void ) { };
    virtual void                OnLoseFocus         ( void ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem ) { (void)pItem; };

    virtual void                OnRender            ( void ) { };
    virtual void                OnRenderActive      ( void ) { };
    virtual void                OnPreRender         ( void ) { };
    virtual void                OnPreRenderActive   ( void ) { };

protected:
            const char*                 m_pTitle;
            xarray<debug_menu_item*>    m_Items;
            s32                         m_iActiveItem;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_HPP