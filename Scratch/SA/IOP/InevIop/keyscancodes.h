// We convert internal keyboard scancodes to the closest ascii
// equivalent
#include "e_input_gadgets.hpp"

u8 Scancode_Convert[128]=
{
    0x00,                                               // No event
    0x00,                                               // Roll over (input buffer full or too many keys pressed)
    0x00,                                               // Post fail
    0x00,                                               // Error undefined
    INPUT_KBD_A                     - INPUT_KBD__BEGIN, // 'A'
    INPUT_KBD_B                     - INPUT_KBD__BEGIN, // 'B'
    INPUT_KBD_C                     - INPUT_KBD__BEGIN, // 'C'
    INPUT_KBD_D                     - INPUT_KBD__BEGIN, // 'D'
    INPUT_KBD_E                     - INPUT_KBD__BEGIN, // 'E'
    INPUT_KBD_F                     - INPUT_KBD__BEGIN, // 'F'
    INPUT_KBD_G                     - INPUT_KBD__BEGIN, // 'G'
    INPUT_KBD_H                     - INPUT_KBD__BEGIN, // 'H'
    INPUT_KBD_I                     - INPUT_KBD__BEGIN, // 'I'
    INPUT_KBD_J                     - INPUT_KBD__BEGIN, // 'J'
    INPUT_KBD_K                     - INPUT_KBD__BEGIN, // 'K'
    INPUT_KBD_L                     - INPUT_KBD__BEGIN, // 'L'
                                                        
    INPUT_KBD_M                     - INPUT_KBD__BEGIN, // 'M'
    INPUT_KBD_N                     - INPUT_KBD__BEGIN, // 'N'
    INPUT_KBD_O                     - INPUT_KBD__BEGIN, // 'O'
    INPUT_KBD_P                     - INPUT_KBD__BEGIN, // 'P'
    INPUT_KBD_Q                     - INPUT_KBD__BEGIN, // 'Q'
    INPUT_KBD_R                     - INPUT_KBD__BEGIN, // 'R'
    INPUT_KBD_S                     - INPUT_KBD__BEGIN, // 'S'
    INPUT_KBD_T                     - INPUT_KBD__BEGIN, // 'T'
    INPUT_KBD_U                     - INPUT_KBD__BEGIN, // 'U'
    INPUT_KBD_V                     - INPUT_KBD__BEGIN, // 'V'
    INPUT_KBD_W                     - INPUT_KBD__BEGIN, // 'W'
    INPUT_KBD_X                     - INPUT_KBD__BEGIN, // 'X'
    INPUT_KBD_Y                     - INPUT_KBD__BEGIN, // 'Y'
    INPUT_KBD_Z                     - INPUT_KBD__BEGIN, // 'Z'
    INPUT_KBD_1                     - INPUT_KBD__BEGIN, // '1'
    INPUT_KBD_2                     - INPUT_KBD__BEGIN, // '2'
                                                        
    INPUT_KBD_3                     - INPUT_KBD__BEGIN, // '3'
    INPUT_KBD_4                     - INPUT_KBD__BEGIN, // '4'
    INPUT_KBD_5                     - INPUT_KBD__BEGIN, // '5'
    INPUT_KBD_6                     - INPUT_KBD__BEGIN, // '6'
    INPUT_KBD_7                     - INPUT_KBD__BEGIN, // '7'
    INPUT_KBD_8                     - INPUT_KBD__BEGIN, // '8'
    INPUT_KBD_9                     - INPUT_KBD__BEGIN, // '9'
    INPUT_KBD_0                     - INPUT_KBD__BEGIN, // '0'
    INPUT_KBD_RETURN                - INPUT_KBD__BEGIN, // Return
    INPUT_KBD_ESCAPE                - INPUT_KBD__BEGIN, // Escape
    INPUT_KBD_DELETE                - INPUT_KBD__BEGIN, // Delete
    INPUT_KBD_TAB                   - INPUT_KBD__BEGIN, // Tab
    INPUT_KBD_SPACE                 - INPUT_KBD__BEGIN, // Spacebar
    INPUT_KBD_MINUS                 - INPUT_KBD__BEGIN, // '-'
    INPUT_KBD_EQUALS                - INPUT_KBD__BEGIN, // '='
    INPUT_KBD_LBRACKET              - INPUT_KBD__BEGIN, // '['

    INPUT_KBD_RBRACKET              - INPUT_KBD__BEGIN, // ']'
    INPUT_KBD_BACKSLASH             - INPUT_KBD__BEGIN, // '\'
    0x00,                                               // '#'
    INPUT_KBD_SEMICOLON             - INPUT_KBD__BEGIN, // ';'
    INPUT_KBD_APOSTROPHE            - INPUT_KBD__BEGIN, // '''
    INPUT_KBD_GRAVE                 - INPUT_KBD__BEGIN, // '`'
    INPUT_KBD_COMMA                 - INPUT_KBD__BEGIN, // ','
    INPUT_KBD_PERIOD                - INPUT_KBD__BEGIN, // '.'
    INPUT_KBD_SLASH                 - INPUT_KBD__BEGIN, // '/'
    INPUT_KBD_CAPITAL               - INPUT_KBD__BEGIN, // Caps Lock
    INPUT_KBD_F1                    - INPUT_KBD__BEGIN, // F1
    INPUT_KBD_F2                    - INPUT_KBD__BEGIN, // F2
    INPUT_KBD_F3                    - INPUT_KBD__BEGIN, // F3
    INPUT_KBD_F4                    - INPUT_KBD__BEGIN, // F4
    INPUT_KBD_F5                    - INPUT_KBD__BEGIN, // F5
    INPUT_KBD_F6                    - INPUT_KBD__BEGIN, // F6
                                        
    INPUT_KBD_F7                    - INPUT_KBD__BEGIN, // F7
    INPUT_KBD_F8                    - INPUT_KBD__BEGIN, // F8
    INPUT_KBD_F9                    - INPUT_KBD__BEGIN, // F9
    INPUT_KBD_F10                   - INPUT_KBD__BEGIN, // F10
    INPUT_KBD_F11                   - INPUT_KBD__BEGIN, // F11
    INPUT_KBD_F12                   - INPUT_KBD__BEGIN, // F12
    INPUT_KBD_SYSRQ                 - INPUT_KBD__BEGIN, // Print Screen
    INPUT_KBD_SCROLL                - INPUT_KBD__BEGIN, // Scroll Lock
    INPUT_KBD_PAUSE                 - INPUT_KBD__BEGIN, // Pause
    INPUT_KBD_INSERT                - INPUT_KBD__BEGIN, // Insert
    INPUT_KBD_HOME                  - INPUT_KBD__BEGIN, // Home
    INPUT_KBD_PRIOR                 - INPUT_KBD__BEGIN, // Page up
    INPUT_KBD_DELETE                - INPUT_KBD__BEGIN, // Delete
    INPUT_KBD_END                   - INPUT_KBD__BEGIN, // End
    INPUT_KBD_DOWN                  - INPUT_KBD__BEGIN, // Page down
    INPUT_KBD_RIGHT                 - INPUT_KBD__BEGIN, // Right Arrow

    INPUT_KBD_LEFT                  - INPUT_KBD__BEGIN, // Left Arrow
    INPUT_KBD_DOWN                  - INPUT_KBD__BEGIN, // Down Arrow
    INPUT_KBD_UP                    - INPUT_KBD__BEGIN, // Up Arrow
    INPUT_KBD_NUMLOCK               - INPUT_KBD__BEGIN, // Num Lock
    INPUT_KBD_DIVIDE                - INPUT_KBD__BEGIN, // Keypad '/'
    INPUT_KBD_MULTIPLY              - INPUT_KBD__BEGIN, // Keypad '*'
    INPUT_KBD_SUBTRACT              - INPUT_KBD__BEGIN, // Keypad '-'
    INPUT_KBD_ADD                   - INPUT_KBD__BEGIN, // Keypad '+'
    INPUT_KBD_NUMPADENTER           - INPUT_KBD__BEGIN, // Keypad Enter
    INPUT_KBD_NUMPAD0               - INPUT_KBD__BEGIN, // Keypad '1'
    INPUT_KBD_NUMPAD1               - INPUT_KBD__BEGIN, // Keypad '2'
    INPUT_KBD_NUMPAD3               - INPUT_KBD__BEGIN, // Keypad '3'
    INPUT_KBD_NUMPAD4               - INPUT_KBD__BEGIN, // Keypad '4'
    INPUT_KBD_NUMPAD5               - INPUT_KBD__BEGIN, // Keypad '5'
    INPUT_KBD_NUMPAD6               - INPUT_KBD__BEGIN, // Keypad '6'
    INPUT_KBD_NUMPAD7               - INPUT_KBD__BEGIN, // Keypad '7'
                                                        
    INPUT_KBD_NUMPAD8               - INPUT_KBD__BEGIN, // Keypad '8'
    INPUT_KBD_NUMPAD9               - INPUT_KBD__BEGIN, // Keypad '9'
    INPUT_KBD_NUMPAD0               - INPUT_KBD__BEGIN, // Keypad '0'
    INPUT_KBD_DECIMAL               - INPUT_KBD__BEGIN, // Keypad '.'
    INPUT_KBD_BACKSLASH             - INPUT_KBD__BEGIN, // '\'
    INPUT_KBD_APPS                  - INPUT_KBD__BEGIN, // Application
    INPUT_KBD_POWER                 - INPUT_KBD__BEGIN, // Power
    INPUT_KBD_NUMPADEQUALS          - INPUT_KBD__BEGIN, // Keypad '='
    0x00,                                               // F13
    0x00,                                               // F14
    0x00,                                               // F15
    0x00,                                               // F16
    0x00,                                               // F17
    0x00,                                               // F18
    0x00,                                               // F19
    0x00,                                               // F20
                                                        
    0x00,                                               // F21
    0x00,                                               // F22
    0x00,                                               // F23
    0x00,                                               // F24
    0x00,                                               // Execute
    0x00,                                               // Help
    0x00,                                               // Menu
    0x00,                                               // Select
    0x00,                                               // Stop
    0x00,                                               // Again
    0x00,                                               // Undo
    0x00,                                               // Cut
    0x00,                                               // Copy
    0x00,                                               // Paste
    0x00,                                               // Find
    0x00,                                               // Mute
};                                      
                        
u8 Shiftcode_Convert[8]=
{
    INPUT_KBD_LCONTROL  - INPUT_KBD__BEGIN,             // Left Control  
    INPUT_KBD_LSHIFT    - INPUT_KBD__BEGIN,             // Left Shift
    INPUT_KBD_LMENU     - INPUT_KBD__BEGIN,             // Left Alt
    INPUT_KBD_LWIN      - INPUT_KBD__BEGIN,             // Left Windows
    INPUT_KBD_RCONTROL  - INPUT_KBD__BEGIN,             // Right Control
    INPUT_KBD_RSHIFT    - INPUT_KBD__BEGIN,             // Right Shift
    INPUT_KBD_RMENU     - INPUT_KBD__BEGIN,             // Right Alt
    INPUT_KBD_RWIN      - INPUT_KBD__BEGIN,             // Right Windows
};

// first byte is original value, 2nd byte is translated value using
// the scancode passed back from the keyboard
#if 0
u8 International_Convert_SPAIN[]=
{
    0x04,       0x04,                                   // US KBD 'A'
    0x06,       0x06,                                   // US KBD 'C'
    0x10,       0x10,                                   // US KBD 'M'
    0x12,       0x12,                                   // US KBD 'O'
    0x13,       0x13,                                   // US KBD 'P'
    0x14,       0x14,                                   // US KBD 'Q'
    0x16,       0x16,                                   // US KBD 'S'
    0x1a,       0x1a,                                   // US KBD 'W'
    0x1b,       0x1b,                                   // US KBD 'X'
    0x1c,       0x1c,                                   // US KBD 'Y'
    0x1d,       0x1d,                                   // US KBD 'Z'
    0,          0,  
};
#endif
                
u8 International_Convert_FRANCE[]=
{
    0x04,       0x14,                                   // US KBD 'A' to FR 'Q'
    0x10,       0x36,                                   // US KBD 'M' to FR ','
    0x14,       0x04,                                   // US KBD 'Q' to FR 'A'
    0x1a,       0x1d,                                   // US KBD 'W' to FR 'Z'
    0x1d,       0x1a,                                   // US KBD 'Z' to FR 'W'
    0x33,       0x10,                                   // US KBD ';' to FR 'M'
    0x36,       0x33,                                   // US KBD ',' to FR ';'
    0,          0,  

};

u8 International_Convert_GERMANY[]=
{
    0x1c,       0x1d,                                   // US KBD 'Y' to DE 'z'
    0x1d,       0x1c,                                   // US KBD 'Z' to DE 'y'
    0,          0,  
};

