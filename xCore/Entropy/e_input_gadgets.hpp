//==============================================================================
//
//  e_Input_Gadgets.hpp
//
//
//    Defines the enums input_gadget list
//
//==============================================================================

#ifndef E_INPUT_GADGETS_HPP
#define E_INPUT_GADGETS_HPP

//==============================================================================
// MACROS THAT e_Input_Gadget_Defines.hpp WILL USE TO CREATE THE ENUM LIST
//==============================================================================

#define BEGIN_GADGETS                               enum input_gadget {
#define DEFINE_GADGET(__gadget__)                   __gadget__ ,
#define DEFINE_GADGET_VALUE(__gadget__, __value__)  __gadget__ = __value__ ,
#define END_GADGETS                                 };

//==============================================================================
//  DEFINE THOSE ENUMS
//==============================================================================

#include "e_input_gadget_defines.hpp"

//==============================================================================
#endif  // #ifndef E_INPUT_GADGETS_HPP
//==============================================================================
