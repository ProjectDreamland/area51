///////////////////////////////////////////////////////////////////////////////
//
//  Player.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#include "Object.hpp"


class player : public object
{
public:
    virtual         type    GetType         ( void ) const { return TYPE_PLAYER; } //  Returns the type of the object
    virtual const   char*   GetTypeName     ( void ) const { return "PLAYER"; }; //  Gets the type name



protected:
    



};
