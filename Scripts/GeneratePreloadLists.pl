#==============================================================================
# Build PreLoad list by scanning project for PRELOAD macros
#==============================================================================

use strict;
use Cwd;
use File::Find;

my $cwd             = cwd();
my $SearchDir       = "";
my $OutputDir       = "";
my %SourceFiles     = ();
my %PreloadsGlobal  = ();
my %PreloadsPS2     = ();
my %PreloadsXbox    = ();

#==============================================================================
# main
#==============================================================================

# Read Args
if( scalar( @ARGV ) == 2 )
{
    $SearchDir  = $ARGV[0];
    $OutputDir  = $ARGV[1];

    # Backslashes to Forward slashes and make sure $OutputDir has a trailing forward slash
    $OutputDir =~ s/[\\\/]/\//g;
    $OutputDir .= "/";
    $OutputDir =~ s/\/\//\//g;
}
else
{
    print "Usage: BuildPreloadList <start_path>\n";
    print "\n";
    print "   eg: BuildPreloadList c:\\Projects\\A51\n";

    # Get .. and use that as the starting point
    $cwd =~ /^(.*)[\/|\\].*$/;
    $SearchDir = $1;
}

# Find all the cpp files
print "Searching for files in $SearchDir...\n";
FindSourceFiles( $SearchDir );
my $Total = keys( %SourceFiles );

# Find all the PRELOAD macros in the cpp files
print "Scanning for PRELOAD_FILE...\n";
FindPreloads();
print "Complete!\n";

# Output PC preloads
my @FileNames = keys %PreloadsGlobal;
push( @FileNames, keys %PreloadsPS2 );
open( FH, ">$OutputDir"."PreloadPC.txt" ) || die "Failed to open 'PreloadPC.'\n";
for my $FileName ( sort @FileNames )
{
    print FH "C:\\GameData\\A51\\Release\\PC\\$FileName\n";
}
close( FH );

# Output PS2 preloads
@FileNames = keys %PreloadsGlobal;
push( @FileNames, keys %PreloadsPS2 );
open( FH, ">$OutputDir"."PreloadPS2.txt" ) || die "Failed to open 'PreloadPS2.'\n";
for my $FileName ( sort @FileNames )
{
    print FH "C:\\GameData\\A51\\Release\\PS2\\$FileName\n";
}
close( FH );

# Output Xbox preloads
@FileNames = keys %PreloadsGlobal;
push( @FileNames, keys %PreloadsXbox );
open( FH, ">$OutputDir"."PreloadXbox.txt" ) || die "Failed to open 'PreloadPS2.'\n";
for my $FileName ( sort @FileNames )
{
    print FH "C:\\GameData\\A51\\Release\\XBox\\$FileName\n";
}
close( FH );

# Done
exit 0;

#==============================================================================
# FindSourceFiles
#==============================================================================

sub FindSourceFiles
{
    my $SearchPath = shift;

    find sub
    {
        if( !/^(\.|\.\.)$/ && !/3rdparty/i )
        {
            my $Path = $File::Find::name;

            if( /.*\.cpp$/ )
            {
                if( -f $Path )
                {
                    $SourceFiles{$Path}{'count'}++;
                }
            }
            elsif( -d $Path )
            {
                FindSourceFiles( $Path );
            }
        }
    }, ( $SearchPath );
}

#==============================================================================
# FindPreloads
#==============================================================================

sub FindPreloads
{
    for my $PathName ( sort keys %SourceFiles )
    {
        # Read the file
        open( FH, $PathName ) or die "\nERROR: Can't open file $PathName\n";
        my @Lines = <FH>;
        close( FH );

        for my $Line (@Lines)
        {
            while( $Line =~ /(PRELOAD_FILE|PRELOAD_PS2_FILE|PRELOAD_XBOX_FILE)\(\s*"(.*?)"\s*\)/g )
            {
                my $Type     = $1;
                my $FileName = $2;

                if( $Type =~ /PRELOAD_FILE/ )
                {
                    $PreloadsGlobal{$FileName} = 1;
                }
                elsif( $Type =~ /PRELOAD_PS2_FILE/ )
                {
                    $PreloadsPS2{$FileName} = 1;
                }
                elsif( $Type =~ /PRELOAD_XBOX_FILE/ )
                {
                    $PreloadsXbox{$FileName} = 1;
                }
            }
        }
    }
}