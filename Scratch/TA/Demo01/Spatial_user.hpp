
#ifndef SPATIAL_USER_HPP
#define SPATIAL_USER_HPP

#define NUM_SPATIAL_CHANNELS     2
#define SPATIAL_CHANNEL_GENERIC  0
#define SPATIAL_CHANNEL_LIGHTS   1

struct spacial_user
{
    spacial_user( void )
    {
        for( s32 j=0; j<NUM_SPATIAL_CHANNELS; j++ )
            FirstObjectLink[j] = -1;
    }

    s16             FirstObjectLink[NUM_SPATIAL_CHANNELS];

};

#endif