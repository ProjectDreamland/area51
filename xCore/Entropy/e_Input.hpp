//==============================================================================
//  
//  e_Input.hpp
//
//==============================================================================

#ifndef E_INPUT_HPP
#define E_INPUT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "e_Input_Gadgets.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//  Public functions
//------------------------------------------------------------------------------

xbool           input_UpdateState   ( void );
#ifdef TARGET_XBOX
xbool           input_IsPressed     ( input_gadget GadgetID, s32 ControllerID = -1 );
xbool           input_WasPressed    ( input_gadget GadgetID, s32 ControllerID = -1 );
f32             input_GetValue      ( input_gadget GadgetID, s32 ControllerID = -1 );
#else
xbool           input_IsPressed     ( input_gadget GadgetID, s32 ControllerID = 0 );
xbool           input_WasPressed    ( input_gadget GadgetID, s32 ControllerID = 0 );
f32             input_GetValue      ( input_gadget GadgetID, s32 ControllerID = 0 );
#endif
xbool           input_IsPresent     ( input_gadget GadgetID, s32 ControllerID = -1 );
//------------------------------------------------------------------------------
//  Public functions in e_Input.cpp
//------------------------------------------------------------------------------

// Given name, searches for and returns gadget if found, else INPUT_UNDEFINED
input_gadget    input_LookupGadget  ( const char* pName );

// Enable force feedback for the current controller. A duration or intensity of 0 will disable feedback.
// Intensity is a value between 0 and 1.0, duration is time in seconds for the feedback to run
struct feedback_envelope
{
    f32     Intensity;
    f32     Duration;
    s32     Mode;
};

void            input_Feedback( f32 Duration, f32 Intensity, s32 ControllerID = 0 );
void            input_Feedback( s32 Count, feedback_envelope* pEnvelope, s32 ControllerID = 0 );
void            input_EnableFeedback  ( xbool state, s32 ControllerID = 0 );
void            input_SuppressFeedback( xbool Suppress );
void            input_ClearFeedback   ( void );

//------------------------------------------------------------------------------
//  Private functions
//------------------------------------------------------------------------------

void            input_Init          ( void );
void            input_Kill          ( void );
                
// bw 7/22 removed input_CheckDevices. No longer required with the thread update
//==============================================================================
#endif // E_INPUT_HPP
//==============================================================================
