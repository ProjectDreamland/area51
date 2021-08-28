///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  emotion_controller.hpp
//
//      -The emotion controller handles the emotions for the actor.  The emotions are really
//       just numbers that represent their feelings in different categories.
//
//       As a general rule, emotion values should be added and checked using the enums so that any
//       tuning can be done in the values they use 
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef EMOTION_CONTROLLER_HPP
#define EMOTION_CONTROLLER_HPP


#include "x_math.hpp"
#include "x_debug.hpp"
#include "MiscUtils\RTTI.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"



class emotion_controller 
{
public:
    enum emotion_types
    {
        EMOTION_FEAR        = 0,
        EMOTION_ANGER       = 1,
        EMOTION_HUNGER      = 2,
        EMOTION_CURIOSITY   = 3,
        EMOTION_BOREDOM     = 4,
        EMOTION_LAST           ,
        EMOTION_FORCE32BIT = 0xFFFFFFFF
    };

    enum emotion_modifier_amount
    {
        EMOTION_MODIFIER_DECREASE_MAXIMUM = 0,
        EMOTION_MODIFIER_DECREASE_HUGE,
        EMOTION_MODIFIER_DECREASE_LARGE,
        EMOTION_MODIFIER_DECREASE_MEDIUM,
        EMOTION_MODIFIER_DECREASE_SMALL,
        EMOTION_MODIFIER_DECREASE_MINIMAL,

        EMOTION_MODIFIER_INCREASE_MINIMAL,
        EMOTION_MODIFIER_INCREASE_SMALL,
        EMOTION_MODIFIER_INCREASE_MEDIUM,
        EMOTION_MODIFIER_INCREASE_LARGE,
        EMOTION_MODIFIER_INCREASE_HUGE,
        EMOTION_MODIFIER_INCREASE_MAXIMUM,
        EMOTION_MODIFIER_LAST,

        EMOTION_MODIFIER_FORCE32BIT = 0xFFFFFFFF
    
    
    };

    enum emotion_level
    {
        EMOTION_LEVEL_ZERO,
        EMOTION_LEVEL_MINIMAL,
        EMOTION_LEVEL_MODERATE,
        EMOTION_LEVEL_HIGH,
        EMOTION_LEVEL_EXTREME,
        EMOTION_LEVEL_IMPOSSIBLE,

        EMOTION_LEVEL_FORCE32BIT = 0xFFFFFFFF
    };

                    emotion_controller(void);
    virtual         ~emotion_controller();

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Init()  - Resets all emotions, called by constructor and can be called to clear all emotions
//
//  AddEmotion() - Can be called with either a float or with an enum.  Enum will get translated to
//                  a value and just gives a way to call with more consistent values instead of 
//                  guessing for a good number.
//
//  SetMemoryLength() - Set's how often it will reduce it's emotional level.  If set to zero, it 
//                  never reduces emotion level.
//
//  AdvanceLogic() - Standard advance logic call to give the emotions time to normalize slightly if
//                  the emotions are set with a memory length
//
//  GetEmotion() - Returns the current emotion level as a float
//
//  GetEmotionLevel() - Returns emotions as a level
//
///////////////////////////////////////////////////////////////////////////////////////////////////


    virtual void    Init(void);

    virtual void    AddEmotion(emotion_types thisType, f32 thisMuch );
    virtual void    AddEmotion(emotion_types thisType, emotion_modifier_amount thisMuch );
    
    virtual void    SetMemoryLength( f32 memoryLength );

    virtual void    AdvanceLogic(f32 deltaTime );

    virtual f32     GetEmotion( emotion_types thisType );
    virtual emotion_level GetEmotionLevel( emotion_types thisType );
    static const char* GetEmotionLevelString( emotion_level thisLevel ); 
    static emotion_controller::emotion_level GetEmotionLevelFromString( const char* thisString);


    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );

    


protected:

    f32         m_EmotionValues[ EMOTION_LAST ];

    static f32  m_EmotionModifierValues[ EMOTION_MODIFIER_LAST ];

    f32         m_MemoryLength;

    

};


#endif//EMOTION_CONTROLLER_HPP