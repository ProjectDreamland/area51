#ifdef IS_MAC

#include <sioux.h>

static void set_up_console( int autoclose )
{
  setbuf( stdout, NULL );

  SIOUXSettings.asktosaveonclose = 0;
  SIOUXSettings.autocloseonquit = autoclose;
}

static char buf[128];
static int  buf_len = 0;
static int  buf_cur = 0;

int r_kbhit( void );
int r_getch( void );

int r_kbhit( void )
{
  EventRecord theEvent;
  
  #ifndef IS_CARBON
  SystemTask();
  #endif
  
  if ( WaitNextEvent( everyEvent, &theEvent, 0, NULL ) )
  {
    if ( ( theEvent.what == keyDown ) || ( theEvent.what == autoKey ) )
	{
      char ch = theEvent.message & charCodeMask;
      if ( buf_len < 128 )
      {
        buf[ ( buf_cur+buf_len ) % 128 ] = ch;
        buf_len++;
      }
	}
	else
	{
      SIOUXHandleOneEvent(&theEvent);
    }
  }
 
  return( (buf_len) ? 1 : 0 );
}

int r_getch( void )
{
  char ch;
  
  while ( !r_kbhit() ) {};
  
  ch = buf[ buf_cur ];
  
  buf_cur = ( buf_cur + 1 ) % 128;
  buf_len--; 
  
  return( ch );
}

#define getch r_getch
#define kbhit r_kbhit

#else

#include <conio.h>

static void set_up_console( int autoclose )
{
  autoclose = autoclose;
  setbuf( stdout, NULL );
}

#endif

//
//  Read a set of arguments from a user inputted string.
//

static void get_args( int* argc, char*** argv )
{
  static char filename[260];
  static char* filenamev[16];
  char *c;
  int i;
  
  gets( filename );
   
  filenamev[0] = *argv[0];
  filenamev[1] = filename;
  
  c = filename;
  i = 1;
  while ( *c )
  {
    filenamev[ i++ ] = c;
    while ( ( *c ) > 32 )
      c++;

    *c++ = 0;
    
    while ( ( *c ) && ( ( *c ) <= 32 ) )
      c++;
  }
  
  *argc = i;
  *argv = filenamev;
}