#include "x_files.hpp"
#include "CommandLine.hpp"
#include "RawMesh2.hpp"
#include "aux_Bitmap.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

void CompileBitmap( const char* Input, const char* Output )
{
    xbitmap Bitmap;
    X_FILE*  FP;

    //
    // Load the bitmap
    //
    if( auxbmp_Load( Bitmap, Input ) == FALSE )
        x_throw( xfs("Unable to open the bitmap [%s]", Input) );

    //
    // Open the output file
    //
    FP = x_fopen( Output, "wt" );
    if( FP == 0 )
        x_throw( xfs("Unable to open the output file [%s]", Output ));

    //
    // Output the info
    //
    char FileName[256];
    x_splitpath( Output, NULL, NULL, FileName, NULL );

    x_fprintf( FP, "\\ Data From: %s \n", Input );
    x_fprintf( FP, "\\ NBytes   : %d \n", Bitmap.GetHeight()*Bitmap.GetWidth()*4 );

    x_fprintf( FP, "static u8 s_%s[] = \n", FileName );
    x_fprintf( FP, "{\n");
    for( s32 y=0; y<Bitmap.GetHeight(); y++)
    {
        x_fprintf( FP, "    ");
        for( s32 x=0; x<Bitmap.GetWidth();  x++)
        {
            xcolor Color = Bitmap.GetPixelColor( x, y );
            
            if( (x-1) == Bitmap.GetWidth() && (y-1) == Bitmap.GetHeight() )
                x_fprintf( FP, "%3d, %3d, %3d, %3d ", Color.B, Color.G, Color.R, Color.A );
            else 
                x_fprintf( FP, "%3d, %3d, %3d, %3d, ", Color.B, Color.G, Color.R, Color.A );                
                
        }

        x_fprintf( FP, "\n");
    }
    x_fprintf( FP, "};\n");

    x_fclose( FP );
}

//=========================================================================

void CompileMesh( const char* Input, const char* Output )
{
    rawmesh2 RawMesh;
    X_FILE*  FP;
    s32      i;
    char     FileName[X_MAX_FNAME];

    x_splitpath( Output, NULL, NULL, FileName, NULL );

    //
    // Load the raw mesh
    //
    if( RawMesh.Load( Input ) == FALSE )
        x_throw( xfs("Unable to open the mesh [%s]", Input) );

    //
    // Open the output file
    //
    FP = x_fopen( Output, "wt" );
    if( FP == 0 )
        x_throw( xfs("Unable to open the output file [%s]", Output ));

    //
    // Output the vertex 
    //
    x_fprintf( FP, "//------------------------------------------------------------------------------------------------\n");
    x_fprintf( FP, "// Icon mesh definition.\n");
    x_fprintf( FP, "// This is a generated header file, and should not be hand edited.\n");
    x_fprintf( FP, "//------------------------------------------------------------------------------------------------\n\n");
    x_fprintf( FP, "// struct vertex \n");
    x_fprintf( FP, "// {\n");
    x_fprintf( FP, "//     f32 X, Y, Z;\n");
    x_fprintf( FP, "//     u8  R, G, B, A;\n");
    x_fprintf( FP, "//     f32 U, V;\n");
    x_fprintf( FP, "// };\n\n");
    

    x_fprintf( FP, "// Data From: %s \n\n", Input );
    x_fprintf( FP, "#define NUM_VERTICES_%s\t(%d)\n", x_strtoupper(FileName), RawMesh.m_nVertices );
    x_fprintf( FP, "#define NUM_FACETS_%s\t(%d)\n\n", x_strtoupper(FileName), RawMesh.m_nFacets );

    x_fprintf( FP, "static vertex s_v%s[] = \n", x_strtolower(FileName) );
    x_fprintf( FP, "{\n");
    x_fprintf( FP, "//  POSITION                                  COLOR                UV\n");
    x_fprintf( FP, "//  ----------------------------------------  -------------------  ------------------------------\n");
    for( i=0; i<RawMesh.m_nVertices; i++ )
    {
        x_fprintf( FP, "    { %10.4ff, %10.4ff, %10.4ff,  ",
            RawMesh.m_pVertex[ i ].Position.GetX(),
            RawMesh.m_pVertex[ i ].Position.GetY(),
            RawMesh.m_pVertex[ i ].Position.GetZ() );


        if( RawMesh.m_pVertex[ i ].nColors == 0 )
        {
            x_fprintf( FP, "255, 0, 0, 0,  ");
        }
        else
        {
            x_fprintf( FP, "%3d, %3d, %3d, %3d,  ",
                RawMesh.m_pVertex[ i ].Color[0].R,
                RawMesh.m_pVertex[ i ].Color[0].G,
                RawMesh.m_pVertex[ i ].Color[0].B, 
                255 );
        }

        if( RawMesh.m_pVertex[ i ].nUVs == 0 )
        {
            x_fprintf( FP, "0.0f, 0.0f }" );
        }
        else
        {
            x_fprintf( FP, "%10.6ff,%10.6ff }",
                RawMesh.m_pVertex[ i ].UV[0].X,
                RawMesh.m_pVertex[ i ].UV[0].Y );
        }

        if( i < (RawMesh.m_nVertices-1) )
            x_fprintf( FP, ",\n");
        else 
            x_fprintf( FP, "\n");
            
    }
    x_fprintf( FP, "};\n\n");

    //
    // Output Indices
    //
    x_fprintf( FP, "static s16 s_i%s[] = \n", x_strtolower(FileName) );
    x_fprintf( FP, "{\n");
    for( i=0; i<RawMesh.m_nFacets; i++ )
    {
        x_fprintf( FP, "    %3d, %3d, %3d", 
            RawMesh.m_pFacet[i].iVertex[0], 
            RawMesh.m_pFacet[i].iVertex[1],
            RawMesh.m_pFacet[i].iVertex[2] );
        
        if( i < (RawMesh.m_nFacets-1) )
            x_fprintf( FP, ",\n");
        else 
            x_fprintf( FP, "\n");
    }
    x_fprintf( FP, "};\n\n");


    // 
    // Output the draw macro
    //
    char UCName[X_MAX_FNAME];
    char LCName[X_MAX_FNAME];
    x_strcpy(UCName, FileName);
    x_strcpy(LCName, FileName);
    x_strtoupper(UCName);
    x_strtolower(LCName);

    x_fprintf( FP, "\n#define DRAW_%s() draw_icon( NUM_FACETS_%s, NUM_VERTICES_%s, s_v%s, s_i%s )\n\n", 
        UCName, UCName, UCName, LCName, LCName );

    x_fclose( FP );
}

//=========================================================================

void ExecuteScript( command_line& CommandLine )
{
    s32             i;
    xbool           sType = 0;
    char            Output[256]={0};
    char            Resource[256]={0};

    //
    // Collect all the options
    //
    x_try;

    for( i=0 ; i<CommandLine.GetNumOptions() ; i++ )
    {
        xstring OptName   = CommandLine.GetOptionName( i );
        xstring OptString = CommandLine.GetOptionString( i );

        if( OptName == xstring( "BITMAP" ) )
        {
            if( Resource[0] )
                x_throw( xfs( "You enter multiple resources to be compile\n[%s] and [%s]. Only one is allowed.", Resource, (const char*)OptString));

            x_strcpy( Resource, OptString );
            sType = 1;
        }
        else if( OptName == xstring( "MESH" ) )
        {
            if( Resource[0] )
                x_throw( xfs( "You enter multiple resources to be compile\n[%s] and [%s]. Only one is allowed.", Resource, (const char*)OptString));

            x_strcpy( Resource, OptString );
            sType = 2;
        }
        else if( OptName == xstring( "PC" ) )
        {
            if( Output[0] )
                x_throw( xfs("You enter the more than one output name [%s]\nand [%s]", Output, (const char*)OptString));

            x_strcpy( Output, OptString );
        }
        else
        {
            x_throw("Unkown option");
        }
    }

    //
    // Do a sanity check for all the options we got
    //
    if( sType == 0 )
        x_throw( "You must enter at least one resource to compile");

    if( !Output[0] )
        x_throw( "You must expecify an output name (-PC somethig");

    //
    // Okay we must be good so execute the compile
    //
    if( sType == 1 )
    {
        CompileBitmap( Resource, Output );
    }
    else 
    {
        CompileMesh( Resource, Output );
    }

    //
    // Handle any errors
    //
    x_catch_begin;
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    x_catch_end;

    x_printf( "Done.\n" );
}

//=========================================================================

void PrintHelp( void )
{
    x_printf( "Error: Compiling\n" );
    x_printf( ":Platform to compile for: -PC c:/A51/Level1/PC/FileName/output.hpp  \n" );   
    x_printf( ":Resource    -BITMAP input.tga -MESH rawmesh.matx                   \n" );
}

//=========================================================================

void main( s32 argc, char* argv[] )
{
    x_Init(argc, argv);

    command_line CommandLine;
    
    // Specify all the options
    CommandLine.AddOptionDef( "BITMAP",     command_line::STRING );
    CommandLine.AddOptionDef( "MESH",       command_line::STRING );
    CommandLine.AddOptionDef( "PC",         command_line::STRING );
 
    // Parse the command line
    if( CommandLine.Parse( argc, argv ) )
    {
        PrintHelp();
    }
    else
    {
        ExecuteScript( CommandLine );
    }

    x_Kill();
}