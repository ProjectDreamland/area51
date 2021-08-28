use strict;
use Win32;
require Win32::API;

sub _gen {
    my $UuidCreate = new Win32::API('rpcrt4', 'UuidCreate', 'P', 'N');
    die 'Could not load UuidCreate from rpcrt4.dll' unless $UuidCreate;
    
    my $UuidToString = new Win32::API('rpcrt4', 'UuidToString', 'PP', 'N');
    die 'Could not load UuidToString from rpcrt4.dll' unless $UuidToString;
    
    my $RpcStringFree = new Win32::API('rpcrt4', 'RpcStringFree', 'P', 'N');
    die 'Could not load RpcStringFree from rpcrt4.dll' unless $RpcStringFree;
 
    my $uuid = "*" x 16; # Allocate enough space to store the uuid structure
    
    my $ret = $UuidCreate->Call( $uuid );
    die "UuidCreate failed with error: $ret" unless $ret == 0;
 
    my $ptr_str = pack("P",0);
    $ret = $UuidToString->Call( $uuid, $ptr_str );
    die "UuidToString failed with error: $ret" unless $ret == 0;
 
    my $guid_str = unpack( "p", $ptr_str );
 
    $ret = $RpcStringFree->Call( $ptr_str );
    die "RpcStringFree failed with error: $ret" unless $ret == 0;
 
    return '{' . uc($guid_str) . '}';
}

if( scalar @ARGV < 1 )
{
    die "Usage: NewApp <appname>";
}

my $project_name = shift;
print "Building application \"$project_name\" from template";

open( FH, "Template_CommandLine.sln" ) or die "Failed to open solution template";
my @sln = <FH>;
close( FH );

open( FH, "Template_CommandLine.vcproj" ) or die "Failed to open project template";
my @vcproj = <FH>;
close( FH );

my $sln = join( "", @sln );
my $vcproj = join( "", @vcproj );
my $guid = _gen();

$sln =~ s/\*\*AppName\*\*/$project_name/g;
$sln =~ s/\{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\}/$guid/g;

$vcproj =~ s/\*\*AppName\*\*/$project_name/g;

open( FH, ">$project_name.sln" ) or die "Failed to open solution for write";
print( FH $sln );
close( FH );

open( FH, ">$project_name.vcproj" ) or die "Failed to open project for write";
print( FH $vcproj );
close( FH );
