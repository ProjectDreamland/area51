//==============================================================================
// ElfTool.cpp : Defines the entry point for the console application.
//==============================================================================

#include "stdafx.h"
#include "Elf.hpp"

//==============================================================================

int main( int argc, char* argv[] )
{
    elf Elf;
    
    // Load it
    if( Elf.Load( "c:\\tmp\\Draw01.elf" ) )
    {
        // Print section headers
        Elf.PrintSections();

        // Print symbols
//        Elf.PrintSymbols();

        // Print Mdebug info
        Elf.PrintMdebug();
    }

	return 0;
}

//==============================================================================
