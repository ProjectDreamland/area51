///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  emotion_controller.cpp
//
//      -The emotion controller handles the emotions for the actor.  The emotions are really
//       just numbers that represent their feelings in different categories.
//
//       As a general rule, emotion values should be added and checked using the enums so that any
//       tuning can be done in the values they use 
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "emotion_controller.hpp"


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


f32  emotion_controller::m_EmotionModifierValues[ EMOTION_MODIFIER_LAST ] = 
{
    -1000.0f,   //EMOTION_MODIFIER_DECREASE_MAXIMUM = 0,
     -100.0f,   //EMOTION_MODIFIER_DECREASE_HUGE,
      -50.0f,   //EMOTION_MODIFIER_DECREASE_LARGE,
      -20.0f,   //EMOTION_MODIFIER_DECREASE_MEDIUM,
      -10.0f,   //EMOTION_MODIFIER_DECREASE_SMALL,
       -2.0f,    //EMOTION_MODIFIER_DECREASE_MINIMAL,

        2.0f,   //EMOTION_MODIFIER_INCREASE_MINIMAL,
       10.0f,   //EMOTION_MODIFIER_INCREASE_SMALL,
       20.0f,   //EMOTION_MODIFIER_INCREASE_MEDIUM,
       50.0f,   //EMOTION_MODIFIER_INCREASE_LARGE,
      100.0f,   //EMOTION_MODIFIER_INCREASE_HUGE,
     1000.0f    //EMOTION_MODIFIER_INCREASE_MAXIMUM,

};



emotion_controller::emotion_controller(void)
{
    Init();


}


emotion_controller::~emotion_controller()
{



}



void emotion_controller::Init(void)
{
    u32 count;
    for(count =0; count < EMOTION_LAST; count++)
        m_EmotionValues[count] = 0.0f;

    m_MemoryLength = 1.0f;

}


void emotion_controller::AddEmotion(emotion_types thisType, f32 thisMuch )
{
    ASSERT(thisType < EMOTION_LAST );

    m_EmotionValues[thisType] += thisMuch;

}


void emotion_controller::AddEmotion(emotion_types thisType, emotion_modifier_amount thisMuch )
{
    ASSERT( thisType < EMOTION_LAST );
    ASSERT( thisMuch < EMOTION_MODIFIER_LAST );

    m_EmotionValues[ thisType ] += m_EmotionModifierValues[ thisMuch ];

}

    
void emotion_controller::SetMemoryLength( f32 memoryLength )
{
    
    m_MemoryLength = memoryLength;
}


void emotion_controller::AdvanceLogic(f32 deltaTime )
{
    (void)deltaTime;

}


f32 emotion_controller::GetEmotion( emotion_types thisType )
{
    ASSERT( thisType < EMOTION_LAST );
    
    return ( m_EmotionValues[ thisType ]);

}


emotion_controller::emotion_level emotion_controller::GetEmotionLevel( emotion_types thisType )
{
    ASSERT( thisType < EMOTION_LAST );
    
    if( m_EmotionValues[thisType]  <= 0.0f )
    {
        return EMOTION_LEVEL_ZERO;
    }
    else if( m_EmotionValues[thisType] <= 10.0f )
    {
        return EMOTION_LEVEL_MINIMAL;
    }
    else if( m_EmotionValues[thisType] <= 50.0f )
    {
        return EMOTION_LEVEL_MODERATE;
    }
    else if( m_EmotionValues[thisType] <= 250.0f )
    {
        return EMOTION_LEVEL_HIGH;
    }
    else 
    {
        return EMOTION_LEVEL_EXTREME;
    }



}

const char* emotion_controller::GetEmotionLevelString( emotion_level thisLevel )
{

    switch( thisLevel )
    {
        case EMOTION_LEVEL_ZERO:        return "EMOTION_LEVEL_ZERO";
        case EMOTION_LEVEL_MINIMAL:     return "EMOTION_LEVEL_MINIMAL";
        case EMOTION_LEVEL_MODERATE:    return "EMOTION_LEVEL_MODERATE";
        case EMOTION_LEVEL_HIGH:        return "EMOTION_LEVEL_HIGH";
        case EMOTION_LEVEL_EXTREME:     return "EMOTION_LEVEL_EXTREME";
        case EMOTION_LEVEL_IMPOSSIBLE:  return "EMOTION_LEVEL_IMPOSSIBLE";
    }

    //   Should never reach this point so I'm throwing in an assert false for safety
    ASSERT(FALSE);     
    return NULL;

    
}

emotion_controller::emotion_level emotion_controller::GetEmotionLevelFromString( const char* thisString)
{
    if( !x_strcmp(thisString, "EMOTION_LEVEL_ZERO" ))
    {
        return EMOTION_LEVEL_ZERO;
    }
    else if( !x_strcmp(thisString, "EMOTION_LEVEL_MINIMAL" ))
    {
        return EMOTION_LEVEL_MINIMAL;
    }
    else if( !x_strcmp(thisString, "EMOTION_LEVEL_MODERATE" ))
    {
        return EMOTION_LEVEL_MODERATE;
    }
    else if( !x_strcmp(thisString, "EMOTION_LEVEL_HIGH" ))
    {
        return EMOTION_LEVEL_HIGH;
    }
    else if( !x_strcmp(thisString, "EMOTION_LEVEL_EXTREME" ))
    {
        return EMOTION_LEVEL_EXTREME;
    }
    else if( !x_strcmp(thisString, "EMOTION_LEVEL_IMPOSSIBLE" ))
    {
        return EMOTION_LEVEL_IMPOSSIBLE;
    }

    //   Should never reach this point so I'm throwing in an assert false for safety
    ASSERT(FALSE);

    return EMOTION_LEVEL_ZERO;


}

void emotion_controller::OnEnumProp( prop_enum&  List )
{
    List.AddHeader(     "Emotion Controller",  
                        "Represents an AI's emotional state" 
                        );



    List.AddFloat(     "Emotion Controller\\Fear",  
                        "Represents the level of the AI's Fear "                      
                        );
    List.AddFloat(     "Emotion Controller\\Anger",  
                        "Represents the level of the AI's Anger "                      
                        );
    List.AddFloat(     "Emotion Controller\\Hunger",  
                        "Represents the level of the AI's Hunger "                      
                        );
    List.AddFloat(     "Emotion Controller\\Curiosity",  
                        "Represents the level of the AI's Curiosity "                      
                        );
    List.AddFloat(     "Emotion Controller\\Boredom",  
                        "Represents the level of the AI's Boredom "                      
                        );

    List.AddFloat(     "Emotion Controller\\MemoryLength",  
                        "Represents the length of an AI's memory, base of 1.0f"                      
                        );


}




xbool emotion_controller::OnProperty( prop_query& I    )
{




    if( I.IsVar( "Emotion Controller\\Fear" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_EmotionValues[EMOTION_FEAR] );
  
        }
        else
        {
            m_EmotionValues[EMOTION_FEAR] = I.GetVarFloat();
        }
  
        return TRUE;

    }


    if( I.IsVar( "Emotion Controller\\Anger" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_EmotionValues[EMOTION_ANGER] );
  
        }
        else
        {
            m_EmotionValues[EMOTION_ANGER] = I.GetVarFloat();
        }

        return TRUE;

    }


    if( I.IsVar( "Emotion Controller\\Hunger" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_EmotionValues[EMOTION_HUNGER] );
  
        }
        else
        {
            m_EmotionValues[EMOTION_HUNGER] = I.GetVarFloat();
        }
 
        return TRUE;

    }


    if( I.IsVar( "Emotion Controller\\Curiosity" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_EmotionValues[EMOTION_CURIOSITY] );
  
        }
        else
        {
            m_EmotionValues[EMOTION_CURIOSITY] = I.GetVarFloat();
        }

        return TRUE;

    }


    if( I.IsVar( "Emotion Controller\\Boredom" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_EmotionValues[EMOTION_BOREDOM] );
  
        }
        else
        {
            m_EmotionValues[EMOTION_BOREDOM] = I.GetVarFloat();
        }
 
        return TRUE;

    }


    if( I.IsVar( "Emotion Controller\\MemoryLength" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MemoryLength );
  
        }
        else
        {
            m_MemoryLength = I.GetVarFloat();
        }
 
        return TRUE;

    }

    return FALSE;

}