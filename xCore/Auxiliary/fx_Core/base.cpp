//============================================================================
// INCLUDES
//============================================================================

#include "x_files.hpp"
#include "base.hpp"

namespace fx_core
{

//============================================================================
// DATA
//============================================================================

//============================================================================
//  base
//============================================================================

base::base()
{
}

//============================================================================

base::~base()
{
}

//============================================================================

s32 base::ControllerType_FromString( const char* pString )
{
    if( x_strstr( pString, "Linear" ) )
    {
        return CONTROLLERTYPE_LINEAR;
    }
    else if( x_strstr( pString, "Smooth" ) )
    {
        return CONTROLLERTYPE_SMOOTH;
    }

    ASSERT( 0 );
    return 0;
}

//============================================================================

const char* base::ControllerType_ToString( s32 ControllerType )
{
    switch( ControllerType )
    {
    case CONTROLLERTYPE_LINEAR:
        return "Linear";
        break;
    case CONTROLLERTYPE_SMOOTH:
        return "Smooth";
        break;
    }

    ASSERT( 0 );
    return "";
}

//============================================================================

s32 base::CombineMode_FromString( const char* pString )
{
    if( x_strstr( pString, "Glow - Alpha" ) )
    {
        return COMBINEMODE_GLOW_ALPHA;
    }
    else if( x_strstr( pString, "Glow - Additive" ) )
    {
        return COMBINEMODE_GLOW_ADD;
    }
    else if( x_strstr( pString, "Glow - Subtractive" ) )
    {
        return COMBINEMODE_GLOW_SUB;
    }

    else if( x_strstr( pString, "Alpha" ) )
    {
        return COMBINEMODE_ALPHA;
    }
    else if( x_strstr( pString, "Additive" ) )
    {
        return COMBINEMODE_ADDITIVE;
    }
    else if( x_strstr( pString, "Subtractive" ) )
    {
        return COMBINEMODE_SUBTRACTIVE;
    }

    else if( x_strstr( pString, "Distortion" ) )
    {
        return COMBINEMODE_DISTORT;
    }   

    ASSERT( 0 );
    return 0;
}

//============================================================================

const char* base::CombineMode_ToString( s32 CombineMode )
{
    switch( CombineMode )
    {
    case COMBINEMODE_ALPHA:         return "Alpha";                 break;
    case COMBINEMODE_ADDITIVE:      return "Additive";              break;
    case COMBINEMODE_SUBTRACTIVE:   return "Subtractive";           break;
    case COMBINEMODE_GLOW_ALPHA:    return "Glow - Alpha";          break;
    case COMBINEMODE_GLOW_ADD:      return "Glow - Additive";       break;
    case COMBINEMODE_GLOW_SUB:      return "Glow - Subtractive";    break;
    case COMBINEMODE_DISTORT:       return "Distortion";            break;
    }

    ASSERT( 0 );
    return "";
}

//============================================================================

} // namespace fx_core
