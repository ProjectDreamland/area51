//==============================================================================
//  
//  e_Input_Gadget_Defines.hpp
//
//==============================================================================

/*

  This file allows you to define multiple gadget lists without the need to keep
  retyping the types or keeping multiple lists in sync.
  .
  It is currently used to define the gadget enums (see e_input_gadgets.hpp) and
  a string name lookup table (see e_input.hpp).

  NOTE:

  This file cannot be including on it's own - it must be included from another
  file along which defines the following macros:

   BEGIN_GADGETS                                 - Begins gadget list
   DEFINE_GADGET ( __gadget__ )                  - Defines a gadget with the next consecutive value
   DEFINE_GADGET_VALUE ( __gadget__, __value__ ) - Defines a gadget with a given value
   END_GADGETS                                   - Ends the gadget list    

*/


//==============================================================================
// DEFINE THE GADGETS LIST
//==============================================================================

BEGIN_GADGETS

    //=====================================================================
    // Misc input values
    //=====================================================================
    DEFINE_GADGET ( INPUT_UNDEFINED )

    //=====================================================================
    // General Messages for the input system
    //=====================================================================
    DEFINE_GADGET ( INPUT_MSG__BEGIN )

    DEFINE_GADGET ( INPUT_MSG_EXIT )

    DEFINE_GADGET ( INPUT_MSG__END )

    //=====================================================================
    // General question for the input system
    //=====================================================================
    DEFINE_GADGET ( INPUT_QRY__BEGIN )


    DEFINE_GADGET ( INPUT_QRY__END )
/*
    //=====================================================================
    // PS2 Pad gadgets begins
    //=====================================================================
    DEFINE_GADGET ( INPUT_PS2__BEGIN )            
                                
    //---------------------------------------------------------------------
    // PS2 QUERYS
    //---------------------------------------------------------------------

    DEFINE_GADGET ( INPUT_PS2_PAD_PRESENT )         
    DEFINE_GADGET ( INPUT_PS2_MEM_CARD_PRESENT )           

    //---------------------------------------------------------------------
    // PS2 DIGITAL GADGETS
    //---------------------------------------------------------------------
    DEFINE_GADGET ( INPUT_PS2__DIGITAL )

    DEFINE_GADGET ( INPUT_PS2_BTN_ANALOG_MODE )                

    DEFINE_GADGET ( INPUT_PS2_BTN_SELECT )                 
    DEFINE_GADGET ( INPUT_PS2_BTN_START )                  
                      
    DEFINE_GADGET ( INPUT_PS2_BTN_L_STICK )                
    DEFINE_GADGET ( INPUT_PS2_BTN_R_STICK )                

    //---------------------------------------------------------------------
    // PS2 ANALOG GADGETS
    //---------------------------------------------------------------------
    DEFINE_GADGET ( INPUT_PS2__ANALOG )

    DEFINE_GADGET ( INPUT_PS2_BTN_L_UP )                   
    DEFINE_GADGET ( INPUT_PS2_BTN_L_LEFT )                 
    DEFINE_GADGET ( INPUT_PS2_BTN_L_RIGHT )                
    DEFINE_GADGET ( INPUT_PS2_BTN_L_DOWN )                 
                                
    DEFINE_GADGET ( INPUT_PS2_BTN_R_UP )                   
    DEFINE_GADGET ( INPUT_PS2_BTN_R_LEFT )                 
    DEFINE_GADGET ( INPUT_PS2_BTN_R_RIGHT )                
    DEFINE_GADGET ( INPUT_PS2_BTN_R_DOWN )                 

    DEFINE_GADGET ( INPUT_PS2_BTN_L1 )                     
    DEFINE_GADGET ( INPUT_PS2_BTN_L2 )                     
    DEFINE_GADGET ( INPUT_PS2_BTN_R1 )                     
    DEFINE_GADGET ( INPUT_PS2_BTN_R2 )                     
                                
    DEFINE_GADGET ( INPUT_PS2_BTN_TRIANGLE )               
    DEFINE_GADGET ( INPUT_PS2_BTN_SQUARE )                 
    DEFINE_GADGET ( INPUT_PS2_BTN_CIRCLE )                 
    DEFINE_GADGET ( INPUT_PS2_BTN_CROSS )                  

    DEFINE_GADGET ( INPUT_PS2_STICK_LEFT_X )               
    DEFINE_GADGET ( INPUT_PS2_STICK_LEFT_Y )               
    DEFINE_GADGET ( INPUT_PS2_STICK_RIGHT_X )              
    DEFINE_GADGET ( INPUT_PS2_STICK_RIGHT_Y )              
    
    DEFINE_GADGET ( INPUT_PS2__END )              
*/

    DEFINE_GADGET ( INPUT_PS2__BEGIN )

    DEFINE_GADGET ( INPUT_PS2_QRY_PAD_PRESENT )         
    DEFINE_GADGET ( INPUT_PS2_QRY_MEM_CARD_PRESENT )           
    DEFINE_GADGET ( INPUT_PS2_QRY_ANALOG_MODE )
    DEFINE_GADGET ( INPUT_PS2_QRY_MOUSE_PRESENT)
    DEFINE_GADGET ( INPUT_PS2_QRY_KBD_PRESENT)
            
    DEFINE_GADGET ( INPUT_PS2_BTN_L2 )         //0 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_R2 )         //1 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_L1 )         //2 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_R1 )         //3 ANALOG            

    DEFINE_GADGET ( INPUT_PS2_BTN_TRIANGLE )   //4 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_CIRCLE )     //5 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_CROSS )      //6 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_SQUARE )     //7 ANALOG            

    DEFINE_GADGET ( INPUT_PS2_BTN_SELECT )     //8          
    DEFINE_GADGET ( INPUT_PS2_BTN_L_STICK )    //9            
    DEFINE_GADGET ( INPUT_PS2_BTN_R_STICK )    //10            
    DEFINE_GADGET ( INPUT_PS2_BTN_START )      //11            

    DEFINE_GADGET ( INPUT_PS2_BTN_L_UP )       //12 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_L_RIGHT )    //13 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_L_DOWN )     //14 ANALOG            
    DEFINE_GADGET ( INPUT_PS2_BTN_L_LEFT )     //15 ANALOG            
    
    DEFINE_GADGET ( INPUT_PS2_STICK_LEFT_X )   // ANALOG            
    DEFINE_GADGET ( INPUT_PS2_STICK_LEFT_Y )   // ANALOG            
    DEFINE_GADGET ( INPUT_PS2_STICK_RIGHT_X )  // ANALOG            
    DEFINE_GADGET ( INPUT_PS2_STICK_RIGHT_Y )  // ANALOG            

    DEFINE_GADGET ( INPUT_PS2__END )


    //=====================================================================
    // Xbox Pad gadgets begins
    //=====================================================================

    DEFINE_GADGET ( INPUT_XBOX__BEGIN )

    DEFINE_GADGET ( INPUT_XBOX_QRY_PAD_PRESENT )
    DEFINE_GADGET ( INPUT_XBOX_QRY_MEM_CARD_PRESENT )           
    DEFINE_GADGET ( INPUT_XBOX_QRY_ANALOG_MODE )
    DEFINE_GADGET ( INPUT_XBOX_QRY_MOUSE_PRESENT)
    DEFINE_GADGET ( INPUT_XBOX_QRY_KBD_PRESENT)

    DEFINE_GADGET ( INPUT_XBOX__DIGITAL_BUTTONS_BEGIN )
    DEFINE_GADGET ( INPUT_XBOX_BTN_START   )
    DEFINE_GADGET ( INPUT_XBOX_BTN_BACK    )
    DEFINE_GADGET ( INPUT_XBOX_BTN_LEFT    )
    DEFINE_GADGET ( INPUT_XBOX_BTN_RIGHT   )
    DEFINE_GADGET ( INPUT_XBOX_BTN_UP      )
    DEFINE_GADGET ( INPUT_XBOX_BTN_DOWN    )
    DEFINE_GADGET ( INPUT_XBOX_BTN_L_STICK )
    DEFINE_GADGET ( INPUT_XBOX_BTN_R_STICK )
    DEFINE_GADGET ( INPUT_XBOX__DIGITAL_BUTTONS_END )

    DEFINE_GADGET ( INPUT_XBOX__ANALOG_BUTTONS_BEGIN )
    DEFINE_GADGET ( INPUT_XBOX_BTN_WHITE )
    DEFINE_GADGET ( INPUT_XBOX_BTN_BLACK )
    DEFINE_GADGET ( INPUT_XBOX_BTN_A )
    DEFINE_GADGET ( INPUT_XBOX_BTN_B )
    DEFINE_GADGET ( INPUT_XBOX_BTN_X )
    DEFINE_GADGET ( INPUT_XBOX_BTN_Y )
    DEFINE_GADGET ( INPUT_XBOX_L_TRIGGER )
    DEFINE_GADGET ( INPUT_XBOX_R_TRIGGER )
    DEFINE_GADGET ( INPUT_XBOX__ANALOG_BUTTONS_END )

    DEFINE_GADGET ( INPUT_XBOX__STICKS_BEGIN )
    DEFINE_GADGET ( INPUT_XBOX_STICK_LEFT_X  )
    DEFINE_GADGET ( INPUT_XBOX_STICK_LEFT_Y  )
    DEFINE_GADGET ( INPUT_XBOX_STICK_RIGHT_X )
    DEFINE_GADGET ( INPUT_XBOX_STICK_RIGHT_Y )
    DEFINE_GADGET ( INPUT_XBOX__STICKS_END   )

    DEFINE_GADGET ( INPUT_XBOX__END )


    //=====================================================================
    // PC Pad gadgets begin
    //=====================================================================
    DEFINE_GADGET ( INPUT_PC__BEGIN )

    //---------------------------------------------------------------------
    // PC DIGITAL GADGETS
    //---------------------------------------------------------------------
    DEFINE_GADGET ( INPUT_PC__DIGITAL )

    DEFINE_GADGET ( INPUT_PC_BTN_0 )     
    DEFINE_GADGET ( INPUT_PC_BTN_1 )     
    DEFINE_GADGET ( INPUT_PC_BTN_2 )     
    DEFINE_GADGET ( INPUT_PC_BTN_3 )     
    DEFINE_GADGET ( INPUT_PC_BTN_4 )     
    DEFINE_GADGET ( INPUT_PC_BTN_5 )     
    DEFINE_GADGET ( INPUT_PC_BTN_6 )     
    DEFINE_GADGET ( INPUT_PC_BTN_7 )     
    DEFINE_GADGET ( INPUT_PC_BTN_8 )     
    DEFINE_GADGET ( INPUT_PC_BTN_9 )     
    DEFINE_GADGET ( INPUT_PC_BTN_10 )    
    DEFINE_GADGET ( INPUT_PC_BTN_11 )    
    DEFINE_GADGET ( INPUT_PC_BTN_12 )    
    DEFINE_GADGET ( INPUT_PC_BTN_13 )    
    DEFINE_GADGET ( INPUT_PC_BTN_14 )    
    DEFINE_GADGET ( INPUT_PC_BTN_15 )    
    DEFINE_GADGET ( INPUT_PC_BTN_16 )    
    DEFINE_GADGET ( INPUT_PC_BTN_17 )    
    DEFINE_GADGET ( INPUT_PC_BTN_18 )    
    DEFINE_GADGET ( INPUT_PC_BTN_19 )    
    DEFINE_GADGET ( INPUT_PC_BTN_20 )    
    DEFINE_GADGET ( INPUT_PC_BTN_21 )    
    DEFINE_GADGET ( INPUT_PC_BTN_22 )    
    DEFINE_GADGET ( INPUT_PC_BTN_23 )    
    DEFINE_GADGET ( INPUT_PC_BTN_24 )    
    DEFINE_GADGET ( INPUT_PC_BTN_25 )    
    DEFINE_GADGET ( INPUT_PC_BTN_26 )    
    DEFINE_GADGET ( INPUT_PC_BTN_27 )    
    DEFINE_GADGET ( INPUT_PC_BTN_28 )    
    DEFINE_GADGET ( INPUT_PC_BTN_29 )    
    DEFINE_GADGET ( INPUT_PC_BTN_30 )    
    DEFINE_GADGET ( INPUT_PC_BTN_31 )    

    //---------------------------------------------------------------------
    // PC ANALOG GADGETS
    //---------------------------------------------------------------------
    DEFINE_GADGET ( INPUT_PC__ANALOG )

    DEFINE_GADGET ( INPUT_PC_STICK_X ) 
    DEFINE_GADGET ( INPUT_PC_STICK_Y ) 
    DEFINE_GADGET ( INPUT_PC_STICK_Z ) 
    DEFINE_GADGET ( INPUT_PC_STICK_RX )
    DEFINE_GADGET ( INPUT_PC_STICK_RY )
    DEFINE_GADGET ( INPUT_PC_STICK_RZ )

    DEFINE_GADGET ( INPUT_PC__END )


    //=====================================================================
    // Mouse gadgets begins
    //=====================================================================
    DEFINE_GADGET ( INPUT_MOUSE__BEGIN )          

    //---------------------------------------------------------------------
    // MOUSE DIGITAL GADGETS
    //---------------------------------------------------------------------
    DEFINE_GADGET ( INPUT_MOUSE__DIGITAL )

    DEFINE_GADGET ( INPUT_MOUSE_BTN_L )                
    DEFINE_GADGET ( INPUT_MOUSE_BTN_C )                
    DEFINE_GADGET ( INPUT_MOUSE_BTN_R )                

    DEFINE_GADGET ( INPUT_MOUSE_BTN_0 )
    DEFINE_GADGET ( INPUT_MOUSE_BTN_1 )
    DEFINE_GADGET ( INPUT_MOUSE_BTN_2 )
    DEFINE_GADGET ( INPUT_MOUSE_BTN_3 )
    DEFINE_GADGET ( INPUT_MOUSE_BTN_4 )
            
    //---------------------------------------------------------------------
    // ANALOG ANALOG GADGETS
    //---------------------------------------------------------------------    
    DEFINE_GADGET ( INPUT_MOUSE__ANALOG )

    DEFINE_GADGET ( INPUT_MOUSE_X_REL )                
    DEFINE_GADGET ( INPUT_MOUSE_Y_REL )                
    DEFINE_GADGET ( INPUT_MOUSE_WHEEL_REL )             
                                
    DEFINE_GADGET ( INPUT_MOUSE_X_ABS )                
    DEFINE_GADGET ( INPUT_MOUSE_Y_ABS )                
    DEFINE_GADGET ( INPUT_MOUSE_WHEEL_ABS )             
                                

    DEFINE_GADGET ( INPUT_MOUSE__END )           // Mouse gadgets begins
                                
    //=====================================================================
    // Keyboard gadgets begins
    //=====================================================================
    DEFINE_GADGET ( INPUT_KBD__BEGIN )

    //---------------------------------------------------------------------
    // KEYBOARD DIGITAL GADGETS
    //---------------------------------------------------------------------
    DEFINE_GADGET ( INPUT_KBD__DIGITAL )

    DEFINE_GADGET_VALUE ( INPUT_KBD_ESCAPE           , INPUT_KBD__BEGIN + 0x01 ) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_1                , INPUT_KBD__BEGIN + 0x02 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_2                , INPUT_KBD__BEGIN + 0x03 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_3                , INPUT_KBD__BEGIN + 0x04 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_4                , INPUT_KBD__BEGIN + 0x05 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_5                , INPUT_KBD__BEGIN + 0x06 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_6                , INPUT_KBD__BEGIN + 0x07 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_7                , INPUT_KBD__BEGIN + 0x08 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_8                , INPUT_KBD__BEGIN + 0x09 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_9                , INPUT_KBD__BEGIN + 0x0A )
    DEFINE_GADGET_VALUE ( INPUT_KBD_0                , INPUT_KBD__BEGIN + 0x0B )
    DEFINE_GADGET_VALUE ( INPUT_KBD_MINUS            , INPUT_KBD__BEGIN + 0x0C )   // - on main keyboard 
    DEFINE_GADGET_VALUE ( INPUT_KBD_EQUALS           , INPUT_KBD__BEGIN + 0x0D )
    DEFINE_GADGET_VALUE ( INPUT_KBD_BACK             , INPUT_KBD__BEGIN + 0x0E )    // backspace 
    DEFINE_GADGET_VALUE ( INPUT_KBD_TAB              , INPUT_KBD__BEGIN + 0x0F )
    DEFINE_GADGET_VALUE ( INPUT_KBD_Q                , INPUT_KBD__BEGIN + 0x10 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_W                , INPUT_KBD__BEGIN + 0x11 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_E                , INPUT_KBD__BEGIN + 0x12 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_R                , INPUT_KBD__BEGIN + 0x13 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_T                , INPUT_KBD__BEGIN + 0x14 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_Y                , INPUT_KBD__BEGIN + 0x15 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_U                , INPUT_KBD__BEGIN + 0x16 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_I                , INPUT_KBD__BEGIN + 0x17 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_O                , INPUT_KBD__BEGIN + 0x18 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_P                , INPUT_KBD__BEGIN + 0x19 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_LBRACKET         , INPUT_KBD__BEGIN + 0x1A )
    DEFINE_GADGET_VALUE ( INPUT_KBD_RBRACKET         , INPUT_KBD__BEGIN + 0x1B )
    DEFINE_GADGET_VALUE ( INPUT_KBD_RETURN           , INPUT_KBD__BEGIN + 0x1C )   // Enter on main keyboard 
    DEFINE_GADGET_VALUE ( INPUT_KBD_LCONTROL         , INPUT_KBD__BEGIN + 0x1D )
    DEFINE_GADGET_VALUE ( INPUT_KBD_A                , INPUT_KBD__BEGIN + 0x1E )
    DEFINE_GADGET_VALUE ( INPUT_KBD_S                , INPUT_KBD__BEGIN + 0x1F )
    DEFINE_GADGET_VALUE ( INPUT_KBD_D                , INPUT_KBD__BEGIN + 0x20 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F                , INPUT_KBD__BEGIN + 0x21 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_G                , INPUT_KBD__BEGIN + 0x22 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_H                , INPUT_KBD__BEGIN + 0x23 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_J                , INPUT_KBD__BEGIN + 0x24 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_K                , INPUT_KBD__BEGIN + 0x25 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_L                , INPUT_KBD__BEGIN + 0x26 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_SEMICOLON        , INPUT_KBD__BEGIN + 0x27 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_APOSTROPHE       , INPUT_KBD__BEGIN + 0x28 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_GRAVE            , INPUT_KBD__BEGIN + 0x29 )    // accent grave 
    DEFINE_GADGET_VALUE ( INPUT_KBD_LSHIFT           , INPUT_KBD__BEGIN + 0x2A )
    DEFINE_GADGET_VALUE ( INPUT_KBD_BACKSLASH        , INPUT_KBD__BEGIN + 0x2B )
    DEFINE_GADGET_VALUE ( INPUT_KBD_Z                , INPUT_KBD__BEGIN + 0x2C )
    DEFINE_GADGET_VALUE ( INPUT_KBD_X                , INPUT_KBD__BEGIN + 0x2D )
    DEFINE_GADGET_VALUE ( INPUT_KBD_C                , INPUT_KBD__BEGIN + 0x2E )
    DEFINE_GADGET_VALUE ( INPUT_KBD_V                , INPUT_KBD__BEGIN + 0x2F )
    DEFINE_GADGET_VALUE ( INPUT_KBD_B                , INPUT_KBD__BEGIN + 0x30 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_N                , INPUT_KBD__BEGIN + 0x31 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_M                , INPUT_KBD__BEGIN + 0x32 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_COMMA            , INPUT_KBD__BEGIN + 0x33 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_PERIOD           , INPUT_KBD__BEGIN + 0x34 )    // . on main keyboard 
    DEFINE_GADGET_VALUE ( INPUT_KBD_SLASH            , INPUT_KBD__BEGIN + 0x35 )    // / on main keyboard 
    DEFINE_GADGET_VALUE ( INPUT_KBD_RSHIFT           , INPUT_KBD__BEGIN + 0x36 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_MULTIPLY         , INPUT_KBD__BEGIN + 0x37 )    // * on numeric keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_LMENU            , INPUT_KBD__BEGIN + 0x38 )    // left Alt 
    DEFINE_GADGET_VALUE ( INPUT_KBD_SPACE            , INPUT_KBD__BEGIN + 0x39 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_CAPITAL          , INPUT_KBD__BEGIN + 0x3A )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F1               , INPUT_KBD__BEGIN + 0x3B )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F2               , INPUT_KBD__BEGIN + 0x3C )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F3               , INPUT_KBD__BEGIN + 0x3D )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F4               , INPUT_KBD__BEGIN + 0x3E )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F5               , INPUT_KBD__BEGIN + 0x3F )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F6               , INPUT_KBD__BEGIN + 0x40 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F7               , INPUT_KBD__BEGIN + 0x41 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F8               , INPUT_KBD__BEGIN + 0x42 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F9               , INPUT_KBD__BEGIN + 0x43 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F10              , INPUT_KBD__BEGIN + 0x44 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMLOCK          , INPUT_KBD__BEGIN + 0x45 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_SCROLL           , INPUT_KBD__BEGIN + 0x46 )    // Scroll Lock 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD7          , INPUT_KBD__BEGIN + 0x47 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD8          , INPUT_KBD__BEGIN + 0x48 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD9          , INPUT_KBD__BEGIN + 0x49 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_SUBTRACT         , INPUT_KBD__BEGIN + 0x4A )    // - on numeric keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD4          , INPUT_KBD__BEGIN + 0x4B )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD5          , INPUT_KBD__BEGIN + 0x4C )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD6          , INPUT_KBD__BEGIN + 0x4D )
    DEFINE_GADGET_VALUE ( INPUT_KBD_ADD              , INPUT_KBD__BEGIN + 0x4E )    // + on numeric keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD1          , INPUT_KBD__BEGIN + 0x4F )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD2          , INPUT_KBD__BEGIN + 0x50 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD3          , INPUT_KBD__BEGIN + 0x51 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPAD0          , INPUT_KBD__BEGIN + 0x52 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_DECIMAL          , INPUT_KBD__BEGIN + 0x53 )    // . on numeric keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_OEM_102          , INPUT_KBD__BEGIN + 0x56 )    // < > | on UK/Germany keyboards 
    DEFINE_GADGET_VALUE ( INPUT_KBD_F11              , INPUT_KBD__BEGIN + 0x57 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F12              , INPUT_KBD__BEGIN + 0x58 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_F13              , INPUT_KBD__BEGIN + 0x64 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_F14              , INPUT_KBD__BEGIN + 0x65 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_F15              , INPUT_KBD__BEGIN + 0x66 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_KANA             , INPUT_KBD__BEGIN + 0x70 )    // (Japanese keyboard)            
    DEFINE_GADGET_VALUE ( INPUT_KBD_ABNT_C1          , INPUT_KBD__BEGIN + 0x73 )    // / ? on Portugese (Brazilian) keyboards 
    DEFINE_GADGET_VALUE ( INPUT_KBD_CONVERT          , INPUT_KBD__BEGIN + 0x79 )    // (Japanese keyboard)            
    DEFINE_GADGET_VALUE ( INPUT_KBD_NOCONVERT        , INPUT_KBD__BEGIN + 0x7B )    // (Japanese keyboard)            
    DEFINE_GADGET_VALUE ( INPUT_KBD_YEN              , INPUT_KBD__BEGIN + 0x7D )    // (Japanese keyboard)            
    DEFINE_GADGET_VALUE ( INPUT_KBD_ABNT_C2          , INPUT_KBD__BEGIN + 0x7E )    // Numpad . on Portugese (Brazilian) keyboards 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPADEQUALS     , INPUT_KBD__BEGIN + 0x8D )    // = on numeric keypad (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_PREVTRACK        , INPUT_KBD__BEGIN + 0x90 )    // Previous Track (DEFINE_GADGET_VALUE ( INPUT_KBD_CIRCUMFLEX on Japanese keyboard) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_AT               , INPUT_KBD__BEGIN + 0x91 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_COLON            , INPUT_KBD__BEGIN + 0x92 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_UNDERLINE        , INPUT_KBD__BEGIN + 0x93 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_KANJI            , INPUT_KBD__BEGIN + 0x94 )    // (Japanese keyboard)            
    DEFINE_GADGET_VALUE ( INPUT_KBD_STOP             , INPUT_KBD__BEGIN + 0x95 )    //                     (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_AX               , INPUT_KBD__BEGIN + 0x96 )    //                     (Japan AX) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_UNLABELED        , INPUT_KBD__BEGIN + 0x97 )    //                        (J3100) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NEXTTRACK        , INPUT_KBD__BEGIN + 0x99 )    // Next Track 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPADENTER      , INPUT_KBD__BEGIN + 0x9C )    // Enter on numeric keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_RCONTROL         , INPUT_KBD__BEGIN + 0x9D )
    DEFINE_GADGET_VALUE ( INPUT_KBD_MUTE             , INPUT_KBD__BEGIN + 0xA0 )    // Mute 
    DEFINE_GADGET_VALUE ( INPUT_KBD_CALCULATOR       , INPUT_KBD__BEGIN + 0xA1 )    // Calculator 
    DEFINE_GADGET_VALUE ( INPUT_KBD_PLAYPAUSE        , INPUT_KBD__BEGIN + 0xA2 )    // Play / Pause 
    DEFINE_GADGET_VALUE ( INPUT_KBD_MEDIASTOP        , INPUT_KBD__BEGIN + 0xA4 )    // Media Stop 
    DEFINE_GADGET_VALUE ( INPUT_KBD_VOLUMEDOWN       , INPUT_KBD__BEGIN + 0xAE )    // Volume - 
    DEFINE_GADGET_VALUE ( INPUT_KBD_VOLUMEUP         , INPUT_KBD__BEGIN + 0xB0 )    // Volume + 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBHOME          , INPUT_KBD__BEGIN + 0xB2 )    // Web home 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NUMPADCOMMA      , INPUT_KBD__BEGIN + 0xB3 )    //  ) on numeric keypad (NEC PC98) 
    DEFINE_GADGET_VALUE ( INPUT_KBD_DIVIDE           , INPUT_KBD__BEGIN + 0xB5 )    // / on numeric keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_SYSRQ            , INPUT_KBD__BEGIN + 0xB7 )
    DEFINE_GADGET_VALUE ( INPUT_KBD_RMENU            , INPUT_KBD__BEGIN + 0xB8 )    // right Alt 
    DEFINE_GADGET_VALUE ( INPUT_KBD_PAUSE            , INPUT_KBD__BEGIN + 0xC5 )    // Pause 
    DEFINE_GADGET_VALUE ( INPUT_KBD_HOME             , INPUT_KBD__BEGIN + 0xC7 )    // Home on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_UP               , INPUT_KBD__BEGIN + 0xC8 )    // UpArrow on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_PRIOR            , INPUT_KBD__BEGIN + 0xC9 )    // PgUp on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_LEFT             , INPUT_KBD__BEGIN + 0xCB )    // LeftArrow on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_RIGHT            , INPUT_KBD__BEGIN + 0xCD )    // RightArrow on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_END              , INPUT_KBD__BEGIN + 0xCF )    // End on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_DOWN             , INPUT_KBD__BEGIN + 0xD0 )    // DownArrow on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_NEXT             , INPUT_KBD__BEGIN + 0xD1 )    // PgDn on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_INSERT           , INPUT_KBD__BEGIN + 0xD2 )    // Insert on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_DELETE           , INPUT_KBD__BEGIN + 0xD3 )    // Delete on arrow keypad 
    DEFINE_GADGET_VALUE ( INPUT_KBD_LWIN             , INPUT_KBD__BEGIN + 0xDB )    // Left Windows key 
    DEFINE_GADGET_VALUE ( INPUT_KBD_RWIN             , INPUT_KBD__BEGIN + 0xDC )    // Right Windows key 
    DEFINE_GADGET_VALUE ( INPUT_KBD_APPS             , INPUT_KBD__BEGIN + 0xDD )    // AppMenu key 
    DEFINE_GADGET_VALUE ( INPUT_KBD_POWER            , INPUT_KBD__BEGIN + 0xDE )    // System Power 
    DEFINE_GADGET_VALUE ( INPUT_KBD_SLEEP            , INPUT_KBD__BEGIN + 0xDF )    // System Sleep 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WAKE             , INPUT_KBD__BEGIN + 0xE3 )    // System Wake 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBSEARCH        , INPUT_KBD__BEGIN + 0xE5 )    // Web Search 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBFAVORITES     , INPUT_KBD__BEGIN + 0xE6 )    // Web Favorites 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBREFRESH       , INPUT_KBD__BEGIN + 0xE7 )    // Web Refresh 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBSTOP          , INPUT_KBD__BEGIN + 0xE8 )    // Web Stop 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBFORWARD       , INPUT_KBD__BEGIN + 0xE9 )    // Web Forward 
    DEFINE_GADGET_VALUE ( INPUT_KBD_WEBBACK          , INPUT_KBD__BEGIN + 0xEA )    // Web Back 
    DEFINE_GADGET_VALUE ( INPUT_KBD_MYCOMPUTER       , INPUT_KBD__BEGIN + 0xEB )    // My Computer 
    DEFINE_GADGET_VALUE ( INPUT_KBD_MAIL             , INPUT_KBD__BEGIN + 0xEC )    // Mail 
    DEFINE_GADGET_VALUE ( INPUT_KBD_MEDIASELECT      , INPUT_KBD__BEGIN + 0xED )    // Media Select 

    DEFINE_GADGET ( INPUT_KBD__END )
    
END_GADGETS


//==============================================================================
// END THE GADGETS LIST
//==============================================================================


//==============================================================================
//  MAKE SURE THE MACROS ARE CLEARED SO THEY CAN BE USED AGAIN
//==============================================================================
#undef BEGIN_GADGETS
#undef DEFINE_GADGET
#undef DEFINE_GADGET_VALUE
#undef END_GADGETS

//==============================================================================
