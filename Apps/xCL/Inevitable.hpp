//==============================================================================
//
//  Inevitable.hpp
//
//==============================================================================

#ifndef INEVITABLE_HPP
#define INEVITABLE_HPP

//==============================================================================
//  FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//  
//  FindSubDirectory 
//  
//      Starting at the current directory and working up towards the root, 
//      look for a subdirectory with the given name.
//      
//      For example,
//      
//      C:\
//       |
//       +--Projects
//           |
//           +--KillerGame
//           |   |
//           |   +--Tools
//           |   |
//           |   +--Zone
//           |   
//           +--Tests
//           |   |
//           |   +--Terrain
//           |
//           +--Zone
//      
//      If you are in "C:\Projects\KillerGame" or "C:\Projects\KillerGame\Tools"
//      and you call FindSubDirectory( "Zone" ), you will get back
//      "C:\Projects\KillerGame\Zone"
//      
//      If you're in "C:\Projects\Tests\Terrain", you'd get "C:\Projects\Zone".
//      
//------------------------------------------------------------------------------

char* FindSubDirectory( char* pName );

//==============================================================================
#endif // INEVITABLE_HPP
//==============================================================================

