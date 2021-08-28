use strict;
use File::Find;

my @Files = ();

# Search for files
find sub
{
    if( 1 ) #( /.*\.(layer|bpx)$/ )
    {
        if( -f $File::Find::name )
        {
            push @Files, ($File::Find::name);
            print "Found " .scalar(@Files). " Files\r";
        }
    }
}, ( "C:\\GameData\\A51\\Source\\Levels", "C:\\GameData\\A51\\Source\\Themes" );
print "\n";

# Now find files with matching lines in them
my $Count = 1;
my @MatchedFiles = ();
my $FileName;
my $Errors;
foreach $FileName (@Files)
{
    # Keep user updated
    print( "Checking $Count of " .scalar(@Files). ", found ". scalar(@MatchedFiles) ." matches\r" );
    $Count++;
    
    # Read the file
    my @Lines = ();
    my @NewLines = ();
    my $MatchedThisFile = 0;
    open( FH, $FileName ) or die "Can't open file $FileName\n";
    @Lines = <FH>;
    close( FH );
    
    # Check for a matching line
    foreach $_ (@Lines)
    {
        if( /\\RenderInst\\(NBitmapAnims|NUVAnims|NumLODs).*"INT".*"(\d+)"/ && ($2 != 0) )
        {
            # Replace the expression
            s/(\\RenderInst\\(NBitmapAnims|NUVAnims|NumLODs).*"INT".*)"(\d+)"/$1"0"/;
            
            # Add to list of files processed
            if( !$MatchedThisFile )
            {
                push( @MatchedFiles, $FileName );
                $MatchedThisFile = 1;
            }
        }
        
        # Save modified line
        push( @NewLines, $_ );
    }
    
    # Check out the file and write the new version
    if( $MatchedThisFile )
    {
        `p4 edit "$FileName"`;
        if( open( FH, ">$FileName" ) )
        {
            print FH join( "", @NewLines );
            close( FH );
        }
        else
        {
            my @Out = `p4 opened -a "$FileName"`;
            $Errors .= join( "", @Out );
        }
    }
}

# Print errors
print $Errors;