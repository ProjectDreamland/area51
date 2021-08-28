///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SENSORY_RECORD_HPP
#define SENSORY_RECORD_HPP


enum type_of_contact
{
    SENSORY_TYPE_LINE_OF_SIGHT = 0  ,
    SENSORY_TYPE_HEARING            ,
    SENSORY_TYPE_SMELL              ,
    SENSORY_TYPE_LAST = 0xFFFFFFFF        
};



struct sensory_record 
{
 
    vector3             m_PointOfContact;
    f32                 m_TimeOfContact ;
    type_of_contact     m_TypeOfContact ;
    guid                m_ContactObject ;
    xbool               m_Enemy;

};



#endif//SENSORY_RECORD_HPP