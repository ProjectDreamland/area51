///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  trigger_volume.hpp
//
//      trigger_volume adds volume functionality to triger.  I seperated this class since it seemed
//      the volume functionality would be used in at least a few other triggers but not in all of
//      the triggers.  
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_VOLUME_HPP
#define TRIGGER_VOLUME_HPP

#include "object\spatial_volume.hpp"

class trigger_volume : public trigger
{
public:

                            trigger_volume(void);
    virtual                 ~trigger_volume();


    virtual void            OnUpdate( f32 deltaTime );

    virtual void            SetPeriod(f32 newPeriod );    

    spatial_volume&         GetVolume( void ) { return m_Volume; }
    

protected:

    spatial_volume          m_Volume;



};


#endif//TRIGGER_OBJECT_IN_VOLUME_HPP