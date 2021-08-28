//==============================================================================
//  
//  e_Input.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\e_Input.hpp"

//==============================================================================
//  STRUCTURES
//==============================================================================

// Structure to associate a gadget enum with a string name
struct input_gadget_lookup
{
    input_gadget    Gadget;    // Gadget enum id
    const char*     pName;     // String name of gadget
};

//==============================================================================
// MACROS THAT e_Input_Gadget_Defines.hpp WILL USE TO CREATE THE LOOKUP TABLE
//==============================================================================

#define BEGIN_GADGETS                               static input_gadget_lookup GadgetLookupTable[] = {
#define DEFINE_GADGET(__gadget__)                   { __gadget__, #__gadget__ },
#define DEFINE_GADGET_VALUE(__gadget__, __value__)  { __gadget__, #__gadget__ },
#define END_GADGETS                                 };

//==============================================================================
//  DEFINE THE GADGET LOOKUP TABLE
//==============================================================================

#include "e_input_gadget_defines.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
// Given name, searches for and returns gadget if found, else INPUT_UNDEFINED.

input_gadget input_LookupGadget( const char* pName )
{
    s32 i;

    ASSERT( pName );

    // Loop through all entries in lookup table
    for( i = 0; i < (s32)(sizeof(GadgetLookupTable) / sizeof(input_gadget_lookup)); i++ )
    {
        // Found match?
        if( x_strcmp( GadgetLookupTable[i].pName, pName ) == 0 )
            return( GadgetLookupTable[i].Gadget );
    }

    // Not found
    return( INPUT_UNDEFINED );
}

//==============================================================================
