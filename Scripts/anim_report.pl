#==============================================================================
# .anim resource report script
#==============================================================================

use strict;
use File::Find;

#==============================================================================
# main
#==============================================================================

# Read Args
if( scalar( @ARGV ) != 1 )
{
    print "Usage: anim_report <level_path> or COMBINE";
    exit 1;
}

# Define some paths
#my $LayerPath       = 'C:\GameData\A51\Source\Levels\01_LevelBlue\01_LevelBlue_Main';
my $LayerPath       = $ARGV[0];

# Vars
my @Themes = ();                                # Array of themes
my %Projects;                                   # Hash of project files
my %Blueprints;                                 # Hash of blueprints
my %AnimPackages;                               # Hash of anim packages
my @Layers = ();                                # Array of layers
my %AnimNames;                                  # Hash of anim names for a quick test if a string is an anim name
my %Objects;                                    # Hash of objects
my @Errors = ();                                # Array of errors

my @AnimLines = ();

my @Reports = ();

if( $LayerPath eq 'COMBINE' )
{
    MakeCombinedReport();
    exit 0;
}
else
{
    print STDERR "#==============================================================================\n";
    print STDERR "Level: $LayerPath\n";
    print STDERR "#==============================================================================\n";

    FindThemes();
    FindBlueprints();
    FindAnimPackages();
    LoadAnimPackages();
    FindLayers();
    ProcessForObjects();
    ProcessForAnims();
    #DumpObjects();
    DumpAnimPackages();
    #print @AnimLines;

    print STDERR "#==============================================================================\n";
    print STDERR "\n";
}

#==============================================================================
# MakeCombinedReport
#==============================================================================

sub FindReports
{
    find sub
    {
        if( /.*\.anims\.csv$/ )
        {
            if( -f $File::Find::name )
            {
                push( @Reports, $_ );
            }
        }
    }, ( "." );
}

sub MakeCombinedReport
{
    %AnimPackages = ();

    FindReports();

    foreach my $File (@Reports)
    {
        # Read the file
        open( FH, $File ) or die "\nERROR: Can't open file $File\n";
        my @Lines = <FH>;
        close( FH );

        foreach my $Line (@Lines)
        {
            if( $Line =~ /"(.*?)","(.*?)",(\d+),"(.*?)"/ )
            {
                my $Package = $1;
                my $Anim    = $2;
                my $Count   = $3;
                my $Path    = $4;

                $AnimPackages{$Package}{'path'} = $Path;
                $AnimPackages{$Package}{'anims'}{$Anim} += $Count;
            }
        }
    }

    DumpAnimPackages();
}

#==============================================================================
# error
#==============================================================================

sub error
{
    my $String = shift;
    $String =~ s/\//\\/g;
    print STDERR "ERROR: $String";
    push( @Errors, $String );
}

#==============================================================================
# Set an anim as used
#==============================================================================

sub SetAnimUsed
{
    my $Package = shift;
    my $Anim    = shift;
    my $Line    = shift;
    my $File    = shift;

    if( exists $AnimPackages{$Package}{'anims'}{$Anim} )
    {
        $AnimPackages{$Package}{'anims'}{$Anim}++;
    }
    else
    {
        error( "Anim $Anim in Package $Package does not exist at line $Line in $File\n" );
    }
}

#==============================================================================
# Set an anim as used
#==============================================================================

sub SetAnimUsedNoPackage
{
    my $Anim    = shift;
    my $Line    = shift;
    my $File    = shift;

    if( $AnimNames{$Anim} > 1 )
    {
        error( "Ambiguous anim $Anim at line $Line in $File\n" );
    }

    for my $AnimPackage ( sort keys %AnimPackages )
    {
        for my $AnimName ( sort keys %{$AnimPackages{$AnimPackage}{'anims'}} )
        {
            if( $AnimName eq $Anim )
            {
                $AnimPackages{$AnimPackage}{'anims'}{$Anim}++;
                return;
            }
        }
    }

    error( "Anim $Anim does not exist at line $Line in $File\n" );
}

#==============================================================================
# Find the themes by reading the project file
#==============================================================================

sub FindThemes
{
    find sub
    {
        if( /.*\.project$/ )
        {
            if( -f $File::Find::name && ($File::Find::name !~ /(\\|\/)Backup(\\|\/)/) )
            {
                if( exists $Projects{$_} )
                {
                    # Multiple project files
                    error( "$_ multiple\n  $Projects{$_}{'path'}\n  $File::Find::name\n\n" );
                }
                else
                {
                    # Store project file
                    $Projects{$_}{'path'} = $File::Find::name;

                    # Load project file
                    my $Path = $File::Find::name;
                    open( FH, $Path ) or die "\nERROR: Can't open file $Path\n";
                    my @Lines = <FH>;
                    close( FH );

                    foreach $_ (@Lines)
                    {
                        if( /.*"(.*)\\.*?\.theme"\s*$/ )
                        {
                            push( @Themes, $1 );
                        }
                    }
                }
            }
        }
    }, ( "$LayerPath" );
}

#==============================================================================
# Find the blueprints in the theme folder
#==============================================================================

sub FindBlueprints
{
    my $Count = 0;

    find sub
    {
        if( /.*\.bpx$/ )
        {
            if( -f $File::Find::name && ($File::Find::name !~ /(\\|\/)Backup(\\|\/)/) )
            {
                if( exists $Blueprints{$_} )
                {
                    # Duplicate blueprint name
                    error( "$_ duplicated\n  $Blueprints{$_}{'path'}\n  $File::Find::name\n\n" );
                }
                else
                {
                    # Create mapping of blueprint name to path
                    $Blueprints{$_}{'path'} = $File::Find::name;
                }
            }
        }
    }, ( "$LayerPath", @Themes );
}

#==============================================================================
# Find the anim packages in the theme folder
#==============================================================================

sub FindAnimPackages
{
    my $Count = 0;

    find sub
    {
        if( /.*\.anim$/ )
        {
            if( -f $File::Find::name && ($File::Find::name !~ /(\\|\/)Backup(\\|\/)/) )
            {
                if( exists $AnimPackages{$_} )
                {
                    # Duplicate anim package name
                    error( "$_ duplicated\n  $AnimPackages{$_}{ 'path' }\n  $File::Find::name\n\n" );
                }
                else
                {
                    # Create mapping of anim name to path
                    $AnimPackages{$_}{'path'} = $File::Find::name;
                }
            }
        }
    }, ( "$LayerPath", @Themes );
}

#==============================================================================
# Load the anim packages
#==============================================================================

sub LoadAnimPackages
{
    for my $AnimPackage ( keys %AnimPackages )
    {
        # Read the file
        my $Path = $AnimPackages{$AnimPackage}{'path'};
        open( FH, $Path ) or die "\nERROR: Can't open file $Path\n";
        my @Lines = <FH>;
        close( FH );

        # Process the lines
        for $_ (@Lines )
        {
            if( /\s*"(.*?)"\s*"(.*?)"\s*"(.*?)"/ )
            {
                my $Field = $1;
                my $Type  = $2;
                my $Value = $3;

                if( $Field =~ /ResDesc\\AnimList\[\d+\]\\Name/ )
                {
                    $AnimPackages{$AnimPackage}{'anims'}{$Value} = 0;
                    $AnimNames{$Value}++;
                }
            }
        }
    }
}

#==============================================================================
# Dump anim packages
#==============================================================================

sub DumpAnimPackages
{
    print "\"Package\",\"Anim\",\"Count\",\"Path\"\n";

    for my $AnimPackage ( sort keys %AnimPackages )
    {
        my $Path = $AnimPackages{$AnimPackage}{'path'};

        for my $AnimName ( sort keys %{$AnimPackages{$AnimPackage}{'anims'}} )
        {
            my $Count = $AnimPackages{$AnimPackage}{'anims'}{$AnimName};

            print "\"$AnimPackage\",\"$AnimName\",$Count,\"$Path\"\n";
        }
    }
}

#==============================================================================
# Find the layer files
#==============================================================================

sub FindLayers
{
    find sub
    {
        if( /.*\.layer$/ )
        {
            if( (-f $File::Find::name) && ($File::Find::name !~ /(\\|\/)Backup(\\|\/)/) )
            {
                push @Layers, ($File::Find::name);
            }
        }
    }, ( "$LayerPath" );
}

#==============================================================================
# Process layer files for objects
#==============================================================================

sub ProcessForObjects
{
    # Iterate over the layer files
    foreach my $Layer (@Layers)
    {
        # Read the file
        open( FH, $Layer ) or die "\nERROR: Can't open file $Layer\n";
        my @Lines = <FH>;
        close( FH );

        my $Line = 1;
        my $AnimPackageDefined = 0;
        my $ObjectGUIDDefined = 0;
        my $ObjectGUID;
        foreach $_ (@Lines)
        {
            # Clear Guid Defined and AnimPackage Defined when we encounter a new object
            if( (/^\[ Object/) || (/^\[ BP_OverwriteData/) )
            {
                $ObjectGUIDDefined = 0;
                $AnimPackageDefined = 0;
            }

            # Process a line of data
            if( /\s*"(.*?)"\s*"(.*?)"\s*"(.*?)"\s*/ )
            {
                my $Field = $1;
                my $Type  = $2;
                my $Value = $3;

                # Does this look like a GUID
                if( $Type =~ /[a-fA-F0-9]{8}:[a-fA-F0-9]{8}/ )
                {
                    $ObjectGUIDDefined = 1;
                    $ObjectGUID = $Type;
                    if( exists $Objects{$ObjectGUID} )
                    {
                        error( "Object $ObjectGUID defined twice at line $Line in: $Layer also in $Objects{$ObjectGUID}{'layer'}\n" );
                    }
                    $Objects{$ObjectGUID}{'defined'} = 1;
                    $Objects{$ObjectGUID}{'layer'} = $Layer;
                }
                elsif(  ($Value =~ /\.anim$/) &&
                       !($Field =~ /^Do\\Action/) &&
                       !($Field =~ /^Cover Node\\(Civilian|Soldier|Grey) Cover Anim Package/) &&
                       !($Field =~ /^Spawned Object Anim\\AnimSelect/) &&
                       !($Field =~ /^TaskList\\Task\[/) )
                {
                    # Found an Anim Package for this object
                    if( $ObjectGUIDDefined )
                    {
                        if( $AnimPackageDefined )
                        {
                            error( "Anim Package defined multiple times at line $Line in $Layer\n" );
                        }
                        else
                        {
                            $AnimPackageDefined = 1;
                            $Objects{$ObjectGUID}{'animpkg'} = $Value;
                        }
                    }
                    else
                    {
                        error( "Anim Package defined with no GUID at line $Line in $Layer\n" );
                    }
                }
            }
            # This catches Blueprint Overwrite GUIDs
            elsif( /\s*"([a-fA-F0-9]{8}:[a-fA-F0-9]{8})"\s*/ )
            {
                $ObjectGUIDDefined = 1;
                $ObjectGUID = $1;
                $Objects{$ObjectGUID}{'defined'} = 1;
            }
            $Line++;
        }
    }
}

#==============================================================================
# Dump objects
#==============================================================================

sub DumpObjects
{
    for my $GUID ( keys %Objects )
    {
        my $AnimPackage = $Objects{$GUID}{'animpkg'};
        if( !defined $AnimPackage )
        {
            $AnimPackage = '<undefined>';
        }

        print "$GUID = $AnimPackage\n";
    }
}

#==============================================================================
# Process layer files for animation info
#==============================================================================

sub DecimalToHexGUID
{
    my $Decimal = shift;
    my $Hex;

    if( $Decimal =~ /(-?\d+)\s(-?\d+)/ )
    {
        $Hex = sprintf( "%08lX:%08lX", $2, $1 );
    }

    return $Hex;
}

sub ProcessForAnims
{
    # Iterate over the layer files
    foreach my $Layer (@Layers)
    {
        # Read the file
        open( FH, $Layer ) or die "\nERROR: Can't open file $Layer\n";
        my @Lines = <FH>;
        close( FH );

        my $Line = 1;
        my $ObjectGUIDDefined = 0;
        my $ObjectGUID;
        my $LastAnimSelectIndex = -1;
        my $LastAnimSelectPackage;
        my $LastGUIDIndex = -1;
        my $LastGUID;
        my $LastPropertyIndex = -1;
        my $LastPropertyType;

        foreach $_ (@Lines)
        {
            # Clear Guid Defined when we encounter a new object
            if( (/^\[ Object/) || (/^\[ BP_OverwriteData/) )
            {
                $ObjectGUIDDefined = 0;
            }

            # Process a line of data
            if( /\s*"(.*?)"\s*"(.*?)"\s*"(.*?)"/ )
            {
                my $Field = $1;
                my $Type  = $2;
                my $Value = $3;

                # Does this look like a GUID
                if( $Type =~ /[a-fA-F0-9]{8}:[a-fA-F0-9]{8}/ )
                {
                    $ObjectGUIDDefined = 1;
                    $ObjectGUID = $Type;
                    if( !exists $Objects{$ObjectGUID} )
                    {
                        error( "Object $ObjectGUID not defined at line $Line in $Layer\n" );
                    }
                    $Objects{$ObjectGUID}{'defined'} = 1;
                }
                # Is this a Do Action or Task List or Spawn
                elsif( ($Field =~ /^Do\\Action\[/) ||
                       ($Field =~ /^TaskList\\Task\[/) ||
                       ($Field =~ /^Spawned Object Anim\\/) )
                {
                    # Is this a package selection
                    if( $Field =~ /Do\\Action\[(\d+)\]\\AnimSelect/ )
                    {
                        $LastAnimSelectPackage = $Value;
                        $LastAnimSelectIndex = $1;
                    }
                    elsif( $Field =~ /TaskList\\Task\[(\d+)\]\\SubTask\[(\d+)\]\\AnimSelect/ )
                    {
                        $LastAnimSelectPackage = $Value;
                        $LastAnimSelectIndex = $1 * 1000 + $2;
                    }
                    elsif( $Field =~ /Spawned Object Anim\\AnimSelect/ )
                    {
                        $LastAnimSelectPackage = $Value;
                        $LastAnimSelectIndex = 1;
                    }
                    # Is this a property set operation
                    elsif( $Field =~ /Do\\Action\[(\d+)\]\\Property/ )
                    {
                        $LastPropertyType = $Value;
                        $LastPropertyIndex = $1;
                    }
                    # Is this a target state
                    elsif( $Field =~ /Do\\Action\[(\d+)\]\\Target State/ )
                    {
                        # Just ignore it
                    }
                    # Is this a GUID
                    elsif( $Field =~ /Do\\Action\[(\d+)\]\\Object Guid/ )
                    {
                        $LastGUID = DecimalToHexGUID( $Value );
                        $LastGUIDIndex = $1;
                    }
                    # Is this an Anim Name but not excluded fields?
                    elsif(  (exists $AnimNames{$Value}) &&
                           !($Field =~ /^Trigger Object\\Reset Style/) &&
                           !($Field =~ /^Do\\Action\[(\d+)\]\\Next State/) &&
                           !($Field =~ /^Do\\Action\[(\d+)\]\\Go To Label/) &&
                           !($Field =~ /^Do\\Action\[(\d+)\]\\Label/) )
                    {
                        if( ($Field =~ /Do\\Action\[(\d+)\]\\(AnimName|Set Enum)/) ||
                            ($Field =~ /TaskList\\Task\[(\d+)\]\\SubTask\[(\d+)\]\\AnimName/) ||
                            ($Field =~ /Spawned Object Anim\\AnimName/) )
                        {
                            push( @AnimLines, $_ );

                            if( $Field =~ /Do\\Action\[(\d+)\]\\AnimName/ )
                            {
                                if( $LastAnimSelectIndex == $1 )
                                {
                                    SetAnimUsed( $LastAnimSelectPackage, $Value, $Line, $Layer );
                                    $LastAnimSelectIndex = -1;
                                }
                                else
                                {
                                    error( "AnimName[$1] without AnimSelect[$LastAnimSelectIndex] at line $Line in $Layer\n" );
                                }
                            }
                            elsif( $Field =~ /TaskList\\Task\[(\d+)\]\\SubTask\[(\d+)\]\\AnimName/ )
                            {
                                if( $LastAnimSelectIndex == ($1 * 1000 + $2) )
                                {
                                    SetAnimUsed( $LastAnimSelectPackage, $Value, $Line, $Layer );
                                    $LastAnimSelectIndex = -1;
                                }
                                else
                                {
                                    error( "AnimName[$1] without AnimSelect[$LastAnimSelectIndex] at line $Line in $Layer\n" );
                                }
                            }
                            elsif( $Field =~ /Spawned Object Anim\\AnimName/ )
                            {
                                if( $LastAnimSelectIndex == 1 )
                                {
                                    SetAnimUsed( $LastAnimSelectPackage, $Value, $Line, $Layer );
                                    $LastAnimSelectIndex = -1;
                                }
                                else
                                {
                                    error( "AnimName[$1] without AnimSelect[$LastAnimSelectIndex] at line $Line in $Layer\n" );
                                }
                            }
                            elsif( ($Field =~ /Do\\Action\[(\d+)\]\\Set Enum/) &&
                                   ($LastPropertyIndex == $1) &&
                                   ( ($LastPropertyType eq 'AnimSurface\PlayAnim') ||
                                     ($LastPropertyType eq 'SkinPropSurf\PlayAnim') )
                                 )
                            {
                                if( $LastGUIDIndex == $1 )
                                {
                                    my $AnimPackage = $Objects{$LastGUID}{'animpkg'};
                                    if( defined $AnimPackage )
                                    {
                                        SetAnimUsed( $AnimPackage, $Value, $Line, $Layer );
                                        $LastGUIDIndex = -1;
                                    }
                                    else
                                    {
                                        SetAnimUsedNoPackage( $Value, $Line, $Layer );
                                        $LastGUIDIndex = -1;
                                    }
                                }
                                else
                                {
                                    error( "Set Enum[$1] without GUID at line $Line in $Layer\n" );
                                }
                            }
                            else
                            {
                                error( "Unaccounted (01) at line $Line in $Layer\n" );
                            }
                        }
                        else
                        {
                            error( "Unaccounted (02) at line $Line in $Layer\n" );
                        }
                    }
                }
                # Is this an Anim Name but not a Trigger Reset or Light Action or DeadBody AnimName or Check Enum
                elsif(  (exists $AnimNames{$Value}) &&
                       !($Field =~ /^Trigger Object\\Reset Style/) &&
                       !($Field =~ /Light\\ActionWhenDone/) &&
                       !($Field =~ /^DeadBody\\AnimName/) &&
                       !($Field =~ /^If\\Condition\[\d+\]\\Check Enum/) &&
                       !($Field =~ /^Else If\\Else Condition\[\d+\]\\Check Enum/) )
                {
                    push( @AnimLines, $_ );

                    # Found an Anim Name
                    if( !$ObjectGUIDDefined )
                    {
                        # Whoops, no object GUID defined yet
                        error( "Anim Package without object GUID at line $Line in $Layer\n" );
                    }
                    else
                    {
                        # Increment the count of references for the anim
                        my $AnimPackage = $Objects{$ObjectGUID}{'animpkg'};
                        if( !defined $AnimPackage )
                        {
                            error( "Object $ObjectGUID has no anim package at line $Line in $Layer\n" );
                        }
                        else
                        {
                            SetAnimUsed( $AnimPackage, $Value, $Line, $Layer );
                        }
                    }
                }
            }
            # This catches Blueprint Overwrite GUIDs
            elsif( /\s*"([a-fA-F0-9]{8}:[a-fA-F0-9]{8})"\s*/ )
            {
                $ObjectGUIDDefined = 1;
                $ObjectGUID = $1;
                $Objects{$ObjectGUID}{'defined'} = 1;
            }
            $Line++;
        }
    }
}
