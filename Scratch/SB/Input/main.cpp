//==============================================================================
//
//  Main.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"


//==============================================================================
//  DATA
//==============================================================================

// Gadgets array
struct gadget
{
    char m_Name[256] ;
} ;
gadget g_InputGadgets[INPUT_KBD__END+1] = {0} ;


//==============================================================================
//  DEFINES
//==============================================================================


//==============================================================================
// FUNCTIONS
//==============================================================================

static xbool AppInput( f32 DeltaTime )
{
    s32 i ;

    CONTEXT("Input") ;
    (void)DeltaTime ;

#ifdef TARGET_PC
    while( input_UpdateState() )
#else
    input_UpdateState() ;
#endif
    {
        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        // Check all gadgets
        for (i = INPUT_PS2__BEGIN ; i < INPUT_PS2__END ; i++)
        {
            input_gadget ID = (input_gadget)i ;
            if (input_IsPressed(ID))
                x_printf("%s\n", g_InputGadgets[ID].m_Name) ;
        }

    }

    return TRUE ;
}

//==============================================================================

void AppRender( void )
{
    CONTEXT("Render") ;

    eng_Begin( "Render" );
    eng_End();
}

//==============================================================================

void AppAdvance( f32 DeltaTime )
{
    CONTEXT("Advance") ;
    
    // Cap incase we are in the debugger
    if (DeltaTime > (1.0f / 5.0f))
        DeltaTime = (1.0f / 5.0f) ;

    // Run at PS2 speed
    static f32 AccumTime = 0 ;
    static f32 Step = 1.0f / 30.0f ;
    AccumTime += DeltaTime ;
    if (AccumTime >= Step)
    {
        AccumTime -= Step ;
        DeltaTime = Step ;
    }
    else
        DeltaTime = 0 ;
}

//==============================================================================

// Sets gadget entry name
void SetGadgetName( s32 Index, const char* pName )
{
    // Setup name
    x_strcpy(g_InputGadgets[Index].m_Name, pName) ;
}

//==============================================================================

void AppInit( void )
{
    eng_Init();
    eng_SetBackColor(XCOLOR_BLACK) ;

    // Cunningly fill out the gadget name table
    x_DebugMsg("..Gadget table\n") ;
    s32 Index = 0 ;
    #define BEGIN_GADGETS
    #define DEFINE_GADGET(__gadget__)                   SetGadgetName(Index++, #__gadget__) ;
    #define DEFINE_GADGET_VALUE(__gadget__, __value__)  { Index = __value__ ; SetGadgetName(Index++, #__gadget__ ) ; }
    #define END_GADGETS
    #include "e_input_gadget_defines.hpp"
}

//==============================================================================

void AppKill( void )
{
    eng_Kill() ;
}

//==============================================================================

void AppMain( s32, char** )
{
    AppInit() ;

    xtimer Timer;
    Timer.Start() ;
    view    View ;

    while( TRUE )
    {
        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );
        eng_ActivateView    ( 0 );

        f32 DeltaTime = Timer.TripSec() ;

        if( !AppInput( DeltaTime ) )
            break;

        AppRender();

        AppAdvance( DeltaTime ) ;

        // DONE!
        eng_PageFlip();

        // Profile
        x_ContextPrintProfile();
        x_ContextResetProfile();
    }

    AppKill() ;
}

//==============================================================================
