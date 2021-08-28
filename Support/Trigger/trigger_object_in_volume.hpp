///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  trigger_object_in_volume.hpp
//
//      trigger_object_in_volume checks to see if an object is in a spatial_volume.  It can either
//      check every update or periodicly if a period is set
//   
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_VOLUME_HPP
#define TRIGGER_VOLUME_HPP

#include "object\spatial_volume.hpp"
#include "trigger_volume.hpp"

class trigger_object_in_volume : public trigger_volume
{
public:

                            trigger_object_in_volume(void);
    virtual                 ~trigger_object_in_volume();



    virtual trigger_type    GetTriggerType(void)  { return trigger::TYPE_OBJECT_IN_VOLUME; }

    virtual xbool           IsTriggered( void );


    virtual void            OnUpdate( f32 deltaTime );

    

protected:



};


#endif//TRIGGER_OBJECT_IN_VOLUME_HPP