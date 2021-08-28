#ifndef __CONVERSATION_PACKET_HPP__
#define __CONVERSATION_PACKET_HPP__

enum conversation_type
{
    TYPE_INVALID = -1,
    CONV_REQUEST_FOLLOW,
    CONV_REQUEST_STAY,
    CONV_REQUEST_SENTRY,
    CONV_REQUEST_COVER_ME,
    CONV_REQUEST_TAKE_COVER,
    CONV_REQUEST_ACTIVATE_ITEM,
    CONV_REQUEST_COMPLAIN,
    
    CONV_ENEMY_SIGHTED,
    TYPE_MAX
};

struct conversation_packet
{
    conversation_packet() :
        m_ConvType( TYPE_INVALID ),
        m_SoundEmitterGuid( 0 ),
        m_SpeakerGuid( 0 )
    {
    }
        
    conversation_type m_ConvType ;
    guid              m_SoundEmitterGuid ;
    guid              m_SpeakerGuid ;
};

#endif