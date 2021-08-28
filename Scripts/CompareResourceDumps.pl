# Compare 2 resource logs and output CSV
use strict;
use Cwd;
use Win32::OLE qw(in with);
use Win32::OLE::Const 'Microsoft Excel';

$Win32::OLE::Warn = 3;                                # die on errors...

my $CurrentFolder = cwd();
my $XLSName = "$CurrentFolder/ResourceDiff.xls";

# Delete any existing file
unlink $XLSName;
if( -f $XLSName ) { die "Failed to delete $XLSName"; }

# Read args
if( scalar(@ARGV) != 2 )
{
    die "usage: CompareResourceDumps <file 1> <file 2>\n";
}
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
    if( /^(\d+),\s*([^,\n]+)$/ )
    {
        my ( $Size, $Name ) = ( $1, $2 );
        $Symbols{$Name}{'size1'} += $Size;
    }
}

# Parse File2
foreach $_ (@File2)
{
    if( /^(\d+),\s*([^,\n]+)$/ )
    {
        my ( $Size, $Name ) = ( $1, $2 );
        $Symbols{$Name}{'size2'} += $Size;
    }
}

my %TypeSummary1 = ();
my %TypeSummary2 = ();

# get already active Excel application or open new
my $Excel = Win32::OLE->new('Excel.Application');

# Add a workbook
my $Book = $Excel->Workbooks->Add;

# select worksheet number 1 (you can also select a worksheet by name)
my $Sheet = $Book->Worksheets(1);
$Sheet->{Name} = 'ResourceDiff';

# Insert column titles
my $Range = $Sheet->Range("A1:E1");
$Range->{Value} = [qw(FileName Type Size1 Size2 Size1-Size2)];
$Range->Font->{Bold} = 1;

# Output report
my $row = 2;
foreach my $Key (keys %Symbols)
{
    my $Size1 = $Symbols{$Key}{'size1'};
    my $Size2 = $Symbols{$Key}{'size2'};
    
    $Key =~ /.*\.(.*)/;
    my $Type = $1;
    
    $TypeSummary1{$Type} += $Size1;
    $TypeSummary2{$Type} += $Size2;

    $Sheet->Cells( $row, 1 )->{'Value'} = $Key;
    $Sheet->Cells( $row, 2 )->{'Value'} = $Type;
    $Sheet->Cells( $row, 3 )->{'Value'} = $Size1;
    $Sheet->Cells( $row, 4 )->{'Value'} = $Size2;
    $Sheet->Cells( $row, 5 )->{'Value'} = "=C$row-D$row";
    
    $row++;
}

# Insert column titles
$row++;
$Range = $Sheet->Range("A$row:D$row");
$Range->{Value} = [qw(Type Size1 Size2 Size1-Size2)];
$Range->Font->{Bold} = 1;
$row++;

#Output summary
foreach my $Key (keys %TypeSummary1)
{
    $Sheet->Cells( $row, 1 )->{'Value'} = $Key;
    $Sheet->Cells( $row, 2 )->{'Value'} = $TypeSummary1{$Key};
    $Sheet->Cells( $row, 3 )->{'Value'} = $TypeSummary2{$Key};
    $Sheet->Cells( $row, 4 )->{'Value'} = "=B$row-C$row";
    $row++;
}

# Set column sizes
$Sheet->columns("A:E")->AutoFit();

# Save it & Close it
$Book->SaveAs( $XLSName );
$Book->Close;

`cmd /c $XLSName`;
