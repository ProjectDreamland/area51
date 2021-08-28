
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include "RSCDesc.hpp"

#define   OUT_BUFF_SIZE 512
#define   READ_HANDLE 0
#define   WRITE_HANDLE 1

char szBuffer[OUT_BUFF_SIZE];
char* Prameter [] = {
                     "script_256.txt",
                     NULL             
                    };

char Process [] = {"xbmpTool.exe"};
//=========================================================================
// TYPES
//=========================================================================

//
// This will be a resorce
//
class my_rsc_desc : public rsc_desc
{
public:
    CREATE_RTTI( my_rsc_desc, rsc_desc, rsc_desc );

    virtual void    EnumProp            ( xarray<prop_enum>& List ) {}
    virtual xbool   Property            ( prop_query& I ) {return TRUE; }
    virtual void    GetDependencies     ( xarray<prop_enum>& List    ); 
    virtual void    GetCompilerRules    ( xstring& CompilerRules     );

    my_rsc_desc( void );
};

//
// lets the compiler know about the resource desc type
//
DEFINE_RSC_TYPE( s_Desc, my_rsc_desc, "txt", "txt,asc", "This is a text file resource" );

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================
// Generates some random data ( This is only for the test )
my_rsc_desc::my_rsc_desc( void ) : rsc_desc( s_Desc ) 
{
    SetRscName( "Test" );    
}

//=========================================================================
void my_rsc_desc::GetDependencies( xarray<prop_enum>& List ) 
{
    List.Append().Set( "DependTest1.txt", 0 );
}

//=========================================================================

void my_rsc_desc::GetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules = xfs( "CompilerName.exe -INPUT test1.txt -PC c:\\%s.%s", GetRscName(), m_Type.GetName() );
}


//====================================================  =====================
// MAIN STUFF
//=========================================================================
rsc_desc_mgr s_DescMgr;

void main ( void )
{
    x_Init();

    // Begin error handling
    e_begin;

    //
    // Collect all the known types from the manager
    //
    {
        for( const rsc_desc_type* pLoop = s_DescMgr.GetFirstType(); pLoop; pLoop = s_DescMgr.GetNextType( pLoop ) )
        {
            x_printf( "Known Types: %s\n", pLoop->GetName() );
        }
    }

    //
    // Create a few RscDesc
    //
    s_DescMgr.CreateRscDesc( "Txt" ).SetRscName( "Test1" );
    s_DescMgr.CreateRscDesc( "Txt" ).SetRscName( "Test2" );
    s_DescMgr.CreateRscDesc( "Txt" ).SetRscName( "Test3" );

    //
    // Print all exiting resources
    //
    {
        for( s32 i=0; i<s_DescMgr.GetRscDescCount(); i++ )
        {
            x_printf( "rscDesc: %s\n", s_DescMgr.GetRscDescIndex(i).pDesc->GetRscName() );
        }
    }

    //
    // Determine whether we need to compile the exiting resources
    //

    //for( s32 i=0; i<s_DescMgr.GetRscDescCount(); i++ )
    s32 RscIndex = s_DescMgr.BeginCompiling();
    while( RscIndex != -1 )
    {
//        rsc_desc_mgr::node& Rsc = s_DescMgr.GetRscDescIndex(i);

        // Check if any of the dep are newwer that then date stamp
        //Rsc.pDesc->GetDependencies();
        //Rsc.Date
    
        x_printf( "rscDesc: %s\n", s_DescMgr.GetRscDescIndex(RscIndex).pDesc->GetRscName() );

/*        _finddata_t     c_file;
        _finddata_t     c_depfile;
        long            hFile;
        long            hDepFile;
        xarray<prop_enum> List;
        xbool           Compile;

        if( (hFile = _findfirst( xfs( "%s*.txt", s_DescMgr.GetRscDescIndex(i).pDesc->GetRscName() ), &c_file )) == -1L )
           ASSERTS( FALSE, "Resource Desc File Missing" );

        s_DescMgr.GetRscDescIndex(i).pDesc->GetDependencies( List );
        for( s32 j = 0; j < List.GetCount(); j++ )
        {
            if( (hDepFile = _findfirst( xfs( "%s*", List[j].String ), &c_depfile )) == -1L )
                ASSERTS( FALSE, "Dependency File Missing" );
            
            if( c_depfile.time_access > Rsc.TimeStamp )
                Compile = TRUE;
            
            _findclose( hDepFile );
        }
                

        if( c_file.time_access > Rsc.TimeStamp )
            Compile = TRUE;
*/        
//        if( Compile )
        {
            int nExitCode = STILL_ACTIVE;
            HANDLE hProcess;
            int hStdOut;
            int hStdOutPipe[2];

            // Create the pipe
            if(_pipe(hStdOutPipe, 512, O_TEXT | O_NOINHERIT) == -1)
                ASSERT( FALSE );

            // Duplicate stdout handle (next line will close original)
            hStdOut = _dup(_fileno(stdout));

            // Duplicate write end of pipe to stdout handle
            if(_dup2(hStdOutPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
                ASSERT( FALSE );

            // Close original write end of pipe
            close(hStdOutPipe[WRITE_HANDLE]);

            // Spawn process
            hProcess = (HANDLE)spawnvp(P_NOWAIT, Process, (const char* const*)Prameter);

            // Duplicate copy of original stdout back into stdout
            if(_dup2(hStdOut, _fileno(stdout)) != 0)
                ASSERT( FALSE );

            // Close duplicate copy of original stdout
            close(hStdOut);
        
            if(hProcess)
            {
                int nOutRead;
                FILE* File = fopen( "Output.txt", "at" );
                while   (nExitCode == STILL_ACTIVE)
                {
                    nOutRead = read(hStdOutPipe[READ_HANDLE], szBuffer, OUT_BUFF_SIZE);
                    if(nOutRead)
                    {
                       //nOutRead = Filter(szBuffer, nOutRead, BEEP_CHAR);
                       fwrite(szBuffer, 1, nOutRead, File);
                    }

                    if(!GetExitCodeProcess(hProcess,(unsigned long*)&nExitCode))
                       ASSERT( FALSE );
                }
                fclose( File );
            }

        }              

//        _findclose( hFile );
        
        RscIndex = s_DescMgr.NextCompiling();
    }
    
    // Finish the compiling.
    s_DescMgr.EndCompiling();

    // display error and exit
    e_handle;

    x_Kill();

}

/*
int main()
{
   int   i;
   for(i=0;i<100;++i)
      {
         printf("\nThis is speaker beep number %d... \n\7", i+1);
      }
   return 0;
}


// BeepFilter.Cpp
// Compile options needed: none
//   Execute as: BeepFilter.exe <path>Beeper.exe
//
#include <windows.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define   OUT_BUFF_SIZE 512
#define   READ_HANDLE 0
#define   WRITE_HANDLE 1
#define   BEEP_CHAR 7

char szBuffer[OUT_BUFF_SIZE];

int Filter(char* szBuff, ULONG nSize, int nChar)
{
   char* szPos = szBuff + nSize -1;
   char* szEnd = szPos;
   int nRet = nSize;

   while (szPos > szBuff)
   {
      if (*szPos == nChar)
         {
            memmove(szPos, szPos+1, szEnd - szPos);
            --nRet;
         }
      --szPos;
   }
   return nRet;
}

int main(int argc, char** argv)
{
   int nExitCode = STILL_ACTIVE;
   if (argc >= 2)
   {
      HANDLE hProcess;
      int hStdOut;
      int hStdOutPipe[2];

      // Create the pipe
      if(_pipe(hStdOutPipe, 512, O_BINARY | O_NOINHERIT) == -1)
         return   1;

      // Duplicate stdout handle (next line will close original)
      hStdOut = _dup(_fileno(stdout));

      // Duplicate write end of pipe to stdout handle
      if(_dup2(hStdOutPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
         return   2;

      // Close original write end of pipe
      close(hStdOutPipe[WRITE_HANDLE]);

      // Spawn process
      hProcess = (HANDLE)spawnvp(P_NOWAIT, argv[1], 
       (const char* const*)&argv[1]);

      // Duplicate copy of original stdout back into stdout
      if(_dup2(hStdOut, _fileno(stdout)) != 0)
         return   3;

      // Close duplicate copy of original stdout
      close(hStdOut);

      if(hProcess)
      {
         int nOutRead;
         while   (nExitCode == STILL_ACTIVE)
         {
            nOutRead = read(hStdOutPipe[READ_HANDLE], 
             szBuffer, OUT_BUFF_SIZE);
            if(nOutRead)
            {
               nOutRead = Filter(szBuffer, nOutRead, BEEP_CHAR);
               fwrite(szBuffer, 1, nOutRead, stdout);
            }

            if(!GetExitCodeProcess(hProcess,(unsigned long*)&nExitCode))
               return 4;
         }
      }
   }

   printf("\nPress \'ENTER\' key to continue... ");
   getchar();
   return nExitCode;
}
*/
