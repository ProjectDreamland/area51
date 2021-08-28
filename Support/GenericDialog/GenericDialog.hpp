//=========================================================================
//
// GenericDialog.hpp
//
//=========================================================================
#ifndef GENERIC_DIALOG_HPP
#define GENERIC_DIALOG_HPP
//=========================================================================

//#include "stdafx.h"
#include "x_files.hpp"

//=========================================================================
// generic_dialog
//=========================================================================

class generic_dialog
{
public:    
                        generic_dialog          ( void );
                        ~generic_dialog         ( void );

    // Use these functions for quick setup
    virtual s32         Execute_OK              ( const char* pTitle, const char* pMessage );
    virtual s32         Execute_OK_CANCEL       ( const char* pTitle, const char* pMessage );

    // Use these functions in order for manual setup
    virtual void        Clear                   ( void );
    virtual void        SetTitle                ( const char* pTitle ); 
    virtual void        SetMessage              ( const char* pMessage ); 
    virtual void        AppendButton            ( const char* pButtonTitle );
    virtual s32         Execute                 ( void );

    // These are usefull after the dialog returns
    virtual s32         GetResult               ( void ) const;
    virtual const char* GetButtonTitle          ( s32 iButton ) const;
    virtual const char* GetTitle                ( void ) const;
    virtual const char* GetMessage              ( void ) const;
    virtual s32         GetNButtons             ( void ) const;

private:

    const char*     m_pTitle;               
    const char*     m_pMessage;
    const char*     m_pButtonTitle[8];
    s32             m_nButtons;
    s32             m_Result;
    s32             m_iButton_OK;
    s32             m_iButton_CANCEL;
};

//=========================================================================
// END
//=========================================================================
#endif
