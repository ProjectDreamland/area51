# Compare 2 memory dumps and output CSV
use strict;

# Read args
die "usage: CompareMemDumps <file 1> <file 2>\n" if( scalar(@ARGV) != 2 );
my( $FileName1, $FileName2 ) = @ARGV;

# Read file1
open( FH, $FileName1 ) || die "Failed to open $FileName1\n";
my @File1 = <FH>;
close( FH );

# Read file2
open( FH, $FileName2 ) || die "Failed to open $FileName2\n";
my @File2 = <FH>;
close( FH );

my %Symbols = ();

# Parse File1
foreach $_ (@File1)
{
    if( /\s*(\d+) ...... ........\s*(\d+) .... (.*)/ )
    {
        my ( $Count, $Bytes, $Identifier ) = ( $1, $2, $3 );
        
        if( !($Identifier =~ /\.(cpp|hpp)/) )
        {
            $Symbols{$Identifier}{'count1'} += $Count;
            $Symbols{$Identifier}{'bytes1'} += $Bytes;
        }
    }
}

# Parse File2
foreach $_ (@File2)
{
    if( /\s*(\d+) ...... ........\s*(\d+) .... (.*)/ )
    {
        my ( $Count, $Bytes, $Identifier ) = ( $1, $2, $3 );
        
        if( !($Identifier =~ /\.(cpp|hpp)/) )
        {
            $Symbols{$Identifier}{'count2'} += $Count;
            $Symbols{$Identifier}{'bytes2'} += $Bytes;
        }
    }
}

# Output report
foreach my $Key (keys %Symbols)
{
    print "\"$Key\",$Symbols{$Key}{'count1'},$Symbols{$Key}{'bytes1'},$Symbols{$Key}{'count2'},$Symbols{$Key}{'bytes2'}\n";
}