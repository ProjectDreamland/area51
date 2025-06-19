//==============================================================================
// ElfTool.cpp : Defines the entry point for the console application.
//==============================================================================

#include "stdafx.h"
#include "x_files.hpp"
#include "Auxiliary/CommandLine/CommandLine.hpp"
#include "Elf.hpp"

//==============================================================================
// Display Help
//==============================================================================

void DisplayHelp() {
    x_printf("\n");
    x_printf("ElfTool (c)2001 Inevitable Entertainment Inc.\n");
    x_printf("\n");
    x_printf("Usage:\n");
    x_printf("      ElfTool [options] <File name or path to file>\n");
    x_printf("Options:\n");
    x_printf("      -help                Display this help message\n");
    x_printf("      -symbols             Display symbols of ELF file\n");
    x_printf("      -mdebug              Display Mdebug information <requires .mdebug file>\n");
}

//==============================================================================
// Execute Script
//==============================================================================

void ExecuteScript(command_line& CommandLine, elf& Elf, const xstring& ElfFilePath) {
    xstring OutputFolder;
    xbool OutputFolderSet = FALSE;
	
	// Display ELF sections.
    Elf.PrintSections();

    // Loop through all options
    for (s32 i = 0; i < CommandLine.GetNumOptions(); i++) {
        xstring OptName = CommandLine.GetOptionName(i);
        xstring OptString = CommandLine.GetOptionString(i);
		
	    // Display ELF symbols if -sybmbols option is provided
        if (OptName == "SYMBOLS") 
		{
            Elf.PrintSymbols();
        }

        // Parse out and display .mdebug section from the elf if -mdebug option is provided
        if (OptName == "MDEBUG") 
		{
            Elf.PrintMdebug();
        }
    }
}

//==============================================================================
// Main
//==============================================================================

int main(int argc, char* argv[]) {
    s32             i;
    command_line    CommandLine;
    xbool           NeedHelp;

    // Setup recognized command line options
    CommandLine.AddOptionDef("help",     command_line::SWITCH );
    CommandLine.AddOptionDef("symbols",  command_line::SWITCH );
    CommandLine.AddOptionDef("mdebug",   command_line::SWITCH );

    // Parse command line
    NeedHelp = CommandLine.Parse( argc, argv );
    if( NeedHelp || (CommandLine.GetNumArguments() == 0) )
	{
        DisplayHelp();
        return 10;
    }

    // Check if help was requested
    if (CommandLine.FindOption(xstring("help")) >= 0) 
	{
        DisplayHelp();
        return 10;
    }

    // Get ELF file path from the first argument
    const xstring& ElfFilePath = CommandLine.GetArgument(0);
    elf Elf;

    // Load the specified ELF file
    if (Elf.Load(ElfFilePath)) 
	{
        x_printf("Loaded ELF file: \"%s\"\n", ElfFilePath);

        // Execute the script based on parsed options
        ExecuteScript(CommandLine, Elf, ElfFilePath);
    } 
	else 
	{
		// Display error
        x_printf("Error - Can't load file \"%s\"\n", ElfFilePath);
    }
    // Return Success
    return 0;
}
