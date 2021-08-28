//=========================================================================
//
//  LocoUtil.hpp
//
//=========================================================================
#ifndef __LOCO_UTIL_HPP__
#define __LOCO_UTIL_HPP__


//=========================================================================
// INLCLUDES
//=========================================================================

#include "Loco.hpp"


//=========================================================================
// FORWARD REFERENCES
//=========================================================================

class prop_enum ;
class prop_query ;


//=========================================================================
// FUNCTIONS
//=========================================================================


//=========================================================================
// DESCRIPTION:
//  Automatically adds the specified locomotion animations flag
//  properties to the property list.
//-------------------------------------------------------------------------
// IN:
//  rPropList     - The property list container
//  PropAnimFlags - The flags for which you want properties to be added
//  CurrAnimFlags - The current animation flag values you have
//-------------------------------------------------------------------------
// OUT:
//  None
//=========================================================================
void LocoUtil_OnEnumPropAnimFlags( prop_enum& rPropList, 
                                          u32 PropAnimFlags,
                                          u32 CurrAnimFlags ) ;



//=========================================================================
// DESCRIPTION:
//  Automatically updates your animation members from the properties
//  NOTE: You must use in conjuction with "LocoUtil_OnEnumPropAnimFlags"
//-------------------------------------------------------------------------
// IN:
//  rPropQuery    - The current property you are querying
//  AnimGroupName - String manger index of the animation group name
//  AnimName      - String manger index of the animation name
//  AnimFlags     - The current animation flag values you have
//  PlayTime      - The current play time value you have
//-------------------------------------------------------------------------
// OUT:
//  AnimGroupName - String manger index of the animation group name
//  AnimName      - String manger index of the animation name
//  AnimFlags     - The current animation flag values you have
//  PlayTime      - The current play time value you have
//=========================================================================
xbool LocoUtil_OnPropertyAnimFlags( prop_query& rPropQuery, 
                                           s32& AnimGroupName, 
                                           s32& AnimName, 
                                           u32& AnimFlags, 
                                           f32& PlayTime ) ;

// Same as above function, but you don't need a "PlayTime" parameter
xbool LocoUtil_OnPropertyAnimFlags( prop_query& rPropQuery, 
                                           s32& AnimGroupName, 
                                           s32& AnimName, 
                                           u32& AnimFlags ) ;

// Same as above function, but you don't need a "PlayTime" parameter and it sets the anim group name
xbool LocoUtil_OnPropertyAnimFlags(         prop_query& rPropQuery, 
                                                   s32& AnimGroupName, 
                                                   s32& AnimName, 
                                                   u32& AnimFlags,
                                    anim_group::handle& hAnimGroup ) ;

//=========================================================================
#endif  // #ifndef __LOCO_UTIL_HPP__

