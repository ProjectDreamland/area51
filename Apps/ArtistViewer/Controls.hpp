#ifndef __CONTROLS_HPP__
#define __CONTROLS_HPP__


//=========================================================================
// Main keys
//=========================================================================
#define JOY_MAIN_TOGGLE_STATS   INPUT_PS2_BTN_R_STICK
#define JOY_MAIN_TOGGLE_HELP    INPUT_PS2_BTN_L_STICK
#define JOY_MAIN_RELOAD         INPUT_PS2_BTN_START
#define JOY_MAIN_SELECT_CONFIG  INPUT_PS2_BTN_SELECT
#define JOY_MAIN_TOGGLE_MODE    INPUT_PS2_BTN_TRIANGLE
#define JOY_MAIN_TOGGLE_PAUSE   INPUT_PS2_BTN_L_DOWN
#define JOY_MAIN_SINGLE_STEP    INPUT_PS2_BTN_L_UP

#define KEY_MAIN_TOGGLE_STATS   INPUT_KBD_Y
#define KEY_MAIN_TOGGLE_HELP    INPUT_KBD_H
#define KEY_MAIN_RELOAD         INPUT_KBD_NUMPADENTER
#define KEY_MAIN_SELECT_CONFIG  INPUT_KBD_RETURN
#define KEY_MAIN_TOGGLE_MODE    INPUT_KBD_ADD
#define KEY_MAIN_TOGGLE_PAUSE   INPUT_KBD_P
#define KEY_MAIN_SINGLE_STEP    INPUT_KBD_L
#define KEY_MAIN_QUIT           INPUT_KBD_ESCAPE


//=========================================================================
// Config select screen
//=========================================================================
#define JOY_CFG_SEL_UP          INPUT_PS2_BTN_L_UP
#define JOY_CFG_SEL_DOWN        INPUT_PS2_BTN_L_DOWN
#define JOY_CFG_SEL_SELECT      INPUT_PS2_BTN_CROSS
#define JOY_CFG_SEL_QUIT        INPUT_PS2_BTN_SELECT
#define JOY_CFG_SEL_AUTOLOAD    INPUT_PS2_BTN_TRIANGLE

#define KEY_CFG_SEL_UP          INPUT_KBD_UP
#define KEY_CFG_SEL_DOWN        INPUT_KBD_DOWN
#define KEY_CFG_SEL_SELECT      INPUT_KBD_RETURN
#define KEY_CFG_SEL_QUIT        INPUT_KBD_ESCAPE


//==============================================================================
//  Light mode keys
//==============================================================================
#define JOY_LIGHT_SPEEDUP       INPUT_PS2_BTN_L2
#define JOY_LIGHT_PITCH         INPUT_PS2_STICK_RIGHT_Y
#define JOY_LIGHT_YAW           INPUT_PS2_STICK_RIGHT_X
#define JOY_LIGHT_INC_INTENSITY INPUT_PS2_BTN_R1
#define JOY_LIGHT_DEC_INTENSITY INPUT_PS2_BTN_R2
#define JOY_LIGHT_INC_RADIUS    INPUT_PS2_BTN_L1
#define JOY_LIGHT_DEC_RADIUS    INPUT_PS2_BTN_L2
#define JOY_LIGHT_TOGGLE_TYPE   INPUT_PS2_BTN_CROSS
#define JOY_LIGHT_RESET         INPUT_PS2_BTN_CIRCLE

//==============================================================================
//  Object mode keys
//==============================================================================
#define JOY_OBJECT_PITCH        INPUT_PS2_STICK_RIGHT_Y
#define JOY_OBJECT_YAW          INPUT_PS2_STICK_RIGHT_X
#define JOY_OBJECT_SPEEDUP      INPUT_PS2_BTN_L2
#define JOY_OBJECT_RESET        INPUT_PS2_BTN_CIRCLE
#define JOY_OBJECT_SHIFT        INPUT_PS2_BTN_L1

#define JOY_OBJECT_RAGDOLL_SHIFT INPUT_PS2_BTN_L2
#define JOY_OBJECT_BLAST_RAGDOLL INPUT_PS2_BTN_CROSS
#define JOY_OBJECT_DROP_RAGDOLL  INPUT_PS2_BTN_SQUARE

#define JOY_OBJECT_PLAY_ANIM    INPUT_PS2_BTN_CROSS
#define JOY_OBJECT_PREV_ANIM    INPUT_PS2_BTN_L_LEFT
#define JOY_OBJECT_NEXT_ANIM    INPUT_PS2_BTN_L_RIGHT

#define JOY_OBJECT_PREV         INPUT_PS2_BTN_L_LEFT
#define JOY_OBJECT_NEXT         INPUT_PS2_BTN_L_RIGHT

#define JOY_OBJECT_MOVE_X       INPUT_PS2_STICK_LEFT_X
#define JOY_OBJECT_MOVE_Z       INPUT_PS2_STICK_LEFT_Y
#define JOY_OBJECT_LOOK_X       INPUT_PS2_STICK_RIGHT_X
#define JOY_OBJECT_LOOK_Z       INPUT_PS2_STICK_RIGHT_Y
#define JOY_OBJECT_LOOK_UP      INPUT_PS2_BTN_R1
#define JOY_OBJECT_LOOK_DOWN    INPUT_PS2_BTN_R2
#define JOY_OBJECT_MOVE_STYLE   INPUT_PS2_BTN_SQUARE
#define KEY_OBJECT_MOVE_STYLE   INPUT_KBD_M

#define JOY_OBJECT_MOVE_UP      INPUT_PS2_BTN_R1
#define JOY_OBJECT_MOVE_DOWN    INPUT_PS2_BTN_R2

#define MSE_OBJECT_SPEEDUP      INPUT_MOUSE_BTN_R   
#define MSE_OBJECT_MOVE         INPUT_MOUSE_BTN_C   
#define MSE_OBJECT_MOVE_VERT    INPUT_MOUSE_Y_REL
#define MSE_OBJECT_MOVE_HORIZ   INPUT_MOUSE_X_REL
#define MSE_OBJECT_PITCH        INPUT_MOUSE_Y_REL
#define MSE_OBJECT_PITCH        INPUT_MOUSE_Y_REL
#define MSE_OBJECT_YAW          INPUT_MOUSE_X_REL
#define MSE_OBJECT_ZOOM         INPUT_MOUSE_WHEEL_REL

#define KEY_OBJECT_SPEEDUP      INPUT_KBD_LSHIFT   
#define KEY_OBJECT_FORWARD      INPUT_KBD_W   
#define KEY_OBJECT_BACK         INPUT_KBD_S   
#define KEY_OBJECT_LEFT         INPUT_KBD_A   
#define KEY_OBJECT_RIGHT        INPUT_KBD_D   
#define KEY_OBJECT_UP           INPUT_KBD_R   
#define KEY_OBJECT_DOWN         INPUT_KBD_F   

#define KEY_OBJECT_SET_MOVE_AT  INPUT_KBD_SPACE   
#define KEY_OBJECT_SET_LOOK_AT  INPUT_KBD_C


//==============================================================================
//  Screen shot mode keys
//==============================================================================

#define JOY_SCREEN_SHOT_INC_SIZE        INPUT_PS2_BTN_L_RIGHT
#define JOY_SCREEN_SHOT_DEC_SIZE        INPUT_PS2_BTN_L_LEFT
#define JOY_SCREEN_SHOT_PIC_MODE        INPUT_PS2_BTN_SQUARE
#define JOY_SCREEN_SHOT_FOV             INPUT_PS2_BTN_R1
#define JOY_SCREEN_SHOT_TAKE            INPUT_PS2_BTN_CROSS
#define JOY_SCREEN_SHOT_RESET           INPUT_PS2_BTN_CIRCLE
#define JOY_SCREEN_SHOT_MOVIE1          INPUT_PS2_BTN_L2
#define JOY_SCREEN_SHOT_MOVIE2          INPUT_PS2_BTN_R2
#define JOY_SCREEN_SHOT_CANCEL_MOVIE    INPUT_PS2_BTN_CIRCLE


//==============================================================================
//  FX mode keys
//==============================================================================

#define JOY_FX_NEXT                 INPUT_PS2_BTN_L_RIGHT
#define JOY_FX_PREV                 INPUT_PS2_BTN_L_LEFT
#define JOY_FX_PLAY                 INPUT_PS2_BTN_CROSS
#define JOY_FX_CLEAR                INPUT_PS2_BTN_SQUARE
#define JOY_FX_RESET                INPUT_PS2_BTN_CIRCLE




//==============================================================================

#endif  //#define __CONTROLS_HPP__
