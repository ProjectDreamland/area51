//=============================================================================
//
//  Audio Manager Stress Test
//
//=============================================================================

#include "Entropy.hpp"

//=============================================================================
//  CONSTANTS
//=============================================================================

//=============================================================================

//=============================================================================
//  GLOBALS
//=============================================================================
s32 s_CursorPos = 0;
s32 s_CursorFlash = 0;

struct group_entry
{
    s32             x;
    s32             y;
    const char*     Body;
};

group_entry s_GroupTable[]=
{
    {15,5,    "ABCD"},
    {22,9,    "EFGH"},
    {26,13,   "IJKL"},
    {22,17,   "MNOP"},
    {15,21,   "QRST"},
    { 8,17,   "UVWX"},
    { 4,13,   "YZ<>"},
    { 8,9,    "0123"},
};

char s_TextBuff[128];
s32  s_TextIndex = 0;
//=============================================================================
//
//  Game Update Function
//
//=============================================================================
void ProcessInput( f32 DeltaTime )
{
    f32     x,y;
    char    ch;

    (void)DeltaTime;

    input_UpdateState();

    x = input_GetValue(INPUT_PS2_STICK_LEFT_X);
    y = input_GetValue(INPUT_PS2_STICK_LEFT_Y);

    if (x > 0.5f)
    {
        if (y > 0.5f)           s_CursorPos = 2;
        else if (y < -0.5f)     s_CursorPos = 4;
        else                    s_CursorPos = 3;
    }
    else if (x < -0.5f)
    {
        if (y > 0.5f)           s_CursorPos = 8;
        else if (y < -0.5f)     s_CursorPos = 6;
        else                    s_CursorPos = 7;
    }
    else
    {
        if (y > 0.5f)           s_CursorPos = 1;
        else if (y < -0.5f)     s_CursorPos = 5;
        else                    s_CursorPos = 0;
    }

    ch = 0x0;
    if (s_CursorPos > 0)
    {
        if (input_WasPressed(INPUT_PS2_BTN_SQUARE))
        {
            ch = s_GroupTable[s_CursorPos-1].Body[0];
        }

        if (input_WasPressed(INPUT_PS2_BTN_TRIANGLE))
        {
            ch = s_GroupTable[s_CursorPos-1].Body[1];
        }

        if (input_WasPressed(INPUT_PS2_BTN_CIRCLE))
        {
            ch = s_GroupTable[s_CursorPos-1].Body[2];
        }

        if (input_WasPressed(INPUT_PS2_BTN_CROSS))
        {
            ch = s_GroupTable[s_CursorPos-1].Body[3];
        }
    }

    if (ch == '<')
    {
        if (s_TextIndex > 0)
        {
            s_TextIndex--;
        }
        s_TextBuff[s_TextIndex]=0;
    }
    else if (ch == '>')
    {
        s_TextBuff[s_TextIndex]=' ';
        s_TextIndex++;
    }
    else if (ch != 0)
    {
        s_TextBuff[s_TextIndex]=ch;
        s_TextIndex++;
    }

    x_printfxy(10,0,"Input = %2.2f, %2.2f",x,y);
    x_printfxy(5,1,"String = %s",s_TextBuff);
}

//=============================================================================
void Update( f32 DeltaTime )
{
    s32 x,y;
    s32 i;

    (void)DeltaTime;

    for (i=0;i<8;i++)
    {
        x_printfxy(s_GroupTable[i].x-1,s_GroupTable[i].y  , "%c", s_GroupTable[i].Body[0]);
        x_printfxy(s_GroupTable[i].x  ,s_GroupTable[i].y-1, "%c", s_GroupTable[i].Body[1]);
        x_printfxy(s_GroupTable[i].x+1,s_GroupTable[i].y  , "%c", s_GroupTable[i].Body[2]);
        x_printfxy(s_GroupTable[i].x  ,s_GroupTable[i].y+1, "%c", s_GroupTable[i].Body[3]);
    }
    if (s_CursorPos == 0)
    {
        x = 15; y = 13;
    }
    else
    {
        x = s_GroupTable[s_CursorPos-1].x;
        y = s_GroupTable[s_CursorPos-1].y;
    }

    if (s_CursorFlash & 4)
    {
        x_printfxy(x,y,"*");
    }


    s_CursorFlash ++;

    eng_PageFlip();
}

//=============================================================================
//  MAIN
//=============================================================================
xtimer g_GameTimer;
xbool g_Exit = FALSE;

void AppMain( s32 argc, char* argv[] )
{
    (void)argc;
    (void)argv;

    // Initialize general systems
    x_DebugMsg("Entered app\n");
    eng_Init();
    
    // Setup the global game timer
    g_GameTimer.Reset();
    g_GameTimer.Start();

    x_memset(s_TextBuff,0,sizeof(s_TextBuff));
    s_TextIndex = 0;
    while( g_Exit == FALSE )
    {
        
        // Compute the duration of the last frame
        f32 DeltaTime = MAX( g_GameTimer.ReadSec(), 0.001f );

		g_GameTimer.Reset();
		g_GameTimer.Start();
                
        ProcessInput ( DeltaTime );
        Update       ( DeltaTime );

    }
}

//=============================================================================

